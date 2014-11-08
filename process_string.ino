// our point structure to make things nice.
struct LongPoint {
  long x;
  long y;
};

struct FloatPoint {
  float x;
  float y;
};

FloatPoint current_units;
FloatPoint target_units;
FloatPoint delta_units;

LongPoint current_steps;
LongPoint target_steps;
LongPoint delta_steps;

boolean abs_mode = true;   //0 = incremental; 1 = absolute

//default to mm for units
float x_units = X_STEPS_PER_MM;
float y_units = X_STEPS_PER_MM;
float curve_section = CURVE_SECTION_MM;

//our direction vars
byte x_direction = 1;
byte y_direction = 1;

//our feedrate variables.
float feedrate = 0.0;
long feedrate_micros = 0;

//Read the string and execute instructions
void process_string(char instruction[], int size)
{
  Serial.println("ok");
  //the character / means delete block... used for comments and stuff.
  if (instruction[0] == '/')
  {
    return;
  }
  //init baby!
  FloatPoint fp;
  fp.x = 0.0;
  fp.y = 0.0;

  byte code = 0;

  //what line are we at?
  //	long line = -1;
  //	if (has_command('N', instruction, size))
  //		line = (long)search_string('N', instruction, size);

  /*
	Serial.print("line: ");
   	Serial.println(line);
   	Serial.println(instruction);
   */
  //did we get a gcode?
  if (
  has_command('G', instruction, size) ||
    has_command('X', instruction, size) ||
    has_command('Y', instruction, size) 
    )
  {
    //which one?
    code = (int)search_string('G', instruction, size);

    // Get co-ordinates if required by the code type given
    switch (code)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      if(abs_mode)
      {
        //we do it like this to save time. makes curves better.
        //eg. if only x and y are specified, we dont have to waste time looking up z.
        if (has_command('X', instruction, size))
          fp.x = search_string('X', instruction, size);
        else
          fp.x = current_units.x;

        if (has_command('Y', instruction, size))
          fp.y = search_string('Y', instruction, size);
        else
          fp.y = current_units.y;
      }
      else
      {
        fp.x = search_string('X', instruction, size) + current_units.x;
        fp.y = search_string('Y', instruction, size) + current_units.y;
      }
      break;
    }

    //do something!
    switch (code)
    {
      //Rapid Positioning
      //Linear Interpolation
      //these are basically the same thing.
    case 0:
    case 1:
      //set our target.
      set_target(fp.x, fp.y);

      //do we have a set speed?
      if (has_command('G', instruction, size))
      {
        //adjust if we have a specific feedrate.
        if (code == 1)
        {
          //how fast do we move?
          feedrate = search_string('F', instruction, size);
          if (feedrate > 0)
            feedrate_micros = calculate_feedrate_delay(feedrate);
          //nope, no feedrate
          else
            feedrate_micros = get_max_speed();
        }
        //use our max for normal moves.
        else
          feedrate_micros = get_max_speed();
      }
      //nope, just coordinates!
      else
      {
        //do we have a feedrate yet?
        if (feedrate > 0)
          feedrate_micros = calculate_feedrate_delay(feedrate);
        //nope, no feedrate
        else
          feedrate_micros = get_max_speed();
      }

      //finally move.
      dda_move(feedrate_micros);
      break;

      //Clockwise arc
    case 2:
      //Counterclockwise arc
    case 3:
      FloatPoint cent;

      // Centre coordinates are always relative
      cent.x = search_string('I', instruction, size) + current_units.x;
      cent.y = search_string('J', instruction, size) + current_units.y;
      float angleA, angleB, angle, radius, length, aX, aY, bX, bY;

      aX = (current_units.x - cent.x);
      aY = (current_units.y - cent.y);
      bX = (fp.x - cent.x);
      bY = (fp.y - cent.y);

      if (code == 2) { // Clockwise
        angleA = atan2(bY, bX);
        angleB = atan2(aY, aX);
      } 
      else { // Counterclockwise
        angleA = atan2(aY, aX);
        angleB = atan2(bY, bX);
      }

      // Make sure angleB is always greater than angleA
      // and if not add 2PI so that it is (this also takes
      // care of the special case of angleA == angleB,
      // ie we want a complete circle)
      if (angleB <= angleA) angleB += 2 * M_PI;
      angle = angleB - angleA;

      radius = sqrt(aX * aX + aY * aY);
      length = radius * angle;
      int steps, s, step;
      steps = (int) ceil(length / curve_section);

      FloatPoint newPoint;
      for (s = 1; s <= steps; s++) {
        step = (code == 3) ? s : steps - s; // Work backwards for CW
        newPoint.x = cent.x + radius * cos(angleA + angle * ((float) step / steps));
        newPoint.y = cent.y + radius * sin(angleA + angle * ((float) step / steps));
        set_target(newPoint.x, newPoint.y);

        // Need to calculate rate for each section of curve
        if (feedrate > 0)
          feedrate_micros = calculate_feedrate_delay(feedrate);
        else
          feedrate_micros = get_max_speed();

        // Make step
        dda_move(feedrate_micros);
      }

      break;

      //Dwell
    case 4:
      delay((int)search_string('P', instruction, size));
      break;

      //Inches for Units
    case 20:
      x_units = X_STEPS_PER_INCH;
      y_units = Y_STEPS_PER_INCH;
      curve_section = CURVE_SECTION_INCHES;

      calculate_deltas();
      break;

      //mm for Units
    case 21:
      x_units = X_STEPS_PER_MM;
      y_units = Y_STEPS_PER_MM;
      curve_section = CURVE_SECTION_MM;

      calculate_deltas();
      break;

      //go home.
    case 28:
      set_target(0.0, 0.0);
      dda_move(get_max_speed());
      break;

#ifdef EXTENDED_GCODE
      //go home via an intermediate point.
    case 30:
      fp.x = search_string('X', instruction, size);
      fp.y = search_string('Y', instruction, size);

      //set our target.
      if(abs_mode)
      {
        if (!has_command('X', instruction, size))
          fp.x = current_units.x;
        if (!has_command('Y', instruction, size))
          fp.y = current_units.y;

        set_target(fp.x, fp.y);
      }
      else
        set_target(current_units.x + fp.x, current_units.y + fp.y);

      //go there.
      dda_move(get_max_speed());

      //go home.
      set_target(0.0, 0.0);
      dda_move(get_max_speed());
      break;

      //Absolute Positioning
#endif
    case 90:
      abs_mode = true;
      break;

      //Incremental Positioning
    case 91:
      abs_mode = false;
      break;

      //Set as home
    case 92:
      set_position(0.0, 0.0);
      break;

    default:
      Serial.print("unknown G");
      Serial.println(code,DEC);
    }
  }

  //find us an m code.
  if (has_command('M', instruction, size))
  {
    code = search_string('M', instruction, size);
    switch (code)
    {
      //TODO: this is a bug because search_string returns 0.  gotta fix that.
    case 0:
      true;
      break;
      /*
			case 0:
       				//todo: stop program
       			break;
       
       			case 1:
       				//todo: optional stop
       			break;
       
       			case 2:
       				//todo: program end
       			break;
       	*/

    default:
      Serial.print("unknown M");
      Serial.println(code);
    }		
  }
#ifdef ENABLE_SERVO_TOOL
  //find us an t code.
  if (has_command('T', instruction, size))
  {
    code = search_string('T', instruction, size);
    switch (code)
    {
      //TODO: this is a bug because search_string returns 0.  gotta fix that.
    case 0:
      changeTool(NO_TOOL);
      break;
    case 1:
      changeTool(TOOL1);
      break;
    case 2:
      changeTool(TOOL2);
      break;
    default:
      Serial.print("unknown T");
      Serial.println(code);
    }		
  }
#endif
  //tell our host we're done.
  ////////Serial.println("ok");
}

//look for the number that appears after the char key and return it
double search_string(char key, char instruction[], int string_size)
{
  char temp[10] = "";

  for (byte i=0; i<string_size; i++)
  {
    if (instruction[i] == key)
    {
      i++;      
      int k = 0;
      while (i < string_size && k < 10)
      {
        if (instruction[i] == 0 || instruction[i] == ' ')
          break;

        temp[k] = instruction[i];
        i++;
        k++;
      }
      return strtod(temp, NULL);
    }
  }
  return 0;
}

//look for the command if it exists.
bool has_command(char key, char instruction[], int string_size)
{
  for (byte i=0; i<string_size; i++)
  {
    if (instruction[i] == key)
      return true;
  }
  return false;
}

