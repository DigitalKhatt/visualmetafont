#pragma once

#if COMPILING_DLL
#define VMF_EXPORT __declspec(dllexport)
#else
#define VMF_EXPORT __declspec(dllimport)
#endif
#ifdef WIN32
#define SLPREFIX ""
#define SLEXT ".dll"
#define dlopen(x, y) LoadLibrary(x)
#define dlsym(x, y) (void*)GetProcAddress(x, y)
#define dlerror() GetLastError()
#define dlclose(x) FreeLibrary(x)
#define dlhandle HINSTANCE
#define DIGITALKHATT_DLOPEN_MODE 0
#elif defined(__EMSCRIPTEN__)
// Emscripten's own <dlfcn.h> provides real dlopen/dlsym/dlclose backed by
// its MAIN_MODULE/SIDE_MODULE dynamic-linking runtime (dylink.js) -- same
// calling convention as native POSIX, no macro changes needed there. Only
// the naming convention differs: side modules are plain .wasm files, not
// .so, so name them that way rather than relying on the generic branch
// below (which would silently produce ".so" since __APPLE__ isn't defined
// under Emscripten).
#include <dlfcn.h>
#define dlhandle void*
#define SLPREFIX "lib"
#define SLEXT ".wasm"
#define DIGITALKHATT_DLOPEN_MODE RTLD_NOW
#else
#include <dlfcn.h>
#define dlhandle void*
#define SLPREFIX "lib"
#ifdef __APPLE__
#define SLEXT ".dylib"
#else
#define SLEXT ".so"
#endif
#define DIGITALKHATT_DLOPEN_MODE RTLD_NOW
#endif

