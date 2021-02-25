#ifndef ARC_
#define ARC_

#include "../PrimativeProperties.h"
#include "../../json/json.h"
#include "../../geometry/geometry.h"
#include <string>

class Arc{
    public:
        nlohmann::json mouse_event;
        PrimativeProperties *properties;
        
        double_point_t center;
        float start_angle;
        float end_angle;
        float radius;
        float width;
        std::string style; //solid, dashed

        Arc(double_point_t c, float r, float sa, float ea)
        {
            this->properties = new PrimativeProperties();
            this->center = c;
            this->radius = r;
            this->start_angle = sa;
            this->end_angle = ea;
            this->width = 1;
            this->style = "solid";
            mouse_event = NULL;
        }
        std::string get_type_name();
        void process_mouse(float mpos_x, float mpos_y);
        void render_arc(double cx, double cy, double radius, double start_angle, double end_angle);
        void render();
};

#endif //ARC_