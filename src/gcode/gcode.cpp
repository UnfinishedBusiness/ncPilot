#include <Xrender.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include "gcode/gcode.h"
#include "logging/loguru.h"
#include "geometry/geometry.h"
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
        gcode.line_count = count_lines(file);
        gcode.lines_consumed = 0;
        gcode.filename = file;
        LOG_F(INFO, "Opened file: %s, size: %lu lines", file.c_str(), gcode.line_count);
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
void gcode_push_current_path_to_viewer()
{
    if (current_path.points.size() > 0)
    {
        Geometry geo;
        try
        {
            std::vector<double_point_t> simplified = geo.simplify(current_path.points, 0.010);
            nlohmann::json path;
            for (int i = 0; i < simplified.size(); i++)
            {
                path.push_back({{"x", simplified[i].x}, {"y", simplified[i].y}});
            }
            Xrender_object_t *o = Xrender_push_path({
                {"points", path},
                {"color", {
                    {"r", 255},
                    {"g", 255},
                    {"b", 255},
                    {"a", 255},
                }},
            });
            o->matrix_data = &view_matrix;
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
    for (int x = 0; x < 100; x++)
    {
        if (std::getline(gcode.file, line))
        {
            gcode.lines_consumed++;
            if (line.find("G0") != std::string::npos)
            {
                gcode_push_current_path_to_viewer();
                if (current_path.points.size() > 0) paths.push_back(current_path);
                current_path.points.clear();
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
            gcode_push_current_path_to_viewer();
            if (current_path.points.size() > 0) paths.push_back(current_path);
            current_path.points.clear();
            gcode.file.close();
            return false;
        }
    }
    //LOG_F(INFO, "Progress: %.4f", (float)(((float)gcode.lines_consumed / (float)gcode.line_count) * 100.0f));
    return true;
}