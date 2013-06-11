
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
  target_units.x = 0.0;
  target_units.y = 0.0;

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);

  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);

  //figure our stuff.
  calculate_deltas();
}

void dda_move(long micro_delay)
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
  current_units.x = target_units.x;
  current_units.y = target_units.y;
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

byte last_x_direction = x_direction;
byte last_y_direction = y_direction;
void calculate_deltas()
{
  //figure our deltas.
  delta_units.x = abs(target_units.x - current_units.x);
  delta_units.y = abs(target_units.y - current_units.y);

  //set our steps current, target, and delta
 // current_steps.x = to_steps(x_units, current_units.x);
 // current_steps.y = to_steps(y_units, current_units.y);

  target_steps.x = to_steps(x_units, target_units.x);
  target_steps.y = to_steps(y_units, target_units.y);

  delta_steps.x = abs(target_steps.x - current_steps.x);
  delta_steps.y = abs(target_steps.y - current_steps.y);

  //what is our direction
  x_direction = (target_units.x >= current_units.x);
  y_direction = (target_units.y >= current_units.y);
  
  //set our direction pins as well
  digitalWrite(X_DIR_PIN, x_direction);
  digitalWrite(Y_DIR_PIN, y_direction);
      
  //TODO: esto funciona fatal
  if((last_x_direction!=x_direction) && (delta_steps.x>0)){
    Serial.print("X_DIRECTION CHANGED;");
    Serial.println(x_correction_steps);
    last_x_direction = x_direction;
    correct_position(x_correction_steps, X_DIR_PIN);
  }
  if((last_y_direction!=y_direction) && (delta_steps.y>0)){
    Serial.print("Y_DIRECTION CHANGED:");
    Serial.println(y_correction_steps);
    last_y_direction = y_direction;
    correct_position(y_correction_steps, Y_DIR_PIN);
  }
}

void correct_position(int steps, int pin){
  return;
  for(int i=0; i< steps; i++){
    do_step(pin);
    //wait for next step.
    delayMicroseconds(500);
  } 
}

long calculate_feedrate_delay(float feedrate)
{
  //how long is our line length?
  float distance = sqrt(delta_units.x*delta_units.x + delta_units.y*delta_units.y );
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
  return calculate_feedrate_delay(FAST_XY_FEEDRATE);
}

void disable_steppers()
{
}





