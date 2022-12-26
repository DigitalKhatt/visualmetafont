// harfbuzz.Feature
#include "luadigitalkhatt.h"

static int tag_new(lua_State *L) {
  Tag *t;

  t = (Tag *)lua_newuserdata(L, sizeof(*t));
  luaL_getmetatable(L, "dkharfbuzz.Tag");
  lua_setmetatable(L, -2);

  if (lua_gettop(L) == 1 || lua_isnil(L, -2))
    *t = HB_TAG_NONE;
  else
    *t = hb_tag_from_string(luaL_checkstring(L, -2), -1);

  return 1;
}

static int tag_to_string(lua_State *L) {
  Tag* t = (Tag *)luaL_checkudata(L, 1, "dkharfbuzz.Tag");
  char tag[5];

  hb_tag_to_string(*t, tag);
  tag[4] = '\0';
  lua_pushstring(L, tag);
  return 1;
}

static int tag_equals(lua_State *L) {
  Tag* lhs = (Tag *)luaL_checkudata(L, 1, "dkharfbuzz.Tag");
  Tag* rhs = (Tag *)luaL_checkudata(L, 2, "dkharfbuzz.Tag");

  lua_pushboolean(L, *lhs == *rhs);
  return 1;
}

static const struct luaL_Reg tag_methods[] = {
  { "__tostring", tag_to_string },
  { "__eq", tag_equals },
  { NULL, NULL }
};

static const struct luaL_Reg tag_functions[] = {
  { "new", tag_new },
  { NULL,  NULL }
};

int dk_register_tag(lua_State *L) {
  return  dk_register_class(L, "dkharfbuzz.Tag", tag_methods, tag_functions, NULL);
}
