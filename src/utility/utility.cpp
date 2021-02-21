#include <Xrender.h>
#include <json/json.h>
#include <utility/utility.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

bool utility_dir_exists(const char *path)
{
    struct stat info;
    if(stat( path, &info ) != 0)
    {
        return false;
    }
    else if(info.st_mode & S_IFDIR)
    {
        return true;
    }
    else
    {
        return false;
    }
}
