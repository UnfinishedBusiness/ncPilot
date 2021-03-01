#include <stdio.h>
#include <string>
#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../gui/stb_image.h"
#include "../../geometry/geometry.h"
#include "../../logging/loguru.h"

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

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

std::string EasyPrimative::Image::get_type_name()
{
    return "image";
}
void EasyPrimative::Image::process_mouse(float mpos_x, float mpos_y)
{
    mpos_x = (mpos_x - this->properties->offset[0]) / this->properties->scale;
    mpos_y = (mpos_y - this->properties->offset[1]) / this->properties->scale;
    if (mpos_x > this->position[0] && mpos_x < (this->position[0] + this->width) &&
        mpos_y > this->position[1] && mpos_y < (this->position[1] + this->height)
    )
    {
        if (this->properties->mouse_over == false)
        {
            this->mouse_event = {
                {"event", "mouse_in"},
                {"pos", {
                    {"x", mpos_x},
                    {"y", mpos_y}
                }},
            };
            this->properties->mouse_over = true;
        }    
    }
    else
    {
        if (this->properties->mouse_over == true)
        {
            this->mouse_event = {
                {"event", "mouse_out"},
                {"pos", {
                    {"x", mpos_x},
                    {"y", mpos_y}
                }},
            };
            this->properties->mouse_over = false;
        }
    }
}
bool EasyPrimative::Image::ImageToTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
    {
        return false;
    }
    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
    // Upload pixels into texture
    #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    #endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;
    return true;
}
void EasyPrimative::Image::render()
{
    glPushMatrix();
        glTranslatef(this->properties->offset[0], this->properties->offset[1], this->properties->offset[2]);
        glScalef(this->properties->scale, this->properties->scale, this->properties->scale);
        glColor4f(this->properties->color[3] / 255, this->properties->color[3] / 255, this->properties->color[3] / 255, this->properties->color[3] / 255);
        if (this->texture == -1)
        {
            int my_image_width = 0;
            int my_image_height = 0;
            GLuint my_image_texture = 0;
            bool ret = this->ImageToTextureFromFile(this->image_file.c_str(), &this->texture, &my_image_width, &my_image_height);
            if (ret)
            {
                //printf("Loaded image %s, width: %d, height: %d\n", this->image_file.c_str(), my_image_width, my_image_height);
                this->image_size[0] = my_image_width;
                this->image_size[1] = my_image_height;
                if (this->width == 0 && this->height == 0)
                {
                    this->width = my_image_width;
                    this->height = my_image_height;
                }
            }
            else
            {
                LOG_F(ERROR, "Error loading image!");
                this->texture = -1;
            }
        }
        else
        {
            glPushMatrix();
                glTranslatef(this->position[0], this->position[1], 0.0);
                glRotatef(this->properties->angle, 0.0, 0.0, 1.0);
                glScalef(1.0f, -1.0f, 1.0f);
                double imgWidth = this->width;
                double imgHeight = this->height;
                glBindTexture(GL_TEXTURE_2D, this->texture);
                glEnable(GL_TEXTURE_2D);
                    glBegin(GL_QUADS);
                        glTexCoord2f(0, 0); glVertex2f(-imgWidth, -imgHeight);
                        glTexCoord2f(1, 0); glVertex2f(imgWidth, -imgHeight);
                        glTexCoord2f(1,1);  glVertex2f(imgWidth, imgHeight);
                        glTexCoord2f(0, 1); glVertex2f(-imgWidth, imgHeight);
                    glEnd();
                glDisable(GL_TEXTURE_2D);
            glPopMatrix();
        }
    glPopMatrix();
}
void EasyPrimative::Image::destroy()
{
    glDeleteTextures(1, &this->texture);
    delete this->properties;
}
nlohmann::json EasyPrimative::Image::serialize()
{
    nlohmann::json j;
    j["position"]["x"] = this->position[0];
    j["position"]["y"] = this->position[1];
    j["position"]["z"] = this->position[2];
    j["width"] = this->width;
    j["height"] = this->height;
    j["image_file"] = this->image_file;
    return j;
}