#include <stdio.h>
#include <iostream>
#include <cstring>
#include "hex.h"

bool HexFileRecord::decode()
{
   if((char)ascii_line[0] == ':')
   {
      if(1 == sscanf(ascii_line + 1, "%2x", &byteCount))
      {
        if(1 == sscanf(ascii_line + 3, "%4x", &address)) 
        {
            if(1 == sscanf(ascii_line + 7, "%2x", &recordType))
            {
               data = ascii_line + 9;
               if(1 == sscanf(data + (byteCount*2) , "%2x", &checkSum))
               {
                  return true;
               }
            }
         }
      }
   }
   printf("Invalid hex record!\n");
   return false;
}
void HexFileClass::close()
{
   hexfile.close();
   hexfile_chars_consumed = 0;
   hexfile_total_bytes = 0;
}
bool HexFileClass::open(const char* targFilePath){
   hexfile_chars_consumed = 0;
   hexfile.open(targFilePath, std::ios::out | std::ios::binary | std::ios::ate);
   if(hexfile.is_open())
   {
      hexfile_total_bytes = hexfile.tellg();
      hexfile.clear();
   }
   else
   {
       printf("Could not open file!\n");
       return false;
   }
}
bool HexFileClass::consume_hex_record(HexFileRecord &targRecord)
{
   uint8_t bytesToRead = MAX_CHARS_PER_HEX_RECORD;
   if((hexfile_total_bytes - hexfile_chars_consumed) < bytesToRead)
   {
      bytesToRead = (hexfile_total_bytes - hexfile_chars_consumed);
   }
   if (!hexfile.seekg(hexfile_chars_consumed))
   {
       printf("Could not seekg file!\n");
       return false;
   }
   hexfile.read(buf, bytesToRead);
   targRecord.ascii_line = buf;
   if(strlen(targRecord.ascii_line) < 11)
   {
      printf("Incomplete hex record @ uint8_t:  %d\n", hexfile_chars_consumed);
      return false;
   }
   if(!targRecord.decode())
   {
      return false;
   }
   //Record length is always 11 + num data bytes
   hexfile_chars_consumed += (11 + (targRecord.byteCount * 2) + 2);
   return true;
}
unsigned int HexFileClass::load_hex_records_flash_data_block(flash_page_block_t &targBlock)
{
   //Load a hexFile record using some TODO: SD card load hexFile record function
   HexFileRecord targRecord;
   uint16_t bytesEncoded = 0;
   //Loop through records
   while(consume_hex_record(targRecord))
   {
        uint16_t bytesProcessed = 0;
        //If no uint8_ts have been encoded yet, then this is the first hex record being processed.
        //That means that the address contained in this hex record corresponds to the word-oriented
        //address of the target flash block
        if(!bytesEncoded)
        {
            targBlock.addressStart = targRecord.address / 2;  //Remember to convert to word-oriented address!
        }
        //Encode data uint8_ts of flash record
        while(bytesProcessed < targRecord.byteCount)
        {
            int result = sscanf(targRecord.data + (bytesProcessed * 2), "%2x", &targBlock.dataBytes[bytesEncoded]);
            //Check that decoding worked
            if(result != 1)
            {
                printf("Invalid record data digits in record: %2s --> hex file corrupt!", targRecord.data + bytesProcessed);
                return 0;
            }
            bytesProcessed ++;
            bytesEncoded ++;
            //Check that we haven't filled the block
            if(bytesEncoded >= sizeof(targBlock.dataBytes))
            {
                return bytesEncoded;
            }
        }
        //Check if there are any more records before consuming another one
        if(!moreBytesToConsume())
        {
            return bytesEncoded;
        }
   };
   return bytesEncoded;
}