#include "Part.h"
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

std::string EasyPrimative::Part::get_type_name()
{
    return "part";
}
void EasyPrimative::Part::process_mouse(float mpos_x, float mpos_y)
{
    if (this->properties->visable == true)
    {
        mpos_x = (mpos_x - this->properties->offset[0]) / this->properties->scale;
        mpos_y = (mpos_y - this->properties->offset[1]) / this->properties->scale;
        Geometry g;
        bool mouse_is_over_path = false;
        for (size_t x = 0; x < this->paths.size(); x++)
        {
            for (size_t i = 1; i < this->paths[x].points.size(); i++)
            {
                if (g.line_intersects_with_circle({{this->paths[x].points.at(i-1).x, this->paths[x].points.at(i-1).y}, {this->paths[x].points.at(i).x, this->paths[x].points.at(i).y}}, {mpos_x, mpos_y}, this->properties->mouse_over_padding / this->properties->scale))
                {
                    mouse_is_over_path = true;
                    break;
                }
            }
            if (this->paths[x].is_closed == true)
            {
                if (g.line_intersects_with_circle({{this->paths[x].points.at(0).x, this->paths[x].points.at(0).y}, {this->paths[x].points.at(this->paths[x].points.size() - 1).x, this->paths[x].points.at(this->paths[x].points.size() - 1).y}}, {mpos_x, mpos_y}, this->properties->mouse_over_padding / this->properties->scale))
                {
                    mouse_is_over_path = true;
                }
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
double EasyPrimative::Part::PerpendicularDistance(const double_point_t &pt, const double_point_t &lineStart, const double_point_t &lineEnd)
{
	double dx = lineEnd.x - lineStart.x;
	double dy = lineEnd.y - lineStart.y;

	//Normalise
	double mag = pow(pow(dx,2.0)+pow(dy,2.0),0.5);
	if(mag > 0.0)
	{
		dx /= mag; dy /= mag;
	}

	double pvx = pt.x - lineStart.x;
	double pvy = pt.y - lineStart.y;

	//Get dot product (project pv onto normalized direction)
	double pvdot = dx * pvx + dy * pvy;

	//Scale line direction vector
	double dsx = pvdot * dx;
	double dsy = pvdot * dy;

	//Subtract this from pv
	double ax = pvx - dsx;
	double ay = pvy - dsy;

	return pow(pow(ax,2.0)+pow(ay,2.0),0.5);
}
void EasyPrimative::Part::Simplify(const std::vector<double_point_t> &pointList, std::vector<double_point_t> &out, double epsilon)
{
	if(pointList.size() < 2)
    {
        throw "Not enough points to simplify";
    }
	double dmax = 0.0;
	size_t index = 0;
	size_t end = pointList.size()-1;
	for(size_t i = 1; i < end; i++)
	{
		double d = this->PerpendicularDistance(pointList[i], pointList[0], pointList[end]);
		if (d > dmax)
		{
			index = i;
			dmax = d;
		}
	}
	if(dmax > epsilon)
	{
		// Recursive call
		std::vector<double_point_t> recResults1;
		std::vector<double_point_t> recResults2;
		std::vector<double_point_t> firstLine(pointList.begin(), pointList.begin()+index+1);
		std::vector<double_point_t> lastLine(pointList.begin()+index, pointList.end());
		this->Simplify(firstLine, recResults1, epsilon);
		this->Simplify(lastLine, recResults2, epsilon);
		// Build the result list
		out.assign(recResults1.begin(), recResults1.end()-1);
		out.insert(out.end(), recResults2.begin(), recResults2.end());
		if(out.size() < 2)
        {
            throw "Problem assembling output";
        }
	} 
	else 
	{
		//Just return start and end points
		out.clear();
		out.push_back(pointList[0]);
		out.push_back(pointList[end]);
	}
}
void EasyPrimative::Part::render()
{
    glPushMatrix();
        glTranslatef(this->properties->offset[0], this->properties->offset[1], this->properties->offset[2]);
        glScalef(this->properties->scale, this->properties->scale, this->properties->scale);
        glLineWidth(this->width);
        for (std::vector<path_t>::iterator it = this->paths.begin(); it != this->paths.end(); ++it)
        {
            glColor4f(it->color[0] / 255, it->color[1] / 255, it->color[2] / 255, it->color[3] / 255);
            if (it->is_closed == true)
            {
                glBegin(GL_LINE_LOOP);
            }
            else
            {
                glBegin(GL_LINE_STRIP);
            }
            if (this->style == "dashed")
            {
                glPushAttrib(GL_ENABLE_BIT);
                glLineStipple(10, 0xAAAA);
                glEnable(GL_LINE_STIPPLE);
            }
            try
            {
                std::vector<double_point_t> simplified;
                this->Simplify(it->points, simplified, this->control.smoothing);
                for (int i = 0; i < simplified.size(); i++)
                {
                    glVertex3f((simplified[i].x + this->control.offset.x) * this->control.scale, (simplified[i].y + this->control.offset.y) * this->control.scale, (simplified[i].z + this->control.offset.z) * this->control.scale);
                }       
            }
            catch (std::exception& e)
            {
                LOG_F(ERROR, "(EasyPrimative::Part::render) Exception: %s, setting visability to false to avoid further exceptions!", e.what());
                this->properties->visable = false;
            }
            glEnd();
        }
        glLineWidth(1);
        glDisable(GL_LINE_STIPPLE);
    glPopMatrix();
}
void EasyPrimative::Part::destroy()
{
    delete this->properties;
}
nlohmann::json EasyPrimative::Part::serialize()
{
    nlohmann::json paths;
    for (std::vector<path_t>::iterator it = this->paths.begin(); it != this->paths.end(); ++it)
    {
        nlohmann::json path;
        path["is_closed"] = it->is_closed;
        for (int i = 0; i < it->points.size(); i++)
        {
            path["points"].push_back({
                {"x", it->points[i].x},
                {"y", it->points[i].y},
                {"z", it->points[i].z}
            });
        }
        paths.push_back(path);
    }
    nlohmann::json j;
    j["paths"] = paths;
    j["width"] = this->width;
    j["style"] = this->style;
    return j;
}