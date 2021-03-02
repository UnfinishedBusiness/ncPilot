#include "stk500.h"

serial::Serial stk_serial;
HexFileClass hexFile;

std::vector<uint8_t> device_parameters;

std::vector<uint8_t> stk500_wait_response(uint8_t bytes)
{
    std::vector<uint8_t> r;
    for (int x = 0; x < bytes; x++)
    {
        r.push_back((uint8_t)stk_serial.read(1)[0]);
    }
    return r;
}

void stk500_write_bytes(std::vector<uint8_t> bytes)
{
    stk_serial.write(bytes);
}
void stk500_get_parameters()
{
    const uint8_t params_list[] = {
        PARAM_STK_HW_VER, //0
        PARAM_STK_SW_MAJOR, //1
        PARAM_STK_SW_MINOR, //2
        PARAM_STK_LEDS, //3
        PARAM_STK_VTARGET, //4
        PARAM_STK_VADJUST, //5
        PARAM_STK_OSC_PSCALE, //6
        PARAM_STK_OSC_CMATCH, //7
        PARAM_STK_RESET_DURATION, //8
        PARAM_STK_SCK_DURATION, //9
        PARAM_STK_BUFSIZEL, //10
        PARAM_STK_BUFSIZEH, //11
        PARAM_STK_DEVICE, //12
        PARAM_STK_PROGMODE, //13
        PARAM_STK_PARAMODE, //14
        PARAM_STK_POLLING, //15
        PARAM_STK_SELFTIMED //16
    };
    std::vector<uint8_t> r;
    for (int x = 0; x < 16; x++)
    {
        stk500_write_bytes({STK_GET_PARAMETER, params_list[x] ,CRC_EOP});
        EasyRender::Delay(50);
        r = stk500_wait_response(1);
        if (r[0] == STK_INSYNC)
        {
            r = stk500_wait_response(2);
            if (r[1] == STK_OK)
            {
                device_parameters.push_back(r[0]);
            }
            else
            {
                printf("Recieved STK_FAILED!\n");
                return;
            }
        }
        else
        {
            printf("Not in sync!\n");
            return;
        }
        r.clear();
    }
    printf("PARAM_STK_HW_VER : %d\n", (int)device_parameters[0]);
    printf("PARAM_STK_SW_MAJOR : %d\n", (int)device_parameters[1]);
    printf("PARAM_STK_BUFSIZEL : %d\n", (int)device_parameters[10]);
    printf("PARAM_STK_BUFSIZEH : %d\n", (int)device_parameters[11]);
    printf("PARAM_STK_DEVICE : %d\n", (int)device_parameters[12]);
}
/*void stk500_set_parameter(byte p)
{
    stk500_write_bytes({STK_SET_DEVICE, 
    STK_328P_DEVICE_CODE,
    STK_328P_DEVICE_REVISION,
    STK_328P_PROG_TYPE,
    STK_328P_PARALLELMODE,
    STK_328P_POLLING,
    STK_328P_SELF_TIMED,
    STK_328P_LOCK_BYTES,
    STK_328P_FUSE_BYTES,
    STK_328P_TIMEOUT,
    STK_328P_STAB_DELAY,
    STK_328P_CMD_EXEC_DELAY,
    STK_328P_SYNC_LOOPS,
    STK_328P_BYTE_DELAY,
    STK_328P_POLL_INDEX,
    STK_328P_POLL_VALUE,
    CRC_EOP});
}*/
void stk500_load_address(uint16_t target_address)
{
    stk500_write_bytes({STK_LOAD_ADDRESS, 
    (uint8_t)(target_address&0xFF),
    (uint8_t)((target_address>>8)&0xFF),
    CRC_EOP});
}
void stk500_send_program_page(flash_page_block_t &targBlock)
{
    std::vector<uint8_t> cmd;
    cmd.push_back(STK_PROG_PAGE);
    cmd.push_back((BYTES_PER_FLASH_BLOCK >> 8) & 0xFF);
    cmd.push_back(BYTES_PER_FLASH_BLOCK & 0xFF);
    cmd.push_back(STK_MEMTYPE_FLASH);
    for(uint16_t count = 0; count < BYTES_PER_FLASH_BLOCK; count++)
    {
        if(count < targBlock.block_size_bytes)
        {
            cmd.push_back(targBlock.dataBytes[count]);
        }
        else
        {
            //Pad the rest of the flash block with 0xFF's
            cmd.push_back(0xFF);
        }
    }
    cmd.push_back(CRC_EOP);
    stk500_write_bytes(cmd);
}
bool stk500_write_program(const char *intelhex, const char *port)
{
    //try
    //{
        hexFile.open(intelhex);
        auto timeout = serial::Timeout::simpleTimeout(5000);
        stk_serial.setTimeout(timeout);
        stk_serial.setPort(port);
        stk_serial.setBaudrate(115200);
        stk_serial.open();
        if (stk_serial.isOpen())
        {
            stk_serial.setDTR(true);
            stk_serial.setRTS(true);
            EasyRender::Delay(250);
            stk_serial.setDTR(false);
            stk_serial.setRTS(false);
            bool have_sync = false;
            for (int x = 0; x < 10; x++)
            {
                stk500_write_bytes({STK_GET_SYNC, CRC_EOP});
                std::vector<uint8_t> r = stk500_wait_response(2);
                if (r[0] == STK_INSYNC && r[1] == STK_OK)
                {
                    printf("Established sync!\n");
                    have_sync = true;
                    break;
                }
                EasyRender::Delay(250);
            }
            if (have_sync == true)
            {
                stk500_write_bytes({STK_READ_SIGN, CRC_EOP});
                std::vector<uint8_t> r = stk500_wait_response(1);
                if (r[0] == STK_INSYNC)
                {
                    std::vector<uint8_t> r = stk500_wait_response(4);
                    printf("Recieved signature: %02x-%02x-%02x\n", r[0], r[1], r[2]);
                    r.clear();
                    //stk500_get_parameters();
                    printf("Beginning firmware write!\n");
                    flash_page_block_t programBlock;
                    bool error = false;
                    unsigned long current_page_count = 0;
                    while(hexFile.moreBytesToConsume())
                    {
                        programBlock.block_size_bytes = hexFile.load_hex_records_flash_data_block(programBlock);
                        r.clear();
                        stk500_load_address(programBlock.addressStart);
                        r = stk500_wait_response(2);
                        if (r[0] != STK_INSYNC)
                        {
                            error = true;
                            printf("Could not load address: %hu\n", programBlock.addressStart);
                            break;
                        }
                        r.clear();
                        stk500_send_program_page(programBlock);
                        r = stk500_wait_response(2);
                        if (r[0] != STK_INSYNC)
                        {
                            error = true;
                            printf("Could not write page at address: %hu\n", programBlock.addressStart);
                            break;
                        }
                        printf("Wrote page %lu at address: %hu\n", current_page_count, programBlock.addressStart);
                        current_page_count++;
                    }
                    if (error == false)
                    {
                        r.clear();
                        stk500_write_bytes({STK_LEAVE_PROGMODE, CRC_EOP});
                        r = stk500_wait_response(2);
                        if(r[0] == STK_INSYNC && r[1] == STK_OK)
                        {
                            printf("Finished programming successfully!\n");
                            stk_serial.close();
                            hexFile.close();
                            return true;
                        }
                        else
                        {
                            printf("Something went wrong!\n");
                        }
                    }
                }
                else
                {
                    printf("Did not recieve signature!\n");
                    stk_serial.close();
                    hexFile.close();
                    return false;
                }
            }
            else
            {
                printf("Could not establish sync!\n");
                stk_serial.close();
                hexFile.close();
                return false;
            }
        }
        else
        {
            printf("Could not open %s\n", port);
            hexFile.close();
            return false;
        }
        stk_serial.close();
        hexFile.close();
        /*for (int x = 0; x < programPages.size(); x++)
        {
            std::cout << "Page: " << programPages[x] << std::endl;
        }*/
        return false;
    //}
    /*catch(...)
    {
        printf("Could not open %s\n", port);
        return false;
    }*/
}