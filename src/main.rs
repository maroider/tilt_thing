#![allow(clippy::unused_io_amount)]

use std::io::{self, Write};

use serialport::{SerialPortInfo, SerialPortType};

fn main() {
    match serialport::available_ports() {
        Ok(ports) => match ports.len() {
            0 => println!("Could not find any serial ports"),
            1 => {
                print_serial_port_info(&ports[0], None);
                talk_to_tilt_thing(&ports[0]);
            }
            _ => {
                for (i, port) in ports.iter().enumerate() {
                    print_serial_port_info(port, Some(&i.to_string()));
                }
                print!("Select a COM port: ");
                io::stdout().lock().flush().unwrap();
                let selected_port = {
                    let mut buffer = String::new();
                    io::stdin().read_line(&mut buffer).unwrap();
                    usize::from_str_radix(buffer.trim(), 10)
                        .unwrap_or_else(|_| panic!("Invalid number!"))
                };
                let port = ports
                    .get(selected_port)
                    .unwrap_or_else(|| panic!("Invalid option!"));
                talk_to_tilt_thing(&port);
            }
        },
        Err(err) => {
            println!(
                "An error uccured while searching for available serial ports: {}",
                err
            );
        }
    }
}

fn print_serial_port_info(port_info: &SerialPortInfo, prefix: Option<&str>) {
    let indent_base = if prefix.is_some() { 2 } else { 0 };

    if let Some(prefix) = prefix {
        println!("{}:", prefix);
    }

    println!("{}{}", " ".repeat(indent_base), port_info.port_name);

    match &port_info.port_type {
        SerialPortType::UsbPort(usb_port_info) => {
            println!("{}Type: USB", " ".repeat(indent_base * 2));

            println!("{}VID: {}", " ".repeat(indent_base * 3), usb_port_info.vid);
            println!("{}PID: {}", " ".repeat(indent_base * 3), usb_port_info.pid);
            if let Some(serial_number) = &usb_port_info.serial_number {
                println!(
                    "{}Serial Number: {}",
                    " ".repeat(indent_base * 3),
                    serial_number
                );
            }
            if let Some(manufacturer) = &usb_port_info.manufacturer {
                println!(
                    "{} Manufacturer: {}",
                    " ".repeat(indent_base * 3),
                    manufacturer
                );
            }
            if let Some(product) = &usb_port_info.product {
                println!("{}      Product: {}", " ".repeat(indent_base * 3), product);
            }
        }
        SerialPortType::PciPort => {
            println!("{}Type: PCI", " ".repeat(indent_base * 2));
        }
        SerialPortType::BluetoothPort => {
            println!("{}Type: Bluetooth", " ".repeat(indent_base * 2));
        }
        SerialPortType::Unknown => {
            println!("{}Type: Unknown", " ".repeat(indent_base * 2));
        }
    }
}

fn talk_to_tilt_thing(port: &SerialPortInfo) {
    let mut port = serialport::open(&port.port_name).unwrap_or_else(|err| {
        panic!(
            "Could not open serial port {}: {}",
            port.port_name, err.description
        )
    });

    println!("Sending connection request");
    port.write(&[SerialMessageType::ConnectionRequest as u8])
        .unwrap();
    let mut buffer = [0u8; 2];
    while port.bytes_to_read().unwrap() == 0 {
        std::thread::sleep(std::time::Duration::from_millis(100));
    }
    port.read(&mut buffer).unwrap();
    if buffer[0] != SerialMessageType::ConnectionAccepted as u8 {
        panic!("Connection request was not accepted");
    }
    port.write(&[SerialMessageType::ConnectionAcknowledged as u8])
        .unwrap();

    println!("Connectino established");

    let adapters = monman::DisplayAdapters::new().unwrap();
    let adapter = adapters.nth(0).unwrap();

    loop {
        while port.bytes_to_read().unwrap() < 2 {
            std::thread::sleep(std::time::Duration::from_millis(100));
        }

        port.read(&mut buffer).unwrap();
        if buffer[0] != SerialMessageType::Tilt as u8 {
            panic!("Only tilt messages are allowed once connected");
        }
        let tilted = match buffer[1] {
            0 => false,
            1 => true,
            n => panic!("Invalid bool: {}", n),
        };

        if tilted {
            dbg!(adapter.set_orientation(monman::DisplayOrientation::Rotate90));
        } else {
            dbg!(adapter.set_orientation(monman::DisplayOrientation::Default));
        }
    }
}

enum SerialMessageType {
    ConnectionRequest = 0x1,
    ConnectionAccepted = 0x2,
    ConnectionAcknowledged = 0x3,
    Tilt = 0x4,
}
