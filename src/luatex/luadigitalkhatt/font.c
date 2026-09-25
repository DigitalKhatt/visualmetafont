#include "luadigitalkhatt.h"
#include <math.h>

static char instance_registry_key;

typedef struct {
  hb_buffer_t *buffer;
  unsigned length;
} dk_info_view_t;

/* The registry owns both the ID array and the interning map until Lua closes.
 * No font owns these states: buffer:get_glyphs() has no font argument. */
static void push_instance_store(lua_State *L) {
  lua_pushlightuserdata(L, &instance_registry_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  if (!lua_isnil(L, -1)) return;
  lua_pop(L, 1);
  lua_newtable(L);
  lua_newtable(L);
  lua_setfield(L, -2, "ids");
  lua_pushlightuserdata(L, &instance_registry_key);
  lua_pushvalue(L, -2);
  lua_rawset(L, LUA_REGISTRYINDEX);
}

dk_glyph_instance_t dk_get_instance(lua_State *L, uint32_t id) {
  dk_glyph_instance_t state = {0};
  if (!id) return state;
  push_instance_store(L);
  lua_rawgeti(L, -1, id);
  if (!lua_isuserdata(L, -1)) luaL_error(L, "Invalid glyph instance ID");
  memcpy(&state, lua_touserdata(L, -1), sizeof(state));
  lua_pop(L, 2);
  return state;
}

uint32_t dk_intern_instance(lua_State *L, const dk_glyph_instance_t *state) {
  /* Explicit serialization excludes struct padding from the interning key. */
  unsigned char key[2 * sizeof(double) + 3 * sizeof(uint32_t) + 1];
  unsigned char *p = key;
  double axes[2] = {state->left == 0 ? 0 : state->left, state->right == 0 ? 0 : state->right};
  uint32_t provenance[3] = {0};
  if (!isfinite(axes[0]) || !isfinite(axes[1])) luaL_error(L, "Non-finite glyph parameter");
  if (!axes[0] && !axes[1] && !state->has_positioning) return 0;
  if (state->has_positioning) {
    provenance[0] = state->positioning.lookup_index;
    provenance[1] = state->positioning.subtable_index;
    provenance[2] = state->positioning.base_codepoint;
  }
  memcpy(p, axes, sizeof(axes)); p += sizeof(axes);
  memcpy(p, provenance, sizeof(provenance)); p += sizeof(provenance);
  *p = !!state->has_positioning;
  push_instance_store(L);
  lua_getfield(L, -1, "ids");
  lua_pushlstring(L, (const char *)key, sizeof(key));
  lua_rawget(L, -2);
  if (!lua_isnil(L, -1)) {
    uint32_t id = (uint32_t)lua_tointeger(L, -1);
    lua_pop(L, 3);
    return id;
  }
  lua_pop(L, 1);
  size_t count = lua_rawlen(L, -2);
  if (count >= UINT32_MAX) luaL_error(L, "Glyph instance store exhausted");
  uint32_t id = (uint32_t)count + 1;
  memcpy(lua_newuserdata(L, sizeof(*state)), state, sizeof(*state));
  lua_rawseti(L, -3, id);
  lua_pushlstring(L, (const char *)key, sizeof(key));
  lua_pushinteger(L, id);
  lua_rawset(L, -3);
  lua_pop(L, 2);
  return id;
}

hb_bool_t dk_access_instance(hb_font_t *font, hb_glyph_instance_operation_t operation,
                            hb_glyph_info_t *info, void *payload, void *user_data) {
  lua_State *L = (lua_State *)user_data;
  dk_glyph_instance_t state = dk_get_instance(L, info->instance_id);
  (void)font;
  switch (operation) {
    case HB_INSTANCE_READ_TATWEELS: {
      hb_glyph_tatweels_t *value = (hb_glyph_tatweels_t *)payload;
      value->left = state.left; value->right = state.right; value->native_parameters = 0;
      return 1;
    }
    case HB_INSTANCE_WRITE_TATWEELS: {
      const hb_glyph_tatweels_t *value = (const hb_glyph_tatweels_t *)payload;
      state.left = value->left; state.right = value->right;
      break;
    }
    case HB_INSTANCE_READ_POSITIONING:
      memset(payload, 0, sizeof(hb_glyph_provenance_t));
      if (state.has_positioning) *(hb_glyph_provenance_t *)payload = state.positioning;
      return 1;
    case HB_INSTANCE_CLEAR_POSITIONING:
      state.has_positioning = 0;
      memset(&state.positioning, 0, sizeof(state.positioning));
      break;
    case HB_INSTANCE_WRITE_POSITIONING:
      state.positioning = *(hb_glyph_provenance_t *)payload;
      state.has_positioning = 1;
      break;
    default: return 0;
  }
  info->instance_id = dk_intern_instance(L, &state);
  return 1;
}

static int  setInfoField(lua_State* L) {
    dk_info_view_t *view = (dk_info_view_t *)luaL_checkudata(L, 1, "dkharfbuzz.InfoView");
    lua_Integer index = luaL_checkinteger(L, 2);
    luaL_argcheck(L, view->buffer != NULL, 1, "Glyph view is no longer active");
    luaL_argcheck(L, index >= 0 && (uint64_t)index < view->length, 2, "Glyph index out of range");
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(view->buffer, NULL);
    const char* filedName = luaL_checkstring(L, 3);    
    if (!strcmp(filedName,"lefttatweel")) {        
        double value = luaL_checknumber(L, 4);
        dk_glyph_instance_t state = dk_get_instance(L, info[index].instance_id);
        state.left = value;
        info[index].instance_id = dk_intern_instance(L, &state);
    }
    else if (!strcmp(filedName,"righttatweel")) {
        double value = luaL_checknumber(L, 4);
        dk_glyph_instance_t state = dk_get_instance(L, info[index].instance_id);
        state.right = value;
        info[index].instance_id = dk_intern_instance(L, &state);
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

    dk_info_view_t *infouserdata = (dk_info_view_t *)lua_newuserdata(L, sizeof(*infouserdata));
    luaL_newmetatable(L, "dkharfbuzz.InfoView");
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "infouserdata");
    infouserdata->buffer = context->buffer;
    infouserdata->length = len;
    /* Keep the view alive even if the callback removes it from its argument. */
    lua_getfield(L, -1, "infouserdata");
    int view_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_createtable(L, len, 0); // parent table

    for (int i = 0; i < len; i++) {
        lua_pushinteger(L, i + 1); // 1-indexed key parent table
        lua_createtable(L, 0, 4); // child table

        lua_pushinteger(L, info[i].codepoint);
        lua_setfield(L, -2, "codepoint");

        dk_glyph_instance_t state = dk_get_instance(L, info[i].instance_id);
        lua_pushnumber(L, state.left);
        lua_setfield(L, -2, "lefttatweel");

        lua_pushnumber(L, state.right);
        lua_setfield(L, -2, "righttatweel");

        lua_pushboolean(L, info[i].var1.u16[0] & 0x08u);
        lua_setfield(L, -2, "isMark");

        lua_settable(L, -3); // Add child table at index i+1 to parent table
    }

    lua_setfield(L, -2, "info");


    // Call the function with 1 arguments, returning 1 result
    int status = lua_pcall(L, 1, 1, 0);
    infouserdata->buffer = NULL;
    luaL_unref(L, LUA_REGISTRYINDEX, view_ref);
    if (status != 0) return lua_error(L);

    // Get the result 
    /* Preserve the legacy callback's zero return value: Lua updates parameters,
     * while this table's caller decides whether to replace the glyph. */

    // The one result that was returned needs to be popped off.  If the 3rd
    //  parameter to lua_call was larger than 1, we would need to pop off more
    //  elements from the lua stack.
    lua_pop(L, 2); /* result and digitalkhatt global */

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
  hb_font_set_instance_func(*f, dk_access_instance, L);

  //TODO TEMP
  hb_font_t* subfont = hb_font_create_sub_font(*f);

  hb_font_funcs_t* ffunctions = hb_font_funcs_create();

  hb_font_funcs_set_substitution_func(ffunctions, get_substitution, 0, 0);

  hb_font_set_funcs(subfont, ffunctions, L, 0);
  hb_font_funcs_destroy(ffunctions);
  hb_font_destroy(*f); /* The subfont retains its parent. */

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

  hb_glyph_info_t info = {0};
  info.codepoint = glyph;
  hb_position_t advance;

  hb_font_get_glyph_h_advances(*f, 1, &info.codepoint, sizeof(info), &advance, sizeof(advance));


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
    hb_font_funcs_destroy(ffunctions);
    hb_font_set_instance_func(*font, dk_access_instance, L);
    return 0;
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
