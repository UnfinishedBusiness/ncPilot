#include "Line.h"
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

std::string EasyPrimative::Line::get_type_name()
{
    return "line";
}
void EasyPrimative::Line::process_mouse(float mpos_x, float mpos_y)
{
    mpos_x = (mpos_x - this->properties->offset[0]) / this->properties->scale;
    mpos_y = (mpos_y - this->properties->offset[1]) / this->properties->scale;
    Geometry g;
    if (g.line_intersects_with_circle({{this->start.x, this->start.y}, {this->end.x, this->end.y}}, {mpos_x, mpos_y}, (this->properties->mouse_over_padding / this->properties->scale)))
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
void EasyPrimative::Line::render()
{
    if (this->properties->visable == true)
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
            glBegin(GL_LINES);
                glVertex3f(this->start.x, this->start.y, this->start.z);
                glVertex3f(this->end.x, this->end.y, this->end.z);
            glEnd();
            glLineWidth(1);
            glDisable(GL_LINE_STIPPLE);
        glPopMatrix();
    }
}
void EasyPrimative::Line::destroy()
{
    delete this->properties;
}
nlohmann::json EasyPrimative::Line::serialize()
{
    nlohmann::json j;
    j["start"]["x"] = this->start.x;
    j["start"]["y"] = this->start.y;
    j["start"]["z"] = this->start.z;
    j["end"]["x"] = this->end.x;
    j["end"]["y"] = this->end.y;
    j["end"]["z"] = this->end.z;
    j["width"] = this->width;
    j["style"] = this->style;
    return j;
}