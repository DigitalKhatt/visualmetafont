#include "newmp.h"
#include "mp.c"

//Amine
double static mp_get_numeric_internal(MP mp, char*n) {
	double ret = 0;
	size_t l = strlen(n);
	char err[256];
	const char*errid = NULL;
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
mp_string static mp_get_string_internal(MP mp, char*n) {
	mp_string ret = "";
	size_t l = strlen(n);
	char err[256];
	const char*errid = NULL;
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
double static setParameters(MP mp, mp_edge_object*hh) {

	double codechar = hh->charcode;

	hh->lefttatweel = mp_get_numeric_internal(mp, "lefttatweel");

	hh->xleftanchor = hh->yleftanchor = hh->xrightanchor = hh->yrightanchor = NAN;

	hh->charname = mp_get_string_internal(mp, "charname")->str;

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

	mp_flush_token_list(mp, p);


}

static void mymplib_shipout_backend(MP mp, void*voidh) {
	mp_edge_header_node h = (mp_edge_header_node)voidh;
	mp_edge_object*hh = mp_gr_export(mp, h);
	if (hh) {
		setParameters(mp, hh);
		mp_run_data*run = mp_rundata(mp);
		if (run->edges == NULL) {
			run->edges = hh;
		}
		else {
			mp_edge_object*p = run->edges;
			mp_edge_object*prev = NULL;
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
					mp_gr_toss_objects(p);
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


