#ifndef EASY_SERIAL_
#define EASY_SERIAL_

#include <Xrender.h>
#include <serial/serial.h>
#include <stdio.h>
#include <iostream>

class easy_serial{
    public:
        std::string connect_description;
        bool auto_connect;
        int baudrate;
        bool is_connected;
        void (*read_line_handler)(std::string line);
        void (*read_byte_handler)(char byte);
        easy_serial(std::string c, void (*b)(char), void (*r)(std::string))
        {
            connect_description = c;
            auto_connect = true;
            baudrate = 115200;
            read_line_handler = r;
            read_byte_handler = b;
            connection_retry_timer = 0;
            serial_port = "";
        }
        /*
            access to main loop
        */
        void tick();

        /*
            Send a single byte
        */
        void send_byte(char b);

        /*
            Send a un-encoded string
        */
        void send_string(std::string s);

        /*
            Send a CRC32 encoded string
        */
        void send_crc32(std::string s);
    private:
        serial::Serial serial;
        std::string serial_port;
        std::string read_line;
        unsigned long connection_retry_timer;
        /* 
            CRC-32C (iSCSI) polynomial in reversed bit order.
        */
        #define POLY 0x82f63b78
        uint32_t crc32c(uint32_t crc, const char *buf, size_t len);

        /*
            Equivilent to arduino delay
        */
        void delay(int ms);


};

#endif