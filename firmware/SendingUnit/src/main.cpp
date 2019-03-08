#include <Arduino.h>
#include <SoftwareSerial.h>

#define NUMBER_OF_BLOCKS 50
#define BLOCK_BUFF_SIZE 20
#define RX_BUFF_SIZE 127

typedef struct
{
  char buff[BLOCK_BUFF_SIZE];
} block_t;

block_t blocks[NUMBER_OF_BLOCKS];
int blocks_pos;
int current_block;
bool run;

SoftwareSerial mcSerial(3, 2); // RX, TX

char hmi_buff[RX_BUFF_SIZE];
uint8_t hmi_pos;
char mc_buff[RX_BUFF_SIZE];
uint8_t mc_pos;

void setup()
{
  mcSerial.begin(9600);
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  hmi_pos = 0;
  mc_pos = 0;
  blocks_pos = 0;
  run = false;
  current_block = 0;
}

void send_block()
{
  if (run == true)
  {
    if (current_block < blocks_pos + 1)
    {
      Serial.print("{send_block} -> ");
      Serial.println(blocks[current_block].buff);
      mcSerial.println(blocks[current_block].buff);
      current_block++;
    }
    else
    {
      run = false;
      current_block = 0;
    }
  }

}

void loop()
{
  if (Serial.available())
  {
    char c = Serial.read();
    if (hmi_pos < RX_BUFF_SIZE - 1)
    {
      if (c != '\r')
      {
        hmi_buff[hmi_pos] = c;
        hmi_pos++;
      }
    }
    else //We overflowed our buffer
    {
      Serial.println("hmi->RX Overflowed!");
      hmi_pos = RX_BUFF_SIZE - 1;
      hmi_buff[hmi_pos] = '\n';
    }
    if (hmi_buff[hmi_pos-1] == '\r' || hmi_buff[hmi_pos-1] == '\n')
    {
      Serial.println("hmi->RX Returned!");
      if (hmi_buff[0] == '>') //Push a block to blocks array
      {
        Serial.println("hmi->Found Block add command!");
          if (blocks_pos < NUMBER_OF_BLOCKS)
          {
            for (int x = 1; x < hmi_pos-1; x++)
            {
              if (x > BLOCK_BUFF_SIZE - 1) break;
              blocks[blocks_pos].buff[x-1] = hmi_buff[x];
            }
            blocks[blocks_pos].buff[hmi_pos] = '\0';
            Serial.print("Added-> ");
            Serial.print(blocks[blocks_pos].buff);
            Serial.println(" to blocks array!");
            blocks_pos++;
          }
          else
          {
            Serial.println("Block Buffer is full!");
          }
      }
      else if (hmi_buff[0] == '#') //We are a run signal! Start sending blocks to mc
      {
        mcSerial.println("~");
        run = true;
        send_block();
      }
      else if (hmi_buff[0] == '*') //List the blocks
      {
        Serial.println("Blocks on stack->");
        for (int x = 0; x < blocks_pos; x++)
        {
          Serial.print("{block: ");
          Serial.print(x);
          Serial.print("} ");
          Serial.println(blocks[x].buff);
        }
      }
      else if (hmi_buff[0] == '^')
      {
        Serial.println("Adding test blocks!");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
        strcpy(blocks[blocks_pos++].buff, "g0x10");
        strcpy(blocks[blocks_pos++].buff, "g0x0");
      }
      else //Pass to mc
      {
        //mcSerial.print(hmi_buff);
      }
      while(hmi_pos > 0) //Reset mc_buff pointer and nullify
      {
        hmi_buff[hmi_pos] = '\0';
        hmi_pos--;
      }
    }
  }

  if (mcSerial.available())
  {
    char c = mcSerial.read();
    if (mc_pos < RX_BUFF_SIZE - 1)
    {
      if (c != '\r')
      {
        mc_buff[mc_pos] = c;
        mc_pos++;
      }
    }
    else //We overflowed our buffer
    {
      //Serial.println("mc->RX Overflowed!");
      mc_pos = RX_BUFF_SIZE - 1;
      mc_buff[mc_pos] = '\n';
    }
    if (mc_buff[mc_pos-1] == '\r' || mc_buff[mc_pos-1] == '\n')
    {
      //Serial.println("mc->RX Returned!");
      if (mc_buff[0] == 'o' && mc_buff[1] == 'k')
      {
        Serial.println("Recieved OK from MC!");
        send_block(); //Send next line of gcode, but only if we are in a "Auto Run" state
      }
      if (mc_buff[0] == '<') //We are a report, pass it directly to Serial1
      {
        Serial.println(mc_buff);
      }
      while(mc_pos > 0) //Reset mc_buff pointer and nullify
      {
        mc_buff[mc_pos] = '\0';
        mc_pos--;
      }
    }
  }
}
