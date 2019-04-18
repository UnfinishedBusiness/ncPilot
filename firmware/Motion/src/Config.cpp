#include "Arduino.h"
#include "Config.h"
#include "Machine.h"
#include <SD.h>
#include <SPI.h>
#include <IniFile.h>

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
      Serial.print("section 'motion' has an entry 'number_of_axis' with value ");
      Serial.println(buffer);
    }
    else
    {
      Serial.print("Could not read 'motion' from section 'number_of_axis', error was ");
      Serial.println(ini.getError());
    }
}
