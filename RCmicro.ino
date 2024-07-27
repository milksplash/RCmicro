#include <XInput.h>

//To reflash Micro w/ Xinput, use Visual Studio Code and setup arduino.json "port" to "8"(should be) and "board" to "arduino:avr:micro", connect micro pro to USB, short RST and GND, press "Arduino: Upload" and short RST and GND again

const int ST_PIN = 2; // Arduino pin for steering input
const int TH_PIN = 3; // Arduino pin for throttle input

const int ST_MAXZONE = 10; // Reduce stick travel required for maximum output value
const int TH_MAXZONE = 10;
const int ST_MIN = (1000) + ST_MAXZONE; // Min value for steering input
const int ST_MAX = (1977) - ST_MAXZONE;
const int TH_MIN = (1000) + TH_MAXZONE;
const int TH_MAX = (1977) - TH_MAXZONE; // Max value for throttle input

const float JOYSTICK_DEADZONE_PERCENTAGE = .05; // Deadzone, 1 = 100%
const float TRIGGER_DEADZONE_PERCENTAGE = .05; //

const long JOYSTICK_RESOLUTION = 65535L; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const int TRIGGER_RESOLUTION = 510; //

const long JOYSTICK_DEADZONE = JOYSTICK_RESOLUTION * JOYSTICK_DEADZONE_PERCENTAGE;
const long JOYSTICK_RESOLUTION_HALF = JOYSTICK_RESOLUTION >> 1;  // !!!!! Joystick resolution cannot be calculated const !!!!!
const int TRIGGER_DEADZONE = TRIGGER_RESOLUTION * TRIGGER_DEADZONE_PERCENTAGE;
const int TRIGGER_RESOLUTION_HALF = TRIGGER_RESOLUTION >> 1;
const float ST_RATIO = JOYSTICK_RESOLUTION / (float)(ST_MAX - ST_MIN); // !!!!! Joystick resolution cannot be calculated const !!!!!
const float TH_RATIO = TRIGGER_RESOLUTION / (float)(TH_MAX - TH_MIN);

signed long STvalueRaw;
signed long THvalueRaw;
signed long STvalue;
signed long THvalue;

void setup()
{
    //TODO use interrupt
    Serial.begin(9600);
    pinMode(ST_PIN, INPUT);
    pinMode(TH_PIN, INPUT);

    /*
    delay(1000);
    for(int i = ST_MIN; i <= ST_MAX; i++)
    {
        Serial.println((i - ST_MIN) * ST_RATIO - JOYSTICK_RESOLUTION_HALF);
    }
    */

    XInput.begin();
}

void loop()
{
    STvalueRaw = pulseIn(ST_PIN, HIGH); // Overflow protection (somehow?) do NOT simplify this
    THvalueRaw = pulseIn(TH_PIN, HIGH);
    STvalue = (STvalueRaw - ST_MIN) * ST_RATIO - JOYSTICK_RESOLUTION_HALF;
    THvalue = (THvalueRaw - TH_MIN) * TH_RATIO - TRIGGER_RESOLUTION_HALF;

    //XInput.setJoystickY(JOY_LEFT, (abs(THvalue) > JOYSTICK_DEADZONE) ? THvalue : 0); // with deadzone calculation
    XInput.setJoystickX(JOY_LEFT, (abs(STvalue) > JOYSTICK_DEADZONE) ? STvalue : 0); // with deadzone calculation

    if (THvalue >= 0)
    {
        XInput.setTrigger(TRIGGER_RIGHT, (abs(THvalue) > TRIGGER_DEADZONE) ? THvalue : 0);
        XInput.setTrigger(TRIGGER_LEFT, 0);
    } else
    {
        XInput.setTrigger(TRIGGER_LEFT, (abs(THvalue) > TRIGGER_DEADZONE) ? -THvalue : 0);
        XInput.setTrigger(TRIGGER_RIGHT, 0);
    }

    //Debug
    //Serial.print(STvalue); Serial.print("\t"); Serial.println(THvalue);
    //Serial.print(pulseIn(ST_PIN, HIGH)); Serial.print("\t"); Serial.println(pulseIn(TH_PIN, HIGH));
}