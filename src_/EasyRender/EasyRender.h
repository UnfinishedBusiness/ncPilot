#ifndef EASYRENDER_
#define EASYRENDER_

#include <string>
#include <vector>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <algorithm> 
#include "geometry/geometry.h"
#include "json/json.h"
#include "PrimativeContainer.h"


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

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

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif


class EasyRender{
    private:
        GLFWwindow* Window;
        unsigned long RenderPerformance;
        float ClearColor[3];
        std::string WindowTitle;
        int WindowSize[2];
        bool ShowCursor;
        bool AutoMaximize;
        char* GuiIniFileName;
        char* GuiLogFileName;
        std::string MainLogFileName;
        std::string GuiStyle;

        std::vector<PrimativeContainer *> primative_stack;

    public:
        EasyRender()
        {
            //Load Defaults
            this->SetWindowTitle("EasyRender");
            this->SetWindowSize(500, 400);
            this->SetShowCursor(true);
            this->SetAutoMaximize(false);
            this->SetGuiIniFileName("EasyRenderGui.ini");
            this->SetGuiLogFileName("EasyRenderGio.log");
            this->SetMainLogFileName("EasyRender.log");
            this->SetGuiStyle("light");
            this->SetClearColor(100, 100, 100);
        };
        Line* PushPrimative(Line* l);
        void SetWindowTitle(std::string w);
        void SetWindowSize(int width, int height);
        void SetShowCursor(bool s);
        void SetAutoMaximize(bool m);
        void SetGuiIniFileName(std::string i);
        void SetGuiLogFileName(std::string l);
        void SetMainLogFileName(std::string l);
        void SetGuiStyle(std::string s);
        void SetClearColor(float r, float g, float b);
        unsigned long Millis();
        void Delay(unsigned long ms);
        std::string GetEvironmentVariable(const std::string & var);
        std::string GetConfigDirectory();
        double_point_t GetWindowMousePosition();
        void SetColorByName(float *c, std::string color);
        bool Init(int argc, char** argv);
        bool Poll(bool should_quit);
        void Close();

    
};

#endif //EASYREANDER_