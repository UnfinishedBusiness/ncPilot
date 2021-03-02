#ifndef STK500_
#define STK500_

#include <stdio.h>
#include <iostream>
#include <vector>
#include <EasyRender/EasyRender.h> //For EasyRender::Millis()
#include "../serial/serial.h"
#include "hex/hex.h"

/* STK500 constants list, from AVRDUDE */
#define STK_OK                   0x10
#define STK_FAILED               0x11  // Not used
#define STK_UNKNOWN              0x12  // Not used
#define STK_NODEVICE             0x13  // Not used
#define STK_INSYNC               0x14  // ' '
#define STK_NOSYNC               0x15  // Not used
#define ADC_CHANNEL_ERROR        0x16  // Not used
#define ADC_MEASURE_OK           0x17  // Not used
#define PWM_CHANNEL_ERROR        0x18  // Not used
#define PWM_ADJUST_OK            0x19  // Not used
#define CRC_EOP                  0x20  // 'SPACE'
#define STK_GET_SYNC             0x30  // '0'
#define STK_GET_SIGN_ON          0x31  // '1'
#define STK_SET_PARAMETER        0x40  // '@'
#define STK_GET_PARAMETER        0x41  // 'A'
#define STK_SET_DEVICE           0x42  // 'B'
#define STK_SET_DEVICE_EXT       0x45  // 'E'
#define STK_ENTER_PROGMODE       0x50  // 'P'
#define STK_LEAVE_PROGMODE       0x51  // 'Q'
#define STK_CHIP_ERASE           0x52  // 'R'
#define STK_CHECK_AUTOINC        0x53  // 'S'
#define STK_LOAD_ADDRESS         0x55  // 'U'
#define STK_UNIVERSAL            0x56  // 'V'
#define STK_PROG_FLASH           0x60  // '`'
#define STK_PROG_DATA            0x61  // 'a'
#define STK_PROG_FUSE            0x62  // 'b'
#define STK_PROG_LOCK            0x63  // 'c'
#define STK_PROG_PAGE            0x64  // 'd'
#define STK_PROG_FUSE_EXT        0x65  // 'e'
#define STK_READ_FLASH           0x70  // 'p'
#define STK_READ_DATA            0x71  // 'q'
#define STK_READ_FUSE            0x72  // 'r'
#define STK_READ_LOCK            0x73  // 's'
#define STK_READ_PAGE            0x74  // 't'
#define STK_READ_SIGN            0x75  // 'u'
#define STK_READ_OSCCAL          0x76  // 'v'
#define STK_READ_FUSE_EXT        0x77  // 'w'
#define STK_READ_OSCCAL_EXT      0x78  // 'x'

#define PARAM_STK_HW_VER         0x80  // ' ' - R
#define PARAM_STK_SW_MAJOR       0x81  // ' ' - R
#define PARAM_STK_SW_MINOR       0x82  // ' ' - R
#define PARAM_STK_LEDS           0x83  // ' ' - R/W
#define PARAM_STK_VTARGET        0x84  // ' ' - R/W
#define PARAM_STK_VADJUST        0x85  // ' ' - R/W
#define PARAM_STK_OSC_PSCALE     0x86  // ' ' - R/W
#define PARAM_STK_OSC_CMATCH     0x87  // ' ' - R/W
#define PARAM_STK_RESET_DURATION 0x88  // ' ' - R/W
#define PARAM_STK_SCK_DURATION   0x89  // ' ' - R/
#define PARAM_STK_BUFSIZEL       0x90  // ' ' - R/W, Range {0..255}
#define PARAM_STK_BUFSIZEH       0x91  // ' ' - R/W, Range {0..255}
#define PARAM_STK_DEVICE         0x92  // ' ' - R/W, Range {0..255}
#define PARAM_STK_PROGMODE       0x93  // ' ' - 'P' or 'S'
#define PARAM_STK_PARAMODE       0x94  // ' ' - TRUE or FALSE
#define PARAM_STK_POLLING        0x95  // ' ' - TRUE or FALSE
#define PARAM_STK_SELFTIMED      0x96  // ' ' - TRUE or FALSE

#define STK_328P_DEVICE_CODE     0x86
#define STK_328P_DEVICE_REVISION 0x00
#define STK_328P_PROG_TYPE       0x00
#define STK_328P_PARALLELMODE    0x01
#define STK_328P_POLLING         0x01
#define STK_328P_SELF_TIMED      0x01
#define STK_328P_LOCK_BYTES      0x01
#define STK_328P_FUSE_BYTES      0x03
#define STK_328P_TIMEOUT         0xC8
#define STK_328P_STAB_DELAY      0x64
#define STK_328P_CMD_EXEC_DELAY  0x19
#define STK_328P_SYNC_LOOPS      0x20
#define STK_328P_BYTE_DELAY      0x00
#define STK_328P_POLL_INDEX      0x03
#define STK_328P_POLL_VALUE      0x53

#define STK_MEMTYPE_FLASH        'F'

#define PAGE_SIZE_WORDS 64
#define BYTES_PER_WORD 2
#define BYTES_PER_FLASH_BLOCK (PAGE_SIZE_WORDS * BYTES_PER_WORD)

bool stk500_write_program(const char *intelhex, const char *port);

#endif //STK500_