#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <string>
#include <utility>
#include <stdexcept>

#include <algorithm>
#include "../json/json.h"
#include "clipper.h"
#include "geometry.h"

using namespace std;

bool Geometry::between(double x, double min, double max)
{
    return x >= min && x <= max;
}
bool Geometry::points_match(double_point_t p1, double_point_t p2)
{
    float tolorance = 0.001;
	if (this->between(p1.x, p2.x - tolorance, p2.x + tolorance) && this->between(p1.y, p2.y - tolorance, p2.y + tolorance))
	{
		return true;
	}
	else
	{
		return false;
	}
}
nlohmann::json Geometry::arc_to_line_segments(double cx, double cy, double r, double start_angle, double end_angle, double num_segments)
{
    vector<Point> pointList;
	vector<Point> pointListOut; //List after simplification
    double_point_t start;
    double_point_t sweeper;
    double_point_t end;
    start.x = cx + (r * cosf((start_angle) * 3.1415926f / 180.0f));
	start.y = cy + (r * sinf((start_angle) * 3.1415926f / 180.0f));
    end.x = cx + (r * cosf((end_angle) * 3.1415926f / 180.0f));
	end.y = cy + (r * sinf((end_angle) * 3.1415926 / 180.0f));
    pointList.push_back(Point(start.x, start.y));
    double diff = MAX(start_angle, end_angle) - MIN(start_angle, end_angle);
	if (diff > 180) diff = 360 - diff;
	double angle_increment = diff / num_segments;
	double angle_pointer = start_angle + angle_increment;
    for (int i = 0; i < num_segments; i++)
	{
		sweeper.x = cx + (r * cosf((angle_pointer) * 3.1415926f / 180.0f));
		sweeper.y = cy + (r * sinf((angle_pointer) * 3.1415926f / 180.0f));
		angle_pointer += angle_increment;
        pointList.push_back(Point(sweeper.x, sweeper.y));
	}
    pointList.push_back(Point(end.x, end.y));
    this->RamerDouglasPeucker(pointList, 0.0005, pointListOut);
    nlohmann::json geometry_stack;
    nlohmann::json line;
    for(size_t i=1; i< pointListOut.size(); i++)
	{
        line["type"] = "line";
        line["start"]["x"] = pointListOut[i-1].first;
        line["start"]["y"] = pointListOut[i-1].second;
        line["end"]["x"] = pointListOut[i].first;
        line["end"]["y"] = pointListOut[i].second;
        geometry_stack.push_back(line);
	}
    return geometry_stack;
}
nlohmann::json Geometry::circle_to_line_segments(double cx, double cy, double r)
{
    vector<Point> pointList;
	vector<Point> pointListOut; //List after simplification
    for(int i = 0; i < 360; i++)
    {
        double theta = 2.0f * 3.1415926f * i / 360.0f;//get the current angle
        double tx = r * cosf(theta);//calculate the x component
        double ty = r * sinf(theta);//calculate the y component
        pointList.push_back(Point((tx + cx), (ty + cy)));
    }
    this->RamerDouglasPeucker(pointList, 0.0005, pointListOut);
    nlohmann::json geometry_stack;
    nlohmann::json line;
    for(size_t i=1; i< pointListOut.size(); i++)
	{
        line["type"] = "line";
        line["start"]["x"] = pointListOut[i-1].first;
        line["start"]["y"] = pointListOut[i-1].second;
        line["end"]["x"] = pointListOut[i].first;
        line["end"]["y"] = pointListOut[i].second;
        geometry_stack.push_back(line);
	}
    line["type"] = "line";
    line["start"]["x"] = pointListOut[pointListOut.size() -1].first;
    line["start"]["y"] = pointListOut[pointListOut.size() -1].second;
    line["end"]["x"] = pointListOut[0].first;
    line["end"]["y"] = pointListOut[0].second;
    geometry_stack.push_back(line);
    return geometry_stack;
}
nlohmann::json Geometry::normalize(nlohmann::json geometry_stack)
{
    nlohmann::json return_stack;
    for (nlohmann::json::iterator it = geometry_stack.begin(); it != geometry_stack.end(); ++it)
    {
        //std::cout << it.key() << " : " << it.value() << "\n";
        if (it.value()["type"] == "line")
        {
            return_stack.push_back(it.value());
        }
        else if (it.value()["type"] == "arc")
        {
            nlohmann::json line_segments = this->arc_to_line_segments(it.value()["center"]["x"], it.value()["center"]["y"], it.value()["radius"], it.value()["start_angle"], it.value()["end_angle"], 100);
            for (nlohmann::json::iterator sub_it = line_segments.begin(); sub_it != line_segments.end(); ++sub_it) return_stack.push_back(sub_it.value());
        }
        else if (it.value()["type"] == "circle")
        {
            nlohmann::json line_segments = this->circle_to_line_segments(it.value()["center"]["x"], it.value()["center"]["y"], it.value()["radius"]);
            for (nlohmann::json::iterator sub_it = line_segments.begin(); sub_it != line_segments.end(); ++sub_it) return_stack.push_back(sub_it.value());
        }
    }
    return return_stack;
}
nlohmann::json Geometry::get_extents(nlohmann::json geometry_stack)
{
    nlohmann::json extents;
    double_point_t min;
    min.x = 1000000000000;
    min.y = 1000000000000;
    double_point_t max;
    max.x = -1000000000000;
    max.y = -1000000000000;
    for (nlohmann::json::iterator it = geometry_stack.begin(); it != geometry_stack.end(); ++it)
    {
        if (it.value()["type"] == "line")
        {
            if (it.value()["start"]["x"] < min.x) min.x = it.value()["start"]["x"];
			if (it.value()["start"]["y"] < min.y) min.y = it.value()["start"]["y"];
            if (it.value()["end"]["x"] < min.x) min.x = it.value()["end"]["x"];
			if (it.value()["end"]["y"] < min.y) min.y = it.value()["end"]["y"];
            if (it.value()["start"]["x"] > max.x) max.x = it.value()["start"]["x"];
			if (it.value()["start"]["y"] > max.y) max.y = it.value()["start"]["y"];
            if (it.value()["end"]["x"] > max.x) max.x = it.value()["end"]["x"];
			if (it.value()["end"]["y"] > max.y) max.y = it.value()["end"]["y"];
        }
        else if (it.value()["type"] == "arc" || it.value()["type"] == "circle")
        {
            if (double(it.value()["center"]["x"]) - double(it.value()["radius"]) < min.x) min.x = double(it.value()["center"]["x"]) - double(it.value()["radius"]);
			if (double(it.value()["center"]["y"]) - double(it.value()["radius"]) < min.y) min.y = double(it.value()["center"]["y"]) - double(it.value()["radius"]);
			if (double(it.value()["center"]["x"]) + double(it.value()["radius"]) > max.x) max.x = double(it.value()["center"]["x"]) + double(it.value()["radius"]);
			if (double(it.value()["center"]["y"]) + double(it.value()["radius"]) > max.y) max.y = double(it.value()["center"]["x"]) + double(it.value()["radius"]);
        }
    }
    extents["min"]["x"] = min.x;
    extents["min"]["y"] = min.y;
    extents["max"]["x"] = max.x;
    extents["max"]["y"] = max.y;
    return extents;
}
double_point_t Geometry::rotate_point(double_point_t center, double_point_t point, double angle)
{
    double_point_t return_point;
    double radians = (3.1415926f / 180.0f) * angle;
	double cos = cosf(radians);
	double sin = sinf(radians);
	return_point.x = (cos * (point.x - center.x)) + (sin * (point.y - center.y)) + center.x;
	return_point.y = (cos * (point.y - center.y)) - (sin * (point.x - center.x)) + center.y;
    return return_point;
}
double_point_t Geometry::mirror_point(double_point_t point, double_line_t line)
{
    double dx, dy, a, b, x, y;
    double_point_t p, p0, p1;
    p = point;
    p0 = line.start;
    p1 = line.end;

    dx = p1.x - p0.x;
    dy = p1.y - p0.y;
    a = (dx * dx - dy * dy) / (dx * dx + dy * dy);
    b = 2 * dx * dy / (dx * dx + dy * dy);
    x = a * (p.x - p0.x) + b * (p.y - p0.y) + p0.x;
    y = b * (p.x - p0.x) - a * (p.y - p0.y) + p0.y;
    return { x, y };
}
nlohmann::json Geometry::chainify(nlohmann::json geometry_stack, double tolorance)
{
    nlohmann::json contours;
    std::vector<double_line_t> haystack;
    nlohmann::json point;
    //Make a copy of geometry stack so we're not modifying it during the chaining process!
    for (nlohmann::json::iterator it = geometry_stack.begin(); it != geometry_stack.end(); ++it)
    {
        if (it.value()["type"] == "line")
        {
            double_line_t l;
            l.start.x = it.value()["start"]["x"];
            l.start.y = it.value()["start"]["y"];
            l.end.x = it.value()["end"]["x"];
            l.end.y = it.value()["end"]["y"];
            haystack.push_back(l);
        }
    }
    while(haystack.size() > 0)
    {
        nlohmann::json chain;
        double_point_t p1;
        double_point_t p2;
        double_line_t first = haystack.back();
        haystack.pop_back();
        point["x"] = first.start.x;
        point["y"] = first.start.y;
        chain.push_back(point);
        point["x"] = first.end.x;
        point["y"] = first.end.y;
        chain.push_back(point);
        bool didSomething;
        do{
            didSomething = false;
            nlohmann::json current_point = chain.back();
            p1.x = current_point["x"];
            p1.y = current_point["y"];
            for (int x = 0; x < haystack.size(); x++)
            {
                p2.x = haystack[x].start.x;
                p2.y = haystack[x].start.y;
                if (this->distance(p1, p2) < tolorance)
                {
                    point["x"] = haystack[x].end.x;
                    point["y"] = haystack[x].end.y;
                    chain.push_back(point);
                    haystack.erase(haystack.begin()+x);
                    didSomething = true;
                    break;
                }
                p2.x = haystack[x].end.x;
                p2.y = haystack[x].end.y;
                if (this->distance(p1, p2) < tolorance)
                {
                    point["x"] = haystack[x].start.x;
                    point["y"] = haystack[x].start.y;
                    chain.push_back(point);
                    haystack.erase(haystack.begin()+x);
                    didSomething = true;
                    break;
                }
            }
        } while(didSomething);
        if (chain.size() != 2)
        {
            contours.push_back(chain);
        }
    }
    return contours;
}
nlohmann::json Geometry::offset(nlohmann::json path, double offset)
{
    /*
        Only supports a stack of {x: xxxx, y: xxxx} points
    */
    double scale = 100.0f;
    vector<Point> pointList;
	vector<Point> pointListOut; //List after simplification
    for (nlohmann::json::iterator it = path.begin(); it != path.end(); ++it)
    {
        pointList.push_back(Point(((double)it.value()["x"]) * scale, ((double)it.value()["y"]) * scale));
    }
    this->RamerDouglasPeucker(pointList, 0.001 * scale, pointListOut);

    
    nlohmann::json ret;
    ClipperLib::Path subj;
    ClipperLib::Paths solution;
    //printf("Path: %s, offset: %.4f\n", path.dump().c_str(), offset);
    for (int x = 0; x < pointListOut.size(); x++)
    {
        subj << ClipperLib::FPoint((pointListOut[x].first), (pointListOut[x].second));
    }
    //ClipperLib::CleanPolygon(subj, 0.01 * scale);
    ClipperLib::ClipperOffset co;
    co.AddPath(subj, ClipperLib::jtRound, ClipperLib::etClosedPolygon);
    co.Execute(solution, offset * scale);
    ClipperLib::CleanPolygons(solution, 0.001 * scale);
    for (int x = 0; x < solution.size(); x++)
    {
        //printf("Solution - %d\n", x);
        nlohmann::json path;
        double_point_t first_point;
        for (int y = 0; y < solution[x].size(); y++)
        {
            if (y == 0)
            {
                first_point.x = double(solution[x][y].X) / scale;
                first_point.y = double(solution[x][y].Y) / scale;
            }
            //printf("\t x: %.4f, y: %.4f\n", (float)(solution[x][y].X / 1000.0f), (float)(solution[x][y].Y / 1000.0f));
            nlohmann::json point;
            point["x"] = double(solution[x][y].X) / scale;
            point["y"] = double(solution[x][y].Y) / scale;
            path.push_back(point);
        }
        nlohmann::json point;
        point["x"] = first_point.x;
        point["y"] = first_point.y;
        path.push_back(point);
        ret.push_back(path);
    }
    return ret;
}
std::vector<double_point_t> Geometry::simplify(const std::vector<double_point_t> points, double smoothing)
{
    double scale = 100.0f;
    vector<Point> pointList;
	vector<Point> pointListOut; //List after simplification
    vector<double_point_t> out;
    for (int x = 0; x < points.size(); x++)
    {
        pointList.push_back(Point(points[x].x * scale, points[x].y * scale));
    }
    this->RamerDouglasPeucker(pointList, smoothing * scale, pointListOut);
    for (int x = 0; x < pointListOut.size(); x++)
    {
        out.push_back({ (double)pointListOut[x].first / scale, (double)pointListOut[x].second / scale});
    }
    return out;
}
nlohmann::json Geometry::slot(nlohmann::json path, double offset)
{
    /*
        Only supports a stack of {x: xxxx, y: xxxx} points
    */
    double scale = 100.0f;
    vector<Point> pointList;
	vector<Point> pointListOut; //List after simplification
    for (nlohmann::json::iterator it = path.begin(); it != path.end(); ++it)
    {
        pointList.push_back(Point(((double)it.value()["x"]) * scale, ((double)it.value()["y"]) * scale));
    }
    this->RamerDouglasPeucker(pointList, 0.001 * scale, pointListOut);

    
    nlohmann::json ret;
    ClipperLib::Path subj;
    ClipperLib::Paths solution;
    //printf("Path: %s, offset: %.4f\n", path.dump().c_str(), offset);
    for (int x = 0; x < pointListOut.size(); x++)
    {
        subj << ClipperLib::FPoint((pointListOut[x].first), (pointListOut[x].second));
    }
    //ClipperLib::CleanPolygon(subj, 0.01 * scale);
    ClipperLib::ClipperOffset co;
    co.AddPath(subj, ClipperLib::jtRound, ClipperLib::etOpenRound);
    co.Execute(solution, offset * scale);
    ClipperLib::CleanPolygons(solution, 0.001 * scale);
    for (int x = 0; x < solution.size(); x++)
    {
        //printf("Solution - %d\n", x);
        nlohmann::json path;
        double_point_t first_point;
        for (int y = 0; y < solution[x].size(); y++)
        {
            if (y == 0)
            {
                first_point.x = double(solution[x][y].X) / scale;
                first_point.y = double(solution[x][y].Y) / scale;
            }
            //printf("\t x: %.4f, y: %.4f\n", (float)(solution[x][y].X / 1000.0f), (float)(solution[x][y].Y / 1000.0f));
            nlohmann::json point;
            point["x"] = double(solution[x][y].X) / scale;
            point["y"] = double(solution[x][y].Y) / scale;
            path.push_back(point);
        }
        nlohmann::json point;
        point["x"] = first_point.x;
        point["y"] = first_point.y;
        path.push_back(point);
        ret.push_back(path);
    }
    return ret;
}
double Geometry::PerpendicularDistance(const Point &pt, const Point &lineStart, const Point &lineEnd)
{
	double dx = lineEnd.first - lineStart.first;
	double dy = lineEnd.second - lineStart.second;

	//Normalise
	double mag = pow(pow(dx,2.0)+pow(dy,2.0),0.5);
	if(mag > 0.0)
	{
		dx /= mag; dy /= mag;
	}

	double pvx = pt.first - lineStart.first;
	double pvy = pt.second - lineStart.second;

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
void Geometry::RamerDouglasPeucker(const vector<Point> &pointList, double epsilon, vector<Point> &out)
{
	if(pointList.size()<2)
		throw invalid_argument("Not enough points to simplify");

	// Find the point with the maximum distance from line between start and end
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

	// If max distance is greater than epsilon, recursively simplify
	if(dmax > epsilon)
	{
		// Recursive call
		vector<Point> recResults1;
		vector<Point> recResults2;
		vector<Point> firstLine(pointList.begin(), pointList.begin()+index+1);
		vector<Point> lastLine(pointList.begin()+index, pointList.end());
		RamerDouglasPeucker(firstLine, epsilon, recResults1);
		RamerDouglasPeucker(lastLine, epsilon, recResults2);
 
		// Build the result list
		out.assign(recResults1.begin(), recResults1.end()-1);
		out.insert(out.end(), recResults2.begin(), recResults2.end());
		if(out.size()<2)
			throw runtime_error("Problem assembling output");
	} 
	else 
	{
		//Just return start and end points
		out.clear();
		out.push_back(pointList[0]);
		out.push_back(pointList[end]);
	}
}
double Geometry::distance(double_point_t p1, double_point_t p2)
{
    double x = p1.x - p2.x;
    double y = p1.y - p2.y;
    return sqrtf(x*x + y*y);
}
double_point_t Geometry::midpoint(double_point_t p1, double_point_t p2)
{
    double_point_t mid;
    mid.x = (p1.x+p2.x)/2;
    mid.y = (p1.y+p2.y)/2;
    return mid;
}
double Geometry::measure_polar_angle(double_point_t p1, double_point_t p2)
{
    double angle = (180 / 3.1415926f) * (atan2f(p1.y - p2.y, p1.x - p2.x));
	angle += 180;
	if (angle >= 360) angle -= 360;
	return angle;
}
double Geometry::measure_arc_circumference(double start_angle, double end_angle, double radius)
{
    double angle = MAX(start_angle, end_angle) - MIN(start_angle, end_angle);
    double circle_circumference = 2 * 3.1415926f * radius;
    return (angle / 360.0f) * circle_circumference;
}
double_line_t Geometry::create_polar_line(double_point_t start_point, double angle, double length)
{
    double_line_t ret;
    ret.start.x = start_point.x;
    ret.start.y = start_point.y;
    ret.end.x = start_point.x + length;
    ret.end.y = start_point.y;
    double_point_t rotated_endpoint = this->rotate_point(ret.start, ret.end, angle * -1);
    ret.end.x = rotated_endpoint.x;
    ret.end.y = rotated_endpoint.y;
    return ret;
}
double_point_t Geometry::three_point_circle_center(double_point_t p1, double_point_t p2, double_point_t p3)
{
    double_point_t center;
    double ax = (p1.x + p2.x) / 2;
    double ay = (p1.y + p2.y) / 2;
    double ux = (p1.y - p2.y);
    double uy = (p2.x - p1.x);
    double bx = (p2.x + p3.x) / 2;
    double by = (p2.y + p3.y) / 2;
    double vx = (p2.y - p3.y);
    double vy = (p3.x - p2.x);
    double dx = ax - bx;
    double dy = ay - by;
    double vu = vx * uy - vy * ux;
    if (vu == 0)
    {
        center.x = 0;
        center.y = 0;
        return center;
    }
    double g = (dx * uy - dy * ux) / vu;
    center.x = bx + g * vx;
    center.y = by + g * vy;
    return center;
}
bool Geometry::lines_intersect(double_line_t l1, double_line_t l2)
{
    // Store the values for fast access and easy
    // equations-to-code conversion
    double_point_t p1 = l1.start;
    double_point_t  p2 = l1.end;
    double_point_t p3 = l2.start;
    double_point_t  p4 = l2.end;

    float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
    float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

    float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    // If d is zero, there is no intersection
    if (d == 0) return false;

    // Get the x and y
    float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
    float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
    float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;

    // Check if the x and y coordinates are within both lines
    if ( x < min(x1, x2) || x > max(x1, x2) ||
    x < min(x3, x4) || x > max(x3, x4) ) return false;
    if ( y < min(y1, y2) || y > max(y1, y2) ||
    y < min(y3, y4) || y > max(y3, y4) ) return false;

    return true;
}
bool Geometry::line_intersects_with_circle(double_line_t l, double_point_t center, double radius)
{
    double r, cx, cy, ax, ay, bx, by, a,b, c, disc, t1, t2, sqrtdisc;
    cx = center.x;
    cy = center.y;
    ax = l.start.x;
    ay = l.start.y;
    bx = l.end.x;
    by = l.end.y;
    r = radius;

    ax -= cx;
    ay -= cy;
    bx -= cx;
    by -= cy;
    a = powf((bx - ax), 2) + powf((by - ay), 2);
    b = 2*(ax*(bx - ax) + ay*(by - ay));
    c = powf(ax, 2) + powf(ay, 2) - powf(r, 2);
    disc = powf(b, 2) - 4*a*c;
    if(disc <= 0) return false;
    sqrtdisc = sqrtf(disc);
    t1 = (-b + sqrtdisc)/(2*a);
    t2 = (-b - sqrtdisc)/(2*a);
    if((0 < t1 && t1 < 1) || (0 < t2 && t2 < 1)) return true;
    return false;
}
bool Geometry::point_is_inside_polygon(nlohmann::json polygon, nlohmann::json point)
{
    std::vector<double> polyX;
    std::vector<double> polyY;
    int polyCorners = 0;
    for (nlohmann::json::iterator it = polygon.begin(); it != polygon.end(); ++it)
    {
        polyX.push_back(double(it.value()["x"]));
        polyY.push_back(double(it.value()["y"]));
        polyCorners++;
    }
    double x = point["x"];
    double y = point["y"];
    return this->pointInPolygon(polyCorners, polyX, polyY, x, y);
}
bool Geometry::polygon_is_inside_polygon(nlohmann::json polygon1, nlohmann::json polygon2)
{
    nlohmann::json point;
    for (nlohmann::json::iterator it = polygon1.begin(); it != polygon1.end(); ++it)
    {
        point["x"] = double(it.value()["x"]);
        point["y"] = double(it.value()["y"]);
        if (point_is_inside_polygon(polygon2, point) == false) return false;
    }
    return true;
}
bool Geometry::pointInPolygon(int polyCorners, std::vector<double> polyX, std::vector<double> polyY, double x, double y)
{
  int i;
  int j = polyCorners-1;
  bool oddNodes=false;
  for (i=0; i<polyCorners; i++) {
    if (((polyY[i]< y && polyY[j]>=y)
    ||   (polyY[j]< y && polyY[i]>=y))
    &&  (polyX[i]<=x || polyX[j]<=x)) {
      oddNodes^=(polyX[i]+(y-polyY[i])/(polyY[j]-polyY[i])*(polyX[j]-polyX[i])<x); }
    j=i; }

  return oddNodes;
}