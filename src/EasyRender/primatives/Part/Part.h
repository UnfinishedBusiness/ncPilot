#ifndef PART_
#define PART_

#include "../PrimativeProperties.h"
#include "../../json/json.h"
#include "../../geometry/geometry.h"
#include <string>
#include <vector>

class EasyPrimative::Part{
    public:
        struct path_t{
            std::vector<double_point_t> points;
            bool is_closed;
            bool is_inside_contour;
            float color[4];
        };
        struct part_control_data_t{
            double_point_t offset;
            double scale;
            double smoothing;
            double angle;
        };
        nlohmann::json mouse_event;
        PrimativeProperties *properties;

        std::vector<path_t> paths;
        float width;
        std::string style; //solid, dashed
        std::string part_name;
        part_control_data_t control;


        Part(std::string name, std::vector<path_t> p)
        {
            this->properties = new PrimativeProperties();
            this->paths = p;
            this->width = 1;
            this->style = "solid";
            this->part_name = name;
            this->control.offset = {0, 0};
            this->control.scale = 1.0f;
            this->control.smoothing = 0.003f;
            this->control.angle = 0.0f;
            this->mouse_event = NULL;
        }
        std::string get_type_name();
        void process_mouse(float mpos_x, float mpos_y);
        void render();
        void destroy();
        nlohmann::json serialize();

        double PerpendicularDistance(const double_point_t &pt, const double_point_t &lineStart, const double_point_t &lineEnd);
        void Simplify(const std::vector<double_point_t> &pointList, std::vector<double_point_t> &out, double epsilon);
};

#endif //PATH_