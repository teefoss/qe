#include "plat.h"
#include "qe.h"

#include <sys/stat.h>
#include <errno.h>

char * ApplicationDirectory(void)
{
    static char path[PATH_MAX] = { 0 };

    if ( path[0] == '\0') {
        const char * home = getenv("HOME");
        if ( home == NULL ) {
            DieGracefully("could not get user home directory\n");
        }

        strcpy(path, home);
        strcat(path, "/.qe");
    }

    return path;
}

bool ApplicationDirectoryExists(void)
{
    char * app_dir = ApplicationDirectory();
    struct stat info;
    if ( stat(app_dir, &info) != 0 ) {
        return false;
    } else if ( info.st_mode & S_IFDIR ) {
        return true;
    }

    // TODO: test this
    DieGracefully("Error: a file with the same name as the application "
                  "directory already exists -- %s\n", app_dir);
    return false;
}

void CreateApplicationDirectory(void)
{
    char * path = ApplicationDirectory();

    errno = 0;
    if ( mkdir(path, 0755) == -1 ) {
        if ( errno != EEXIST ) {
            DieGracefully("could not create application directory "
                          "'%s': (%s)", path, strerror(errno));
        }
    } else {
        printf("created directory '%s'\n", path);
    }
}
