#ifndef APPLICATION_
#define APPLICATION_

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <dirent.h>
#include <ftw.h>
#include <EasyRender/EasyRender.h>
#include "ncControlView/ncControlView.h"
#include "jetCamView/jetCamView.h"



/*
    Primary structure to store global variables in
*/

class ncControlView;
class jetCamView;

struct global_variables_t{
    bool quit;
    double zoom;
    double_point_t pan;
    double_point_t mouse_pos_in_screen_coordinates;
    double_point_t mouse_pos_in_matrix_coordinates;
    unsigned long start_timestamp;
    EasyRender *renderer;
    /*  Define view classes here */
    ncControlView *nc_control_view;
    jetCamView *jet_cam_view;
};
extern global_variables_t *globals;

#endif //APPLICATION_