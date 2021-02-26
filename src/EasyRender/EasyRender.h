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
#include "primatives/PrimativeContainer.h"

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
        struct EasyRenderTimer{
            unsigned long timestamp;
            unsigned long intervol;
            bool (*callback)();
        };
        GLFWwindow* Window;
        unsigned long RenderPerformance;
        float ClearColor[3];
        std::string WindowTitle;
        int WindowSize[2];
        bool ShowCursor;
        bool AutoMaximize;
        bool ShowFPS;
        Text *FPS_Label;
        std::vector<float> FPS_Average;
        char* GuiIniFileName;
        char* GuiLogFileName;
        std::string MainLogFileName;
        std::string GuiStyle;

        std::vector<PrimativeContainer *> primative_stack;
        std::vector<EasyRenderTimer *> timer_stack;

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
        static void window_size_callback(GLFWwindow* window, int width, int height);

    public:
        struct EasyRenderGui{
            bool visable;
            void (*callback)();
        };
        struct EasyRenderEvent{
            std::string key;
            std::string type; //keyup, keydown, scroll, mouse_click, mouse_move, window_resize
            void (*callback)(nlohmann::json);
        };
        std::vector<EasyRenderGui *> gui_stack;
        std::vector<EasyRenderEvent *> event_stack;

        EasyRender()
        {
            //Load Defaults
            this->SetWindowTitle("EasyRender");
            this->SetWindowSize(800, 600);
            this->SetShowCursor(true);
            this->SetAutoMaximize(false);
            this->SetGuiIniFileName("EasyRenderGui.ini");
            this->SetGuiLogFileName("EasyRenderGio.log");
            this->SetMainLogFileName("EasyRender.log");
            this->SetGuiStyle("light");
            this->SetClearColor(21, 22, 34);
            this->SetShowFPS(false);
            this->FPS_Label = NULL;
        };
        /* Primative Creation */
        Line* PushPrimative(Line* l);
        Text* PushPrimative(Text* t);
        Image* PushPrimative(Image* i);
        Path* PushPrimative(Path* i);
        Arc* PushPrimative(Arc* i);
        Circle* PushPrimative(Circle* ci);
        Box* PushPrimative(Box* b);

        /* Timer Creation */
        void PushTimer(unsigned long intervol, bool (*c)());

        /* Gui Creation */
        EasyRenderGui *PushGui(bool v, void (*c)());

        /* Key Event Creation */
        void PushEvent(std::string key, std::string type, void (*callback)(nlohmann::json));


        void SetWindowTitle(std::string w);
        void SetWindowSize(int width, int height);
        void SetShowCursor(bool s);
        void SetAutoMaximize(bool m);
        void SetGuiIniFileName(std::string i);
        void SetGuiLogFileName(std::string l);
        void SetMainLogFileName(std::string l);
        void SetGuiStyle(std::string s);
        void SetClearColor(float r, float g, float b);
        void SetShowFPS(bool show_fps);
        static unsigned long Millis();
        static void Delay(unsigned long ms);
        std::string GetEvironmentVariable(const std::string & var);
        std::string GetConfigDirectory();
        double_point_t GetWindowMousePosition();
        double_point_t GetWindowSize();
        uint8_t GetFramesPerSecond();
        void SetColorByName(float *c, std::string color);
        bool Init(int argc, char** argv);
        bool Poll(bool should_quit);
        void Close();

    
};

#endif //EASYREANDER_