#include "Path.h"
#include "../../geometry/geometry.h"
#include "../../logging/loguru.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #include <GL/freeglut.h>
   #include <GL/gl.h>
   #define GL_CLAMP_TO_EDGE 0x812F
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #include <OpenGL/glu.h>
#elif __linux__
    #include <GL/glu.h>
#elif __unix__
    #include <GL/glu.h>
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif

std::string EasyPrimative::Path::get_type_name()
{
    return "path";
}
void EasyPrimative::Path::process_mouse(float mpos_x, float mpos_y)
{
    if (this->properties->visable == true)
    {
        mpos_x = (mpos_x - this->properties->offset[0]) / this->properties->scale;
        mpos_y = (mpos_y - this->properties->offset[1]) / this->properties->scale;
        Geometry g;
        bool mouse_is_over_path = false;
        for (int i = 1; i < this->points.size(); i++)
        {
            if (g.line_intersects_with_circle({{this->points.at(i-1).x, this->points.at(i-1).y}, {this->points.at(i).x, this->points.at(i).y}}, {mpos_x, mpos_y}, this->properties->mouse_over_padding / this->properties->scale))
            {
                mouse_is_over_path = true;
                break;
            }
        }
        if (this->is_closed == true)
        {
            if (g.line_intersects_with_circle({{this->points.at(0).x, this->points.at(0).y}, {this->points.at(this->points.size() - 1).x, this->points.at(this->points.size() - 1).y}}, {mpos_x, mpos_y}, this->properties->mouse_over_padding / this->properties->scale))
            {
                mouse_is_over_path = true;
            }
        }
        if (mouse_is_over_path == true)
        {
            if (this->properties->mouse_over == false)
            {
                this->mouse_event = {
                    {"event", "mouse_in"},
                    {"pos", {
                        {"x", mpos_x},
                        {"y", mpos_y}
                    }},
                };
                this->properties->mouse_over = true;
            }       
        }
        else
        {
            if (this->properties->mouse_over == true)
            {
                this->mouse_event = {
                    {"event", "mouse_out"},
                    {"pos", {
                        {"x", mpos_x},
                        {"y", mpos_y}
                    }},
                };
                this->properties->mouse_over = false;
            }
        }
    }
}
void EasyPrimative::Path::render()
{
    glPushMatrix();
        glTranslatef(this->properties->offset[0], this->properties->offset[1], this->properties->offset[2]);
        glScalef(this->properties->scale, this->properties->scale, this->properties->scale);
        glColor4f(this->properties->color[0] / 255, this->properties->color[1] / 255, this->properties->color[2] / 255, this->properties->color[3] / 255);
        glLineWidth(this->width);
        if (this->style == "dashed")
        {
            glPushAttrib(GL_ENABLE_BIT);
            glLineStipple(10, 0xAAAA);
            glEnable(GL_LINE_STIPPLE);
        }
        if (this->is_closed == true)
        {
            glBegin(GL_LINE_LOOP);
        }
        else
        {
            glBegin(GL_LINE_STRIP);
        }
            for (int i = 0; i < this->points.size(); i++)
            {
                glVertex3f(this->points[i].x, this->points[i].y, this->points[i].z);
            }
        glEnd();
        glLineWidth(1);
        glDisable(GL_LINE_STIPPLE);
    glPopMatrix();
}
void EasyPrimative::Path::destroy()
{
    delete this->properties;
}
nlohmann::json EasyPrimative::Path::serialize()
{
    nlohmann::json p;
    for (int i = 0; i < this->points.size(); i++)
    {
        p.push_back({
            {"x", this->points[i].x},
            {"y", this->points[i].y},
            {"z", this->points[i].z}
        });
    }
    nlohmann::json j;
    j["points"] = p;
    j["is_closed"] = this->is_closed;
    j["width"] = this->width;
    j["style"] = this->style;
    return j;
}