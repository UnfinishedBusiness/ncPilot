#include "Arc.h"
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

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

std::string Arc::get_type_name()
{
    return "arc";
}
void Arc::process_mouse(float mpos_x, float mpos_y)
{
    if (this->properties->visable == true)
    {
        mpos_x = (mpos_x - this->properties->offset[0]) / this->properties->scale;
        mpos_y = (mpos_y - this->properties->offset[1]) / this->properties->scale;
        Geometry g;
        if (g.distance(this->center, {mpos_x, mpos_y}) > (this->radius - 5) &&
        g.distance(this->center, {mpos_x, mpos_y}) < (this->radius + 5) &&
        g.lines_intersect({g.create_polar_line(this->center, this->start_angle, this->radius).end, g.create_polar_line(this->center, this->end_angle, this->radius).end}, {this->center, {mpos_x, mpos_y}}))
        {
            if (this->properties->mouse_over == false)
            {
                this->mouse_event = {
                    {"type", "mouse_in"},
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
                    {"type", "mouse_out"},
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
void Arc::render_arc(double cx, double cy, double radius, double start_angle, double end_angle)
{
    if ( (((MAX(start_angle, end_angle) - MIN(start_angle, end_angle)) / 360.0f) * (2 * 3.1415926f * radius)) > 8)
    {
        double num_segments = 50;
        double_point_t start;
        double_point_t sweeper;
        double_point_t end;
        double_point_t last_point;
        start.x = cx + (radius * cosf((start_angle) * 3.1415926f / 180.0f));
        start.y = cy + (radius * sinf((start_angle) * 3.1415926f / 180.0f));
        end.x = cx + (radius * cosf((end_angle) * 3.1415926f / 180.0f));
        end.y = cy + (radius * sinf((end_angle) * 3.1415926 / 180.0f));
        double diff = MAX(start_angle, end_angle) - MIN(start_angle, end_angle);
        if (diff > 180) diff = 360 - diff;
        double angle_increment = diff / num_segments;
        double angle_pointer = start_angle + angle_increment;
        last_point = start;
        for (int i = 0; i < num_segments; i++)
        {
            sweeper.x = cx + (radius * cosf((angle_pointer) * 3.1415926f / 180.0f));
            sweeper.y = cy + (radius * sinf((angle_pointer) * 3.1415926f / 180.0f));
            angle_pointer += angle_increment;
            glBegin(GL_LINES);
                glVertex3f(last_point.x, last_point.y, 0);
                glVertex3f(sweeper.x, sweeper.y, 0);
            glEnd();
            last_point = sweeper;
        }
        glBegin(GL_LINES);
            glVertex3f(last_point.x, last_point.y, 0);
            glVertex3f(end.x, end.y, 0);
        glEnd();
    }
    else
    {
        double_point_t start;
        double_point_t end;
        start.x = cx + (radius * cosf((start_angle) * 3.1415926f / 180.0f));
        start.y = cy + (radius * sinf((start_angle) * 3.1415926f / 180.0f));
        end.x = cx + (radius * cosf((end_angle) * 3.1415926f / 180.0f));
        end.y = cy + (radius * sinf((end_angle) * 3.1415926 / 180.0f));
        glBegin(GL_LINES);
            glVertex3f(start.x, start.y, 0);
            glVertex3f(end.x, end.y, 0);
        glEnd();
    }
}
void Arc::render()
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
            this->render_arc(this->center.x, this->center.y, this->radius, this->start_angle, this->end_angle);
            glLineWidth(1);
            glDisable(GL_LINE_STIPPLE);
        glPopMatrix();
    }
}
