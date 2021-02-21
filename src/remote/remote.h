#ifndef REMOTE_
#define REMOTE_

#include <Xrender.h>
#include <json/json.h>
#include <application.h>
#include <string>
#include <stdarg.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <dirent.h>
#include <ftw.h>
#include "net_skeleton/net_skeleton.h"

using namespace std;

class commands;

class command_t{
    public:
        std::string name;
        int (*cmd_func)(std::vector<std::string> args, void *this_pointer);
        command_t(std::string n, int (*c)(std::vector<std::string> args, void *this_pointer)){
            this->name = n;
            this->cmd_func = c;
        };
};

class commands{
    private:
        std::string cwd;
        struct ns_connection *nc;
        std::vector<command_t> command_stack;
    public:
        static int test(std::vector<std::string> args, void *this_pointer);
        static int quit(std::vector<std::string> args, void *this_pointer);
        static int exit(std::vector<std::string> args, void *this_pointer);
        static int clear(std::vector<std::string> args, void *this_pointer);
        static int dmesg(std::vector<std::string> args, void *this_pointer);
        static int ls(std::vector<std::string> args, void *this_pointer);
        static int cat(std::vector<std::string> args, void *this_pointer);
        static int dump_stack(std::vector<std::string> args, void *this_pointer);
        static int rm(std::vector<std::string> args, void *this_pointer);

        commands(struct ns_connection *nc_, std::string cmd)
        {
            this->command_stack.push_back(command_t("test", &this->test));
            this->command_stack.push_back(command_t("quit", &this->quit));
            this->command_stack.push_back(command_t("exit", &this->exit));
            this->command_stack.push_back(command_t("clear", &this->clear));
            this->command_stack.push_back(command_t("dmesg", &this->dmesg));
            this->command_stack.push_back(command_t("ls", &this->ls));
            this->command_stack.push_back(command_t("cat", &this->cat));
            this->command_stack.push_back(command_t("dump_stack", &this->dump_stack));
            this->command_stack.push_back(command_t("rm", &this->rm));

            this->nc = nc_;
            this->cwd = Xrender_get_config_dir("ncPilot");
            std::vector<std::string> args = this->split(cmd, ' ');
            for (int x = 0; x < command_stack.size(); x++)
            {
                if (command_stack.at(x).name == args[0])
                {
                    int ret = command_stack.at(x).cmd_func(args, this);
                    if (ret > 0) 
                    {
                        this->printf("Return: %d\n", ret);
                    }
                    break;
                }
            }
        }
        void printf(const char* Format, ... );
        std::vector<std::string> split(std::string str, char delimiter);
        
};

void remote_init();
void remote_tick();
void remote_close();

#endif //REMOTE_