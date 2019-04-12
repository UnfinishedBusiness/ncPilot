#include "Arduino.h"
#include "Machine.h"
#include "StepGen.h"

#include <mk20dx128.h>

void setup()
{
  pinMode(LED, OUTPUT);

  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);

  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);

  Serial.begin(115200);

  stepgen_init(2);
  stepgen_init_gen(0, Y_STEP, Y_DIR);
  stepgen_init_gen(1, X_STEP, X_DIR);
}
void plan_acceleration(float initial_velocity, float target_velocity, float acceleration_rate)
{
  stepgen_segment_t segment;
  float segment_length = 0.005;
  float velocity = initial_velocity;

  //How long will acceleration take?
  float accel_time = (target_velocity - initial_velocity) / acceleration_rate;
  float accel_distance = 0.5 * ((target_velocity + initial_velocity) * accel_time);
  /*Serial.print("Accel Time: ");
  Serial.println(accel_time);
  Serial.print("Accel Distance: ");
  Serial.println(accel_distance);
  Serial.print("Number of segments: ");
  Serial.println((int)(accel_distance / segment_length));*/

  float velocity_inc_per_cycle = (float)(target_velocity - initial_velocity) / (float)(accel_distance / segment_length);
  /*Serial.print("Velocity inc per cycle: ");
  Serial.println(velocity_inc_per_cycle, 4);*/

  for (int x = 0; x < (int)(accel_distance / segment_length); x++)
  {
    segment.steps_to_move[0] = Y_SCALE * segment_length;

    segment.segment_speed[0] = (int)(1000000.0 / (velocity * Y_SCALE));
    segment.steps_to_move[1] = 0;
    segment.segment_speed[1] = (MIN_FEED_RATE * X_SCALE);
    stepgen_push_segment_to_stack(segment);
    velocity += velocity_inc_per_cycle;
  }
}
void plan_decceleration(float initial_velocity, float target_velocity, float acceleration_rate)
{
  stepgen_segment_t segment;
  float segment_length = 0.005;
  float velocity = initial_velocity;

  //How long will acceleration take?
  float accel_time = (initial_velocity - target_velocity) / acceleration_rate;
  float accel_distance = 0.5 * ((target_velocity + initial_velocity) * accel_time);
  /*Serial.print("Accel Time: ");
  Serial.println(accel_time);
  Serial.print("Accel Distance: ");
  Serial.println(accel_distance);
  Serial.print("Number of segments: ");
  Serial.println((int)(accel_distance / segment_length));*/

  float velocity_inc_per_cycle = (float)(initial_velocity - target_velocity) / (float)(accel_distance / segment_length);
  /*Serial.print("Velocity inc per cycle: ");
  Serial.println(velocity_inc_per_cycle, 4);*/

  for (int x = 0; x < (int)(accel_distance / segment_length); x++)
  {
    segment.steps_to_move[0] = Y_SCALE * segment_length;

    segment.segment_speed[0] = (int)(1000000.0 / (velocity * Y_SCALE));
    segment.steps_to_move[1] = 0;
    segment.segment_speed[1] = (MIN_FEED_RATE * X_SCALE);
    stepgen_push_segment_to_stack(segment);
    velocity -= velocity_inc_per_cycle;
  }
}
void loop()
{
  if (Serial.available())
  {
    Serial.read();
    plan_acceleration(MIN_FEED_RATE, 4.0, 30.0);
    plan_decceleration(4.0, MIN_FEED_RATE, 30.0);
  }
  plan_acceleration(MIN_FEED_RATE, 6.0, 20.0);
  plan_decceleration(6.0, MIN_FEED_RATE, 20.0);
  delay(1500);
  digitalWrite(LED, !digitalRead(LED));
  delay(100);
}
