#define OUTPUT_ENABLE  18  // GPIO18 - Push Button 1
#define OUTPUT_SELECT  19  // GPIO19 - Push Button 2
#define LED_DATA       23  // GPIO23 - Debug LED for DATA
#define LED_SYNC       5   // GPIO5 - Debug LED for SYNC

// Define timing parameters
#define TON1  1200  // First pulse on-time (us)
#define TOFF  600   // Pulse off-time (us)
#define PULSES 13   // Number of pulses
#define IDLE  6000  // Idle time before SYNC (us)
#define SYNC_ON 50  // SYNC pulse on-time (us)

// Variables to track state
bool dataEnabled = false;
bool reversedMode = false;

// Button state tracking
bool lastEnableButtonState = HIGH;
bool lastSelectButtonState = HIGH;
unsigned long lastDebounceTimeEnable = 0;
unsigned long lastDebounceTimeSelect = 0;
const int debounceDelay = 200; // 200ms debounce time

void setup() 
{
    Serial.begin(115200);
    Serial.println("Setup started...");

    pinMode(OUTPUT_ENABLE, INPUT_PULLUP);
    pinMode(OUTPUT_SELECT, INPUT_PULLUP);
    pinMode(LED_DATA, OUTPUT);
    pinMode(LED_SYNC, OUTPUT);
}

void loop() 
{
    int enableButtonState = digitalRead(OUTPUT_ENABLE);
    int selectButtonState = digitalRead(OUTPUT_SELECT);
    unsigned long currentTime = millis();


    if (enableButtonState == LOW && lastEnableButtonState == HIGH && (currentTime - lastDebounceTimeEnable > debounceDelay))
     {
        dataEnabled = !dataEnabled;  // Toggle data enable state
        Serial.print("Toggling dataEnabled to: ");
        Serial.println(dataEnabled ? "true" : "false");
        digitalWrite(LED_DATA, dataEnabled ? HIGH : LOW); // LED feedback
        lastDebounceTimeEnable = currentTime;  // Update debounce time
    }
    lastEnableButtonState = enableButtonState;

    if (selectButtonState == LOW && lastSelectButtonState == HIGH && (currentTime - lastDebounceTimeSelect > debounceDelay))
     {
        reversedMode = !reversedMode;  // Toggle reverse mode
        Serial.print("Toggling reversedMode to: ");
        Serial.println(reversedMode ? "true" : "false");
        digitalWrite(LED_SYNC, reversedMode ? HIGH : LOW);
        lastDebounceTimeSelect = currentTime;  // Update debounce time
    }
    lastSelectButtonState = selectButtonState;

    if (dataEnabled) 
    {
        Serial.println("Generating waveform...");
        generateWaveform();
    }
    delay(10);
}

void generateWaveform()
 {
    Serial.print("Generating ");
    Serial.print(reversedMode ? "reversed" : "normal");
    Serial.println(" waveform.");

    if (!reversedMode)
     {
        // Normal waveform
        for (int n = 1; n <= PULSES; n++) {
            Serial.print("Pulse ");
            Serial.print(n);
            Serial.print(": LED_DATA HIGH for ");
            Serial.print(TON1 + (n - 1) * 50);
            Serial.println(" us");

            digitalWrite(LED_DATA, HIGH);
            delayMicroseconds(TON1 + (n - 1) * 50);
            digitalWrite(LED_DATA, LOW);
            delayMicroseconds(TOFF);
        }
    } else
     {
        for (int n = PULSES; n >= 1; n--)
         {
            Serial.print("Pulse ");
            Serial.print(n);
            Serial.print(": LED_DATA HIGH for ");
            Serial.print(TON1 + (n - 1) * 50);
            Serial.println(" us");

            digitalWrite(LED_DATA, HIGH);
            delayMicroseconds(TON1 + (n - 1) * 50);
            digitalWrite(LED_DATA, LOW);
            delayMicroseconds(TOFF);
        }
    }

    Serial.print("Idle for ");
    Serial.print(IDLE);
    Serial.println(" us");
    delayMicroseconds(IDLE);

    Serial.println("Generating SYNC pulse...");
    digitalWrite(LED_SYNC, HIGH);
    delayMicroseconds(SYNC_ON);
    digitalWrite(LED_SYNC, LOW);
}
