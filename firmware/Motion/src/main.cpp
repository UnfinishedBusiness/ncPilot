#include "Arduino.h"
#include "Machine.h"

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
unsigned long motion_timestamp = 0;
unsigned long move_start_timestamp = 0;
unsigned long move_decel_timestamp = 0;
boolean InMotion = false;
long x_dist_in_steps;
long y_dist_in_steps;
float decceleration_dtg_marker;
unsigned long time_required_to_accelerate;
/* ---------------- */

float targets[][3] = {
            {1, 1, 10},
            {1, 3, 20},
            {3, 3, 30},
            {3, 1, 40},
            {1, 1, 50},

            //{1, 1, 60},
            {1, 3, 70},
            {3, 3, 80},
            {3, 1, 90},
            {10, 18, 100},
    };
int target_pointer = 0;
int target_pointer_length = 9;

unsigned long cycle_speed = 0; //Interupt timer in ms, needs to be in microseconds once we are ported to ARM embedded
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
void setFeedrate(float feedrate)
{
    linear_velocity = feedrate;
    x_dist_in_steps = fabs(target_position[0] - machine_position[0]);
    y_dist_in_steps = fabs(target_position[1] - machine_position[1]);
    float linear_distance_in_scaled_units = getDistance(machine_position_dro, target_position_in_real_units);
    if (x_dist_in_steps > y_dist_in_steps) //The x axis has farther to travel. Coordinate feedrate on X axis
    {
        cycle_speed = (int)((((linear_distance_in_scaled_units / feedrate) * (60000.0 * 1000)) / x_dist_in_steps) - 0.7); //The -n is about what the interupt loop costs in time. This is critical for accel/deccel timing
    }
    else
    {
        cycle_speed = (int)((((linear_distance_in_scaled_units / feedrate) * (60000.0 * 1000)) / y_dist_in_steps) - 0.7);
    }
}
void set_target_position(float x, float y, float target_feed_rate)
{
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
    setFeedrate(MIN_FEED_RATE); //Set initial feedrate for move

    xx1 = target_position[0];
    yy1 = target_position[1];
    xx0 = machine_position[0];
    yy0 = machine_position[1];
    dx = abs(xx1-xx0);
    sx = xx0<xx1 ? 1 : -1;
    dy = abs(yy1-yy0);
    sy = yy0<yy1 ? 1 : -1;
    err = (dx>dy ? dx : -dy)/2;
    move_start_timestamp = millis();
    motion_timestamp = 0; //This ensures that as soon as there is a target we don't have to wait for the next cycle at "min_feed_rate" for the move to begin
}
void interupt()
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
    if ((micros() - motion_timestamp) > cycle_speed)
    {
        machine_position[0] = xx0;
        machine_position[1] = yy0;
        machine_position_dro[0] = machine_position[0] * X_SCALE_INVERSE;
        machine_position_dro[1] = machine_position[1] * Y_SCALE_INVERSE;
        if (xx0==xx1 && yy0==yy1)
        {
            //We are at our target position. Set next target position
            if (InMotion == true) //If we are in motion when the due ends, set the next target position as long as there is one.
            {
                if (target_pointer_length > target_pointer)
                {
                    set_target_position(targets[target_pointer][0], targets[target_pointer][1], targets[target_pointer][2]);
                    target_pointer++;
                }
                else //We reached the end of the program. Set pointer back to beginning
                {
                    target_pointer = 0;
                }
            }
            InMotion = false;
            linear_velocity = 0;
        }
        else
        {
            InMotion = true;
            e2 = err;
            if (e2 >-dx)
            {
              err -= dy; xx0 += sx;
              digitalWrite(X_STEP, HIGH);
              delayMicroseconds(5);
              digitalWrite(X_STEP, LOW);
            }
            if (e2 < dy)
            {
              err += dx; yy0 += sy;
              digitalWrite(Y_STEP, HIGH);
              delayMicroseconds(5);
              digitalWrite(Y_STEP, LOW);
            }
        }
        motion_timestamp = micros();
    }
}
void setup()
{
  pinMode(LED, OUTPUT);

  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);

  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);

  set_target_position(targets[target_pointer][0], targets[target_pointer][1], targets[target_pointer][2]);
  target_pointer++;
}

void loop()
{
  while(true)
  {
    interupt();
  }
  digitalWrite(LED, HIGH);
  digitalWrite(X_STEP, HIGH);
  digitalWrite(Y_STEP, HIGH);
  delayMicroseconds(5);
  digitalWrite(LED, LOW);
  digitalWrite(X_STEP, LOW);
  digitalWrite(Y_STEP, LOW);
  delayMicroseconds(900);
}
