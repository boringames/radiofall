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

static const char* radiofall_dir() {
    const char *home_dir = get_home_dir();
    if (!home_dir) {
        return false;
    }
    return TextFormat("%s/.radiofall", home_dir);
}

bool data_init() {
    // const char *app_dir = GetApplicationDirectory();
    const char *radiofall_data_dir = radiofall_dir();
    if (!DirectoryExists(radiofall_data_dir)) {
        MakeDirectory(radiofall_data_dir);
    }

    return true;
}

i64 data_read_i64(const char *key) {
    const char *radiofall_data_dir = radiofall_dir();
    i32 sz = 0; u8 *data = LoadFileData(TextFormat("%s/%s", radiofall_data_dir, key), &sz);
    if (sz != sizeof(i64)) {
        RL_FREE(data);
        return -1;
    }

    i64 value = 0;
    mem_copy(&value, data, sizeof(i64));
    return value;
}

bool data_write_i64(const char *key, i64 value) {
    const char *radiofall_data_dir = radiofall_dir();
    i32 sz = 0; bool ok = SaveFileData(TextFormat("%s/%s", radiofall_data_dir, key), &value, sizeof(i64));
    return ok;
}