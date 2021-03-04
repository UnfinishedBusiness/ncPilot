#ifndef PRIMATIVES_
#define PRIMATIVES_

#include <string>
#include "../json/json.h"

class PrimativeContainer;

namespace EasyPrimative
{
    class Arc;
    class Box;
    class Circle;
    class Image;
    class Line;
    class Path;
    class Part;
    class Text;
};

/*
    Properties that all primatives will share
*/
class PrimativeProperties{
    public:
        bool visable;
        bool mouse_over;
        float mouse_over_padding;
        int zindex;
        float color[4];
        float offset[3];
        float scale;
        float angle;
        std::string id;
        std::string view;
        nlohmann::json data;

        /* Event Callbacks */
        void (*mouse_callback)(PrimativeContainer*, nlohmann::json);
        void (*matrix_callback)(PrimativeContainer*);

        PrimativeProperties()
        {
            this->visable = true;
            this->zindex = 1;
            this->color[0] = 255;
            this->color[1] = 255;
            this->color[2] = 255;
            this->color[3] = 255;
            this->scale = 1;
            this->offset[0] = 0;
            this->offset[1] = 0;
            this->offset[2] = 0;
            this->angle = 0;
            this->id = "";
            this->view = "main";
            this->mouse_over = false;
            this->mouse_over_padding = 5;
            this->mouse_callback = NULL;
            this->matrix_callback = NULL;
        }
};

#endif //PRIMATIVES_