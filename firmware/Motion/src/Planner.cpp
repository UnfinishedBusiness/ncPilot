#include "Arduino.h"
#include "Planner.h"
#include "Machine.h"
#include "Config.h"
#include <SD.h>
#include <SPI.h>
#include <IniFile.h>
#include <ArduinoJson.h>
#include "CircularBuffer.h"

CircularBuffer<action_t, LOOK_AHEAD_SLOTS> action_buffer;
CircularBuffer<planned_action_t, LOOK_AHEAD_SLOTS> planned_action_buffer;

void Planner_Init()
{

}
/*
Read move file and fill action buffer until its full. Function needs to return the line that buffer became full. Function takes argument of line to start at.
Function needs to return as quickly as possible and is used to fill action buffer so that there are always segments to be executed to make motion fluent
*/
int Planner_FillActionBuffer(int start_line)
{
  int count = 0;
  File myFile = SD.open("/Moves.xmp");
  int can_add = action_buffer.available();
  if (can_add > 0)
  {
    if (myFile)
    {
      while (myFile.available())
      {
        String line = myFile.readStringUntil('\n');
        if (count >= start_line)
        {
          action_t action;
          line.toCharArray(action.json, 1024);
          action_buffer.push(action);
          can_add--;
          if (can_add < 1) return count;
        }
        count++;
      }
      myFile.close();
      return count;
    }
    else
    {
      Serial.println("error opening \"/Moves.xmp\"");
      return -1;
    }
  }
  else
  {
    return start_line;
  }
}
/*
Look at each action in buffer and plan acceleration and deceleration across these moves.
This function gets called every time the action buffer gets topped off so inherently changes
as new actions become available.
*/
void Planner_PlanActionBuffer()
{

}
/*
If we are not in motion, start at planned action 0 and create small segments to add to the stepgen segment buffer
while we facilitate acceration or deceleration across these segments. Never let the segmentizer push segments
past LOOK_AHEAD_SLOTS/2 so that PlanActionBuffer is able to react to new moves that become availble. 
*/
void Planner_SegmentizeActionsInBuffer()
{

}
int Planner_ExecuteMoves_JSON(int start_line)
{
  // re-open the file for reading:
 File myFile = SD.open("/Moves.xmp");
 if (myFile) {
   // read from the file until there's nothing else in it:
   while (myFile.available()) {
     //Serial.write(myFile.read());
     //Each line is a parsable JSON object that represents a single machine action
     String line = myFile.readStringUntil('\n');
     DynamicJsonDocument move(1024);
     deserializeJson(move, line);
     char move_string[1024];
     sprintf(move_string, "action: %s", (const char*)move["action"]);
     Serial.println(move_string);
     if (strcmp((const char*)move["action"], "feed") == 0)
     {
       sprintf(move_string, "\tnumber_of_axis: %d", MachineConfig.number_of_axis);
       Serial.println(move_string);

       for (int x = 0; x < MachineConfig.number_of_axis; x++)
       {
         char axis_letter[2];
         axis_letter[0] = tolower(MachineConfig.axis[x].axis_letter);
         axis_letter[1] = '\0';
         Serial.print("\t\t");
         Serial.print(axis_letter);
         Serial.print(" = ");
         Serial.println((float)move["dest"][axis_letter]);
       }

       sprintf(move_string, "\tfeedrate: %0.4f", (float)move["feedrate"]);
       Serial.println(move_string);

     }
   }
   // close the file:
   myFile.close();
 } else {
   // if the file didn't open, print an error:
   Serial.println("error opening \"/Moves.xmp\"");
 }
}
