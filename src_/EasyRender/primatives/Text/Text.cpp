#include <stdio.h>
#include <string>
#include "Text.h"
#include "fonts/Sans.ttf.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../gui/stb_truetype.h"
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

std::string Text::get_type_name()
{
    return "text";
}
void Text::process_mouse(float mpos_x, float mpos_y)
{
    //printf("X%.4f, Y%.4f\n", mpos_x, mpos_y);
    if (this->properties->visable == true)
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
                    {"type", "mouse_in"},
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
                    {"type", "mouse_out"},
                    {"pos", {
                        {"x", mpos_x},
                        {"y", mpos_y}
                    }},
                };
                this->properties->mouse_over = false;
            }
        }
    }
}
bool Text::InitFontFromFile(const char* filename, float font_size)
{
    unsigned char temp_bitmap[512*512];
    unsigned char ttf_buffer[1<<20];
    FILE *fp;
    if (std::string(filename) == "default")
    {
        for (int x = 0; x < (1<<20); x++)
        {
            ttf_buffer[x] = Sans_ttf[x];
        }
    }
    else
    {
        fp = fopen(std::string(filename).c_str(), "rb");
        if (fp)
        {
            fread(ttf_buffer, 1, 1<<20, fp);
        }
        else
        {
            return false;
        }
    }
    stbtt_BakeFontBitmap(ttf_buffer,0, font_size, temp_bitmap ,512,512, 32,96, this->cdata); // no guarantee this fits!
    glGenTextures(1, &this->texture);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    fclose(fp);
    return true;
}
void Text::RenderFont(float pos_x, float pos_y, std::string text)
{
    double width, height = 0;
    glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, this->texture);
        glPushMatrix();
            //glColor4f(this->properties->color[0], this->properties->color[2], this->properties->color[2], this->properties->color[3]);
            glRotatef(this->properties->angle, 0.0, 0.0, 1.0);
            glBegin(GL_QUADS);
            for (int x = 0; x < text.size(); x++)
            {
                if (text[x] >=32 && text[x] < 128)
                {
                    stbtt_aligned_quad q;
                    stbtt_GetBakedQuad(this->cdata, 512,512, text[x]-32, &pos_x,&pos_y,&q,1);
                    glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,-q.y1);
                    glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,-q.y1);
                    glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,-q.y0);
                    glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,-q.y0);
                    width += MAX(q.x0, q.x1) - MIN(q.x0, q.x1);
                    if ((MAX(q.y0, q.y1) - MIN(q.y0, q.y1)) > height) height = (MAX(q.y0, q.y1) - MIN(q.y0, q.y1));
                }
            }
            glEnd();
        glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glFlush();
    if (this->width == 0 || this->height == 0)
    {
        this->width = width;
        this->height = height;
    }
}
void Text::render()
{
    if (this->properties->visable == true)
    {
        glPushMatrix();
            glTranslatef(this->properties->offset[0], this->properties->offset[1], this->properties->offset[2]);
            glScalef(this->properties->scale, this->properties->scale, this->properties->scale);
            glColor4f(this->properties->color[0] / 255, this->properties->color[1] / 255, this->properties->color[2] / 255, this->properties->color[3] / 255);
            if (this->texture == -1)
            {
                bool ret = this->InitFontFromFile(this->font_file.c_str(), this->font_size);
                if (ret == false)
                {
                    LOG_F(WARNING, "Could not init font: %s\n", this->font_file.c_str());
                    this->texture = -1;
                }
            }
            else
            {
                this->RenderFont(this->position[0], -this->position[1], this->textval);
            }
        glPopMatrix();
    }
}
