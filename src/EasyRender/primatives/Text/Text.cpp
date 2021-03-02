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

std::string EasyPrimative::Text::get_type_name()
{
    return "text";
}
void EasyPrimative::Text::process_mouse(float mpos_x, float mpos_y)
{
    mpos_x = (mpos_x - this->properties->offset[0]) / this->properties->scale;
    mpos_y = (mpos_y - this->properties->offset[1]) / this->properties->scale;
    if (mpos_x > this->position.x && mpos_x < (this->position.x + this->width) &&
        mpos_y > this->position.y && mpos_y < (this->position.y + this->height)
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
bool EasyPrimative::Text::InitFontFromFile(const char* filename, float font_size)
{
    unsigned char temp_bitmap[512 * 512];
    size_t ttf_buffer_size = 0;
    unsigned char *ttf_buffer = NULL;
    if (std::string(filename) == "default")
    {
        ttf_buffer_size = sizeof(Sans_ttf);
        ttf_buffer = (unsigned char*)malloc(ttf_buffer_size * sizeof(unsigned char));
        if (ttf_buffer != NULL)
        {
            for (int x = 0; x < sizeof(Sans_ttf); x++)
            {
                ttf_buffer[x] = Sans_ttf[x];
            }
        }
        else
        {
            LOG_F(INFO, "Failed to allocate memory to copy Sans_ttf into!");
            return false;
        }
    }
    else
    {
        FILE *fp;
        fp = fopen(std::string(filename).c_str(), "rb");
        if (fp)
        {
            fseek(fp, 0L, SEEK_END);
            ttf_buffer_size = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            ttf_buffer = (unsigned char*)malloc(ttf_buffer_size * sizeof(unsigned char));
            if (ttf_buffer != NULL)
            {
                fread(ttf_buffer, 1, ttf_buffer_size, fp);
            }
            else
            {
                LOG_F(INFO, "Failed to allocate memory to copy Sans_ttf into!");
                return false;
            }
            fclose(fp);
        }
        else
        {
            LOG_F(WARNING, "Could not open font file: %s", filename);
            return false;
        }
    }
    stbtt_BakeFontBitmap(ttf_buffer,0, font_size, temp_bitmap, 512, 512, 32,96, this->cdata);
    glGenTextures(1, &this->texture);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return true;
}
void EasyPrimative::Text::RenderFont(float pos_x, float pos_y, std::string text)
{
    this->width = 0;
    this->height = 0;
    glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, this->texture);
        glPushMatrix();
            glRotatef(this->properties->angle, 0.0, 0.0, 1.0);
            glBegin(GL_QUADS);
            for (int x = 0; x < text.size(); x++)
            {
                if ((int)text[x] >=32 && (int)text[x] < 128)
                {
                    stbtt_aligned_quad q;
                    stbtt_GetBakedQuad(this->cdata, 512, 512, text[x]-32, &pos_x, &pos_y, &q, 1);
                    glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,-q.y1);
                    glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,-q.y1);
                    glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,-q.y0);
                    glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,-q.y0);
                    this->width += (float)MAX(q.x0, q.x1) - (float)MIN(q.x0, q.x1);
                    if (((float)MAX(q.y0, q.y1) - (float)MIN(q.y0, q.y1)) > this->height)
                    {
                        this->height = ((float)MAX(q.y0, q.y1) - (float)MIN(q.y0, q.y1));
                    }
                }
            }
            glEnd();
        glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}
void EasyPrimative::Text::render()
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
            else
            {
                this->render();
            }
        }
        else
        {
            this->RenderFont(this->position.x, -this->position.y, this->textval);
        }
    glPopMatrix();
}
void EasyPrimative::Text::destroy()
{
    glDeleteTextures(1, &this->texture);
    delete this->properties;
}
nlohmann::json EasyPrimative::Text::serialize()
{
    nlohmann::json j;
    j["textval"] = this->textval;
    j["font_file"] = this->font_file;
    j["font_size"] = this->font_size;
    j["position"]["x"] = this->position.x;
    j["position"]["y"] = this->position.y;
    j["width"] = this->width;
    j["height"] = this->height;
    return j;
}