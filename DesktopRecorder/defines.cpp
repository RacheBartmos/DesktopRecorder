#include "defines.h"

namespace System_Helper {
HMODULE load_system_library(const char* name) {
    if (get_system_path() == false) return nullptr;
    char base_path[MAX_PATH] = { 0 };
    strcpy_s(base_path, system_path);
    strcat_s(base_path, "\\");
    strcat_s(base_path, name);
    HMODULE module = GetModuleHandleA(base_path);
    if (module) return module;
    module = LoadLibraryA(base_path);
    if (!module) printf("failed load system library :%lu", GetLastError());
    return module;
}

void free_system_library(HMODULE handle)
{
    FreeModule(handle);
}
};


