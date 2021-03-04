#ifndef GEOMETRY_H
#define GEOMETRY_H

/*********************
 *      INCLUDES
 *********************/
#include <stddef.h>
#include <string>
#include <utility>
#include "../json/json.h"
#include "clipper.h"
/*********************
 *      DEFINES
 *********************/
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
/**********************
 *      TYPEDEFS
 **********************/
typedef std::pair<double, double> Point;
struct double_point_t{
    double x = 0;
    double y = 0;
    double z = 0;
};
struct double_line_t{
    double_point_t start;
    double_point_t end;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/
class Geometry{
    public:
        /*
            Check if number is bewteen a given min/max range
        */
        bool between(double x, double min, double max);

        /*
            Check if both points match
        */
        bool points_match(double_point_t p1, double_point_t p2);

        /*
            Output line segments from described arc
        */
        nlohmann::json arc_to_line_segments(double cx, double cy, double r, double start_angle, double end_angle, double num_segments);

        /*
            Output line segments from described circle
        */
        nlohmann::json circle_to_line_segments(double cx, double cy, double r);

        /*
            Input a json stack of lines, arc, circles, etc and Output a stack with non-lines converted to line segments.
            ! Does not reguire stack to be vectorized !
        */
        nlohmann::json normalize(nlohmann::json geometry_stack);

        /*
            Input a json stack of lines, arcs, circles and return a json object
            min/max = {x: xxx, max: xxx};
            return {min, max};
        */
        nlohmann::json get_extents(nlohmann::json geometry_stack);

        /*
            Rotate a point around a specified point by angle in degrees
            Returns new point with rotation applied
        */
        double_point_t rotate_point(double_point_t center, double_point_t point, double angle);

        /*
            Mirror a point about a line
            Returns new point with mirror applied
        */
        double_point_t mirror_point(double_point_t point, double_line_t line);

        /*
            Chainify line segments
            Inputs a "normalized" (meaning only line segments) stack of entities and vectors them aka chaining
            Returns an array of contours
        */
        nlohmann::json chainify(nlohmann::json geometry_stack, double tolorance);

        /*
            Offset contour
            Inputs a list of points representing a closed contour and a distance to offset
                negative offsets offset inside and positive offsets offset to the outside
            Returns a list of points with offset applied
        */
        nlohmann::json offset(nlohmann::json path, double offset);

        /*
            Slot Path
            Inputs a list of points representing a closed contour and a distance to offset
                negative offsets offset inside and positive offsets offset to the outside
            Returns a list of points (polygon) with offset applied to path
        */
        nlohmann::json slot(nlohmann::json path, double offset);

        /*
            Simplify a vector of double_point_t
                returns the simplified vector
        */
        std::vector<double_point_t> simplify(const std::vector<double_point_t> points, double smoothing);

        /*
            Ramer Douglas Peucker Algorythm for simplifying points
        */
        void RamerDouglasPeucker(const std::vector<Point> &pointList, double epsilon, std::vector<Point> &out);

        /*
            Get distance between two points
        */
        double distance(double_point_t p1, double_point_t p2);

        /*
            Get Midpoint between two points
        */
        double_point_t midpoint(double_point_t p1, double_point_t p2);

        /*
            Create a polar line
        */
        double_line_t create_polar_line(double_point_t start_point, double angle, double length);

        /*
            Calculate circle center given three points to describe the circle
        */
        double_point_t three_point_circle_center(double_point_t p1, double_point_t p2, double_point_t p3);

        /*
            Return the polar angle between two lines
        */
        double measure_polar_angle(double_point_t p1, double_point_t p2);

        /*
            Return the circumference of an arc segment
        */
        double measure_arc_circumference(double start_angle, double end_angle, double radius);

        /*
            Check if two lines intersect
                returns bool
        */
        bool lines_intersect(double_line_t l1, double_line_t l2);

        /*
            Check is line intersects with circle
        */
       bool line_intersects_with_circle(double_line_t l, double_point_t center, double radius);

        /*
            Check if point lies inside polygon
        */
        bool point_is_inside_polygon(nlohmann::json polygon, nlohmann::json point);

        /*
            Check if polygon1 is completly inside polygon2
        */
        bool polygon_is_inside_polygon(nlohmann::json polygon1, nlohmann::json polygon2);
    private:
        /*
            Used by RamerDouglasPeucker method
        */
        double PerpendicularDistance(const Point &pt, const Point &lineStart, const Point &lineEnd);

        /*
            Check if a point is inside a polygon
                returns boolean
        */
        bool pointInPolygon(int polyCorners, std::vector<double> polyX, std::vector<double> polyY, double x, double y);
};

/**********************
 * VIEWER PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif