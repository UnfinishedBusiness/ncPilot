#include "Arduino.h"
#include "Motion.h"

motion_t motion;
axis_t axis[MAX_NUMBER_OF_AXIS];


void motion_init(int number_of_axis, int min_feed_rate, int max_linear_velocity)
{
  if (number_of_axis < MAX_NUMBER_OF_AXIS)
  {
    for (int x = 0; x < number_of_axis; x++)
    {
      motion.target_position[x] = 0;
      motion.last_position[x] = 0;
      motion.target_position_in_real_units[x] = 0;
      motion.target_velocity = 0;
      motion.linear_velocity = 0;
      motion.move_start_timestamp = 0;
      motion.move_decel_timestamp = 0;
      motion.InMotion = false;
      motion.decceleration_dtg_marker = 0;
      motion.time_required_to_accelerate = 0;
      motion.number_of_axis = number_of_axis;
      motion.min_feed_rate = min_feed_rate;
      motion.max_linear_velocity = max_linear_velocity;
    }
  }
}
void motion_init_axis(int axis_number, int axis_type, int step_pin, int dir_pin, char axis_letter, char* scaled_units, float scale, float max_accel, float max_velocity)
{
  if (axis_number < motion.number_of_axis)
  {
    axis[axis_number].axis_type = axis_type;
    axis[axis_number].step_pin = step_pin;
    axis[axis_number].dir_pin = dir_pin;
    axis[axis_number].axis_letter = axis_letter;
    axis[axis_number].scale_units = scaled_units;
    axis[axis_number].scale = scale;
    axis[axis_number].max_accel = max_accel;
    axis[axis_number].max_velocity = max_velocity;
  }
}
float getAxisTargetByLetter(char letter)
{
  for (int x = 0; x < motion.number_of_axis; x++)
  {
    if (toupper(axis[x].axis_letter) == toupper(axis_word))
    {
      return axis[x].target_position;
    }
  }
}
float getAxisPositionByLetter(char letter)
{
  for (int x = 0; x < motion.number_of_axis; x++)
  {
    if (toupper(axis[x].axis_letter) == toupper(axis_word))
    {
      return axis[x].current_position;
    }
  }
}
//We can only have XYZ on linear axis. There can be more than one axis with the same letter but with different scales
float getLinearDistance(float start_point[3], float end_point[3])
{
  float x,y,z;
  x = end_point[0] - start_point[0];
  y = end_point[1] - start_point[1];
  z = end_point[2] - start_point[2];
  return sqrtf(x*x + y*y + z*z);
}
//Initial Velocity is in units per minute, acceleration is in units/min^2, distance is in "scale_units" (Hopefully inches because MERICA!)
float getAccelerationVelocity(float initial_velocity, float acceleration_rate, float distance_into_move) //Returns feedrate in inches/min
{
    //Calculate the time required to accelerate from initial_velocity to target_feedrate in seconds
    float time = ((motion.target_velocity / 60.0) - (initial_velocity / 60.0)) / (acceleration_rate / 60.0);
    //Calculate what the velocity should be at "distance_into_move"
    return ((2 * distance_into_move)/time) - (acceleration_rate / 60.0);
}
/*
When we set the target we calculate how long the move is supposed to take in each linear axis at "min_feed_rate"
Then we divide that time by the number of steps that each axis has to travel. This gives us the amount of time
that we need to wait between steps to arive at the endpoint at the same time. In order to change the feedrate
we scale the "cycle_speed" by a scale factor of "min_feed_rate" from input feedrate.
This function needs to scale the cycle speed for all axis, not just linear beacuse
angular axis need to arive at the endpoint at the same time as the linear axis
*/
void motion_set_feedrate(float feedrate)
{
    float feed_scale_factor = MIN_FEED_RATE / feedrate;
    linear_velocity = feedrate;
    for (int x = 0; x < number_of_axis; x++)
    {
      axis[x].cycle_speed = axis[x].cycle_speed_at_min_feed_rate * feed_scale_factor;
      axis[x].current_velocity = axis[x].initial_velocity * feed_scale_factor;
    }
}
/*
When we set the target position we need to calculate how long the move will take at "min_feed_rate" in
linear distance. Divide the amount of time the move should take at "min_feed_rate" by the amount of steps
each axis has to travel to get the cycle_speed for each axis.
*/
void motion_set_target_position(char* target_words, float target_feed_rate)
{
  //Set Target positions for each axis by parsing target_words
  int x = 0;
  char axis_word;
  char number[10];
  int num_p;
  float axis_value;
  while(true)
  {
      if (target_words[x] == '\0') break;
      if (isalpha(target_words[x]) && target_words[x] != '-' && target_words[x] != '.') //We are a Axis leter
      {
        axis_word = target_words[x];
        number[0] = '\0';
        num_p = 0;
        do{
              x++;
              number[num_p] = gline[x];
              num_p++;
        }while(isdigit(target_words[x]) || target_words[x] == '.' || target_words[x] == '-');
        number[num_p-1] = '\0';
        x--;
        axis_value = atof(number);
        for (int x = 0; x < motion.number_of_axis; x++)
        {
          if (toupper(axis[x].axis_letter) == toupper(axis_word))
          {
            axis[x].target_position = axis_value;
            float diff = axis[x].target_position - axis[x].current_position;
            if (diff < 0)
            {
              digitalWrite(axis[x].dir_pin, false);
            }
            else
            {
              digitalWrite(axis[x].dir_pin, true);
            }
            axis[x].steps_left_to_travel = fabs(diff) * axis[x].scale;
          }
        }
      }
      x++;
  }
  //Calculate the amount of time the move will take at min_feed_rate
  float target_position[3];
  target_position[0] = getAxisTargetByLetter("x");
  target_position[1] = getAxisTargetByLetter("y");
  target_position[2] = getAxisTargetByLetter("z");
  float current_position[3];
  current_position[0] = getAxisPositionByLetter("x");
  current_position[1] = getAxisPositionByLetter("y");
  current_position[2] = getAxisPositionByLetter("z");
  float total_move_distance = getDistance(target_position, current_position);
  unsigned long move_time = (total_move_distance / motion.min_feed_rate) * (60 * 1000 * 1000);
  for (int x = 0; x < number_of_axis; x++)
  {
    axis[x].cycle_speed_at_min_feed_rate = ((move_time / axis[x].steps_left_to_travel);
    axis[x].initial_velocity = ((60 * 1000 * 1000) / cycle_speed_at_min_feed_rate) / axis[x].scale;
  }
  
}
/* Facilitate High Priority Step Train timing */
void motion_timer_tick()
{

}
