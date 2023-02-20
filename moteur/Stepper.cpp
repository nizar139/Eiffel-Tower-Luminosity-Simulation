#include "Stepper.h"
#include "mbed.h"

stepper::stepper(PinName _en, PinName ms1, PinName ms2, PinName ms3, PinName _stepPin, PinName dir):en(_en),
    microstepping(ms1, ms2, ms3),
    stepPin(_stepPin),
    direction(dir)
{
}
void stepper::step(int microstep, int dir, float speed)
{
    direction = dir;
    microstepping = microstep;
    // Step...
    if (speed != 0)
    {
        stepPin = 1;
        wait(1/speed);
        stepPin = 0;
        wait(1/speed);
    }
}
void stepper::enable()
{
    en = 0;
}
void stepper::disable()
{
    en = 1;
}