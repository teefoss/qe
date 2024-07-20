#include "../plat.h"
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>

int __builtin_ctz(int mask) {
	int count = 0;
	int num_bits = sizeof(mask) * 8;
	
	for ( int i = 0; i < num_bits; i++ ) {
		if ( ((1 << i) & mask) == 0 ) {
			count++;
		} else {
			break;
		}
	}
	
	return count;
}

char * ApplicationDirectory(void)
{
    static char path[MAX_PATH] = { 0 };
    
    if ( path[0] == '\0' ) {
        HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
        if (result != S_OK) {
            fprintf(stderr, "Failed to get APPDATA path.\n");
            return NULL;
        }
    }
    
    return path;
}

bool ApplicationDirectoryExists(void)
{
    char * path = ApplicationDirectory();
    DWORD attr = GetFileAttributes(path);
    
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

void CreateApplicationDirectory(void)
{
    char * path = ApplicationDirectory();
    if ( !CreateDirectory(path, NULL) ) {
        DWORD error = GetLastError();
        if ( error == ERROR_ALREADY_EXISTS ) {
            puts("Application directory already exists");
        } else if ( error == ERROR_PATH_NOT_FOUND ) {
            printf("Could not create directory: path '%s' not found\n", path);
        }
    }
}