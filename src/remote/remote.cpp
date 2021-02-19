#include <application.h>
#include <remote/remote.h>
#include "net_skeleton/net_skeleton.h"
#include "logging/loguru.h"
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <Xrender.h>

int commands::test(std::vector<std::string> args, void *this_pointer)
{
    commands *self = static_cast<commands*>(this_pointer);
    self->printf("Test!\n");
    return 0;
}
int commands::quit(std::vector<std::string> args, void *this_pointer)
{
    commands *self = static_cast<commands*>(this_pointer);
    self->printf("Shutting down ncPilot!\n");
    globals->quit = true;
    return 0;
}
int commands::exit(std::vector<std::string> args, void *this_pointer)
{
    commands *self = static_cast<commands*>(this_pointer);
    self->printf("Bye!\n");
    ns_close_conn(self->nc);
    return 0;
}
int commands::clear(std::vector<std::string> args, void *this_pointer)
{
    commands *self = static_cast<commands*>(this_pointer);
    self->printf("\033[2J");
    return 0;
}
int commands::dmesg(std::vector<std::string> args, void *this_pointer)
{
    commands *self = static_cast<commands*>(this_pointer);
    std::string line;
    unsigned long count = 0;
    try
    {
        std::ifstream in(string(Xrender_get_config_dir("ncPilot") + "ncPilot.log"));
        while(std::getline(in, line))
        {
            if (args.size() == 4)
            {
                if (args[1] == "|" && args[2] == "grep")
                {
                    if (line.find(args[3]) != std::string::npos)
                    {
                        self->printf("%d> %s\n", count, line.c_str());
                    }
                }
            }
            else
            {
                self->printf("%d> %s\n", count, line.c_str());
            }
            count++;
        }
        in.close();
        return 0;
    }
    catch(...)
    {
        self->printf("Could not open log file!\n");
        return 1;
    }
}
int commands::ls(std::vector<std::string> args, void *this_pointer)
{
    commands *self = static_cast<commands*>(this_pointer);
    namespace fs = std::filesystem;
    std::string path = Xrender_get_config_dir("ncPilot");
    for (const auto & entry : fs::directory_iterator(path))
    {
        self->printf("%s\n", entry.path().c_str());
    }
    return 0;
}
int commands::cat(std::vector<std::string> args, void *this_pointer)
{
    commands *self = static_cast<commands*>(this_pointer);
    std::string line;
    unsigned long count = 0;
    if (args.size() > 1)
    {
        try
        {
            std::ifstream in(self->cwd + args[1]);
            while(std::getline(in, line))
            {
                if (args.size() == 5)
                {
                    if (args[2] == "|" && args[3] == "grep")
                    {
                        if (line.find(args[4]) != std::string::npos)
                        {
                            self->printf("%d> %s\n", count, line.c_str());
                        }
                    }
                }
                else
                {
                    self->printf("%d> %s\n", count, line.c_str());
                }
                count++;
            }
            in.close();
            return 0;
        }
        catch(...)
        {
            self->printf("Could not open log file!\n");
            return 1;
        } 
    }
    return 2;
}
int commands::dump_stack(std::vector<std::string> args, void *this_pointer)
{
    commands *self = static_cast<commands*>(this_pointer);
    std::vector<Xrender_object_t *> *stack = Xrender_get_object_stack();
    for (int x = 0; x < stack->size(); x++)
    {
        if (args.size() == 4)
        {
            if (args[1] == "|" && args[2] == "grep")
            {
                if (stack->at(x)->data.dump().find(args[3]) != std::string::npos)
                {
                    self->printf("%d> %s\n", x, stack->at(x)->data.dump().c_str());
                }
            }
        }
        else
        {
            self->printf("%d> %s\n", x, stack->at(x)->data.dump().c_str());
        }
    }
    return 0;
}



std::vector<std::string> commands::split(std::string str, char delimiter)
{ 
    vector<string> internal; 
    stringstream ss(str); // Turn the string into a stream. 
    string tok; 
    while(getline(ss, tok, delimiter))
    { 
        internal.push_back(tok); 
    } 
    return internal; 
}
void commands::printf(const char* Format, ...)
{
    char Buffer[1024];
    va_list args;
    va_start(args,Format);
    vsprintf(Buffer,Format,args);
    va_end(args);
    ns_printf(this->nc, "%s", Buffer);
}

struct ns_mgr mgr;

static void ev_handler(struct ns_connection *nc, int ev, void *ev_data)
{
    struct iobuf *io = &nc->recv_iobuf;
    if (ev == NS_RECV)
    {
        char src[60], dst[60];
        ns_sock_to_str(nc->sock, src, sizeof(src), 3);
        ns_sock_to_str(nc->sock, dst, sizeof(dst), 7);
        std::string buffer = string(io->buf, io->len);
        buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'),buffer.end());
        buffer.erase(std::remove(buffer.begin(), buffer.end(), '\r'),buffer.end());
        LOG_F(INFO, "Recieved (%s) => %s", src, buffer.c_str());
        commands(nc, buffer);
        ns_printf(nc, "ncPilot> ");
        //ns_send(nc, io->buf, io->len);  // Echo received data back
        iobuf_remove(io, io->len);      // Discard data from recv buffer
    }
    else if (ev == NS_ACCEPT) //Client Connected!
    {
        char src[60], dst[60];
        ns_sock_to_str(nc->sock, src, sizeof(src), 3);
        ns_sock_to_str(nc->sock, dst, sizeof(dst), 7);
        std::string buffer = string(io->buf, io->len);
        LOG_F(INFO, "Client Connected: src_ip=> %s dest_ip=> %s", src, dst);
        ns_printf(nc, "ncPilot> ");
    }
    else if (ev == NS_CLOSE) //Client Disconnected!
    {
        char src[60], dst[60];
        ns_sock_to_str(nc->sock, src, sizeof(src), 3);
        ns_sock_to_str(nc->sock, dst, sizeof(dst), 7);
        std::string buffer = string(io->buf, io->len);
        LOG_F(INFO, "Client Disconnected: src_ip=> %s dest_ip=> %s", src, dst);
    }
    else if (ev != 0)
    {
        LOG_F(INFO, "NS EVENT: %d", ev);
    }
}
void remote_init()
{
    ns_mgr_init(&mgr, NULL);
    ns_bind(&mgr, "1414", ev_handler);
}
void remote_tick()
{
    ns_mgr_poll(&mgr, 1);
}
void remote_close()
{
    ns_mgr_free(&mgr);
}