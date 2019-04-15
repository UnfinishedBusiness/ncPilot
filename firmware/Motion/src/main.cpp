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
    motion_plan_move("X0Y0", "X1Y2", MIN_FEED_RATE, 5.0, MIN_FEED_RATE);
  }
  digitalWrite(LED, !digitalRead(LED));
  delay(100);
}
