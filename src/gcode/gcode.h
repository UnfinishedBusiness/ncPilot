#ifndef GCODE__
#define GCODE__

#include <Xrender.h>
#include <json/json.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

struct gcode_path_t{
    std::vector<double_point_t> points;
};
struct gcode_global_t{
    std::ifstream file;
    std::string filename;
    unsigned long line_count;
    unsigned long lines_consumed;
};
bool gcode_open_file(std::string file);
std::string gcode_get_filename();
bool gcode_parse_timer();


#endif //GCODE__
