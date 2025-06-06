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
#else
#include <dlfcn.h>
#define dlhandle void*
#define SLPREFIX "lib"
#ifdef __APPLE__
#define SLEXT ".dylib"
#else
#define SLEXT ".so"
#endif
#endif

