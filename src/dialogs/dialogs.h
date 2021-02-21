#ifndef DIALOGS_
#define DIALOGS_

#include <Xrender.h>

using namespace std;

void dialogs_show_machine_parameters(bool s);
void dialogs_show_preferences(bool s);
void dialogs_show_terminal_window(bool s);
void dialogs_set_progress_value(float p);
void dialogs_show_progress_window(bool s);
void dialogs_show_info_window(bool s);
void dialogs_set_info_value(std::string i);
void dialogs_show_thc_window(bool s);
void dialogs_init();

#endif //DIALOGS_