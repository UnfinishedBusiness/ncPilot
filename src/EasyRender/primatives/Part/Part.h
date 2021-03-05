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
            std::vector<double_point_t> built_points;
            bool is_closed;
            bool is_inside_contour;
            float color[4];
        };
        struct part_control_data_t{
            double_point_t offset;
            double scale;
            double smoothing;
            double angle;
            int mouse_mode;

            bool operator==(const part_control_data_t& a) const
            {
                return (offset.x == a.offset.x && offset.y == a.offset.y && scale == a.scale && smoothing == a.smoothing && angle == a.angle);
            }
        };
        nlohmann::json mouse_event;
        PrimativeProperties *properties;

        std::vector<path_t> paths;
        float width;
        std::string style; //solid, dashed
        std::string part_name;
        part_control_data_t control;
        part_control_data_t last_control;
        bool is_part_selected;


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
            this->control.mouse_mode = 0; //0 is paths, 1 is parts
            this->last_control.offset = {-1, -1};
            this->last_control.scale = 1.0f;
            this->last_control.smoothing = 0.003f;
            this->last_control.angle = 0.0f;
            this->mouse_event = NULL;
            this->is_part_selected = false;
        }
        std::string get_type_name();
        void process_mouse(float mpos_x, float mpos_y);
        void render();
        void destroy();
        nlohmann::json serialize();

        void GetBoundingBox(double_point_t *bbox_min, double_point_t *bbox_max);
        bool CheckIfPointIsInsidePath(std::vector<double_point_t> path, double_point_t point);
        bool CheckIfPathIsInsidePath(std::vector<double_point_t> path1, std::vector<double_point_t> path2);

        double PerpendicularDistance(const double_point_t &pt, const double_point_t &lineStart, const double_point_t &lineEnd);
        void Simplify(const std::vector<double_point_t> &pointList, std::vector<double_point_t> &out, double epsilon);
};

#endif //PATH_