#include <XInput.h>

//To reflash Micro w/ Xinput, use Visual Studio Code and setup arduino.json "port" to "8"(should be) and "board" to "arduino:avr:micro", connect micro pro to USB, short pin RST and GND, press "Arduino: Upload" and short pin RST and GND again

const int ST_PIN = 2; // Arduino pin for steering input
const int TH_PIN = 3; // Arduino pin for throttle input

const float ST_DEADZONE_PERCENTAGE = .05; // Increase minimum stick travel required for output value
const float ST_MAXZONE_PERCENTAGE = .05; // Reduce maximum stick travel required for maximum output value
const int ST_MIN = (1000) * (1 + ST_MAXZONE_PERCENTAGE); // Max value for steering input (LS left)
const int ST_MAX = (1977) * (1 - ST_MAXZONE_PERCENTAGE); // Max value for steering input (LS right)

const float TH_DEADZONE_PERCENTAGE = .05;
const float TH_MAXZONE_PERCENTAGE = .05;
const int TH_MIN = (1001) * (1 + TH_MAXZONE_PERCENTAGE); // Max value for throttle input (LT)
const int TH_MAX = (1978) * (1 - TH_MAXZONE_PERCENTAGE); // Max value for throttle input (RT)

//New deadzone implementation
const float ST_MIDDLE_POINT_TRUE = (ST_MIN + ST_MAX) / 2.; // True middle point
const float ST_MIDDLE_POINT_CUSTOM  = 1480; // Custom middle point
const long JOYSTICK_MIN = -32768L; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const long JOYSTICK_MAX = 32767L; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const float ST_DEADZONE_POSITIVE = ST_MIDDLE_POINT_CUSTOM + (ST_MAX - ST_MIDDLE_POINT_TRUE) * ST_DEADZONE_PERCENTAGE;
const float ST_RATIO_POSITIVE = JOYSTICK_MAX / (ST_MAX - ST_DEADZONE_POSITIVE);
const float ST_DEADZONE_NEGATIVE = ST_MIDDLE_POINT_CUSTOM - (ST_MAX - ST_MIDDLE_POINT_TRUE) * ST_DEADZONE_PERCENTAGE;
const float ST_RATIO_NEGATIVE = JOYSTICK_MIN / (ST_MIN - ST_DEADZONE_NEGATIVE);

const float TH_MIDDLE_POINT_TRUE = (TH_MIN + TH_MAX) / 2.;
const float TH_MIDDLE_POINT_CUSTOM  = 1480;
const int TRIGGER_MIN = -255; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const int TRIGGER_MAX = 255; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const float TH_DEADZONE_POSITIVE = TH_MIDDLE_POINT_CUSTOM + (TH_MAX - TH_MIDDLE_POINT_TRUE) * TH_DEADZONE_PERCENTAGE;
const float TH_RATIO_POSITIVE = TRIGGER_MAX / (TH_MAX - TH_DEADZONE_POSITIVE);
const float TH_DEADZONE_NEGATIVE = TH_MIDDLE_POINT_CUSTOM - (TH_MAX - TH_MIDDLE_POINT_TRUE) * TH_DEADZONE_PERCENTAGE;
const float TH_RATIO_NEGATIVE = TRIGGER_MIN / (TH_MIN - TH_DEADZONE_NEGATIVE);

//Old deadzone implementation
const long JOYSTICK_RESOLUTION = 65535L; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const int TRIGGER_RESOLUTION = 510; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const float JOYSTICK_DEADZONE_PERCENTAGE = .0; // Deadzone, 1 = 100%
const float TRIGGER_DEADZONE_PERCENTAGE = .0; //
const long JOYSTICK_DEADZONE = JOYSTICK_RESOLUTION * JOYSTICK_DEADZONE_PERCENTAGE;
const int TRIGGER_DEADZONE = TRIGGER_RESOLUTION * TRIGGER_DEADZONE_PERCENTAGE;
const float ST_RATIO = JOYSTICK_RESOLUTION / (float)(ST_MAX - ST_MIN); // !!!!! Joystick resolution cannot be calculated const !!!!!
const float TH_RATIO = TRIGGER_RESOLUTION / (float)(TH_MAX - TH_MIN);
const long JOYSTICK_RESOLUTION_HALF = JOYSTICK_RESOLUTION >> 1;  // !!!!! Joystick resolution cannot be calculated const !!!!!
const int TRIGGER_RESOLUTION_HALF = TRIGGER_RESOLUTION >> 1;

signed long STvalueRaw;
signed long THvalueRaw;
signed long STvalueXinput;
signed long THvalueXinput;

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

    //New deadzone implementation
    if (STvalueRaw >= ST_MIDDLE_POINT_CUSTOM)
    {
        STvalueXinput = ST_RATIO_POSITIVE * (STvalueRaw - ST_DEADZONE_POSITIVE);
        XInput.setJoystickX(JOY_LEFT, (STvalueRaw > ST_DEADZONE_POSITIVE) ? STvalueXinput : 0);
    }
    else
    {
        STvalueXinput = ST_RATIO_NEGATIVE * (STvalueRaw - ST_DEADZONE_NEGATIVE);
        XInput.setJoystickX(JOY_LEFT, (STvalueRaw < ST_DEADZONE_NEGATIVE) ? STvalueXinput : 0);
    }

    if (THvalueRaw >= TH_MIDDLE_POINT_CUSTOM)
    {
        THvalueXinput = TH_RATIO_POSITIVE * (THvalueRaw - TH_DEADZONE_POSITIVE);
        XInput.setTrigger(TRIGGER_RIGHT, (THvalueXinput > TH_DEADZONE_POSITIVE) ? THvalueXinput : 0);
        XInput.setTrigger(TRIGGER_LEFT, 0);
    } else
    {
        THvalueXinput = -TH_RATIO_NEGATIVE * (THvalueRaw - TH_DEADZONE_NEGATIVE);
        XInput.setTrigger(TRIGGER_LEFT, (THvalueXinput > TH_DEADZONE_NEGATIVE) ? THvalueXinput : 0);
        XInput.setTrigger(TRIGGER_RIGHT, 0);
    }
    
    //Old deadzone implementation
    //STvalueXinput = (STvalueRaw - ST_MIN) * ST_RATIO - JOYSTICK_RESOLUTION_HALF;
    //XInput.setJoystickX(JOY_LEFT, (abs(STvalueXinput) > JOYSTICK_DEADZONE) ? STvalueXinput : 0);

    /*
    THvalueXinput = (THvalueRaw - TH_MIN) * TH_RATIO - TRIGGER_RESOLUTION_HALF;
    if (THvalueXinput >= 0)
    {
        XInput.setTrigger(TRIGGER_RIGHT, (abs(THvalueXinput) > TRIGGER_DEADZONE) ? THvalueXinput : 0);
        XInput.setTrigger(TRIGGER_LEFT, 0);
    } else
    {
        XInput.setTrigger(TRIGGER_LEFT, (abs(THvalueXinput) > TRIGGER_DEADZONE) ? -THvalueXinput : 0);
        XInput.setTrigger(TRIGGER_RIGHT, 0);
    }
    */

    //Debug
    Serial.print("\n");
    //Serial.print(STvalueRaw); Serial.print("\t"); Serial.println(THvalueRaw);
    Serial.print(THvalueRaw); Serial.print("\t"); Serial.println(THvalueXinput);
    Serial.print("\n");
    delay(500);
}