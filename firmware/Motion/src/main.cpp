#include "Arduino.h"
#include "Machine.h"
#include "StepGen.h"
#include "Motion.h"

#include <mk20dx128.h>

void setup()
{
  pinMode(LED, OUTPUT);

  pinMode(X_STEP, OUTPUT);
  //pinMode(X_DIR, OUTPUT);

  pinMode(Y_STEP, OUTPUT);
  //pinMode(Y_DIR, OUTPUT);

  Serial.begin(115200);

  stepgen_init(2);
  motion_init();

  stepgen_init_gen(0, Y_STEP, Y_DIR);
  stepgen_init_gen(1, X_STEP, X_DIR);

  motion_init_axis(0, 'Y', Y_ACCEL, Y_SCALE);
  motion_init_axis(1, 'X', X_ACCEL, X_SCALE);

}
void loop()
{
  if (Serial.available())
  {
    Serial.read();
    stepgen_segment_t segment;
    float x_target = 10;
    float y_target = 5;
    float initial_feedrate = MIN_FEED_RATE;
    float target_feedrate = 5.0;
    float target_accel = 20.0;
    //Find the axis that has to travel the farthest. Then divide each axis target position by the big axis. This is the scale factor for each axis' target feedrate
    // and acceleration rate
    accel_t x_accel = stepgen_plan_acceleration(MIN_FEED_RATE, target_feedrate, target_accel);
    accel_t y_accel = stepgen_plan_acceleration(MIN_FEED_RATE, target_feedrate * (y_target / x_target), target_accel * (y_target / x_target));
    Serial.print("x_accel.target_velocity: ");
    Serial.println(x_accel.target_velocity);
    Serial.print("x_accel.accel_distance: ");
    Serial.println(x_accel.accel_distance);
    Serial.print("y_accel.target_velocity: ");
    Serial.println(y_accel.target_velocity);
    Serial.print("y_accel.accel_distance: ");
    Serial.println(y_accel.accel_distance);
    int number_of_cycles = (int)(x_accel.accel_distance / SEGMENT_LENGTH);
    float x_velocity_inc_per_cycle = (float)(x_accel.target_velocity - x_accel.initial_velocity) / number_of_cycles;
    float y_velocity_inc_per_cycle = (float)(y_accel.target_velocity - y_accel.initial_velocity) / number_of_cycles;
    Serial.print("number_of_cycles: ");
    Serial.println(number_of_cycles);
    Serial.print("x_velocity_inc_per_cycle: ");
    Serial.println(x_velocity_inc_per_cycle, 4);
    Serial.print("y_velocity_inc_per_cycle: ");
    Serial.println(y_velocity_inc_per_cycle, 4);
    float x_velocity = initial_feedrate;
    float y_velocity = initial_feedrate;
    for (int x = 0; x < number_of_cycles; x++)
    {
      segment.steps_to_move[0] = X_SCALE * SEGMENT_LENGTH;
      segment.segment_velocity[0] = (x_velocity * X_SCALE);
      segment.segment_rate[0] = (int)(1000000.0 / (x_velocity * X_SCALE));
      x_velocity += x_velocity_inc_per_cycle;

      segment.steps_to_move[1] = (Y_SCALE * (SEGMENT_LENGTH * (y_target / x_target)));
      segment.segment_velocity[1] = (y_velocity * Y_SCALE);
      segment.segment_rate[1] = (int)(1000000.0 / (y_velocity * Y_SCALE));
      y_velocity += y_velocity_inc_per_cycle;

      stepgen_push_segment_to_stack(segment);
    }
  }
  digitalWrite(LED, !digitalRead(LED));
  delay(100);
}
