#ifndef TEXT_
#define TEXT_

#include "../PrimativeProperties.h"
#include "../../json/json.h"
#include "../../geometry/geometry.h"
#include "../../gui/stb_truetype.h"
#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #include <GL/freeglut.h>
   #include <GL/gl.h>
   #define GL_CLAMP_TO_EDGE 0x812F
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #include <OpenGL/glu.h>
#elif __linux__
    #include <GL/glu.h>
#elif __unix__
    #include <GL/glu.h>
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif


class EasyPrimative::Text{
    public:
        nlohmann::json mouse_event;
        PrimativeProperties *properties;

        std::string textval;
        std::string font_file;
        float font_size;
        double_point_t position;
        float width;
        float height;
        GLuint texture;
        stbtt_bakedchar cdata[96];
        
        Text(double_point_t p, std::string t, float s)
        {
            this->properties = new PrimativeProperties();
            this->position = p;
            this->width = 0;
            this->height = 0;
            this->textval = t;
            this->font_file = "default";
            this->font_size = s;
            this->texture = -1;
            this->mouse_event = NULL;
        }
        std::string get_type_name();
        void process_mouse(float mpos_x, float mpos_y);
        void render();
        bool InitFontFromFile(const char* filename, float font_size);
        void RenderFont(float pos_x, float pos_y, std::string text);
        void destroy();
        nlohmann::json serialize();
};

#endif //TEXT_