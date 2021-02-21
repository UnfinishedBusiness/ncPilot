#ifndef MOTION_CONTROL_
#define MOTION_CONTROL_

#include <json/json.h>

using namespace std;

/*
    Callbacks that get called via okay_callback
*/
void torch_off_and_retract();
void fire_torch_and_pierce();
void run_pop();
/*      End callbacks       */

/*      Private Functions       */
bool byte_handler(uint8_t);
void line_handler(std::string);
/*      End Private Functions       */

nlohmann::json motion_controller_get_dro();
nlohmann::json motion_controller_get_run_time();
void motion_controller_cmd(std::string cmd);
bool motion_control_update_firmware();
void motion_controller_clear_stack();
void motion_controller_push_stack(std::string gcode);
void motion_controller_save_machine_parameters();
void motion_controller_run_stack();
void motion_controller_send(std::string s);
void motion_controller_send_crc32(std::string s);
void motion_controller_trigger_reset();
void motion_control_init();
void motion_control_tick();
bool motion_controller_is_torch_on();

void removeSubstrs(std::string& s, std::string p);

#endif //MOTION_CONTROL_