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
// Emscripten's dlopen() rejects mode=0 outright ("invalid mode for
// dlopen(): Either RTLD_LAZY or RTLD_NOW is required"), unlike native
// POSIX dlopen() implementations which tolerate it -- OtLayout.cpp's call
// site passes 0, so translate it here rather than touching the call site.
#define dlopen(x, y) dlopen(x, (y) ? (y) : RTLD_NOW)
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

