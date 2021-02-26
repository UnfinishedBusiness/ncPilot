#include <Xrender.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <json/json.h>
#include "gcode/gcode.h"
#include "logging/loguru.h"
#include "geometry/geometry.h"
#include "hmi/hmi.h"
#include "dialogs/dialogs.h"
#include "application.h"



gcode_global_t gcode;
gcode_path_t current_path;
std::vector<gcode_path_t> paths;

std::vector<std::string> gcode_split(std::string str, char delimiter)
{ 
    vector<string> internal; 
    stringstream ss(str); // Turn the string into a stream. 
    string tok; 
    while(getline(ss, tok, delimiter))
    { 
        internal.push_back(tok); 
    } 
    return internal; 
}
std::string gcode_get_filename()
{
    return gcode.filename;
}
unsigned long count_lines(std::string file)
{
    std::string line;
    unsigned long count = 0;
    std::ifstream in(file);
    while(std::getline(in, line))
    {
        count++;
    }
    return count;
}

bool gcode_open_file(std::string file)
{
    gcode.file.open(file);
    if (gcode.file.is_open())
    {
        std::vector<Xrender_object_t*> *stack = Xrender_get_object_stack();
        for (int x = 0; x < stack->size(); x++)
        {
            if (stack->at(x)->data["id"] == "gcode" || stack->at(x)->data["id"] == "gcode_arrows" || stack->at(x)->data["id"] == "gcode_highlight")
            {
                stack->erase(stack->begin() + x);
                x = 0; //Restart at top because erase will chage count
            }
        }
        gcode.line_count = count_lines(file);
        gcode.lines_consumed = 0;
        gcode.filename = file;
        LOG_F(INFO, "Opened file: %s, size: %lu lines", file.c_str(), gcode.line_count);
        dialogs_show_progress_window(true);
        dialogs_set_progress_value(0.0f);
        return true;
    }
    else
    {
        LOG_F(ERROR, "Could not open file: %s", file.c_str());
        return false;
    }
}
nlohmann::json parse_line(std::string line)
{
    nlohmann::json ret;
    std::transform(line.begin(), line.end(),line.begin(), ::toupper);
    std::vector<std::string> g = gcode_split(line, ' ');
    for (int x = 0; x < g.size(); x++)
    {
        if (g[x][0] == 'X')
        {
            g[x].erase(g[x].begin());
            ret["x"] = atof(string(g[x]).c_str());
        }
        else if (g[x][0] == 'Y')
        {
            g[x].erase(g[x].begin());
            ret["y"] = atof(string(g[x]).c_str());
        }
    }
    return ret;
}
void gcode_push_current_path_to_viewer(int rapid_line)
{
    if (current_path.points.size() > 0)
    {
        Geometry geo;
        try
        {
            std::vector<double_point_t> simplified = geo.simplify(current_path.points, 0.010);
            nlohmann::json path;
            int point_count = 0;
            for (int i = 0; i < simplified.size(); i++)
            {
                if ((i == 0 && i+1 < simplified.size()) || (point_count == 0 && i+1 < simplified.size()))
                {
                    nlohmann::json arrow_path;
                    double_point_t midpoint = geo.midpoint(simplified[i+1], simplified[i]);
                    arrow_path.push_back({{"x", midpoint.x}, {"y", midpoint.y}});
                    double angle = geo.measure_polar_angle(simplified[i + 1], simplified[i]);
                    double_point_t p1 = geo.create_polar_line(midpoint, angle +30, 0.020).end;
                    double_point_t p2 = geo.create_polar_line(midpoint, angle -30, 0.020).end;
                    arrow_path.push_back({{"x", p1.x}, {"y", p1.y}});
                    arrow_path.push_back({{"x", p2.x}, {"y", p2.y}});
                    Xrender_object_t *direction_indicator = Xrender_push_path({{"id", "gcode_arrows"}, {"points", arrow_path},{"color", {{"r", 0},{"g", 0},{"b", 255},{"a", 255},}},});
                    direction_indicator->matrix_data = &view_matrix;
                }
                path.push_back({{"x", simplified[i].x}, {"y", simplified[i].y}});
                point_count++;
                if (point_count > 5)
                {
                    point_count = 0;
                }
            }
            Xrender_object_t *o = Xrender_push_path({{"id", "gcode"}, {"rapid_line", rapid_line}, {"points", path},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
            o->matrix_data = &view_matrix;
            o->mouse_callback = &hmi_mouse_callback;
        }
        catch(const std::exception& e)
        {
            LOG_F(ERROR, "%s", e.what());
        }
    }
}
bool gcode_parse_timer()
{
    std::string line;
    for (int x = 0; x < 1000; x++)
    {
        if (std::getline(gcode.file, line))
        {
            gcode.lines_consumed++;
            dialogs_set_progress_value((float)gcode.lines_consumed / (float)gcode.line_count);
            if (line.find("G0") != std::string::npos)
            {
                double_point_t last_path_endpoint = {-1000000, -1000000};
                if (current_path.points.size() > 0)
                {
                    last_path_endpoint = current_path.points[current_path.points.size()-1];
                }
                gcode_push_current_path_to_viewer(gcode.last_rapid_line);
                if (current_path.points.size() > 0) paths.push_back(current_path);
                current_path.points.clear();
                nlohmann::json g = parse_line(line);
                try
                {
                    current_path.points.push_back({ (double)g["x"], (double)g["y"]});
                    if (last_path_endpoint.x != -1000000 && last_path_endpoint.y != -1000000)
                    {
                        Xrender_object_t *o = Xrender_push_line({{"id", "gcode"}, {"style", "dashed"},{"start", {{"x", last_path_endpoint.x}, {"y", last_path_endpoint.y}}}, {"end", {{"x", (double)g["x"]}, {"y", (double)g["y"]}}},{"color", {{"r", 100},{"g", 100},{"b", 100},{"a", 255},}},});
                        o->matrix_data = &view_matrix;
                    }
                }
                catch(...)
                {
                    LOG_F(ERROR, "Gcode parsing error at line %lu in file %s", gcode.lines_consumed, gcode.filename.c_str());
                }
                gcode.last_rapid_line = gcode.lines_consumed -1;
            }
            else if (line.find("G1") != std::string::npos)
            {
                nlohmann::json g = parse_line(line);
                try
                {
                    current_path.points.push_back({ (double)g["x"], (double)g["y"]});
                }
                catch(...)
                {
                    LOG_F(ERROR, "Gcode parsing error at line %lu in file %s", gcode.lines_consumed, gcode.filename.c_str());
                }
            }

        }
        else
        {
            LOG_F(INFO, "Reached end of file!");
            gcode_push_current_path_to_viewer(gcode.last_rapid_line);
            if (current_path.points.size() > 0) paths.push_back(current_path);
            current_path.points.clear();
            gcode.file.close();
            dialogs_set_progress_value(1.0f);
            dialogs_show_progress_window(false);
            return false;
        }
    }
    //LOG_F(INFO, "Progress: %.4f", (float)(((float)gcode.lines_consumed / (float)gcode.line_count) * 100.0f));
    return true;
}