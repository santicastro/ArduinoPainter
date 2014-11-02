//#define DEBUG

#define GENERIC_POLAR_PAINTER //Comment this line to use etch_a_sketch

#ifndef GENERIC_POLAR_PAINTER
  #define ETCH_A_SKETCH
#endif

#ifdef ETCH_A_SKETCH
  #define CARTESIAN_PAINTER
  
  #define USE_LCD
  
  // define the parameters of our machine.
  #define X_STEPS_PER_INCH 1
  #define X_STEPS_PER_MM   12 //7 //19 //multiplicacion por 2,7142
  #define X_MOTOR_STEPS    200

  #define Y_STEPS_PER_INCH 1
  #define Y_STEPS_PER_MM   12 //7 //28
  #define Y_MOTOR_STEPS    200
  
  #define ENABLE_INACCURACY_CORRECTION
  #define X_MOTOR_INACCURACY_ANGLE 25
  #define Y_MOTOR_INACCURACY_ANGLE 25
  long x_correction_steps = X_MOTOR_STEPS * ((double)X_MOTOR_INACCURACY_ANGLE / 360);
  long y_correction_steps = Y_MOTOR_STEPS * ((double)Y_MOTOR_INACCURACY_ANGLE / 360);
  
  float x_correction_units = x_correction_steps / (float)X_STEPS_PER_MM;
  float y_correction_units = y_correction_steps / (float)Y_STEPS_PER_MM;

  //our maximum feedrates
  #define FAST_XY_FEEDRATE 1000.0 // default: 1000.0

#endif

#ifdef GENERIC_POLAR_PAINTER
  #define POLAR_PAINTER
  
  #define USE_INTERNAL_PULLUPS

  #define X_STEPS_PER_INCH 1
  #define X_STEPS_PER_MM   27 //7 //19 //multiplicacion por 2,7142
  #define X_MOTOR_STEPS    1600

  #define Y_STEPS_PER_INCH 1
  #define Y_STEPS_PER_MM   27 //7 //28
  #define Y_MOTOR_STEPS    1600
  
  //our maximum feedrates
  #define FAST_XY_FEEDRATE 20000.0 // default: 1000.0
#endif


// Units in curve section
#define CURVE_SECTION_INCHES 0.019685
#define CURVE_SECTION_MM 0.5

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


#define SD_SELECT 10

#define BUTTON1 2
#define BUTTON2 3

