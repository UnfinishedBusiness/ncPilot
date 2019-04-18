#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <Motion.h>
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
 typedef struct
 {
   uint8_t step_pin;
   uint8_t dir_pin;
   float scale;
   float max_accel;
   float max_velocity;
   char axis_letter;
 }config_axis_t;

 typedef struct
 {
   int number_of_axis;
   float max_linear_velocity;
   config_axis_t axis[MAX_NUMBER_OF_AXIS];
 }config_t;

extern config_t MachineConfig;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void Config_Init();
void Config_ParseINI();
void Config_DumpINI();
/**********************
 * CONTROLS PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*CONFIG_H*/
