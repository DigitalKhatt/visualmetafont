/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/


#include "newmp.h"
#include "mp.c"




double static mp_get_numeric_internal(MP mp, char* n) {
  double ret = 0;
  size_t l = strlen(n);
  char err[256];
  const char* errid = NULL;
  if (l > 0) {
    mp_sym p = mp_id_lookup(mp, n, l, false);
    if (p == NULL) {
      errid = "variable does not exist";
    }
    else {
      if (eq_type(p) == mp_internal_quantity) {
        if ((internal_type(equiv(p)) == mp_known)) {
          ret = number_to_double(internal_value(equiv(p)));
        }
        else {
          errid = "value has the wrong type";
        }
      }
      else {
        errid = "variable is not an internal";
      }
    }
  }

  return ret;
}
mp_string static mp_get_string_internal(MP mp, char* n) {
  mp_string ret = "";
  size_t l = strlen(n);
  char err[256];
  const char* errid = NULL;
  if (l > 0) {
    mp_sym p = mp_id_lookup(mp, n, l, false);
    if (p == NULL) {
      errid = "variable does not exist";
    }
    else {
      if (eq_type(p) == mp_internal_quantity) {
        //if ((internal_type(equiv(p)) == mp_known)) {
        ret = internal_string(equiv(p));
        //}
        //else {
        //	errid = "value has the wrong type";
        //}
      }
      else {
        errid = "variable is not an internal";
      }
    }
  }

  return ret;
}
double static setParameters(MP mp, mp_edge_object* hh) {

  double codechar = hh->charcode;

  hh->charlt = mp_get_numeric_internal(mp, "charlt");

  hh->charrt = mp_get_numeric_internal(mp, "charrt");

  hh->lefttatweel = mp_get_numeric_internal(mp, "lefttatweel");

  hh->xleftanchor = hh->yleftanchor = hh->xrightanchor = hh->yrightanchor = NAN;

  hh->charname = mp_get_string_internal(mp, "charname")->str;

  hh->originalglyph = mp_get_string_internal(mp, "originalglyph")->str;

  hh->numAnchors = mp_get_numeric_internal(mp, "number_of_anchors");

  /*
  char* glyphsName = "glyphs";
  mp_sym glyphsNameSymp = mp_id_lookup(mp, glyphsName, strlen(glyphsName), false);
  mp_node p = mp_get_symbolic_node(mp);
  set_mp_sym_sym(p, glyphsNameSymp);
  mp_name_type(p) = 0;

  mp_number arg1;
  new_number(arg1);
  set_number_from_double(arg1, codechar);
  mp_node q = mp_new_num_tok(mp, arg1);
  free_number(arg1);

  mp_link(p) = q;

  char* varName = "leftanchor";
  mp_sym  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  mp_node r = mp_get_symbolic_node(mp);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;


  mp_link(q) = r;

  mp_node varnode = mp_find_variable(mp, p);

  mp_node value = value_node(varnode);
  if (mp_type(value) == mp_pair_node_type) {
    if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
      hh->xleftanchor = number_to_double(value_number(x_part(value)));
      hh->yleftanchor = number_to_double(value_number(y_part(value)));
    }

  }

  varName = "rightanchor";
  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;
  mp_link(q) = r;

  varnode = mp_find_variable(mp, p);

  value = value_node(varnode);
  if (mp_type(value) == mp_pair_node_type) {
    if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
      hh->xrightanchor = number_to_double(value_number(x_part(value)));
      hh->yrightanchor = number_to_double(value_number(y_part(value)));
    }

  }

  mp_flush_token_list(mp, p);*/

  char* varName = "leftanchor";
  mp_sym  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  mp_node r = mp_get_symbolic_node(mp);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;

  mp_node varnode = mp_find_variable(mp, r);

  mp_node value = value_node(varnode);
  if (value != NULL && mp_type(value) == mp_pair_node_type) {
    if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
      hh->xleftanchor = number_to_double(value_number(x_part(value)));
      hh->yleftanchor = number_to_double(value_number(y_part(value)));
    }

  }

  //mp_flush_token_list(mp, r);

  varName = "rightanchor";
  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;


  varnode = mp_find_variable(mp, r);

  value = value_node(varnode);
  if (value != NULL && mp_type(value) == mp_pair_node_type) {
    if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
      hh->xrightanchor = number_to_double(value_number(x_part(value)));
      hh->yrightanchor = number_to_double(value_number(y_part(value)));
    }

  }

  varName = "tr_";
  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;

  varnode = mp_find_variable(mp, r);

  value = value_node(varnode);
  if (value && mp_type(value) == mp_transform_node_type) {
    if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
      hh->xpart = number_to_double(value_number(x_part(value)));
      hh->ypart = number_to_double(value_number(y_part(value)));
    }

  }


  mp_flush_token_list(mp, r);

  setAnchors(mp, hh);


}
void mp_gr_toss_objects_extended(mp_edge_object* hh) {
  for (int i = 0; i < hh->numAnchors; i++) {
    mp_xfree(hh->anchors[i].anchorName);
  }
  mp_gr_toss_objects(hh);
}
void mymplib_shipout_backend(MP mp, void* voidh) {
  mp_edge_header_node h = (mp_edge_header_node)voidh;
  mp_edge_object* hh = mp_gr_export(mp, h);
  if (hh) {
    setParameters(mp, hh);
    mp_run_data* run = mp_rundata(mp);
    if (run->edges == NULL) {
      run->edges = hh;
    }
    else {
      mp_edge_object* p = run->edges;
      mp_edge_object* prev = NULL;
      bool exist = false;
      do {
        if (p->charcode == hh->charcode) {
          if (prev) {
            prev->next = hh;
          }
          else {
            run->edges = hh;
          }
          hh->next = p->next;
          mp_gr_toss_objects_extended(p);
          exist = true;
          break;
        }
        prev = p;
        p = p->next;
      } while (p);

      if (!exist) {
        prev->next = hh;
      }
    }
  }
}
void setAnchors(MP mp, mp_edge_object* hh) {

  if (hh->numAnchors == 0) return;


  char* varName = "anchors";
  mp_sym  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  mp_node r = mp_get_symbolic_node(mp);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;


  varName = "anchorname";
  mp_sym anchornameSymp = mp_id_lookup(mp, varName, strlen(varName), false);

  varName = "anctype";
  mp_sym typeSymp = mp_id_lookup(mp, varName, strlen(varName), false);

  varName = "pairvalue";
  mp_sym pairvalueSymp = mp_id_lookup(mp, varName, strlen(varName), false);

  mp_number arg2;
  new_number(arg2);

  for (int anchorIndex = 0; anchorIndex < hh->numAnchors; anchorIndex++) {

    set_number_from_double(arg2, anchorIndex);
    mp_node s = mp_new_num_tok(mp, arg2);

    mp_link(r) = s;


    mp_node t = mp_get_symbolic_node(mp);
    set_mp_sym_sym(t, anchornameSymp);
    mp_name_type(t) = 0;

    mp_link(s) = t;

    mp_node varnode = mp_find_variable(mp, r);

    if (mp_type(varnode) == mp_string_type) {
      hh->anchors[anchorIndex].anchorName = mp_xstrdup(mp, value_str(varnode)->str);
    }

    mp_flush_token_list(mp, t);


    t = mp_get_symbolic_node(mp);
    set_mp_sym_sym(t, typeSymp);
    mp_name_type(t) = 0;

    mp_link(s) = t;

    varnode = mp_find_variable(mp, r);

    if (mp_type(varnode) == mp_known) {
      hh->anchors[anchorIndex].type = number_to_double(value_number(varnode));
    }

    mp_flush_token_list(mp, t);

    t = mp_get_symbolic_node(mp);
    set_mp_sym_sym(t, pairvalueSymp);
    mp_name_type(t) = 0;

    mp_link(s) = t;

    varnode = mp_find_variable(mp, r);

    mp_node value = value_node(varnode);
    if (value && mp_type(value) == mp_pair_node_type) {
      if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
        hh->anchors[anchorIndex].x = number_to_double(value_number(x_part(value)));
        hh->anchors[anchorIndex].y = number_to_double(value_number(y_part(value)));
      }

    }

    mp_flush_token_list(mp, s);
  }

  mp_link(r) = NULL;
  mp_flush_token_list(mp, r);

  free_number(arg2);

}
static mp_node getTokenList(MP mp, char* varName) {
  mp_node root = mp_get_symbolic_node(mp);
  mp_node lastNode = root;

  bool firstToken = true;

  int i = 0;
  while (varName[i] != '\0') {    
    while (varName[i] == ' ') {
      i++;
    }
    int starti = i;
    while (isalpha(varName[i]) || varName[i] == '_') {
      i++;
    }
    if (starti != i) {
      // alpha
      mp_sym  varrSymp = mp_id_lookup(mp, varName + starti, i - starti, false);
      mp_node t = mp_get_symbolic_node(mp);
      set_mp_sym_sym(t, varrSymp);
      if (firstToken) {
        mp_name_type(t) = 0;
        firstToken = false;
      }
      else {
        mp_name_type(t) = mp_token;
      }
      
      mp_link(lastNode) = t;
      lastNode = t;
    }
    else {
      while (isdigit(varName[i]) || varName[i] == '.') {
        i++;
      }
      if (starti != i) {
        if ((i - starti) == 1 && varName[starti] == '.') continue; // . separator
        char* eptr;
        double result = strtod(varName + starti, &eptr);
        mp_number arg2;
        new_number(arg2);
        set_number_from_double(arg2, result);
        mp_node t = mp_new_num_tok(mp, arg2);
        free_number(arg2);
        mp_link(lastNode) = t;
        lastNode = t;
      }
      else {
        break;
      }
    }

  }


  mp_node ret = mp_link(root);
  mp_link(root) = NULL;
  mp_flush_token_list(mp, root);

  return ret;
}
bool getMPPairVariable(MP mp, char* varName, double* x, double* y) {
  bool ret = false;
  mp_node root = getTokenList(mp, varName);

  mp_node varnode = mp_find_variable(mp, root);

  if (varnode != NULL) {
    mp_node value = value_node(varnode);

    if (value && mp_type(value) == mp_pair_node_type) {
      if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
        *x = number_to_double(value_number(x_part(value)));
        *y = number_to_double(value_number(y_part(value)));
        ret = true;
      }
    }
  }

  mp_flush_token_list(mp, root);

  return ret;
}

bool getMPNumVariable(MP mp, char* varName, double* x) {
  bool ret = false;
  mp_node root = getTokenList(mp, varName);

  mp_node varnode = mp_find_variable(mp, root);

  if (varnode && mp_type(varnode) == mp_known) {
    *x = number_to_double(value_number(varnode));
    ret = true;
  }

  mp_flush_token_list(mp, root);

  return ret;
}

bool getMPStringVariable(MP mp, const char* varName, char** x) {
  bool ret = false;
  mp_node root = getTokenList(mp, varName);

  mp_node varnode = mp_find_variable(mp, root);

  if (varnode && mp_type(varnode) == mp_string_type) {
    *x = (char*)varnode->data.str->str;
    ret = true;
  }

  mp_flush_token_list(mp, root);

  return ret;
}

void getPointParam(MP mp, int index, double* x, double* y) {

  char* varName = "tmp_pair_params_";
  mp_sym  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  mp_node r = mp_get_symbolic_node(mp);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;

  mp_number arg2;
  new_number(arg2);

  set_number_from_double(arg2, index);
  mp_node s = mp_new_num_tok(mp, arg2);

  mp_link(r) = s;

  mp_node varnode = mp_find_variable(mp, r);
  mp_node value = value_node(varnode);

  if (mp_type(value) == mp_pair_node_type) {
    if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
      *x = number_to_double(value_number(x_part(value)));
      *y = number_to_double(value_number(y_part(value)));
    }

  }

  mp_flush_token_list(mp, r);

  free_number(arg2);

}
/*
AnchorPoint getAnchor(MP mp, int charcode, int anchorIndex) {
  AnchorPoint anchor = { NULL,0,0,0 };

  char* glyphsName = "glyphs";
  mp_sym glyphsNameSymp = mp_id_lookup(mp, glyphsName, strlen(glyphsName), false);
  mp_node p = mp_get_symbolic_node(mp);
  set_mp_sym_sym(p, glyphsNameSymp);
  mp_name_type(p) = 0;

  mp_number arg1;
  new_number(arg1);
  set_number_from_double(arg1, charcode);
  mp_node q = mp_new_num_tok(mp, arg1);
  free_number(arg1);

  mp_link(p) = q;

  char* varName = "anchors";
  mp_sym  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  mp_node r = mp_get_symbolic_node(mp);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;

  mp_link(q) = r;

  mp_number arg2;
  new_number(arg2);
  set_number_from_double(arg2, anchorIndex);
  mp_node s = mp_new_num_tok(mp, arg2);
  free_number(arg2);

  mp_link(r) = s;

  varName = "name";
  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  mp_node t = mp_get_symbolic_node(mp);
  set_mp_sym_sym(t, varrSymp);
  mp_name_type(t) = 0;

  mp_link(s) = t;

  mp_node varnode = mp_find_variable(mp, p);

  if (mp_type(varnode) == mp_string_type) {
    anchor.anchorName = value_str(varnode)->str;
  }

  mp_flush_token_list(mp, t);

  varName = "type";
  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  t = mp_get_symbolic_node(mp);
  set_mp_sym_sym(t, varrSymp);
  mp_name_type(t) = 0;

  mp_link(s) = t;

  varnode = mp_find_variable(mp, p);

  if (mp_type(varnode) == mp_known) {
    anchor.type = number_to_double(value_number(varnode));
  }

  mp_flush_token_list(mp, t);


  varName = "pairvalue";
  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  t = mp_get_symbolic_node(mp);
  set_mp_sym_sym(t, varrSymp);
  mp_name_type(t) = 0;

  mp_link(s) = t;

  varnode = mp_find_variable(mp, p);

  mp_node value = value_node(varnode);
  if (value && mp_type(value) == mp_pair_node_type) {
    if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
      anchor.x = number_to_double(value_number(x_part(value)));
      anchor.y = number_to_double(value_number(y_part(value)));
    }

  }

  mp_flush_token_list(mp, p);


  return anchor;
}*/
/*
unsigned int getTotalAnchors(MP mp, int charcode) {

  int totlaAnchors = 0;

  char* glyphsName = "glyphs";
  mp_sym glyphsNameSymp = mp_id_lookup(mp, glyphsName, strlen(glyphsName), false);
  mp_node p = mp_get_symbolic_node(mp);
  set_mp_sym_sym(p, glyphsNameSymp);
  mp_name_type(p) = 0;

  mp_number arg1;
  new_number(arg1);
  set_number_from_double(arg1, charcode);
  mp_node q = mp_new_num_tok(mp, arg1);
  free_number(arg1);

  mp_link(p) = q;

  char* varName = "number_of_anchors";
  mp_sym  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  mp_node r = mp_get_symbolic_node(mp);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;


  mp_link(q) = r;

  mp_node varnode = mp_find_variable(mp, p);

  if (mp_type(varnode) == mp_known) {
    totlaAnchors = number_to_double(value_number(varnode));
  }

  mp_flush_token_list(mp, p);

  return totlaAnchors;





}*/
/*
Transform getMatrix(MP mp, int charcode) {

  Transform ret;

  char* glyphsName = "glyphs";
  mp_sym glyphsNameSymp = mp_id_lookup(mp, glyphsName, strlen(glyphsName), false);
  mp_node p = mp_get_symbolic_node(mp);
  set_mp_sym_sym(p, glyphsNameSymp);
  mp_name_type(p) = 0;

  mp_number arg1;
  new_number(arg1);
  set_number_from_double(arg1, charcode);
  mp_node q = mp_new_num_tok(mp, arg1);
  free_number(arg1);

  mp_link(p) = q;

  char* varName = "matrix";
  mp_sym  varrSymp = mp_id_lookup(mp, varName, strlen(varName), false);
  mp_node r = mp_get_symbolic_node(mp);
  set_mp_sym_sym(r, varrSymp);
  mp_name_type(r) = 0;


  mp_link(q) = r;

  mp_node varnode = mp_find_variable(mp, p);

  mp_node value = value_node(varnode);
  if (value && mp_type(value) == mp_transform_node_type) {
    if (mp_type(x_part(value)) == mp_known && mp_type(y_part(value)) == mp_known) {
      ret.xpart = number_to_double(value_number(x_part(value)));
      ret.ypart = number_to_double(value_number(y_part(value)));
    }

  }

  mp_flush_token_list(mp, p);

  return ret;

}*/


