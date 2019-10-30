const int SWITCH_PIN = 8;

const long POLL_INTERVAL = 100;

// Serial communication constants
const byte SC_CONN_REQ = 0x1;
const byte SC_CONN_ACC = 0x2;
const byte SC_CONN_ACK = 0x3;
const byte SC_TILT = 0x4;

const byte CS_NOT_CONNECTED = 0x0;
const byte CS_CONN_REQ_RECIEVED = 0x1;
const byte CS_CONN_ACK_RECIEVED = 0x2;

// BEGIN GLOBAL STATE

byte conn_state = CS_NOT_CONNECTED;

unsigned long previous_time = 0;
byte prev_switch_state = 0;

// END GLOBAL STATE

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SWITCH_PIN, INPUT);

    Serial.begin(9600);
    while (!Serial) {
        ;
    }
}

void loop() {
    if (conn_state != CS_CONN_ACK_RECIEVED) {
        return;
    }

    unsigned long current_time = millis();
    if (current_time - previous_time > POLL_INTERVAL) {
        previous_time = current_time;

        byte switch_state = digitalRead(SWITCH_PIN);
        if (switch_state != prev_switch_state) {
            prev_switch_state = switch_state;
            Serial.write(SC_TILT);
            Serial.write(switch_state);
        }
    }
}

void serialEvent() {
    if (conn_state == CS_NOT_CONNECTED || conn_state == CS_CONN_ACK_RECIEVED) {
        if (Serial.read() == SC_CONN_REQ) {
            Serial.write(SC_CONN_ACC);
            conn_state = CS_CONN_REQ_RECIEVED;
        }
    } else if (conn_state == CS_CONN_REQ_RECIEVED) {
        if (Serial.read() == SC_CONN_ACK) {
            conn_state = CS_CONN_ACK_RECIEVED;
        }
    }
}

void err_blink() {
    bool on = false;
    while (true) {
        digitalWrite(LED_BUILTIN, on);
        delay(750);
        on = !on;
    }
}

// BEGIN GLOBAL STATE

unsigned long previous_blink = 0;
bool led_state = false;

// END GLOBAL STATE

void async_blink(unsigned long duration) {
    unsigned long current_time = millis();
    if (current_time - previous_blink > duration) {
        previous_blink = current_time;
        led_state = !led_state;
        digitalWrite(LED_BUILTIN, led_state);
    }
}
