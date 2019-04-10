#include "Arduino.h"
#include "Motion.h"

motion_t motion;
axis_t axis[MAX_NUMBER_OF_AXIS];


void motion_init(int number_of_axis, float min_feed_rate, float max_linear_velocity)
{
  if (number_of_axis < MAX_NUMBER_OF_AXIS)
  {
    motion.target_velocity = 0;
    motion.current_velocity = 0;
    motion.exit_velocity = 0;
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
float getAxisTargetByLetter(char axis_word)
{
  for (int x = 0; x < motion.number_of_axis; x++)
  {
    if (toupper(axis[x].axis_letter) == toupper(axis_word))
    {
      return axis[x].target_position;
    }
  }
  return 0.0; //If we can't find the axed for axis we assume zer0;
}
float getAxisPositionByLetter(char axis_word)
{
  for (int x = 0; x < motion.number_of_axis; x++)
  {
    if (toupper(axis[x].axis_letter) == toupper(axis_word))
    {
      return axis[x].current_position;
    }
  }
  return 0.0; //If we can't find the axed for axis we assume zer0;
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
float getAccelerationVelocity(float initial_velocity, float acceleration_rate, float distance_into_move) //Returns feedrate in inches/min at giben distance into move
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
    float feed_scale_factor = motion.min_feed_rate / feedrate;
    motion.current_velocity = feedrate;
    for (int x = 0; x < motion.number_of_axis; x++)
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
void motion_set_target_position(char* target_words, float target_velocity, float exit_velocity)
{
  motion.target_velocity = target_velocity;
  motion.exit_velocity = exit_velocity;

  Serial.print("Setting target -> ");
  Serial.println(target_words);
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
              number[num_p] = target_words[x];
              num_p++;
        }while(isdigit(target_words[x]) || target_words[x] == '.' || target_words[x] == '-');
        number[num_p-1] = '\0';
        x--;
        //Serial.print("\tParsed Number: ");
        //Serial.println(number);
        axis_value = atof(number);
        for (int x = 0; x < motion.number_of_axis; x++)
        {
          if (toupper(axis[x].axis_letter) == toupper(axis_word))
          {
            axis[x].target_position = axis_value;
            Serial.print(axis[x].axis_letter);
            Serial.print(" Axis Target Position is ");
            Serial.print(axis[x].target_position);
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
            Serial.print(" and steps to travel is ");
            Serial.println(axis[x].steps_left_to_travel);
          }
        }
      }
      x++;
  }
  //Calculate the amount of time the move will take at min_feed_rate
  float target_position[3];
  target_position[0] = getAxisTargetByLetter('x');
  target_position[1] = getAxisTargetByLetter('y');
  target_position[2] = getAxisTargetByLetter('z');
  float current_position[3];
  current_position[0] = getAxisPositionByLetter('x');
  current_position[1] = getAxisPositionByLetter('y');
  current_position[2] = getAxisPositionByLetter('z');
  float total_move_distance = getLinearDistance(target_position, current_position);
  Serial.print("Moves Linear Distance: ");
  Serial.println(total_move_distance);
  unsigned long move_time = (total_move_distance / motion.min_feed_rate) * (60 * 1000 * 1000);
  Serial.print("Move will take  ");
  Serial.print(move_time);
  Serial.print(" uSec at ");
  Serial.println(motion.min_feed_rate);
  for (int x = 0; x < motion.number_of_axis; x++)
  {
    axis[x].cycle_speed_at_min_feed_rate = (move_time / axis[x].steps_left_to_travel);
    axis[x].initial_velocity = ((60 * 1000 * 1000) / axis[x].cycle_speed_at_min_feed_rate) / axis[x].scale;
    Serial.print(axis[x].axis_letter);
    Serial.print(" Axis Initial Velocity is ");
    Serial.print(axis[x].initial_velocity);
    Serial.print(" and Initial Cycle speed is ");
    Serial.println(axis[x].cycle_speed_at_min_feed_rate);
  }
  /* Calculate amount of distance required to accelrate to target velocity using the axis that has to travel the farthest's max_accel constraint
  If we don't have enough distance to accelerate from motion.current_velocity to motion.target_velocity, update motion.target_velocity to the highest
  feesable target_velocity given max accel contraints*/

  int biggest_axis = 0;
  int biggest_distance = 0;
  for (int x = 0; x < motion.number_of_axis; x++)
  {
    if (biggest_distance < axis[x].steps_left_to_travel)
    {
      biggest_axis = x;
      biggest_distance = axis[x].steps_left_to_travel;
    }
  }
  //Calculate the time required to accelerate from initial_velocity to target_feedrate in seconds
  float accel_time = ((motion.target_velocity / 60.0) - (motion.current_velocity / 60.0)) / (axis[biggest_axis].max_accel / 60.0);
  //Calculate the distance needed to reach target velocity from current velocity
  float accel_displacement = 0.5 * ((motion.target_velocity / 60) + (motion.current_velocity / 60)) * accel_time;
  Serial.print("Acceleration requires ");
  Serial.print(accel_time);
  Serial.print("s aka a distance of ");
  Serial.print(accel_displacement);
  Serial.println(" to accelerate from current_velocity to target_velocity");

  //Calculate the time required to accelerate from initial_velocity to target_feedrate in seconds
  float deccel_time = ((motion.target_velocity / 60.0) - (motion.exit_velocity / 60.0)) / (axis[biggest_axis].max_accel / 60.0);
  //Calculate the distance needed to reach target velocity from current velocity
  float deccel_displacement = 0.5 * ((motion.target_velocity / 60) + (motion.exit_velocity / 60)) * deccel_time;
  Serial.print("deceleration requires ");
  Serial.print(deccel_time);
  Serial.print("s aka a distance of ");
  Serial.print(deccel_displacement);
  Serial.println(" to decelerate from target_velocity to exit_velocity");

  /* Do we have enough distance to travel to accelerate to target_velocity then decelerate to exit_velocity?*/
  if (total_move_distance > (accel_displacement + deccel_displacement))
  {
    //No Further calculations will be needed here
    Serial.println("We have enough distance to accel and deccel!");
  }
  else
  {
    /* Since we don't have enough distance, we need to find the maximum possible target feedrate that we can accelrate to and decelerate from within
     our constraints for the move */
    Serial.println("We don't enough distance to accel and deccel!");
    /* The idea is to decrement target_velocity by one "min_feedrate" and re-calculate accel and decel until we find a target that fits whithin our contraints*/
    while (motion.target_velocity > motion.min_feed_rate)
    {
      motion.target_velocity -= motion.min_feed_rate;
      accel_time = ((motion.target_velocity / 60.0) - (motion.current_velocity / 60.0)) / (axis[biggest_axis].max_accel / 60.0);
      accel_displacement = 0.5 * ((motion.target_velocity / 60) + (motion.current_velocity / 60)) * accel_time;

      //Calculate the time required to accelerate from initial_velocity to target_feedrate in seconds
      deccel_time = ((motion.target_velocity / 60.0) - (motion.exit_velocity / 60.0)) / (axis[biggest_axis].max_accel / 60.0);
      //Calculate the distance needed to reach target velocity from current velocity
      deccel_displacement = 0.5 * ((motion.target_velocity / 60) + (motion.exit_velocity / 60)) * deccel_time;
      if (total_move_distance > (accel_displacement + deccel_displacement))
      {
        Serial.print("Maximum target_velocity is ");
        Serial.println(motion.target_velocity);

        Serial.print("Acceleration requires ");
        Serial.print(accel_time);
        Serial.print("s aka a distance of ");
        Serial.print(accel_displacement);
        Serial.println(" to accelerate from current_velocity to target_velocity");

        Serial.print("deceleration requires ");
        Serial.print(deccel_time);
        Serial.print("s aka a distance of ");
        Serial.print(deccel_displacement);
        Serial.println(" to decelerate from target_velocity to exit_velocity");        
        break;
      }
    }
  }

}
/* Facilitate High Priority Step Train timing */
void motion_timer_tick()
{

}
