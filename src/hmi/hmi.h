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

void hmi_init();

#endif