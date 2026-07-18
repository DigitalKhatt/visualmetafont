#pragma once

#if defined(_WIN32)
#define DIGITALKHATT_FONT_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define DIGITALKHATT_FONT_EXPORT __attribute__((visibility("default")))
#else
#define DIGITALKHATT_FONT_EXPORT
#endif
