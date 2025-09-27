#include "data.h"
#include "util.h"

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
  #include <shlobj.h>
#elif defined(__APPLE__)
  #include <TargetConditionals.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <pwd.h>
#elif defined(__ANDROID__)
  #include <unistd.h>
#elif defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#else
  #include <unistd.h>
  #include <stdlib.h>
  #include <pwd.h>
#endif


static const char* get_home_dir() {
#if defined(_WIN32) || defined(_WIN64)
    static char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        return path;
    }
    return "C:\\";

#elif defined(PLATOFM_WEB)
    return "/home/web_user";

#elif defined(__ANDROID__)
    const char* home = getenv("HOME");
    return home ? home : "/data/data/com.radiofall.app/files";
#elif defined(__APPLE__) && (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
    const char* home = getenv("HOME");
    return home ? home : "/";
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (home) return home;
    struct passwd* pw = getpwuid(getuid());
    return pw ? pw->pw_dir : "/";
#else
    const char* home = getenv("HOME");
    if (home) return home;
    struct passwd* pw = getpwuid(getuid());
    return pw ? pw->pw_dir : "/";
#endif
}

bool data_init() {
    // const char *app_dir = GetApplicationDirectory();
    const char *home_dir = get_home_dir();
    if (!home_dir) {
        return false;
    }
    const char *radiofall_data_dir = TextFormat("%s/.radiofall", home_dir);
    if (!DirectoryExists(radiofall_data_dir)) {
        MakeDirectory(radiofall_data_dir);
    }

    return true;
}

