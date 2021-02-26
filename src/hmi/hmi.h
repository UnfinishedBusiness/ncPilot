#ifndef HMI_
#define HMI_

#include <application.h>


struct hmi_button_t{
    std::string name;
    EasyPrimative::Box *object;
    EasyPrimative::Text *label;
};
struct hmi_button_group_t{
    hmi_button_t button_one;
    hmi_button_t button_two;
};
struct dro_data_t{
    EasyPrimative::Text *label;
    EasyPrimative::Text *work_readout;
    EasyPrimative::Text *absolute_readout;
    EasyPrimative::Box *divider;
};
struct dro_group_data_t{
    dro_data_t x;
    dro_data_t y;
    dro_data_t z;
    EasyPrimative::Text *feed;
    EasyPrimative::Text *arc_readout;
    EasyPrimative::Text *arc_set;
    EasyPrimative::Text *run_time;
};

void hmi_init();
void hmi_handle_button(std::string id);
void hmi_mouse_callback(PrimativeContainer* c,nlohmann::json e);
void hmi_view_matrix(PrimativeContainer *p);


#endif