#include "luadigitalkhatt.h"

static int  setInfoField(lua_State* L) {
    hb_glyph_info_t** infouserdata = (hb_glyph_info_t**)lua_touserdata(L, 1);
    hb_glyph_info_t* info = *infouserdata;
    int index = luaL_checkinteger(L, 2);
    const char* filedName = luaL_checkstring(L, 3);    
    if (!strcmp(filedName,"lefttatweel")) {        
        double value = luaL_checknumber(L, 4);
        info[index].lefttatweel = value;
    }
    else if (!strcmp(filedName,"righttatweel")) {
        double value = luaL_checknumber(L, 4);
        info[index].righttatweel = value;
    }

    return 0;
    
}
static hb_bool_t get_substitution(hb_font_t* font, void* font_data,
    hb_substitution_context_t* context, void* user_data) {

    lua_State* L = (lua_State*)font_data;

    int digitalkhatt;
    lua_getglobal(L, "digitalkhatt");
    digitalkhatt = lua_gettop(L);    

    lua_getfield(L, digitalkhatt, "get_substitution");

    lua_createtable(L, 0, 4);

    lua_pushnumber(L, context->lookup_index);
    lua_setfield(L, -2, "lookup_index");

    lua_pushnumber(L, context->subtable_index);
    lua_setfield(L, -2, "subtable_index");

    lua_pushnumber(L, context->substitute);
    lua_setfield(L, -2, "substitute");

    lua_pushnumber(L, context->curr);
    lua_setfield(L, -2, "curr");

    lua_pushcfunction(L, setInfoField);
    lua_setfield(L, -2, "setInfoField");    

    unsigned int len = hb_buffer_get_length(context->buffer);
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(context->buffer, NULL);

    hb_glyph_info_t** infouserdata  = (hb_glyph_info_t **)lua_newuserdata(L, sizeof(hb_glyph_info_t*));
    lua_setfield(L, -2, "infouserdata");
    *infouserdata = info;

    lua_createtable(L, len, 0); // parent table

    for (int i = 0; i < len; i++) {
        lua_pushinteger(L, i + 1); // 1-indexed key parent table
        lua_createtable(L, 0, 4); // child table

        lua_pushinteger(L, info[i].codepoint);
        lua_setfield(L, -2, "codepoint");

        lua_pushnumber(L, info[i].lefttatweel);
        lua_setfield(L, -2, "lefttatweel");

        lua_pushnumber(L, info[i].righttatweel);
        lua_setfield(L, -2, "righttatweel");

        lua_pushboolean(L, info[i].var1.u16[0] & 0x08u);
        lua_setfield(L, -2, "isMark");

        lua_settable(L, -3); // Add child table at index i+1 to parent table
    }

    lua_setfield(L, -2, "info");


    // Call the function with 1 arguments, returning 1 result
    lua_call(L, 1, 1);

    // Get the result 
    hb_bool_t ret = (int)lua_tointeger(L, -1);

    // The one result that was returned needs to be popped off.  If the 3rd
    //  parameter to lua_call was larger than 1, we would need to pop off more
    //  elements from the lua stack.
    lua_pop(L, 1);

    return 0;


}

static int font_new(lua_State *L) {
  Font *f;
  Face *face = (Face * )luaL_checkudata(L, 1, "dkharfbuzz.Face");

  f = (Font *)lua_newuserdata(L, sizeof(*f));
  luaL_getmetatable(L, "dkharfbuzz.Font");
  lua_setmetatable(L, -2);

  *f = hb_font_create(*face);  

  // Set default scale to be the face's upem value
  unsigned int upem = hb_face_get_upem(*face);
  hb_font_set_scale(*f, upem, upem);

  // Set shaping functions to OpenType functions
  hb_ot_font_set_funcs(*f);

  //TODO TEMP
  hb_font_t* subfont = hb_font_create_sub_font(*f);

  hb_font_funcs_t* ffunctions = hb_font_funcs_create();

  hb_font_funcs_set_substitution_func(ffunctions, get_substitution, 0, 0);

  hb_font_set_funcs(subfont, ffunctions, L, 0);

  *f = subfont;

  return 1;
}

static int font_set_scale(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  int x_scale = luaL_checkinteger(L, 2);
  int y_scale = luaL_checkinteger(L, 3);

  hb_font_set_scale(*f, x_scale, y_scale);
  return 0;
}

static int font_get_scale(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  int x_scale, y_scale;

  hb_font_get_scale(*f, &x_scale, &y_scale);

  lua_pushinteger(L, x_scale);
  lua_pushinteger(L, y_scale);
  return 2;
}

static int font_get_h_extents(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  hb_font_extents_t extents;

  if (hb_font_get_h_extents(*f, &extents)) {
    lua_createtable(L, 0, 3);

    lua_pushnumber(L, extents.ascender);
    lua_setfield(L, -2, "ascender");

    lua_pushnumber(L, extents.descender);
    lua_setfield(L, -2, "descender");

    lua_pushnumber(L, extents.line_gap);
    lua_setfield(L, -2, "line_gap");
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int font_get_v_extents(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  hb_font_extents_t extents;

  if (hb_font_get_v_extents(*f, &extents)) {
    lua_createtable(L, 0, 3);

    lua_pushnumber(L, extents.ascender);
    lua_setfield(L, -2, "ascender");

    lua_pushnumber(L, extents.descender);
    lua_setfield(L, -2, "descender");

    lua_pushnumber(L, extents.line_gap);
    lua_setfield(L, -2, "line_gap");
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int font_get_glyph_extents(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  hb_codepoint_t glyph = luaL_checkinteger(L, 2);
  hb_glyph_extents_t extents;

  if (hb_font_get_glyph_extents(*f, glyph, &extents)) {
    lua_createtable(L, 0, 4);

    lua_pushnumber(L, extents.x_bearing);
    lua_setfield(L, -2, "x_bearing");

    lua_pushnumber(L, extents.y_bearing);
    lua_setfield(L, -2, "y_bearing");

    lua_pushnumber(L, extents.width);
    lua_setfield(L, -2, "width");

    lua_pushnumber(L, extents.height);
    lua_setfield(L, -2, "height");
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int font_get_glyph_name(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  hb_codepoint_t glyph = luaL_checkinteger(L, 2);

#define NAME_LEN 128
  char name[NAME_LEN];
  if (hb_font_get_glyph_name(*f, glyph, name, NAME_LEN))
    lua_pushstring(L, name);
  else
    lua_pushnil(L);
#undef NAME_LEN

  return 1;
}

static int font_get_glyph_from_name(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  const char *name = luaL_checkstring(L, 2);
  hb_codepoint_t glyph;

  if (hb_font_get_glyph_from_name(*f, name, -1, &glyph))
    lua_pushinteger(L, glyph);
  else
    lua_pushnil(L);

  return 1;
}

static int font_get_glyph_h_advance(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  hb_codepoint_t glyph = luaL_checkinteger(L, 2);

  hb_glyph_info_t info;
  info.codepoint = glyph;
  info.lefttatweel = 0.0;
  info.righttatweel = 0.0;
  hb_position_t advance;

  hb_font_get_glyph_h_advances(*f, 1, &info.codepoint, 0, &advance, 0);


  lua_pushinteger(L, advance);
  return 1;
}

static int font_get_glyph_v_advance(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  hb_codepoint_t glyph = luaL_checkinteger(L, 2);

  lua_pushinteger(L, hb_font_get_glyph_v_advance(*f, glyph));
  return 1;
}

static int font_get_nominal_glyph(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  hb_codepoint_t uni = luaL_checkinteger(L, 2);
  hb_codepoint_t glyph;

  if (hb_font_get_nominal_glyph(*f, uni, &glyph))
    lua_pushinteger(L, glyph);
  else
    lua_pushnil(L);

  return 1;
}


static int font_destroy(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");

  hb_font_destroy(*f);
  return 0;
}

static int font_ot_color_glyph_get_png(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "dkharfbuzz.Font");
  hb_codepoint_t gid = (hb_codepoint_t) luaL_checkinteger(L, 2);
  hb_blob_t* blob = hb_ot_color_glyph_reference_png(*f, gid);

  if (hb_blob_get_length(blob) != 0) {
    Blob *b = (Blob *)lua_newuserdata(L, sizeof(*b));
    luaL_getmetatable(L, "harfbuzz.Blob");
    lua_setmetatable(L, -2);

    *b = blob;
  } else {
    lua_pushnil(L);
  }

  return 1;
}

//Amine
static int font_set_variations(lua_State* L) {
    Font* font = (Font*)luaL_checkudata(L, 1, "dkharfbuzz.Font");
    unsigned int i;
    luaL_checktype(L, 2, LUA_TTABLE);

    unsigned int variations_length = lua_rawlen(L, 2);
    Variation* variations = (Variation*)malloc(variations_length * sizeof(hb_variation_t));    

    for (i = 0; i != variations_length; ++i) {
        lua_geti(L, 2, i + 1);
        Variation* f = (hb_variation_t*)luaL_checkudata(L, -1, "dkharfbuzz.Variation");
        variations[i] = *f;
        lua_pop(L, 1);
    }

    hb_font_set_variations(*font, variations, variations_length);

    free(variations);

    return 1;
}

static int font_get_var_coords_normalized(lua_State* L) {
    Font* f = (Font*)luaL_checkudata(L, 1, "dkharfbuzz.Font");

    unsigned int length = 0;

    const int* coord = hb_font_get_var_coords_normalized(*f, &length);

    lua_newtable(L);
    for (int i = 0; i < length; i++) {
        lua_pushnumber(L, i + 1);
        lua_pushnumber(L, coord[i]);
        lua_settable(L, -3);
    }

    
    return 1;
}
static int font_set_callback(lua_State* L) {

    Font* font = (Font*)luaL_checkudata(L, 1, "dkharfbuzz.Font");

    hb_font_funcs_t* ffunctions = hb_font_funcs_create();

    hb_font_funcs_set_substitution_func(ffunctions, get_substitution, 0, 0);
    
    hb_font_set_funcs(*font, ffunctions, L, 0);

}
static int font_create_sub_font(lua_State* L) {

    Font* font = (Font*)luaL_checkudata(L, 1, "dkharfbuzz.Font");

    Font* subfont;

    subfont = (Font*)lua_newuserdata(L, sizeof(*subfont));
    luaL_getmetatable(L, "dkharfbuzz.Font");
    lua_setmetatable(L, -2);

    *subfont = hb_font_create_sub_font(*font);

    //hb_font_funcs_t* ffunctions = hb_font_funcs_create();

    //hb_font_funcs_set_substitution_func(ffunctions, get_substitution, 0, 0);

    //hb_font_set_funcs(*font, ffunctions, L, 0);

    return 1;

}
static const struct luaL_Reg font_methods[] = {
  { "__gc", font_destroy },
  { "set_scale", font_set_scale },
  { "get_scale", font_get_scale },
  { "set_variations", font_set_variations },
  { "get_h_extents", font_get_h_extents },
  { "get_v_extents", font_get_v_extents },
  { "get_glyph_extents", font_get_glyph_extents },
  { "get_glyph_name", font_get_glyph_name },
  { "get_glyph_from_name", font_get_glyph_from_name },
  { "get_glyph_h_advance", font_get_glyph_h_advance },
  { "get_glyph_v_advance", font_get_glyph_v_advance },
  { "get_nominal_glyph", font_get_nominal_glyph },
  { "ot_color_glyph_get_png", font_ot_color_glyph_get_png },
  { "get_var_coords_normalized", font_get_var_coords_normalized },
  { "create_sub_font", font_create_sub_font },
  { "set_callback", font_set_callback },
  { NULL, NULL }
};

static const struct luaL_Reg font_functions[] = {
  { "new", font_new },
  { NULL,  NULL }
};

int dk_register_font(lua_State *L) {
  return dk_register_class(L, "dkharfbuzz.Font", font_methods, font_functions, NULL);
}

