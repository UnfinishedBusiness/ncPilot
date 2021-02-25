#ifndef PRIMATIVE_CONTAINTER_
#define PRIMATIVE_CONTAINTER_

#include <string>
#include <vector>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <algorithm> 
#include "primatives/PrimativeProperties.h"
#include "primatives/Line/Line.h"
#include "primatives/Text/Text.h"
#include "primatives/Image/Image.h"
#include "primatives/Path/Path.h"
#include "primatives/Arc/Arc.h"

class PrimativeContainer{
    public:
        PrimativeProperties *properties;
        std::string type;
        Line *line;
        Text *text;
        Image *image;
        Path *path;
        Arc *arc;

        void process_mouse(float mpos_x, float mpos_y);
        void render();

        PrimativeContainer(Line *l)
        {
            line = l;
            this->type = line->get_type_name();
            properties = line->properties;
        }
        PrimativeContainer(Text *t)
        {
            text = t;
            this->type = text->get_type_name();
            properties = text->properties;
        }
        PrimativeContainer(Image *i)
        {
            image = i;
            this->type = image->get_type_name();
            properties = image->properties;
        }
        PrimativeContainer(Path *p)
        {
            path = p;
            this->type = path->get_type_name();
            properties = path->properties;
        }
        PrimativeContainer(Arc *a)
        {
            arc = a;
            this->type = arc->get_type_name();
            properties = arc->properties;
        }
};

#endif //PRIMATIVE_CONTAINTER_