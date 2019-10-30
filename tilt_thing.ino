const int SWITCH_PIN = 8;
const int LED_PIN = 7;

const long POLL_INTERVAL = 100;

// Serial communication constants
const byte SC_CONN_REQ = 0x1;
const byte SC_CONN_ACC = 0x2;
const byte SC_CONN_ACK = 0x3;
const byte SC_TILT = 0x4;

// BEGIN GLOBAL STATE

unsigned long previous_time = 0;
byte prev_switch_state = 0;

// END GLOBAL STATE

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SWITCH_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    Serial.begin(9600);
    while (!Serial || Serial.available() != 1) {
        async_blink(500);
    }
    if (Serial.read() == SC_CONN_REQ) {
        Serial.write(SC_CONN_ACC);
        while (Serial.available() != 1) {
            async_blink(250);
        }
        if (Serial.read() == SC_CONN_ACK) {
            return;
        }
    }
    err_blink();
}

void loop() {
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
