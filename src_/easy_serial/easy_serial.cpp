#include <Xrender.h>
#include <serial/serial.h>
#include "easy_serial.h"
#include <json/json.h>
#include "logging/loguru.h"



void easy_serial::send_byte(uint8_t b)
{
    if (this->is_connected == true)
    {
        try
        {
            uint8_t bytes[1];
            bytes[0] = b;
            this->serial.write(bytes, 1);
        }
        catch(...)
        {
            //do nothing
        }
    }
}
void easy_serial::send_string(std::string s)
{
    if (this->is_connected == true)
    {
        try
        {
            this->serial.write(s);
        }
        catch(...)
        {
            //do nothing
        }
    }
}
void easy_serial::delay(int ms)
{
    unsigned long delay_timer = Xrender_millis();
    while((Xrender_millis() - delay_timer) < ms);
}
void easy_serial::tick()
{
    if (this->serial.isOpen())
    {
        this->is_connected = true;
        try
        {
            int bytes_available = this->serial.available();
            if (bytes_available > 0)
            {
                std::string read = this->serial.read(bytes_available);
                bool eat_byte = false;
                for (int x = 0; x < read.size(); x++)
                {
                    if (this->read_byte_handler != NULL)
                    {
                        eat_byte = this->read_byte_handler(read.at(x));
                    }
                    if (read.at(x) == '\n' || read.at(x) == '\r')
                    {
                        this->read_line.erase(std::remove(this->read_line.begin(), this->read_line.end(), '\n'),this->read_line.end());
                        this->read_line.erase(std::remove(this->read_line.begin(), this->read_line.end(), '\r'),this->read_line.end());
                        if (this->read_line != "")
                        {
                            if (this->read_line_handler != NULL) this->read_line_handler(this->read_line);
                        }
                        this->read_line = "";
                    }
                    else
                    {
                        if (eat_byte == false)
                        {
                            this->read_line.push_back(read.at(x));
                        }
                    }
                }
            }
        }
        catch(...)
        {
            LOG_F(WARNING, "Device Disconnected: %s", this->serial_port.c_str());
            this->serial.close();
            this->serial_port = "";
            this->is_connected = false;
        }
    }
    else
    {
        this->is_connected = false;
        this->serial_port = "";
    }
    if (this->is_connected == false)
    {
        if ((Xrender_millis() - this->connection_retry_timer) > 1000)
        {
            this->connection_retry_timer = 0;
            if (this->serial_port == "")
            {
                std::vector<serial::PortInfo> devices_found = serial::list_ports();
                std::vector<serial::PortInfo>::iterator iter = devices_found.begin();
                while( iter != devices_found.end() )
                {
                    serial::PortInfo device = *iter++;
                    if (this->logged_devices_once == false) LOG_F(INFO, "Found Device-> %s - %s", device.port.c_str(), device.description.c_str());
                    std::transform(device.description.begin(), device.description.end(), device.description.begin(),[](unsigned char c){ return std::tolower(c); });
                    std::transform(connect_description.begin(), connect_description.end(), connect_description.begin(),[](unsigned char c){ return std::tolower(c); });
                    if (device.description.find(connect_description) != std::string::npos)
                    {
                        this->serial_port = device.port.c_str();
                    }
                    this->logged_devices_once = true;
                }
            }
            if (this->serial_port != "")
            {
                try
                {
                    this->serial.setPort(this->serial_port.c_str());
                    this->serial.setBaudrate(this->baudrate);
                    auto timeout = serial::Timeout::simpleTimeout(2000);
                    this->serial.setTimeout(timeout);
                    this->serial.open();
                    if (this->serial.isOpen())
                    {
                        this->is_connected = true;
                        this->serial.setDTR(true);
                        this->delay(100);
                        this->serial.setDTR(false);
                        LOG_F(INFO,"Opened port: %s at %d baudrate", this->serial_port.c_str(), this->baudrate);
                    }
                    else
                    {
                        LOG_F(ERROR, "Could not open port! %s", this->serial_port.c_str());
                        this->is_connected = false;
                    }
                }
                catch (...)
                {
                    this->is_connected = false;
                    this->serial_port = "";
                }
            }
        }
    }
}