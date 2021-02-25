#ifndef LINE_
#define LINE_

#include "PrimativeProperties.h"
#include "../json/json.h"
#include "../geometry/geometry.h"
#include <string>

class Line{
    public:
        nlohmann::json mouse_event;
        PrimativeProperties *properties;
        double_point_t start;
        double_point_t end;
        float width;
        std::string style; //solid, dashed

        Line(double_point_t s, double_point_t e)
        {
            this->properties = new PrimativeProperties();
            this->start = s;
            this->end = e;
            mouse_event = NULL;
        }
        std::string get_type_name();
        nlohmann::json *get_data();
        void process_mouse(float mpos_x, float mpos_y);
        void render();
};

#endif //LINE_