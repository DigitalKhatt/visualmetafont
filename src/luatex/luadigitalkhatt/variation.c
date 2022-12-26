// harfbuzz.Feature
#include "luadigitalkhatt.h"

static int variation_new(lua_State *L) {
 
  Variation*fp = (Variation*)lua_newuserdata(L, sizeof(*fp));
  luaL_getmetatable(L, "dkharfbuzz.Variation");
  lua_setmetatable(L, -2);

  return 1;
}

static const char *variation_tag_ptr;
static const char *variation_value_ptr;

static int variation_index(lua_State *L) {
  Variation* f = (Variation*)luaL_checkudata(L, 1, "dkharfbuzz.Variation");
  const char *key = lua_tostring(L, 2);

  if (key == variation_tag_ptr) {
    Tag *tag = (Tag *)lua_newuserdata(L, sizeof(*tag));
    luaL_getmetatable(L, "dkharfbuzz.Tag");
    lua_setmetatable(L, -2);
    *tag = f->tag;
  } else if (key == variation_value_ptr) {
    lua_pushnumber(L, f->value);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

static int variation_newindex(lua_State *L) {
  Variation* f = (Variation*)luaL_checkudata(L, 1, "dkharfbuzz.Variation");
  const char *key = lua_tostring(L, 2);

  if (key == variation_tag_ptr) {
    f->tag = *(Tag *)luaL_checkudata(L, 3, "dkharfbuzz.Tag");
  } else if (key == variation_value_ptr) {
    f->value = luaL_checknumber(L, 3);
  } 
  return 0;
}

static const struct luaL_Reg variation_methods[] = {
  { "__index", variation_index },
  { "__newindex", variation_newindex },
  { NULL, NULL },
};

static const struct luaL_Reg variation_functions[] = {
  { "new", variation_new },
  { NULL,  NULL }
};

int dk_register_variation(lua_State *L) {
  lua_pushliteral(L, "tag");
  variation_tag_ptr = lua_tostring(L, -1);
  (void) luaL_ref (L, LUA_REGISTRYINDEX);
  lua_pushliteral(L, "value");
  variation_value_ptr = lua_tostring(L, -1);
  (void) luaL_ref (L, LUA_REGISTRYINDEX);  

  return dk_register_class(L, "dkharfbuzz.Variation", variation_methods, variation_functions, NULL);
}
