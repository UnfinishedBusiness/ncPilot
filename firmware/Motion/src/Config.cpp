#include "Arduino.h"
#include "Config.h"
#include "Machine.h"
#include <SD.h>
#include <SPI.h>
#include <IniFile.h>

config_t MachineConfig;

const int chipSelect = BUILTIN_SDCARD;
const size_t bufferLen = 80;
char buffer[bufferLen];

//------------------------------------------------------------------------------
void Config_Init() {
  if (!SD.begin(BUILTIN_SDCARD)) {
    while (1)
    {
      digitalWrite(LED, !digitalRead(LED));
      delay(50);
    }
  }
}
void Config_DumpINI()
{
  // re-open the file for reading:
 File myFile = SD.open("/Xmotion.ini");
 if (myFile) {
   Serial.println("/Xmotion.ini");

   // read from the file until there's nothing else in it:
   while (myFile.available()) {
     Serial.write(myFile.read());
   }
   // close the file:
   myFile.close();
 } else {
   // if the file didn't open, print an error:
   Serial.println("error opening \"/Xmotion.ini\"");
 }
}
void Config_ParseINI()
{
    IniFile ini("/Xmotion.ini");
    if (!ini.open())
    {
      Serial.print("Ini file ");
      Serial.print("/Xmotion.ini");
      Serial.println(" does not exist");
      while (1)
      {
        digitalWrite(LED, !digitalRead(LED));
        delay(50);
      }
    }
    if (!ini.validate(buffer, bufferLen))
    {
      Serial.print("ini file ");
      Serial.print(ini.getFilename());
      Serial.print(" not valid: ");
      Serial.println(ini.getError());
      // Cannot do anything else
      while (1)
      {
        digitalWrite(LED, !digitalRead(LED));
        delay(50);
      }
    }

    if (ini.getValue("motion", "number_of_axis", buffer, bufferLen))
    {
      MachineConfig.number_of_axis = atof(buffer);
    }
    if (ini.getValue("motion", "max_linear_velocity", buffer, bufferLen))
    {
      MachineConfig.max_linear_velocity = atof(buffer);
    }

    for (int x = 0; x < MachineConfig.number_of_axis; x++)
    {
      char section[100];
      sprintf(section, "axis:%d", x);
      if (ini.getValue(section, "step_pin", buffer, bufferLen))
      {
        MachineConfig.axis[x].step_pin = atoi(buffer);
      }
      if (ini.getValue(section, "dir_pin", buffer, bufferLen))
      {
        MachineConfig.axis[x].dir_pin = atoi(buffer);
      }
      if (ini.getValue(section, "scale", buffer, bufferLen))
      {
        MachineConfig.axis[x].scale = atof(buffer);
      }
      if (ini.getValue(section, "max_accel", buffer, bufferLen))
      {
        MachineConfig.axis[x].max_accel = atof(buffer);
      }
      if (ini.getValue(section, "max_velocity", buffer, bufferLen))
      {
        MachineConfig.axis[x].max_velocity = atof(buffer);
      }
      if (ini.getValue(section, "axis_letter", buffer, bufferLen))
      {
        MachineConfig.axis[x].axis_letter = toupper(buffer[0]);
      }
    }

}
