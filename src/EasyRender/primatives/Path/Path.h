#ifndef PATH_
#define PATH_

#include "../PrimativeProperties.h"
#include "../../json/json.h"
#include "../../geometry/geometry.h"
#include <string>
#include <vector>

class EasyPrimative::Path{
    public:
        nlohmann::json mouse_event;
        PrimativeProperties *properties;

        std::vector<double_point_t> points;
        bool is_closed;
        float width;
        std::string style; //solid, dashed

        Path(std::vector<double_point_t> p)
        {
            this->properties = new PrimativeProperties();
            this->points = p;
            this->is_closed = true;
            this->width = 1;
            this->style = "solid";
            this->mouse_event = NULL;
        }
        std::string get_type_name();
        void process_mouse(float mpos_x, float mpos_y);
        void render();
        void destroy();
        nlohmann::json serialize();
};

#endif //PATH_