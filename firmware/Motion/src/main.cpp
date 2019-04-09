#include "Arduino.h"
#include "Machine.h"

#include <mk20dx128.h>

IntervalTimer MotionTimer;

/* These units are in steps */
int target_position[] = {0, 0};
int machine_position[] = {0, 0};
/* --------- */

/* These are in scaled units */
float last_position[] = {0.0, 0.0};
float machine_position_dro[] = {0.0, 0.0};
float target_position_in_real_units[] = {0.0, 0.0};
/* ------------ */

int dx, dy, err, e2, sx, sy, xx0, xx1, yy0, yy1;
long velocity_update_timestamp;
unsigned long sample_period = 10; //Sample every x milliseconds

/* Variables updated by sample check and used my move */
float target_velocity = 0.0;
float linear_velocity = 0.0;
float x_velocity = 0.0;
float y_velocity = 0.0;
unsigned long x_motion_timestamp = 0;
unsigned long y_motion_timestamp = 0;
unsigned long move_start_timestamp = 0;
unsigned long move_decel_timestamp = 0;
boolean InMotion = false;
long x_dist_in_steps;
long y_dist_in_steps;
float decceleration_dtg_marker;
unsigned long time_required_to_accelerate;
/* ---------------- */

float targets[][3] = {
            {1, 1, 200},
            {10, 15, 350},
    };
int target_pointer = 0;
int target_pointer_length = 2;

unsigned long x_cycle_speed = 0; //Interupt timer in ms, needs to be in microseconds once we are ported to ARM embedded
unsigned long y_cycle_speed = 0;
bool x_dir = false;
bool y_dir = false;
float weighted_average(float rawValue, float weight, float lastValue)
{
        // run the filter:
        float result = (float) (weight * rawValue + (1.0-weight)*lastValue);
        // return the result:
        return result;
}
float getDistance(float start_point[], float end_point[])
{
  float x,y;
  x = end_point[0] - start_point[0];
  y = end_point[1] - start_point[1];
  return sqrtf(x*x + y*y);
}
//Initial Velocity is in units per minute, acceleration is in units/min^2, time is in milliseconds
float getAcceleratedVelocity(float initial_velocity, float acceleration, float time_into_move) //Returns feedrate in inches/min
{
    return (initial_velocity / 60.0) + ((acceleration / 60.0) * (time_into_move / 1000.0)) * 60.0;
}
/*
Figure out how long the move will take at MIN_FEED_RATE for each axis total distance to travel.
Then multiply the scale factor of feedrate to each axis cycle speed variable
*/
void setFeedrate(float feedrate)
{
    float feed_scale_factor = MIN_FEED_RATE / feedrate;
    linear_velocity = feedrate;
    //Serial.print("Setting Feedrate: ");
    //Serial.println(feedrate);

    float total_move_distance = getDistance(machine_position_dro, target_position_in_real_units);
    unsigned long x_steps = abs((machine_position_dro[0] * X_SCALE) - (target_position_in_real_units[0] * X_SCALE));
    unsigned long y_steps = abs((machine_position_dro[1] * Y_SCALE) - (target_position_in_real_units[1] * Y_SCALE));
    //How long will the move take if feedrate is contstant at MIN_FEED_RATE until we reach target? (in uSec)
    unsigned long move_time = (total_move_distance / MIN_FEED_RATE) * (60 * 1000 * 1000);
    //Serial.print("Move Time: ");
    //Serial.println(move_time);
    //How long do we have to wait between steps so that each axis runs out of steps at the same time at MIN_FEED_RATE?
    x_cycle_speed = ((move_time / x_steps) * feed_scale_factor);
    y_cycle_speed = ((move_time / y_steps) * feed_scale_factor);

    //Serial.print("X Cycle Time: ");
    //Serial.println(x_cycle_speed);

    //Serial.print("Y Cycle Time: ");
    //Serial.println(y_cycle_speed);
}
void set_target_position(float x, float y, float target_feed_rate)
{
    Serial.print("Currently at -> X: ");
    Serial.print(machine_position_dro[0]);
    Serial.print(" Y: ");
    Serial.print(machine_position_dro[1]);
    Serial.print(" Setting target to -> X: ");
    Serial.print(x);
    Serial.print(" Y: ");
    Serial.print(y);
    Serial.print(" at F ");
    Serial.println(target_feed_rate);

    target_position_in_real_units[0] = x;
    target_position_in_real_units[1] = y;

    float total_move_distance = getDistance(machine_position_dro, target_position_in_real_units);
    last_position[0] = machine_position_dro[0];
    last_position[1] = machine_position_dro[1];

    target_position[0] = (int)(x * X_SCALE);
    target_position[1] = (int)(y * Y_SCALE);

    move_decel_timestamp = 0;
    time_required_to_accelerate = (long) ((((target_feed_rate / 60.0) - (MIN_FEED_RATE / 60.0)) / (X_ACCEL / 60.0)) * 1000.0);
    float distance_required_to_accelerate = (float) (((MIN_FEED_RATE / 60.0) * (time_required_to_accelerate / 1000.0)) + 0.5 * (X_ACCEL / 60.0) * pow(time_required_to_accelerate / 1000.0, 2));
    //Can we accelerate to the target velocity and decelerate to exit velocity in the distance we have to travel?
    if (distance_required_to_accelerate * 2 < total_move_distance)
    {
        //System.out.println("We can accelerate to target velocity!");
        target_velocity = target_feed_rate;
        //Set the deceleration marker. We need to start decelerating when we get to "total_move_distance - distance_required_to_accelerate" on distance traveled
        decceleration_dtg_marker = total_move_distance - distance_required_to_accelerate;
        //System.out.println("decceleration_dtg_marker - " + decceleration_dtg_marker);
    }
    else
    {
        //System.out.println("We can't accelerate to target velocity!");
        float dist = 0;
        long time = 0;
        while(dist < (total_move_distance / 2)) //Increment time until distance is more than half of the move, the velocity at this distance is our maximum achievable feedreate
        {
            dist = (float) (((MIN_FEED_RATE / 60.0) * (time / 1000.0)) + 0.5 * (X_ACCEL / 60.0) * pow(time / 1000.0, 2));
            time++;
        }
        target_velocity = getAcceleratedVelocity(MIN_FEED_RATE, X_ACCEL, time);
        //System.out.println("Maximum feedrate is - " + target_velocity);
        time_required_to_accelerate = (long) ((((target_velocity / 60.0) - (MIN_FEED_RATE / 60.0)) / (X_ACCEL / 60.0)) * 1000.0);
        distance_required_to_accelerate = (float) (((MIN_FEED_RATE / 60.0) * (time_required_to_accelerate / 1000.0)) + 0.5 * (X_ACCEL / 60.0) * pow(time_required_to_accelerate / 1000.0, 2));
        decceleration_dtg_marker = total_move_distance - distance_required_to_accelerate;
        //Figure out the maximum feedrate from acceleration on half of the distance to travel
    }

    //We also need to keep track of direction here as well, we can set the direction pins here!
    if (target_position[0] - machine_position[0] < 0)
    {
      x_dir = false;
    }
    else
    {
      x_dir = true;
    }
    digitalWrite(X_DIR, x_dir);
    if (target_position[1] - machine_position[1] < 0)
    {
      y_dir = false;
    }
    else
    {
      y_dir = true;
    }
    digitalWrite(Y_DIR, y_dir);
    x_dist_in_steps = fabs(target_position[0] - machine_position[0]);
    y_dist_in_steps = fabs(target_position[1] - machine_position[1]);
    setFeedrate(MIN_FEED_RATE); //Set initial feedrate for move
    move_start_timestamp = millis();
    x_motion_timestamp = 0; //This ensures that as soon as there is a target we don't have to wait for the next cycle at "min_feed_rate" for the move to begin
    y_motion_timestamp = 0; //And that both axis start moving at the same time
}
void motion_interupt()
{
    if (((micros() - x_motion_timestamp) > x_cycle_speed) && (x_dist_in_steps > 0))
    {
        InMotion = true;
        digitalWrite(X_STEP, HIGH);
        if (x_dir == true)
        {
          machine_position[0]++;
        }
        else
        {
          machine_position[0]--;
        }
         //This needs to be incremented or decremented based on direction of travel!
        x_dist_in_steps--; //Remove one step from axis distance counter
        machine_position_dro[0] = machine_position[0] * X_SCALE_INVERSE;
        x_motion_timestamp = micros();
    }
    else if (((micros() - y_motion_timestamp) > y_cycle_speed) && (y_dist_in_steps > 0))
    {
      InMotion = true;
      digitalWrite(Y_STEP, HIGH);
      if (y_dir == true)
      {
        machine_position[1]++;
      }
      else
      {
        machine_position[1]--;
      }
      y_dist_in_steps--; //Remove one step from axis distance counter
      machine_position_dro[1] = machine_position[1] * Y_SCALE_INVERSE;
      y_motion_timestamp = micros();
    }
    else if (x_dist_in_steps == 0 && y_dist_in_steps == 0) //Move is done
    {
      if (InMotion == true)
      {
        if (target_pointer_length > target_pointer)
        {
            set_target_position(targets[target_pointer][0], targets[target_pointer][1], targets[target_pointer][2]);
            target_pointer++;
        }
        else //We reached the end of the program. Set pointer back to beginning
        {
            target_pointer = 0;
            set_target_position(targets[target_pointer][0], targets[target_pointer][1], targets[target_pointer][2]);
            target_pointer++;
        }
      }
      InMotion = false;
      linear_velocity = 0;
    }
    else
    {
      delayMicroseconds(2);
      digitalWrite(Y_STEP, LOW);
      digitalWrite(X_STEP, LOW);
    }
}
void setup()
{
  pinMode(LED, OUTPUT);

  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);

  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);

  Serial.begin(115200);

  MotionTimer.begin(motion_interupt, 10);

  set_target_position(targets[target_pointer][0], targets[target_pointer][1], targets[target_pointer][2]);
  target_pointer++;
}
void loop()
{
  while(true)
  {
    if ((millis() - velocity_update_timestamp) > sample_period && InMotion == true)
    {
        //float sample_distance = getDistance(machine_position_dro, last_position);
        //System.out.println("Sample Distance: " + sample_distance);
        //(distance/time)*60,000 = velocity in steps per minute
        //linear_velocity = (sample_distance/(System.currentTimeMillis() - move_start_timestamp))*60000;
        x_velocity = (abs(machine_position_dro[0] - last_position[0])/(millis() - move_start_timestamp))*60000; //in steps per minute
        y_velocity = (abs(machine_position_dro[1] - last_position[1])/(millis() - move_start_timestamp))*60000; //in steps per minute
        float distance_traveled = getDistance(machine_position_dro, last_position);
        float new_velocity = getAcceleratedVelocity(MIN_FEED_RATE, X_ACCEL, (millis() - move_start_timestamp));
        //System.out.println("(t=" + (System.currentTimeMillis() - move_start_timestamp) + ") new_velocity - " + new_velocity);
        if (distance_traveled < decceleration_dtg_marker)
        {
            //We accelerate to target velocity
            if (new_velocity < target_velocity)
            {
                setFeedrate(new_velocity);
            }
            else
            {
                setFeedrate(target_velocity);
            }
            move_decel_timestamp = 0;
        }
        else if (move_decel_timestamp == 0)
        {
            move_decel_timestamp = millis();
            //System.out.println("move_decel_timestamp - " + move_decel_timestamp);
        }
        else if (move_decel_timestamp == -1)
        {

        }
        else
        {
            //We deccelerate to target velocity
            //System.out.println("Decelleration Period: " + (time_required_to_accelerate - (System.currentTimeMillis() - move_decel_timestamp)));
            new_velocity = getAcceleratedVelocity(target_velocity, X_ACCEL, time_required_to_accelerate - (millis() - move_decel_timestamp));
            //System.out.println("new_velocity - " + new_velocity);
            if (new_velocity < MIN_FEED_RATE)
            {
                //System.out.println("Clamping feedrate to min feed rate to ensure move finishes!");
                setFeedrate(MIN_FEED_RATE);
                move_decel_timestamp = -1; //Deceleration is finished
            }
            else
            {
                setFeedrate(new_velocity);
            }
        }

        velocity_update_timestamp = millis();
    }
    else
    {
      digitalWrite(LED, !digitalRead(LED));
    }
  }
}
