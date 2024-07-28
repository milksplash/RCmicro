#include <XInput.h>

//To reflash Micro w/ Xinput, use Visual Studio Code and setup arduino.json "port" to "8"(should be) and "board" to "arduino:avr:micro", connect micro pro to USB, short pin RST and GND, press "Arduino: Upload" and short pin RST and GND again

const int ST_PIN = 2; // Arduino pin for steering input
const int TH_PIN = 3; // Arduino pin for throttle input

const float ST_DEADZONE_PERCENTAGE = .05; // Increase minimum travel required for output value for steering input
const float ST_MAXZONE_PERCENTAGE = .05; // Reduce maximum travel required for maximum output value for steering input
const int ST_MINIMUM_ABSOLUTE = 1000; // Absolute minimum value for steering input (left)
const int ST_MAXIMUM_ABSOLUTE = 1977; // Absolute maximum value for steering input (right)
const float ST_MIDDLE_POINT_CUSTOM  = 1480; // Middle point for steering input

const float TH_DEADZONE_PERCENTAGE = .05; // Increase minimum travel required for output value for throttle input
const float TH_MAXZONE_PERCENTAGE = .05; // Reduce maximum travel required for maximum output value for throttle input
const int TH_MINIMUM_ABSOLUTE = 1001; // Absolute minimum value for trigger input (throttle)
const int TH_MAXIMUM_ABSOLUTE = 1978; // Absolute maximum value for trigger input (reverse)
const float TH_MIDDLE_POINT_CUSTOM  = 1480; // Middle point for throttle input

//=======================================================================================================================================

const long JOYSTICK_MIN = -32768L; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const long JOYSTICK_MAX = 32767L; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const int TRIGGER_MIN = -255; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad
const int TRIGGER_MAX = 255; // XInput Standard https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad

const int ST_MIN = ST_MINIMUM_ABSOLUTE + (ST_MAXIMUM_ABSOLUTE - ST_MINIMUM_ABSOLUTE) * ST_MAXZONE_PERCENTAGE; 
const int ST_MAX = ST_MAXIMUM_ABSOLUTE - (ST_MAXIMUM_ABSOLUTE - ST_MINIMUM_ABSOLUTE) * ST_MAXZONE_PERCENTAGE; 
const float ST_MIDDLE_POINT_TRUE = (ST_MIN + ST_MAX) / 2.; // True middle point
const float ST_DEADZONE_POSITIVE = ST_MIDDLE_POINT_CUSTOM + (ST_MAX - ST_MIDDLE_POINT_TRUE) * ST_DEADZONE_PERCENTAGE;
const float ST_RATIO_POSITIVE = JOYSTICK_MAX / (ST_MAX - ST_DEADZONE_POSITIVE);
const float ST_DEADZONE_NEGATIVE = ST_MIDDLE_POINT_CUSTOM - (ST_MAX - ST_MIDDLE_POINT_TRUE) * ST_DEADZONE_PERCENTAGE;
const float ST_RATIO_NEGATIVE = JOYSTICK_MIN / (ST_MIN - ST_DEADZONE_NEGATIVE);

const int TH_MIN = TH_MINIMUM_ABSOLUTE + (TH_MAXIMUM_ABSOLUTE - TH_MINIMUM_ABSOLUTE) * TH_MAXZONE_PERCENTAGE; 
const int TH_MAX = TH_MAXIMUM_ABSOLUTE - (TH_MAXIMUM_ABSOLUTE - TH_MINIMUM_ABSOLUTE) * TH_MAXZONE_PERCENTAGE; 
const float TH_MIDDLE_POINT_TRUE = (TH_MIN + TH_MAX) / 2.;
const float TH_DEADZONE_POSITIVE = TH_MIDDLE_POINT_CUSTOM + (TH_MAX - TH_MIDDLE_POINT_TRUE) * TH_DEADZONE_PERCENTAGE;
const float TH_RATIO_POSITIVE = TRIGGER_MAX / (TH_MAX - TH_DEADZONE_POSITIVE);
const float TH_DEADZONE_NEGATIVE = TH_MIDDLE_POINT_CUSTOM - (TH_MAX - TH_MIDDLE_POINT_TRUE) * TH_DEADZONE_PERCENTAGE;
const float TH_RATIO_NEGATIVE = TRIGGER_MIN / (TH_MIN - TH_DEADZONE_NEGATIVE);

long STvalueRaw;
long THvalueRaw;
long STvalueXinput;
long THvalueXinput;

void setup()
{
    //TODO use interrupt
    Serial.begin(9600);
    pinMode(ST_PIN, INPUT);
    pinMode(TH_PIN, INPUT);
    XInput.begin();
}

void loop()
{
    STvalueRaw = pulseIn(ST_PIN, HIGH); // Overflow protection (somehow?) do NOT simplify this
    THvalueRaw = pulseIn(TH_PIN, HIGH);

    //New deadzone implementation
    if (STvalueRaw >= ST_MIDDLE_POINT_CUSTOM) // Left stick positive (right) movement
    {
        STvalueXinput = ST_RATIO_POSITIVE * (STvalueRaw - ST_DEADZONE_POSITIVE);
        XInput.setJoystickX(JOY_LEFT, (STvalueRaw > ST_DEADZONE_POSITIVE) ? STvalueXinput : 0);
    }
    else // Left stick negative (left) movement
    {
        STvalueXinput = ST_RATIO_NEGATIVE * (STvalueRaw - ST_DEADZONE_NEGATIVE);
        XInput.setJoystickX(JOY_LEFT, (STvalueRaw < ST_DEADZONE_NEGATIVE) ? STvalueXinput : 0);
    }
    if (THvalueRaw >= TH_MIDDLE_POINT_CUSTOM) // RT movement
    {
        THvalueXinput = TH_RATIO_POSITIVE * (THvalueRaw - TH_DEADZONE_POSITIVE);
        XInput.setTrigger(TRIGGER_RIGHT, (THvalueRaw > TH_DEADZONE_POSITIVE) ? THvalueXinput : 0);
        XInput.setTrigger(TRIGGER_LEFT, 0);
    } else // LT movement
    {
        THvalueXinput = -TH_RATIO_NEGATIVE * (THvalueRaw - TH_DEADZONE_NEGATIVE);
        XInput.setTrigger(TRIGGER_LEFT, (THvalueRaw < TH_DEADZONE_NEGATIVE) ? THvalueXinput : 0);
        XInput.setTrigger(TRIGGER_RIGHT, 0);
    }
    
    //Debug
    //Serial.print(STvalueRaw); Serial.print("\t"); Serial.println(THvalueRaw);
    //Serial.print(STvalueXinput); Serial.print("\t"); Serial.println(THvalueXinput);
    //Serial.print(THvalueRaw); Serial.print("\t"); Serial.println(THvalueXinput);
    //delay(500);
}