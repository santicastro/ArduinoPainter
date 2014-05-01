#define DEBUG
//init our variables
long max_delta;
long x_counter;
long y_counter;
bool x_can_step;
bool y_can_step;
int milli_delay;

FloatPoint native_delta_units;
FloatPoint native_target_units;
FloatPoint native_current_units;

void init_steppers()
{
  //turn them off to start.
  disable_steppers();

  //init our points.
  set_position(0.0, 0.0);
  
  copy_point(&target_units, &current_units);
  copy_point(&native_target_units, &current_units);

  current_steps.x = to_steps(x_units, native_current_units.x);
  current_steps.y = to_steps(y_units, native_current_units.y);

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

  distance = module(&delta_units);
  if(distance < 0.001){
    return;
  }
  angle_cos = (target_units.x - current_units.x) / distance;
  angle_sin = (target_units.y - current_units.y) / distance;
  
  copy_point(&final_target, &target_units);

#ifdef CARTESIAN_PAINTER
#ifdef ENABLE_INACCURACY_CORRECTION
  target_units.x += x_correction_units * angle_cos;
  target_units.y += y_correction_units * angle_sin;

  calculate_deltas();
  dda_move_aux(micro_delay);

  copy_point(&target_units, &final_target);
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
  copy_point(&target_units, &final_target);
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
  copy_point(&current_units, &target_units);
  copy_point(&native_current_units, &native_target_units);  
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

void copy_point(struct FloatPoint *target, struct FloatPoint *source){
  (*target).x = (*source).x;
  (*target).y = (*source).y;
  #ifdef DEBUG
  Serial.print("  copy s:");
  Serial.print((*source).x);
  Serial.print(',');
  Serial.println((*source).y);
  
  Serial.print("  copy t:");
  Serial.print((*target).x);
  Serial.print(',');
  Serial.println((*target).y);
  #endif
}

void set_target(float x, float y)
{
  target_units.x = x;
  target_units.y = y;
  calculate_deltas();
}

void set_position(float x, float y){
  current_units.x = x;
  current_units.y = y;
#ifdef POLAR_PAINTER
  cartesian_to_polar(&native_current_units, &current_units);
#endif
#ifdef CARTESIAN_PAINTER
  copy_point(&native_current_units, &current_units);
#endif
  calculate_deltas();
}

void calculate_deltas()
{  
  //figure our deltas.
  delta_units.x = abs(target_units.x - current_units.x);
  delta_units.y = abs(target_units.y - current_units.y);

#ifdef POLAR_PAINTER
  cartesian_to_polar(&native_target_units, &target_units);
  native_delta_units.x = abs(native_target_units.x - native_current_units.x);
  native_delta_units.y = abs(native_target_units.y - native_current_units.y);
#endif
#ifdef CARTESIAN_PAINTER
  copy_point(&native_target_units, &target_units);
  copy_point(&native_delta_units, &delta_units);
#endif
#ifdef DEBUG
  print_values();
#endif

  target_steps.x = to_steps(x_units, native_target_units.x);
  target_steps.y = to_steps(y_units, native_target_units.y);

  delta_steps.x = abs(target_steps.x - current_steps.x);
  delta_steps.y = abs(target_steps.y - current_steps.y);

  //what is our direction
  x_direction = (native_target_units.x >= native_current_units.x);
  y_direction = (native_target_units.y >= native_current_units.y);


  //set our direction pins as well
  digitalWrite(X_DIR_PIN, x_direction);
  digitalWrite(Y_DIR_PIN, y_direction);
}
  #ifdef DEBUG
void print_values(){
Serial.print("current:");
Serial.print(current_units.x);
Serial.print(",");
Serial.println(current_units.y);

Serial.print("target:");
Serial.print(target_units.x);
Serial.print(",");
Serial.println(target_units.y);

Serial.print("n_current:");
Serial.print(native_current_units.x);
Serial.print(",");
Serial.println(native_current_units.y);

Serial.print("n_target:");
Serial.print(native_target_units.x);
Serial.print(",");
Serial.println(native_target_units.y);

}
#endif

long calculate_feedrate_delay(float feedrate)
{
  //how long is our line length?
  float distance = module(&native_delta_units);
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

long get_max_speed()
{
  return calculate_feedrate_delay(FAST_XY_FEEDRATE); // *3 PARA PROBAR
}

void disable_steppers()
{
}

float module(struct FloatPoint * point){
  return sqrt((*point).x *(*point).x + (*point).y * (*point).y);
}

# ifdef POLAR_PAINTER
float canvas_width = 100.0;
float canvas_height = 66.0;
float canvas_padding = 5.0;
void cartesian_to_polar(struct FloatPoint *polar, struct FloatPoint *cartesian){
  struct FloatPoint tmp;
  tmp.x = (*cartesian).x + canvas_padding;
  tmp.y = canvas_height + canvas_padding - (*cartesian).y;
  (*polar).x = module(&tmp);
  tmp.x = canvas_width - (*cartesian).x + canvas_padding;
  (*polar).y = module(&tmp);
}
# endif
