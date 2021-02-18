#ifndef DIALOGS_
#define DIALOGS_

#include <Xrender.h>

void dialogs_show_machine_parameters(bool s);
void dialogs_show_preferences(bool s);
void dialogs_set_progress_value(float p);
void dialogs_show_progress_window(bool s);
void dialogs_show_info_window(bool s);
void dialogs_set_info_value(std::string i);
void dialogs_init();

#endif //DIALOGS_