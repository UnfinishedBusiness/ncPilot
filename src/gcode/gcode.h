#ifndef GCODE__
#define GCODE__

#include <Xrender.h>

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
bool gcode_parse_timer();


#endif //GCODE__
