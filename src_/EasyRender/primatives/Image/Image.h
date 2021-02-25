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


class Image{
    public:
        nlohmann::json mouse_event;
        PrimativeProperties *properties;

        std::string image_file;
        float image_size[2];
        GLuint texture;
        
        Image(double_point_t p, std::string f, double_point_t size)
        {
            this->properties = new PrimativeProperties();
            this->properties->position[0] = p.x;
            this->properties->position[1] = p.y;
            this->properties->size[0] = size.x;
            this->properties->size[1] = size.y;
            this->image_file = f;
            this->texture = -1;
            this->mouse_event = NULL;
        }
        std::string get_type_name();
        void process_mouse(float mpos_x, float mpos_y);
        void render();

        bool ImageToTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
        
};

#endif //IMAGE_