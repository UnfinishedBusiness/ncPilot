#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "application.h"
#include "EasyRender/EasyRender.h"
#include "EasyRender/logging/loguru.h"
#include "EasyRender/gui/imgui.h"

global_variables_t *globals;

void log_uptime()
{
    unsigned long m = (EasyRender::Millis() - globals->start_timestamp);
    unsigned long seconds=(m/1000)%60;
    unsigned long minutes=(m/(1000*60))%60;
    unsigned long hours=(m/(1000*60*60))%24;
    LOG_F(INFO, "Shutting down, Adding Uptime: %luh %lum %lus to total", hours, minutes, seconds);
    std::ifstream uptime_file(globals->renderer->GetConfigDirectory() + "uptime.json");
    if (uptime_file.is_open())
    {
        std::string uptime_json_string((std::istreambuf_iterator<char>(uptime_file)), std::istreambuf_iterator<char>());
        nlohmann::json uptime_json = nlohmann::json::parse(uptime_json_string.c_str());
        try
        {
            hours += (unsigned long)uptime_json["hours"];
            minutes += (unsigned long)uptime_json["minutes"];
            seconds += (unsigned long)uptime_json["seconds"];
        }
        catch(...)
        {
            LOG_F(WARNING, "Error parsing uptime file!");
        }
    }
    uptime_file.close();
    nlohmann::json uptime;
    uptime["hours"] = hours;
    uptime["minutes"] = minutes;
    uptime["seconds"] = seconds;
    std::ofstream out(globals->renderer->GetConfigDirectory() + "uptime.json");
    out << uptime.dump();
    out.close();
}
int main(int argc, char **argv)
{
    globals = new global_variables_t;
    globals->quit = false;
    globals->zoom = 1;
    globals->pan.x = 0;
    globals->pan.y = 0;
    globals->start_timestamp = EasyRender::Millis();
    globals->renderer = new EasyRender();
    globals->nc_control_view = new ncControlView();
    globals->renderer->SetWindowTitle("ncPilot");
    globals->nc_control_view->PreInit();
    globals->renderer->SetGuiIniFileName(globals->renderer->GetConfigDirectory() + "gui.ini");
    globals->renderer->SetGuiLogFileName(globals->renderer->GetConfigDirectory() + "gui.log");
    globals->renderer->SetMainLogFileName(globals->renderer->GetConfigDirectory() + "ncPilot.log");
    globals->renderer->SetShowFPS(true);
    globals->renderer->SetWindowSize(globals->nc_control_view->preferences.window_size[0], globals->nc_control_view->preferences.window_size[1]);
    globals->renderer->Init(argc, argv);

    globals->nc_control_view->Init();

    globals->nc_control_view->MakeActive();

    while(globals->renderer->Poll(globals->quit))
    {
        if (globals->renderer->GetCurrentView() == "ncControlView")
        {
            globals->nc_control_view->Tick();
        }
    }
    log_uptime();
    globals->renderer->Close();
    globals->nc_control_view->Close();
    delete globals->renderer;
    delete globals->nc_control_view;
    delete globals;
    return 0;
}