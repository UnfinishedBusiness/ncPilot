#ifndef PRIMATIVE_CONTAINTER_
#define PRIMATIVE_CONTAINTER_

#include <string>
#include <vector>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <algorithm> 
#include "PrimativeProperties.h"
#include "Line/Line.h"
#include "Text/Text.h"
#include "Image/Image.h"
#include "Path/Path.h"
#include "Arc/Arc.h"
#include "Circle/Circle.h"
#include "Box/Box.h"
#include "Part/Part.h"

class PrimativeContainer{
    public:
        PrimativeProperties *properties;
        std::string type;
        EasyPrimative::Line *line;
        EasyPrimative::Text *text;
        EasyPrimative::Image *image;
        EasyPrimative::Path *path;
        EasyPrimative::Arc *arc;
        EasyPrimative::Circle *circle;
        EasyPrimative::Box *box;
        EasyPrimative::Part *part;

        void process_mouse(float mpos_x, float mpos_y);
        void render();
        void destroy();
        nlohmann::json serialize();

        PrimativeContainer(EasyPrimative::Line *l)
        {
            line = l;
            this->type = line->get_type_name();
            properties = line->properties;
        }
        PrimativeContainer(EasyPrimative::Text *t)
        {
            text = t;
            this->type = text->get_type_name();
            properties = text->properties;
            this->text->render(); //Make sure height and width are calculated right away
        }
        PrimativeContainer(EasyPrimative::Image *i)
        {
            image = i;
            this->type = image->get_type_name();
            properties = image->properties;
        }
        PrimativeContainer(EasyPrimative::Path *p)
        {
            path = p;
            this->type = path->get_type_name();
            properties = path->properties;
        }
        PrimativeContainer(EasyPrimative::Part *p)
        {
            part = p;
            this->type = part->get_type_name();
            properties = part->properties;
        }
        PrimativeContainer(EasyPrimative::Arc *a)
        {
            arc = a;
            this->type = arc->get_type_name();
            properties = arc->properties;
        }
        PrimativeContainer(EasyPrimative::Circle *c)
        {
            circle = c;
            this->type = circle->get_type_name();
            properties = circle->properties;
        }
        PrimativeContainer(EasyPrimative::Box *b)
        {
            box = b;
            this->type = box->get_type_name();
            properties = box->properties;
        }
};

#endif //PRIMATIVE_CONTAINTER_