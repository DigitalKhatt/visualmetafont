#ifndef GLAZEJSON_H
#define GLAZEJSON_H

#ifdef is_number
#pragma push_macro("is_number")
#undef is_number
#define DIGITALKHATT_RESTORE_IS_NUMBER
#endif
#include <glaze/json/generic.hpp>
#ifdef DIGITALKHATT_RESTORE_IS_NUMBER
#pragma pop_macro("is_number")
#undef DIGITALKHATT_RESTORE_IS_NUMBER
#endif

#endif
