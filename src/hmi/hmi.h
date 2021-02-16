#ifndef HMI_
#define HMI_
#include <Xrender.h>
#include <string>

struct hmi_button_t{
    std::string name;
    Xrender_object_t *object;
    Xrender_object_t *label;
};
struct hmi_button_group_t{
    hmi_button_t button_one;
    hmi_button_t button_two;
};
struct dro_data_t{
    Xrender_object_t *label;
    Xrender_object_t *work_readout;
    Xrender_object_t *absolute_readout;
    Xrender_object_t *divider;
};
struct dro_group_data_t{
    dro_data_t x;
    dro_data_t y;
    dro_data_t z;
    Xrender_object_t *feed;
    Xrender_object_t *arc_readout;
    Xrender_object_t *arc_set;
    Xrender_object_t *run_time;
};

void hmi_init();

#endif