const int SWITCH_PIN = 8;
const int LED_PIN = 7;

const long POLL_INTERVAL = 100;

// Serial communication constants
const byte SC_CONN_REQ = 0x1;
const byte SC_CONN_ACC = 0x2;
const byte SC_TILT = 0x3;

// BEGIN GLOBAL STATE

unsigned long previous_time = 0;
int switch_state = 0;

// END GLOBAL STATE

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SWITCH_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    Serial.begin(9600);
    while (!Serial || Serial.available() != 1)
    {
        ;
    }
    if (Serial.read() == SC_CONN_REQ)
    {
        while (Serial.available() != 1)
        {
            ;
        }
        if (Serial.read() == SC_CONN_ACC)
        {
            return;
        }
    }
    err_blink();
}

void loop()
{
    unsigned long current_time = millis();
    if (current_time - previous_time > POLL_INTERVAL)
    {
        previous_time = current_time;

        switch_state = digitalRead(SWITCH_PIN);
    }

    digitalWrite(LED_PIN, switch_state);
}

void err_blink()
{
    bool on = false;
    while (true)
    {
        digitalWrite(LED_BUILTIN, on);
        delay(500);
        on = !on;
    }
}
