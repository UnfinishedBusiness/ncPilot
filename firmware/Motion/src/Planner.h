#ifndef PLANNER_H
#define PLANNER_H

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
#define LOOK_AHEAD_SLOTS 10

#define ACTION_RAPID 0
#define ACTION_FEED 1
/**********************
 *      TYPEDEFS
 **********************/
 typedef struct
 {
   char json[1024];
 }action_t;
 typedef struct
 {
   char json[1024];
 }planned_action_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void Planner_Init();
int Planner_FillActionBuffer(int);
void Planner_PlanActionBuffer();
void Planner_SegmentizeActionsInBuffer();
/**********************
 * CONTROLS PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*PLANNER_H*/
