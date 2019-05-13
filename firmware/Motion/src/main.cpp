#include "Arduino.h"
#include "Machine.h"
#include "StepGen.h"
#include "Motion.h"
#include "Config.h"
#include "Planner.h"
#include <SD.h>

#include <mk20dx128.h>

const int chipSelect = BUILTIN_SDCARD;

void setup()
{
  pinMode(LED, OUTPUT);
  Serial.begin(115200);

  if (!SD.begin(BUILTIN_SDCARD)) {
    while (1)
    {
      digitalWrite(LED, !digitalRead(LED));
      delay(50);
    }
  }

  Config_Init();
  Config_ParseINI();
  Planner_Init();
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

/*
The main loop will be used to plan motion from parsing the Move.xmp action file.

The Serial Communication protocal (single-line JSON objects) will be facilitated via a IntervolTimer
- Will handle Motion Interupt Actions (feedhold, feerate override, ATHC, etc) by adding to a pending event stack
- Will over-write Move.xmp file to SDCARD by reading data from ncPilot HMU and facilitate a checksum to ensure all
moves have been written to controller without communication error

The StepGen will be polled via an IntervolTimer and will push out small sycronized segments at pre-specified frequencies
 that have been preppared by the motion planner.



*/
int action_line = 0;
void loop()
{
  if (Serial.available())
  {
    Serial.read();
    while(true)
    {
      action_line = Planner_FillActionBuffer(action_line);
      Serial.print("Action line - ");
      Serial.println(action_line);
      delay(100);
    }


    /*while(true)
    {
      motion_plan_move("X0Y0Z0", "X4Y2Z0", MIN_FEED_RATE, 15.0, MIN_FEED_RATE);
      delay(2000);
    }*/

    //Config_DumpINI();
  }
  digitalWrite(LED, !digitalRead(LED));
  delay(500);
}
