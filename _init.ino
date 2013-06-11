
// define the parameters of our machine.
#define X_STEPS_PER_INCH 1800
#define X_STEPS_PER_MM   7 //19 //multiplicacion por 2,7142
#define X_MOTOR_STEPS    200

#define Y_STEPS_PER_INCH 1800
#define Y_STEPS_PER_MM   7 //28
#define Y_MOTOR_STEPS    200

#define X_MOTOR_INACCURACY_ANGLE 15
#define Y_MOTOR_INACCURACY_ANGLE 15

int x_correction_steps = X_MOTOR_STEPS * ((double)X_MOTOR_INACCURACY_ANGLE / 360);
int y_correction_steps = Y_MOTOR_STEPS * ((double)Y_MOTOR_INACCURACY_ANGLE / 360);


//our maximum feedrates
#define FAST_XY_FEEDRATE 1000.0
#define FAST_Z_FEEDRATE  50.0

// Units in curve section
#define CURVE_SECTION_INCHES 0.019685
#define CURVE_SECTION_MM 0.5

// Set to one if sensor outputs inverting (ie: 1 means open, 0 means closed)
// RepRap opto endstops are *not* inverting.
#define SENSORS_INVERTING 0


/****************************************************************************************
* digital i/o pin assignment
*
* this uses the undocumented feature of Arduino - pins 14-19 correspond to analog 0-5
****************************************************************************************/

//cartesian bot pins
#define X_STEP_PIN 5
#define X_DIR_PIN 6

#define Y_STEP_PIN 7
#define Y_DIR_PIN 8



