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
        std::string keyname;
        if (glfwGetKeyName(key, scancode) != NULL)
        {
            keyname = std::string(glfwGetKeyName(key, scancode));
        }
        else
        {
            switch(key)
            {
                case 256: keyname = "Escape"; break;
                case 32: keyname = "Space"; break;
                case 258: keyname = "Tab"; break;
                case 265: keyname = "Up"; break;
                case 264: keyname = "Down"; break;
                case 263: keyname = "Left"; break;
                case 262: keyname = "Right"; break;
                default: keyname = "None";
            }
            if (keyname == "None")
            {
                LOG_F(INFO, "(key_callback) Unknown key: %d", key);
            }
        }
        if (!self->imgui_io->WantCaptureKeyboard || !self->imgui_io->WantCaptureMouse)
        {
            for (size_t x = 0; x < self->event_stack.size(); x++)
            {
                if (self->event_stack.at(x)->type == "keyup" && action == 1)
                {
                    if (self->event_stack.at(x)->key == keyname)
                    {
                        if (self->event_stack.at(x)->callback != NULL)
                        {
                            self->event_stack.at(x)->callback({{"type", self->event_stack.at(x)->type}, {"key", keyname}, {"action", action}});
                        }
                    }
                }
                if (self->event_stack.at(x)->type == "keydown" && action == 0)
                {
                    if (self->event_stack.at(x)->key == keyname)
                    {
                        if (self->event_stack.at(x)->callback != NULL)
                        {
                            self->event_stack.at(x)->callback({{"type", self->event_stack.at(x)->type}, {"key", keyname}, {"action", action}});
                        }
                    }
                }
                if (self->event_stack.at(x)->type == "repeat" && action == 2)
                {
                    if (self->event_stack.at(x)->key == keyname)
                    {
                        if (self->event_stack.at(x)->callback != NULL)
                        {
                            self->event_stack.at(x)->callback({{"type", self->event_stack.at(x)->type}, {"key", keyname}, {"action", action}});
                        }
                    }
                }
            }
        }
    }
}
void EasyRender::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    EasyRender *self = reinterpret_cast<EasyRender *>(glfwGetWindowUserPointer(window));
    if (self != NULL)
    {
        if (!self->imgui_io->WantCaptureKeyboard || !self->imgui_io->WantCaptureMouse)
        {
            double_point_t m = self->GetWindowMousePosition();
            std::vector<PrimativeContainer*>::iterator it;
            for ( it = self->primative_stack.end(); it != self->primative_stack.begin(); )
            {
                --it;
                if ((*it)->properties->visable == true && (*it)->properties->mouse_over == true)
                {
                    if ((*it)->properties->mouse_callback != NULL)
                    {
                        std::string event;
                        if (button == 0) //Left click
                        {
                            if (action == 0) //Up
                            {
                                event = "left_click_up";
                            }
                            else if (action == 1) //Down
                            {
                                event = "left_click_down";
                            }
                        }
                        else if (button == 1) //Right Click
                        {
                            if (action == 0) //Up
                            {
                                event = "right_click_up";
                            }
                            else if (action == 1) //Down
                            {
                                event = "right_click_down";
                            }
                        }
                        else if (button == 2) //Middle Click
                        {
                            if (action == 0) //Up
                            {
                                event = "middle_click_up";
                            }
                            else if (action == 1) //Down
                            {
                                event = "middle_click_down";
                            }
                        }
                        double_point_t matrix_mouse = m;
                        matrix_mouse.x = (matrix_mouse.x - (*it)->properties->offset[0]) / (*it)->properties->scale;
                        matrix_mouse.y = (matrix_mouse.y - (*it)->properties->offset[1]) / (*it)->properties->scale;
                        (*it)->properties->mouse_callback((*it), {
                            {"event", event},
                            {"mouse_pos", {
                                {"x", m.x},
                                {"y", m.y}
                            }},
                            {"matrix_mouse_pos", {
                                {"x", matrix_mouse.x},
                                {"y", matrix_mouse.y}
                            }}
                        });
                        return;
                    }
                }
            }
        }
    }
}
void EasyRender::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    EasyRender *self = reinterpret_cast<EasyRender *>(glfwGetWindowUserPointer(window));
    if (self != NULL)
    {
        if (!self->imgui_io->WantCaptureKeyboard || !self->imgui_io->WantCaptureMouse)
        {
            for (size_t x = 0; x < self->event_stack.size(); x++)
            {
                if (yoffset > 0)
                {
                    if (self->event_stack.at(x)->type == "scroll" && self->event_stack.at(x)->key == "up")
                    {
                        self->event_stack.at(x)->callback({{"scroll", yoffset}});
                    }
                }
                else
                {
                    if (self->event_stack.at(x)->type == "scroll" && self->event_stack.at(x)->key == "down")
                    {
                        self->event_stack.at(x)->callback({{"scroll", yoffset}});
                    }
                }
                
            }
        }
    }
}
void EasyRender::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    EasyRender *self = reinterpret_cast<EasyRender *>(glfwGetWindowUserPointer(window));
    if (self != NULL)
    {
        double_point_t m = self->GetWindowMousePosition();
        for (size_t x = 0; x < self->event_stack.size(); x++)
        {
            if (self->event_stack.at(x)->type == "mouse_move")
            {
                self->event_stack.at(x)->callback({{"pos",{{"x", m.x},{"y", m.y}}}});
            }
        }
    }
}
void EasyRender::window_size_callback(GLFWwindow* window, int width, int height)
{
    EasyRender *self = reinterpret_cast<EasyRender *>(glfwGetWindowUserPointer(window));
    if (self != NULL)
    {
        for (size_t x = 0; x < self->event_stack.size(); x++)
        {
            if (self->event_stack.at(x)->type == "window_resize")
            {
                self->event_stack.at(x)->callback({{"size",{{"width", width},{"height", height}}}});
            }
        }
    }
}
/*
    Each primative type must have a method
*/
EasyPrimative::Line* EasyRender::PushPrimative(EasyPrimative::Line* l)
{  
    PrimativeContainer *c = new PrimativeContainer(l);
    primative_stack.push_back(c);
    return c->line;
}
EasyPrimative::Text* EasyRender::PushPrimative(EasyPrimative::Text* t)
{  
    PrimativeContainer *c = new PrimativeContainer(t);
    primative_stack.push_back(c);
    return c->text;
}
EasyPrimative::Image* EasyRender::PushPrimative(EasyPrimative::Image* i)
{  
    PrimativeContainer *c = new PrimativeContainer(i);
    primative_stack.push_back(c);
    return c->image;
}
EasyPrimative::Path* EasyRender::PushPrimative(EasyPrimative::Path* p)
{  
    PrimativeContainer *c = new PrimativeContainer(p);
    primative_stack.push_back(c);
    return c->path;
}
EasyPrimative::Arc* EasyRender::PushPrimative(EasyPrimative::Arc* a)
{  
    PrimativeContainer *c = new PrimativeContainer(a);
    primative_stack.push_back(c);
    return c->arc;
}
EasyPrimative::Circle* EasyRender::PushPrimative(EasyPrimative::Circle* ci)
{  
    PrimativeContainer *c = new PrimativeContainer(ci);
    primative_stack.push_back(c);
    return c->circle;
}
EasyPrimative::Box* EasyRender::PushPrimative(EasyPrimative::Box* b)
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
void EasyRender::PushEvent(std::string key, std::string type, void (*callback)(nlohmann::json))
{
    EasyRenderEvent *e = new EasyRenderEvent;
    e->key = key;
    e->type = type;
    e->callback = callback;
    event_stack.push_back(e);
    if (type == "window_resize" && this->Window != NULL)
    {
        double_point_t size = this->GetWindowSize();
        this->WindowSize[0] = (int)size.x;
        this->WindowSize[1] = (int)size.y;
        EasyRender::window_size_callback(this->Window, this->WindowSize[0], this->WindowSize[1]);
    }
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
    //LOG_F(INFO, "Set Clear Color (%.4f, %.4f, %.4f)", this->ClearColor[0], this->ClearColor[1], this->ClearColor[2]);
}
void EasyRender::SetShowFPS(bool show_fps)
{
    this->ShowFPS = show_fps;
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
    else if (color == "black")
    {
        c[0] = 0;
        c[1] = 0;
        c[2] = 0;
        c[3] = 255;
    }
    else if (color == "grey")
    {
        c[0] = 100;
        c[1] = 100;
        c[2] = 100;
        c[3] = 255;
    }
    else if (color == "light-red")
    {
        c[0] = 190;
        c[1] = 0;
        c[2] = 0;
        c[3] = 255;
    }
    else if (color == "red")
    {
        c[0] = 255;
        c[1] = 0;
        c[2] = 0;
        c[3] = 255;
    }
    else if (color == "light-green")
    {
        c[0] = 0;
        c[1] = 190;
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
    else if (color == "light-blue")
    {
        c[0] = 0;
        c[1] = 0;
        c[2] = 190;
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
  unsigned long delay_timer = EasyRender::Millis();
  while((EasyRender::Millis() - delay_timer) < ms);
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
    mouseX -= 2;
    mouseY -= 4;
    return {mouseX - (this->WindowSize[0]/ 2.0f), (this->WindowSize[1] - mouseY) - (this->WindowSize[1] / 2.0f)};
}
double_point_t EasyRender::GetWindowSize()
{
    glfwGetFramebufferSize(this->Window, &this->WindowSize[0], &this->WindowSize[1]);
    return {(double)this->WindowSize[0], (double)this->WindowSize[1]};
}
uint8_t EasyRender::GetFramesPerSecond()
{
    return (uint8_t)(1000.0f / (float)RenderPerformance);
}
std::vector<PrimativeContainer *> *EasyRender::GetPrimativeStack()
{
    return &this->primative_stack;
}
nlohmann::json EasyRender::DumpPrimativeStack()
{
    nlohmann::json r;
    std::vector<PrimativeContainer*>::iterator it;
    for ( it = this->primative_stack.begin(); it != this->primative_stack.end(); )
    {
        r.push_back((*it)->serialize());
        ++it;
    }
    return r;
}
void EasyRender::DeletePrimativesById(std::string id)
{
    std::vector<PrimativeContainer*>::iterator it;
    for ( it = this->primative_stack.begin(); it != this->primative_stack.end(); )
    {
        if((*it)->properties->id == id)
        {
            (*it)->destroy();
            delete * it;  
            it = this->primative_stack.erase(it);
        }
        else 
        {
            ++it;
        }
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
    glfwSetMouseButtonCallback(this->Window, this->mouse_button_callback);
    glfwSetScrollCallback(this->Window, this->scroll_callback);
    //glfwSetWindowCloseCallback(this->Window, window_close_callback);
    glfwSetCursorPosCallback(this->Window, this->cursor_position_callback);
    glfwSetWindowSizeCallback(this->Window, this->window_size_callback);
    glfwMakeContextCurrent(this->Window);
    glfwSwapInterval(1); // Enable vsync
    //Set calbacks here
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    this->imgui_io = &io;
    this->imgui_io->IniFilename = this->GuiIniFileName;
    this->imgui_io->LogFilename = this->GuiLogFileName;
    this->imgui_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
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
    for (size_t x = 0; x < this->gui_stack.size(); x++)
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
    bool ignore_next_mouse_events = false;
    for (size_t x = 0; x < this->primative_stack.size(); x ++)
    {
        if (this->primative_stack[x]->properties->visable == true)
        {
            primative_stack[x]->render();
            if ((!this->imgui_io->WantCaptureKeyboard || !this->imgui_io->WantCaptureMouse) && ignore_next_mouse_events == false)
            {
                primative_stack[x]->process_mouse(window_mouse_pos.x, window_mouse_pos.y);
            }
        }
    }
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(this->Window);
    glfwSwapBuffers(this->Window);
    glfwPollEvents();
    for (size_t x = 0; x < this->timer_stack.size(); x++)
    {
        if ((this->Millis() - this->timer_stack.at(x)->timestamp) > this->timer_stack.at(x)->intervol)
        {
            this->timer_stack[x]->timestamp = this->Millis();
            if (this->timer_stack[x]->callback != NULL)
            {
                if (this->timer_stack[x]->callback() == false) //Don't repeat
                {
                    delete this->timer_stack[x];
                    this->timer_stack.erase(this->timer_stack.begin()+x);
                }
            }
        }
    }
    this->RenderPerformance = (this->Millis() - begin_timestamp);
    if (this->ShowFPS == true)
    {
        if (this->FPS_Label == NULL)
        {
            this->FPS_Label = this->PushPrimative(new EasyPrimative::Text({0, 0}, "0", 30));
            this->FPS_Label->properties->visable = false;
        } 
        else
        {
            this->FPS_Average.push_back(this->GetFramesPerSecond());
            if (this->FPS_Average.size() > 10)
            {
                float avg = 0;
                for (size_t x = 0; x < this->FPS_Average.size(); x++)
                {
                    avg += this->FPS_Average[x];
                }
                avg = avg / this->FPS_Average.size();
                this->FPS_Label->textval = std::to_string((int)avg);
                this->FPS_Label->properties->visable = true;
                this->FPS_Label->position.x = -(this->WindowSize[0] / 2.0f) + 10;
                this->FPS_Label->position.y = -(this->WindowSize[1] / 2.0f) + 10;
                this->FPS_Average.erase(this->FPS_Average.begin());
            }
        }
    }
    if (should_quit == true) return false;
    return !glfwWindowShouldClose(this->Window);
}
void EasyRender::Close()
{
    for (size_t x = 0; x < this->primative_stack.size(); x++)
    {
        this->primative_stack.at(x)->destroy();
        delete this->primative_stack.at(x);
    }
    for (size_t x = 0; x < this->timer_stack.size(); x++)
    {
        delete this->timer_stack.at(x);
        this->timer_stack.erase(this->timer_stack.begin()+x);
    }
    for (size_t x = 0; x < this->gui_stack.size(); x++)
    {
        delete this->gui_stack.at(x);
        this->gui_stack.erase(this->gui_stack.begin()+x);
    }
    for (size_t x = 0; x < this->event_stack.size(); x++)
    {
        delete this->event_stack.at(x);
        this->event_stack.erase(this->event_stack.begin()+x);
    }
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(this->Window);
    glfwTerminate();
    free(this->GuiIniFileName);
    free(this->GuiLogFileName);
}