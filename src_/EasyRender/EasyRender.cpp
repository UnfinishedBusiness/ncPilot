#include <stdio.h>
#include <string.h>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include "EasyRender.h"
#include "logging/loguru.h"
#include "gui/imgui.h"
#include "gui/imgui_impl_glfw.h"
#include "gui/imgui_impl_opengl2.h"

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
    GLFW Key Callbacks
*/
void EasyRender::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    EasyRender *self = reinterpret_cast<EasyRender *>(glfwGetWindowUserPointer(window));
    if (self != NULL)
    {
        LOG_F(INFO, "Keycallback!");
        for (int x = 0; x < self->primative_stack.size(); x++)
        {
            //LOG_F(INFO, "%d->%s", x, self->primative_stack.at(x)->type.c_str());
        }
    }
}

/*
    Each primative type must have a method
*/
Line* EasyRender::PushPrimative(Line* l)
{  
    PrimativeContainer *c = new PrimativeContainer(l);
    primative_stack.push_back(c);
    return c->line;
}
Text* EasyRender::PushPrimative(Text* t)
{  
    PrimativeContainer *c = new PrimativeContainer(t);
    primative_stack.push_back(c);
    return c->text;
}
Image* EasyRender::PushPrimative(Image* i)
{  
    PrimativeContainer *c = new PrimativeContainer(i);
    primative_stack.push_back(c);
    return c->image;
}
Path* EasyRender::PushPrimative(Path* p)
{  
    PrimativeContainer *c = new PrimativeContainer(p);
    primative_stack.push_back(c);
    return c->path;
}
Arc* EasyRender::PushPrimative(Arc* a)
{  
    PrimativeContainer *c = new PrimativeContainer(a);
    primative_stack.push_back(c);
    return c->arc;
}
Circle* EasyRender::PushPrimative(Circle* ci)
{  
    PrimativeContainer *c = new PrimativeContainer(ci);
    primative_stack.push_back(c);
    return c->circle;
}
Box* EasyRender::PushPrimative(Box* b)
{  
    PrimativeContainer *c = new PrimativeContainer(b);
    primative_stack.push_back(c);
    return c->box;
}
void EasyRender::PushTimer(unsigned long intervol, bool (*c)())
{
    EasyRenderTimer *t = new EasyRenderTimer;
    t->intervol = intervol;
    t->timestamp = this->Millis();
    t->callback = c;
    timer_stack.push_back(t);
}
EasyRender::EasyRenderGui *EasyRender::PushGui(bool v, void (*c)())
{
    EasyRender::EasyRenderGui *g = new EasyRender::EasyRenderGui;
    g->visable = v;
    g->callback = c;
    gui_stack.push_back(g);
    return g;
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
uint8_t EasyRender::GetFramesPerSecond()
{
    return (uint8_t)(1000.0f / (float)RenderPerformance);
}
void EasyRender::SetColorByName(float *c, std::string color)
{
    if (color == "white")
    {
        c[0] = 255;
        c[1] = 255;
        c[2] = 255;
        c[3] = 255;
    }
    else if (color == "red")
    {
        c[0] = 255;
        c[1] = 0;
        c[2] = 0;
        c[3] = 255;
    }
    else if (color == "green")
    {
        c[0] = 0;
        c[1] = 255;
        c[2] = 0;
        c[3] = 255;
    }
    else if (color == "blue")
    {
        c[0] = 0;
        c[1] = 0;
        c[2] = 255;
        c[3] = 255;
    }
    else
    {
        c[0] = 255;
        c[1] = 255;
        c[2] = 255;
        c[3] = 255;
    }
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
    glfwSetWindowUserPointer(this->Window, reinterpret_cast<void *>(this));
    glfwSetKeyCallback(this->Window, this->key_callback);
    //glfwSetMouseButtonCallback(this->Window, Xrender_mouse_button_callback);
    //glfwSetScrollCallback(this->Window, Xrender_scroll_callback);
    //glfwSetWindowCloseCallback(this->Window, window_close_callback);
    //glfwSetCursorPosCallback(this->Window, Xrender_cursor_position_callback);
    //glfwSetWindowSizeCallback(this->Window, Xrender_window_size_callback);
    glfwMakeContextCurrent(this->Window);
    glfwSwapInterval(1); // Enable vsync
    //Set calbacks here
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = this->GuiIniFileName;
    io.LogFilename = this->GuiLogFileName;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    if (this->GuiStyle == "light")
    {
        ImGui::StyleColorsLight();
    }
    else if (this->GuiStyle == "dark")
    {
        ImGui::StyleColorsDark();
    }
    ImGui_ImplGlfw_InitForOpenGL(this->Window, true);
    ImGui_ImplOpenGL2_Init();
    return true;
}
bool EasyRender::Poll(bool should_quit)
{
    unsigned long begin_timestamp = this->Millis();
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
    /* IMGUI */
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    for (int x = 0; x < this->gui_stack.size(); x++)
    {
        if (this->gui_stack[x]->visable == true)
        {
            this->gui_stack[x]->callback();
        }
    }
    ImGui::Render();
    /*********/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( -this->WindowSize[0]/2, this->WindowSize[0]/2, -this->WindowSize[1]/2, this->WindowSize[1]/2, -1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, this->WindowSize[0], this->WindowSize[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    sort(primative_stack.begin(), primative_stack.end(), [](auto* lhs, auto* rhs) {
        return lhs->properties->zindex < rhs->properties->zindex ;
    });
    for (int x = 0; x < this->primative_stack.size(); x ++)
    {
        primative_stack.at(x)->render();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        if (!io.WantCaptureKeyboard || !io.WantCaptureMouse)
        {
            primative_stack.at(x)->process_mouse(window_mouse_pos.x, window_mouse_pos.y);
        }
    }
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(this->Window);
    glfwSwapBuffers(this->Window);
    glfwPollEvents();
    for (int x = 0; x < this->timer_stack.size(); x++)
    {
        if ((this->Millis() - this->timer_stack.at(x)->timestamp) > this->timer_stack.at(x)->intervol)
        {
            this->timer_stack.at(x)->timestamp = this->Millis();
            if (this->timer_stack.at(x)->callback != NULL)
            {
                if (this->timer_stack.at(x)->callback() == false) //Don't repeat
                {
                    delete this->timer_stack.at(x);
                    this->timer_stack.erase(this->timer_stack.begin()+x);
                }
            }
        }
    }
    this->RenderPerformance = (this->Millis() - begin_timestamp);
    if (should_quit == true) return false;
    return !glfwWindowShouldClose(this->Window);
}
void EasyRender::Close()
{
    for (int x = 0; x < primative_stack.size(); x++)
    {
        primative_stack.at(x)->destroy();
        delete primative_stack.at(x);
    }
    for (int x = 0; x < timer_stack.size(); x++)
    {
        delete this->timer_stack.at(x);
        this->timer_stack.erase(this->timer_stack.begin()+x);
    }
    for (int x = 0; x < gui_stack.size(); x++)
    {
        delete this->gui_stack.at(x);
        this->gui_stack.erase(this->gui_stack.begin()+x);
    }
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(this->Window);
    glfwTerminate();
    free(this->GuiIniFileName);
    free(this->GuiLogFileName);
}