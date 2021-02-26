#ifndef GCODE__
#define GCODE__

#include <application.h>

struct gcode_path_t{
    std::vector<double_point_t> points;
};
struct gcode_global_t{
    std::ifstream file;
    std::string filename;
    unsigned long line_count;
    unsigned long lines_consumed;
    unsigned long last_rapid_line;
};
bool gcode_open_file(std::string file);
std::string gcode_get_filename();
bool gcode_parse_timer();


#endif //GCODE__
