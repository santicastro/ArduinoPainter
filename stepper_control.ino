//#include <math.h>

FloatPoint machine_delta_units, machine_target_units, machine_current_units;

//init our variables
long max_delta;
long x_counter;
long y_counter;
bool x_can_step;
bool y_can_step;
int milli_delay;

void init_steppers()
{
  //turn them off to start.
  disable_steppers();

  //init our points.
  current_units.x = 0.0;
  current_units.y = 0.0;
  copy_point(target_units, current_units);
  copy_point(machine_target_units, current_units);
  copy_point(machine_current_units, current_units);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);

  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);

  //figure our stuff.
  calculate_deltas();
}

void dda_move(long micro_delay){
  double distance, angle_cos, angle_sin;
  FloatPoint final_target;

  distance = sqrt(delta_units.x * delta_units.x + delta_units.y * delta_units.y);
  if(distance < 0.001){
    return;
  }
  angle_cos = (target_units.x - current_units.x) / distance;
  angle_sin = (target_units.y - current_units.y) / distance;
  
  copy_point(final_target, target_units);

#ifdef CARTESIAN_PAINTER
#ifdef ENABLE_INACCURACY_CORRECTION
  target_units.x += x_correction_units * angle_cos;
  target_units.y += y_correction_units * angle_sin;

  calculate_deltas();
  dda_move_aux(micro_delay);

  copy_point(target_units, final_target);
  calculate_deltas();
#endif
#endif

#ifdef POLAR_PAINTER
  if(distance > curve_section*1.1){
    int sections = (int)(distance / curve_section);
    for(int i = 1; i <= sections; i++){
      target_units.x = current_units.x + angle_cos * curve_section;
      target_units.y = current_units.y + angle_sin * curve_section;
      calculate_deltas();
      dda_move_aux(micro_delay);
    }
  }
  copy_point(target_units, final_target);
  calculate_deltas();
#endif
  dda_move_aux(micro_delay);
}

void dda_move_aux(long micro_delay)
{
  //figure out our deltas
  max_delta = max(delta_steps.x, delta_steps.y);

  //init stuff.
  long x_counter = -max_delta/2;
  long y_counter = -max_delta/2;

  //our step flags
  bool x_can_step = 0;
  bool y_can_step = 0;

  if (micro_delay >= 16383)
    milli_delay = micro_delay / 1000;
  else
    milli_delay = 0;

  //do our DDA line!
  do
  {
    x_can_step = (current_steps.x != target_steps.x);
    y_can_step = (current_steps.y != target_steps.y);

    if (x_can_step)
    {
      x_counter += delta_steps.x;

      if (x_counter > 0)
      {
        do_step(X_STEP_PIN);
        x_counter -= max_delta;

        if (x_direction)
          current_steps.x++;
        else
          current_steps.x--;
      }
    }

    if (y_can_step)
    {
      y_counter += delta_steps.y;

      if (y_counter > 0)
      {
        do_step(Y_STEP_PIN);
        y_counter -= max_delta;

        if (y_direction)
          current_steps.y++;
        else
          current_steps.y--;
      }
    }

    //wait for next step.
    if (milli_delay > 0)
      delay(milli_delay);			
    else
      delayMicroseconds(micro_delay);
  }
  while (x_can_step || y_can_step);

  //set our points to be the same
  copy_point(current_units, target_units);
  copy_point(machine_current_units, machine_target_units);  
  calculate_deltas();
}


void do_step(byte step_pin)
{
  digitalWrite(step_pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(step_pin, LOW);
}

long to_steps(float steps_per_unit, float units)
{
  return steps_per_unit * units;
}

float to_units(float steps_per_unit, float steps)
{
  return steps/steps_per_unit;
}

void copy_point(struct FloatPoint target, struct FloatPoint source){
  target.x = source.x;
  target.y = source.y;
}

void set_target(float x, float y)
{
  target_units.x = x;
  target_units.y = y;
  calculate_deltas();
}

void set_position(float x, float y)
{
  current_units.x = x;
  current_units.y = y;
  calculate_deltas();
}

void calculate_deltas()
{  
  //figure our deltas.
  delta_units.x = abs(target_units.x - current_units.x);
  delta_units.y = abs(target_units.y - current_units.y);

#ifdef POLAR_PAINTER
  machine_target_units = cartesianToPolar(target_units);
  machine_delta_units.x = abs(machine_target_units.x - machine_current_units.x);
  machine_delta_units.y = abs(machine_target_units.y - machine_current_units.y);
#endif
#ifdef CARTESIAN_PAINTER
  copy_point(machine_target_units, target_units);
  copy_point(machine_delta_units, delta_units);
#endif

  target_steps.x = to_steps(x_units, machine_target_units.x);
  target_steps.y = to_steps(y_units, machine_target_units.y);

  delta_steps.x = abs(target_steps.x - current_steps.x);
  delta_steps.y = abs(target_steps.y - current_steps.y);

  //what is our direction
  x_direction = (machine_target_units.x >= machine_current_units.x);
  y_direction = (machine_target_units.y >= machine_current_units.y);

  //set our direction pins as well
  digitalWrite(X_DIR_PIN, x_direction);
  digitalWrite(Y_DIR_PIN, y_direction);
}

long calculate_feedrate_delay(float feedrate)
{
  //how long is our line length?
  float distance = sqrt(machine_delta_units.x * machine_delta_units.x + machine_delta_units.y * machine_delta_units.y );
  long master_steps = 0;

  //find the dominant axis.
  if (delta_steps.x > delta_steps.y)
  {
    master_steps = delta_steps.x;
  }
  else
  {
    master_steps = delta_steps.y;
  }

  //calculate delay between steps in microseconds.  this is sort of tricky, but not too bad.
  //the formula has been condensed to save space.  here it is in english:
  // distance / feedrate * 60000000.0 = move duration in microseconds
  // move duration / master_steps = time between steps for master axis.

  return ((distance * 60000000.0) / feedrate) / master_steps;	
}

long getMaxSpeed()
{
  return calculate_feedrate_delay(FAST_XY_FEEDRATE); // *3 PARA PROBAR
}

void disable_steppers()
{
}

# ifdef POLAR_PAINTER

struct FloatPoint cartesianToPolar(struct FloatPoint cartesian){
  FloatPoint polar;
  polar.x = cartesian.x * cartesian.x;
  polar.y = atan(cartesian.x / cartesian.y);
  return polar;
}

# endif
