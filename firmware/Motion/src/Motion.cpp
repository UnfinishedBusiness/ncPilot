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
      motion.machine_position[x] = 0;
      motion.last_position[x] = 0;
      motion.machine_position_dro[x] = 0;
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
void setFeedrate(float feedrate)
{
    float feed_scale_factor = MIN_FEED_RATE / feedrate;
    linear_velocity = feedrate;
    for (int x = 0; x < number_of_axis; x++)
    {
      axis[x].cycle_speed = axis[x].cycle_speed_at_min_feed_rate * feed_scale_factor;
    }
}
/* Facilitate High Priority Step Train timing */
void motion_timer_tick()
{

}
