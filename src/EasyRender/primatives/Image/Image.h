#ifndef IMAGE_
#define IMAGE_

#include "../PrimativeProperties.h"
#include "../../json/json.h"
#include "../../geometry/geometry.h"
#include "../../gui/stb_image.h"
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


class EasyPrimative::Image{
    public:
        nlohmann::json mouse_event;
        PrimativeProperties *properties;

        float position[3];
        std::string image_file;
        float image_size[2];
        float width;
        float height;
        GLuint texture;
        
        Image(double_point_t p, std::string f, float w, float h)
        {
            this->properties = new PrimativeProperties();
            this->position[0] = p.x;
            this->position[1] = p.y;
            this->position[2] = 0;
            this->width = w;
            this->height = h;
            this->image_file = f;
            this->texture = -1;
            this->mouse_event = NULL;
        }
        std::string get_type_name();
        void process_mouse(float mpos_x, float mpos_y);
        void render();
        void destroy();
        nlohmann::json serialize();
        bool ImageToTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
        
};

#endif //IMAGE_