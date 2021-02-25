#include <stdio.h>
#include <string.h>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include "EasyRender.h"
#include "logging/loguru.h"


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

const auto EasyRenderProgramStartTime = std::chrono::steady_clock::now();

/*
    Each primative type must have a method
*/
Line* EasyRender::PushPrimative(Line* l)
{  
    PrimativeContainer *c = new PrimativeContainer(l);
    primative_stack.push_back(c);
    return c->line;
}

void EasyRender::SetWindowTitle(std::string w)
{
    this->WindowTitle = w;
}
void EasyRender::SetWindowSize(int width, int height)
{
    this->WindowSize[0] = width;
    this->WindowSize[1] = height;
}
void EasyRender::SetShowCursor(bool s)
{
    this->ShowCursor = s;
}
void EasyRender::SetAutoMaximize(bool m)
{
    this->AutoMaximize = m;
}
void EasyRender::SetGuiIniFileName(std::string i)
{
    this->GuiIniFileName = (char*)malloc(sizeof(char) * strlen(i.c_str()));
    strcpy(this->GuiIniFileName , i.c_str());
}
void EasyRender::SetGuiLogFileName(std::string l)
{
    this->GuiLogFileName = (char*)malloc(sizeof(char) * strlen(l.c_str()));
    strcpy(this->GuiLogFileName , l.c_str());
}
void EasyRender::SetMainLogFileName(std::string l)
{
    this->MainLogFileName = l;
}
void EasyRender::SetGuiStyle(std::string s)
{
    this->GuiStyle = s;
}
void EasyRender::SetClearColor(float r, float g, float b)
{
    this->ClearColor[0] = r / 255;
    this->ClearColor[1] = g / 255;
    this->ClearColor[2] = b / 255;
}
unsigned long EasyRender::Millis()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::seconds;
    using std::chrono::system_clock;
    auto end = std::chrono::steady_clock::now();
    unsigned long m = (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(end - EasyRenderProgramStartTime).count();
    return m;
}
void EasyRender::Delay(unsigned long ms)
{
  unsigned long delay_timer = this->Millis();
  while((this->Millis() - delay_timer) < ms);
  return;
}
std::string EasyRender::GetEvironmentVariable(const std::string & var)
{
    const char * val = std::getenv( var.c_str() );
    if ( val == nullptr )
    {
        return "";
    }
    else
    {
        return val;
    }
}
std::string EasyRender::GetConfigDirectory()
{
    #ifdef __linux__   
        return this->GetEvironmentVariable("HOME") + "/.config/" + this->WindowTitle + "/";
    #elif __APPLE__   
        return this->GetEvironmentVariable("HOME") + "/Library/" + this->WindowTitle + "/";
    #elif _WIN32
        return this->GetEvironmentVariable("CSIDL_DEFAULT_APPDATA") + this->WindowTitle + "\\";
    #else     
        #error Platform not supported! Must be Linux, OSX, or Windows
    #endif
}
double_point_t EasyRender::GetWindowMousePosition()
{
    double mouseX, mouseY;
    glfwGetCursorPos(this->Window, &mouseX, &mouseY);
    return {mouseX - (this->WindowSize[0]/ 2.0f), (this->WindowSize[1] - mouseY) - (this->WindowSize[1] / 2.0f)};
}
bool EasyRender::Init(int argc, char** argv)
{
    loguru::init(argc, argv);
    loguru::g_stderr_verbosity = 1;
    loguru::add_file(this->MainLogFileName.c_str(), loguru::Append, loguru::Verbosity_MAX);
    if (!glfwInit())
    {
        LOG_F(ERROR, "Could not init glfw window!");
        return false;
    }
    this->Window = glfwCreateWindow(this->WindowSize[0], this->WindowSize[1], this->WindowTitle.c_str(), NULL, NULL);
    if (this->Window == NULL)
    {
        LOG_F(ERROR, "Could not open GLFW Window!");
        return false;
    }
    if (this->AutoMaximize == true)
    {
        glfwMaximizeWindow(this->Window);
    }
    if (this->ShowCursor == false)
    {
        glfwSetInputMode(this->Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    glfwMakeContextCurrent(this->Window);
    glfwSwapInterval(1); // Enable vsync
    //Set calbacks here

    return true;
}
bool EasyRender::Poll(bool should_quit)
{
    this->RenderPerformance = this->Millis();
    glfwGetFramebufferSize(this->Window, &this->WindowSize[0], &this->WindowSize[1]);
    float ratio = this->WindowSize[0] / (float)this->WindowSize[1];
    double_point_t window_mouse_pos = this->GetWindowMousePosition();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthRange(0, 1);
    glDepthFunc(GL_LEQUAL);
    glClearColor(this->ClearColor[0], this->ClearColor[1], this->ClearColor[2], 255);
    //Execute GUI stack here
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( -this->WindowSize[0]/2, this->WindowSize[0]/2, -this->WindowSize[1]/2, this->WindowSize[1]/2, -1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, this->WindowSize[0], this->WindowSize[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    for (int x = 0; x < this->primative_stack.size(); x ++)
    {
        primative_stack.at(x)->render();
        primative_stack.at(x)->process_mouse(window_mouse_pos.x, window_mouse_pos.y);
    }
    glfwMakeContextCurrent(this->Window);
    glfwSwapBuffers(this->Window);
    glfwPollEvents();

    //Execute pending timers here

    if (should_quit == true) return false;
    return !glfwWindowShouldClose(this->Window);
}
void EasyRender::Close()
{
    //Delete EntityContainer objects
    //ImGui_ImplOpenGL2_Shutdown();
    //ImGui_ImplGlfw_Shutdown();
    //ImGui::DestroyContext();
    glfwDestroyWindow(this->Window);
    glfwTerminate();
    free(this->GuiIniFileName);
    free(this->GuiLogFileName);
}