#include "Arduino.h"
#include "Machine.h"
#include "StepGen.h"
#include "Motion.h"
#include "Config.h"

#include <mk20dx128.h>

void setup()
{
  pinMode(LED, OUTPUT);

  pinMode(X_STEP, OUTPUT);
  //pinMode(X_DIR, OUTPUT);

  pinMode(Y_STEP, OUTPUT);
  //pinMode(Y_DIR, OUTPUT);

  Serial.begin(115200);

  Config_Init();
  Config_ParseINI();
  stepgen_init(MachineConfig.number_of_axis);
  motion_init();
  for (int x = 0; x < MachineConfig.number_of_axis; x++)
  {
    pinMode(MachineConfig.axis[x].step_pin, OUTPUT);
    pinMode(MachineConfig.axis[x].dir_pin, OUTPUT);
    stepgen_init_gen(x, MachineConfig.axis[x].step_pin, MachineConfig.axis[x].dir_pin);
    motion_init_axis(x, MachineConfig.axis[x].axis_letter, MachineConfig.axis[x].max_accel, MachineConfig.axis[x].scale);
  }
}
void loop()
{
  if (Serial.available())
  {
    Serial.read();
    while(true)
    {
      motion_plan_move("X0Y0Z0", "X4Y2Z4", MIN_FEED_RATE, 2.0, MIN_FEED_RATE);
      delay(2000);
    }

    //Config_DumpINI();
  }
  digitalWrite(LED, !digitalRead(LED));
  delay(500);
}
