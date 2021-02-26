#ifndef HEX_
#define HEX_

#include <stdio.h>
#include <iostream>
#include <fstream>

#define MAX_CHARS_PER_HEX_RECORD 45

struct flash_page_block_t
{
   uint16_t block_size_bytes = 0;
   uint16_t addressStart = 0; //This is a word-oriented address!
   uint8_t dataBytes[128] = {0};
};

class HexFileRecord
{
    public:
        bool decode(); //Decodes ascii hex file record into constituent parts
        const char* ascii_line;
        uint8_t byteCount = 0; //Number of data bytes
        uint16_t address = 0;   //beginning memmory address offset of the data block (2-byte word-oriented)
        uint8_t recordType = 0; //0x46 = flash
        const char* data = 0; //pointer to where the data bytes start
        uint8_t checkSum = 0;
};


class HexFileClass
{

    public:
        bool open(const char* targFilePath);
        void close();
        //Function that will load data from a hexfile on SD card into a flash_page_block_t data structure
        unsigned int load_hex_records_flash_data_block(flash_page_block_t &targBlock );

        bool moreBytesToConsume()
        {
            return (hexfile_chars_consumed < hexfile_total_bytes);
        }
        // unsigned int last_hexRecord_accessed = 0;
    private:
        //Function to read/decode a line from hex file on SD card
        bool consume_hex_record(HexFileRecord &targRecord);
        unsigned int hexfile_chars_consumed = 0;
        unsigned int hexfile_total_bytes = 0;
        char buf[MAX_CHARS_PER_HEX_RECORD];
        std::ifstream hexfile;
};


#endif //HEX_