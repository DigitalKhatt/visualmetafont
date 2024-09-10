/*4:*/
#line 113 "../../../source/texk/web2c/mplibdir/mp.w"

#ifndef MPMP_H
#define MPMP_H 1
#include "avl.h"
#include "mplib.h"
#include <setjmp.h> 
typedef struct psout_data_struct*psout_data;
typedef struct svgout_data_struct*svgout_data;
typedef struct pngout_data_struct*pngout_data;
#ifndef HAVE_BOOLEAN
//VMF typedef int boolean;
#endif
#line 125 "../../../source/texk/web2c/mplibdir/mp.w"

#ifndef INTEGER_TYPE
typedef int integer;
#define MPOST_ABS abs
#else
#line 130 "../../../source/texk/web2c/mplibdir/mp.w"

#if INTEGER_MAX == LONG_MAX 
#ifdef HAVE_LABS
#define MPOST_ABS labs
#else
#line 135 "../../../source/texk/web2c/mplibdir/mp.w"
#define MPOST_ABS abs
#endif
#line 137 "../../../source/texk/web2c/mplibdir/mp.w"
#else
#line 138 "../../../source/texk/web2c/mplibdir/mp.w"
#define MPOST_ABS abs
#endif 
#line 140 "../../../source/texk/web2c/mplibdir/mp.w"
#endif 
#line 141 "../../../source/texk/web2c/mplibdir/mp.w"


/*171:*/
#line 2900 "../../../source/texk/web2c/mplibdir/mp.w"

extern void mp_xfree(void*x);
extern void*mp_xrealloc(MP mp,void*p,size_t nmem,size_t size);
extern void*mp_xmalloc(MP mp,size_t nmem,size_t size);
extern void mp_do_snprintf(char*str,int size,const char*fmt,...);
extern void*do_alloc_node(MP mp,size_t size);

/*:171*/
#line 143 "../../../source/texk/web2c/mplibdir/mp.w"

/*189:*/
#line 3301 "../../../source/texk/web2c/mplibdir/mp.w"

typedef enum{
mp_start_tex= 1,
mp_etex_marker,
mp_mpx_break,
mp_if_test,
mp_fi_or_else,
mp_input,
mp_iteration,
mp_repeat_loop,
mp_exit_test,
mp_relax,
mp_scan_tokens,
mp_runscript,
mp_maketext,
mp_expand_after,
mp_defined_macro,
mp_save_command,
mp_interim_command,
mp_let_command,
mp_new_internal,
mp_macro_def,
mp_ship_out_command,
mp_add_to_command,
mp_bounds_command,
mp_tfm_command,
mp_protection_command,
mp_show_command,
mp_mode_command,
mp_random_seed,
mp_message_command,
mp_every_job_command,
mp_delimiters,
mp_special_command,
mp_write_command,
mp_type_name,
mp_left_delimiter,
mp_begin_group,
mp_nullary,
mp_unary,
mp_str_op,
mp_void_op,
mp_cycle,
mp_primary_binary,
mp_capsule_token,
mp_string_token,
mp_internal_quantity,
mp_tag_token,
mp_numeric_token,
mp_plus_or_minus,
mp_tertiary_secondary_macro,
mp_tertiary_binary,
mp_left_brace,
mp_path_join,
mp_ampersand,
mp_expression_tertiary_macro,
mp_expression_binary,
mp_equals,
mp_and_command,
mp_secondary_primary_macro,
mp_slash,
mp_secondary_binary,
mp_param_type,
mp_controls,
mp_tension,
mp_at_least,
mp_curl_command,
mp_macro_special,
mp_right_delimiter,
mp_left_bracket,
mp_right_bracket,
mp_right_brace,
mp_with_option,
mp_thing_to_add,
mp_of_token,
mp_to_token,
mp_step_token,
mp_until_token,
mp_within_token,
mp_lig_kern_token,
mp_assignment,
mp_skip_to,
mp_bchar_label,
mp_double_colon,
mp_colon,

mp_comma,
mp_semicolon,
mp_end_group,
mp_stop,
mp_outer_tag,
mp_undefined_cs,
}mp_command_code;

/*:189*//*190:*/
#line 3409 "../../../source/texk/web2c/mplibdir/mp.w"

typedef enum{
mp_undefined= 0,
mp_vacuous,
mp_boolean_type,
mp_unknown_boolean,
mp_string_type,
mp_unknown_string,
mp_pen_type,
mp_unknown_pen,
mp_path_type,
mp_unknown_path,
mp_picture_type,
mp_unknown_picture,
mp_transform_type,
mp_color_type,
mp_cmykcolor_type,
mp_pair_type,
mp_numeric_type,
mp_known,
mp_dependent,
mp_proto_dependent,
mp_independent,
mp_token_list,
mp_structured,
mp_unsuffixed_macro,
mp_suffixed_macro,

mp_symbol_node,
mp_token_node_type,
mp_value_node_type,
mp_attr_node_type,
mp_subscr_node_type,
mp_pair_node_type,
mp_transform_node_type,
mp_color_node_type,
mp_cmykcolor_node_type,

mp_fill_node_type,
mp_stroked_node_type,
mp_text_node_type,
mp_start_clip_node_type,
mp_start_bounds_node_type,
mp_stop_clip_node_type,
mp_stop_bounds_node_type,
mp_dash_node_type,
mp_dep_node_type,
mp_if_node_type,
mp_edge_header_node_type,
}mp_variable_type;

/*:190*//*193:*/
#line 3624 "../../../source/texk/web2c/mplibdir/mp.w"

typedef enum{
mp_root= 0,
mp_saved_root,
mp_structured_root,
mp_subscr,
mp_attr,
mp_x_part_sector,
mp_y_part_sector,
mp_xx_part_sector,
mp_xy_part_sector,
mp_yx_part_sector,
mp_yy_part_sector,
mp_red_part_sector,
mp_green_part_sector,
mp_blue_part_sector,
mp_cyan_part_sector,
mp_magenta_part_sector,
mp_yellow_part_sector,
mp_black_part_sector,
mp_grey_part_sector,
mp_capsule,
mp_token,

mp_normal_sym,
mp_internal_sym,
mp_macro_sym,
mp_expr_sym,
mp_suffix_sym,
mp_text_sym,
/*194:*/
#line 3672 "../../../source/texk/web2c/mplibdir/mp.w"

mp_true_code,
mp_false_code,
mp_null_picture_code,
mp_null_pen_code,
mp_read_string_op,
mp_pen_circle,
mp_normal_deviate,
mp_read_from_op,
mp_close_from_op,
mp_odd_op,
mp_known_op,
mp_unknown_op,
mp_not_op,
mp_decimal,
mp_reverse,
mp_make_path_op,
mp_make_pen_op,
mp_oct_op,
mp_hex_op,
mp_ASCII_op,
mp_char_op,
mp_length_op,
mp_turning_op,
mp_color_model_part,
mp_x_part,
mp_y_part,
mp_xx_part,
mp_xy_part,
mp_yx_part,
mp_yy_part,
mp_red_part,
mp_green_part,
mp_blue_part,
mp_cyan_part,
mp_magenta_part,
mp_yellow_part,
mp_black_part,
mp_grey_part,
mp_font_part,
mp_text_part,
mp_path_part,
mp_pen_part,
mp_dash_part,
mp_prescript_part,
mp_postscript_part,
mp_sqrt_op,
mp_m_exp_op,
mp_m_log_op,
mp_sin_d_op,
mp_cos_d_op,
mp_floor_op,
mp_uniform_deviate,
mp_char_exists_op,
mp_font_size,
mp_ll_corner_op,
mp_lr_corner_op,
mp_ul_corner_op,
mp_ur_corner_op,
mp_arc_length,
mp_angle_op,
mp_cycle_op,
mp_filled_op,
mp_stroked_op,
mp_textual_op,
mp_clipped_op,
mp_bounded_op,
mp_plus,
mp_minus,
mp_times,
mp_over,
mp_pythag_add,
mp_pythag_sub,
mp_or_op,
mp_and_op,
mp_less_than,
mp_less_or_equal,
mp_greater_than,
mp_greater_or_equal,
mp_equal_to,
mp_unequal_to,
mp_concatenate,
mp_rotated_by,
mp_slanted_by,
mp_scaled_by,
mp_shifted_by,
mp_transformed_by,
mp_x_scaled,
mp_y_scaled,
mp_z_scaled,
mp_in_font,
mp_intersect,
mp_double_dot,
mp_substring_of,
mp_subpath_of,
mp_direction_time_of,
mp_point_of,
mp_precontrol_of,
mp_postcontrol_of,
mp_pen_offset_of,
mp_arc_time_of,
mp_version,
mp_envelope_of,
mp_boundingpath_of,
mp_glyph_infont,
mp_kern_flag,
mp_m_get_left_endpoint_op,
mp_m_get_right_endpoint_op,
mp_interval_set_op,


/*:194*/
#line 3654 "../../../source/texk/web2c/mplibdir/mp.w"

}mp_name_type_type;

/*:193*/
#line 144 "../../../source/texk/web2c/mplibdir/mp.w"

/*37:*/
#line 872 "../../../source/texk/web2c/mplibdir/mp.w"

typedef unsigned char ASCII_code;

/*:37*//*38:*/
#line 880 "../../../source/texk/web2c/mplibdir/mp.w"

typedef unsigned char text_char;

/*:38*//*45:*/
#line 968 "../../../source/texk/web2c/mplibdir/mp.w"

typedef unsigned char eight_bits;

/*:45*//*167:*/
#line 2843 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_value_node_data*mp_value_node;
typedef struct mp_node_data*mp_node;
typedef struct mp_symbol_entry*mp_sym;
typedef short quarterword;
typedef int halfword;
typedef struct{
integer scale;
integer serial;
}mp_independent_data;
typedef struct{
mp_independent_data indep;
mp_number n;
mp_string str;
mp_sym sym;
mp_node node;
mp_knot p;
}mp_value_data;
typedef struct{
mp_variable_type type;
mp_value_data data;
}mp_value;
typedef struct{
quarterword b0,b1,b2,b3;
}four_quarters;
typedef union{
integer sc;
four_quarters qqqq;
}font_data;


/*:167*//*196:*/
#line 4119 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_given_internal{
mp_output_template= 1,
mp_output_filename,
mp_output_format,
mp_output_format_options,
mp_number_system,
mp_number_precision,
mp_job_name,

mp_tracing_titles,
mp_tracing_equations,
mp_tracing_capsules,
mp_tracing_choices,
mp_tracing_specs,
mp_tracing_commands,
mp_tracing_restores,
mp_tracing_macros,
mp_tracing_output,
mp_tracing_stats,
mp_tracing_lost_chars,
mp_tracing_online,
mp_year,
mp_month,
mp_day,
mp_time,
mp_hour,
mp_minute,
mp_char_code,
mp_char_ext,
mp_char_wd,
mp_char_ht,
mp_char_dp,
mp_char_ic,
mp_design_size,
mp_pausing,
mp_showstopping,
mp_fontmaking,
mp_texscriptmode,
mp_linejoin,
mp_linecap,
mp_miterlimit,
mp_warning_check,
mp_boundary_char,
mp_prologues,
mp_true_corners,
mp_default_color_model,
mp_restore_clip_color,
mp_procset,
mp_hppp,
mp_vppp,
mp_gtroffmode,
};
typedef struct{
mp_value v;
char*intname;
}mp_internal;


/*:196*//*219:*/
#line 4719 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_symbol_entry{
halfword type;
mp_value v;
mp_string text;
void*parent;
}mp_symbol_entry;

/*:219*//*254:*/
#line 5703 "../../../source/texk/web2c/mplibdir/mp.w"

typedef enum{
mp_general_macro,
mp_primary_macro,
mp_secondary_macro,
mp_tertiary_macro,
mp_expr_macro,
mp_of_macro,
mp_suffix_macro,
mp_text_macro,
mp_expr_param,
mp_suffix_param,
mp_text_param
}mp_macro_info;

/*:254*//*296:*/
#line 6998 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_save_data{
quarterword type;
mp_internal value;
struct mp_save_data*link;
}mp_save_data;

/*:296*//*388:*/
#line 9389 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_bb_code{
mp_x_code= 0,
mp_y_code
};

/*:388*//*481:*/
#line 11778 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_dash_node_data*mp_dash_node;

/*:481*//*676:*/
#line 17631 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct{
char*long_name_field;
halfword start_field,loc_field,limit_field;
mp_node nstart_field,nloc_field;
mp_string name_field;
quarterword index_field;
}in_state_record;

/*:676*//*750:*/
#line 19267 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_subst_list_item{
mp_name_type_type info_mod;
quarterword value_mod;
mp_sym info;
halfword value_data;
struct mp_subst_list_item*link;
}mp_subst_list_item;

/*:750*//*828:*/
#line 21124 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_loop_data{
mp_sym var;
mp_node info;
mp_node type;

mp_node list;
mp_node list_start;
mp_number old_value;
mp_number value;
mp_number step_size;
mp_number final_value;
struct mp_loop_data*link;
}mp_loop_data;

/*:828*//*898:*/
#line 22367 "../../../source/texk/web2c/mplibdir/mp.w"

typedef unsigned int readf_index;
typedef unsigned int write_index;

/*:898*//*1062:*/
#line 30583 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct File{
FILE*f;
}File;

/*:1062*//*1224:*/
#line 34330 "../../../source/texk/web2c/mplibdir/mp.w"

typedef unsigned int font_number;

/*:1224*/
#line 145 "../../../source/texk/web2c/mplibdir/mp.w"

/*28:*/
#line 785 "../../../source/texk/web2c/mplibdir/mp.w"

#define bistack_size 1500       


/*:28*/
#line 146 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct MP_instance{
/*30:*/
#line 799 "../../../source/texk/web2c/mplibdir/mp.w"

int error_line;
int half_error_line;

int halt_on_error;
int max_print_line;
void*userdata;
char*banner;
int ini_version;
int utf8_mode;

/*:30*//*47:*/
#line 998 "../../../source/texk/web2c/mplibdir/mp.w"

mp_file_finder find_file;
mp_file_opener open_file;
mp_script_runner run_script;
mp_text_maker make_text;
mp_file_reader read_ascii_file;
mp_binfile_reader read_binary_file;
mp_file_closer close_file;
mp_file_eoftest eof_file;
mp_file_flush flush_file;
mp_file_writer write_ascii_file;
mp_binfile_writer write_binary_file;

/*:47*//*54:*/
#line 1081 "../../../source/texk/web2c/mplibdir/mp.w"

int print_found_names;

/*:54*//*56:*/
#line 1099 "../../../source/texk/web2c/mplibdir/mp.w"

int file_line_error_style;

/*:56*//*72:*/
#line 1343 "../../../source/texk/web2c/mplibdir/mp.w"

char*command_line;

/*:72*//*105:*/
#line 1935 "../../../source/texk/web2c/mplibdir/mp.w"

int interaction;
int noninteractive;
int extensions;

/*:105*//*125:*/
#line 2170 "../../../source/texk/web2c/mplibdir/mp.w"

mp_editor_cmd run_editor;

/*:125*//*157:*/
#line 2671 "../../../source/texk/web2c/mplibdir/mp.w"

int random_seed;

/*:157*//*169:*/
#line 2888 "../../../source/texk/web2c/mplibdir/mp.w"

int math_mode;

/*:169*//*199:*/
#line 4207 "../../../source/texk/web2c/mplibdir/mp.w"

int troff_mode;

/*:199*//*860:*/
#line 21761 "../../../source/texk/web2c/mplibdir/mp.w"

char*mem_name;

/*:860*//*872:*/
#line 21956 "../../../source/texk/web2c/mplibdir/mp.w"

char*job_name;

/*:872*//*893:*/
#line 22320 "../../../source/texk/web2c/mplibdir/mp.w"

mp_makempx_cmd run_make_mpx;

/*:893*//*1275:*/
#line 35345 "../../../source/texk/web2c/mplibdir/mp.w"

mp_backend_writer shipout_backend;

/*:1275*/
#line 148 "../../../source/texk/web2c/mplibdir/mp.w"

/*18:*/
#line 420 "../../../source/texk/web2c/mplibdir/mp.w"

void*math;

/*:18*//*29:*/
#line 792 "../../../source/texk/web2c/mplibdir/mp.w"

int pool_size;

int max_in_open;

int param_size;

/*:29*//*33:*/
#line 835 "../../../source/texk/web2c/mplibdir/mp.w"

integer bad;

/*:33*//*41:*/
#line 894 "../../../source/texk/web2c/mplibdir/mp.w"

ASCII_code xord[256];
text_char xchr[256];

/*:41*//*53:*/
#line 1074 "../../../source/texk/web2c/mplibdir/mp.w"

char*name_of_file;

/*:53*//*66:*/
#line 1241 "../../../source/texk/web2c/mplibdir/mp.w"

size_t buf_size;

ASCII_code*buffer;
size_t first;
size_t last;
size_t max_buf_stack;

/*:66*//*71:*/
#line 1315 "../../../source/texk/web2c/mplibdir/mp.w"

void*term_in;
void*term_out;
void*err_out;

/*:71*//*79:*/
#line 1479 "../../../source/texk/web2c/mplibdir/mp.w"

avl_tree strings;
unsigned char*cur_string;
size_t cur_length;
size_t cur_string_size;

/*:79*//*82:*/
#line 1493 "../../../source/texk/web2c/mplibdir/mp.w"

integer pool_in_use;
integer max_pl_used;
integer strs_in_use;
integer max_strs_used;


/*:82*//*83:*/
#line 1552 "../../../source/texk/web2c/mplibdir/mp.w"

void*log_file;
void*output_file;
unsigned int selector;
integer tally;
unsigned int term_offset;

unsigned int file_offset;

ASCII_code*trick_buf;
integer trick_count;
integer first_count;

/*:83*//*111:*/
#line 2011 "../../../source/texk/web2c/mplibdir/mp.w"

int history;
int error_count;

/*:111*//*115:*/
#line 2040 "../../../source/texk/web2c/mplibdir/mp.w"

boolean use_err_help;
mp_string err_help;

/*:115*//*117:*/
#line 2056 "../../../source/texk/web2c/mplibdir/mp.w"

jmp_buf*jump_buf;

/*:117*//*144:*/
#line 2509 "../../../source/texk/web2c/mplibdir/mp.w"

integer interrupt;
boolean OK_to_interrupt;
integer run_state;
boolean finished;
boolean reading_preload;

/*:144*//*148:*/
#line 2568 "../../../source/texk/web2c/mplibdir/mp.w"

boolean arith_error;

/*:148*//*156:*/
#line 2667 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number randoms[55];
int j_random;

/*:156*//*172:*/
#line 2915 "../../../source/texk/web2c/mplibdir/mp.w"

mp_node token_nodes;
int num_token_nodes;
mp_node pair_nodes;
int num_pair_nodes;
mp_knot knot_nodes;
int num_knot_nodes;
mp_node value_nodes;
int num_value_nodes;
mp_node symbolic_nodes;
int num_symbolic_nodes;

/*:172*//*179:*/
#line 3071 "../../../source/texk/web2c/mplibdir/mp.w"

size_t var_used;
size_t var_used_max;

/*:179*//*185:*/
#line 3215 "../../../source/texk/web2c/mplibdir/mp.w"

mp_dash_node null_dash;
mp_value_node dep_head;
mp_node inf_val;
mp_node zero_val;
mp_node temp_val;
mp_node end_attr;
mp_node bad_vardef;
mp_node temp_head;
mp_node hold_head;
mp_node spec_head;

/*:185*//*198:*/
#line 4202 "../../../source/texk/web2c/mplibdir/mp.w"

mp_internal*internal;
int int_ptr;
int max_internal;

/*:198*//*212:*/
#line 4538 "../../../source/texk/web2c/mplibdir/mp.w"

unsigned int old_setting;

/*:212*//*214:*/
#line 4580 "../../../source/texk/web2c/mplibdir/mp.w"

#define digit_class 0 
int char_class[256];

/*:214*//*220:*/
#line 4727 "../../../source/texk/web2c/mplibdir/mp.w"

integer st_count;
avl_tree symbols;
avl_tree frozen_symbols;
mp_sym frozen_bad_vardef;
mp_sym frozen_colon;
mp_sym frozen_end_def;
mp_sym frozen_end_for;
mp_sym frozen_end_group;
mp_sym frozen_etex;
mp_sym frozen_fi;
mp_sym frozen_inaccessible;
mp_sym frozen_left_bracket;
mp_sym frozen_mpx_break;
mp_sym frozen_repeat_loop;
mp_sym frozen_right_delimiter;
mp_sym frozen_semicolon;
mp_sym frozen_slash;
mp_sym frozen_undefined;
mp_sym frozen_dump;


/*:220*//*229:*/
#line 4856 "../../../source/texk/web2c/mplibdir/mp.w"

mp_sym id_lookup_test;

/*:229*//*297:*/
#line 7005 "../../../source/texk/web2c/mplibdir/mp.w"

mp_save_data*save_ptr;

/*:297*//*331:*/
#line 7705 "../../../source/texk/web2c/mplibdir/mp.w"

mp_knot path_tail;

/*:331*//*346:*/
#line 8027 "../../../source/texk/web2c/mplibdir/mp.w"

int path_size;
mp_number*delta_x;
mp_number*delta_y;
mp_number*delta;
mp_number*psi;

/*:346*//*351:*/
#line 8182 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number*theta;
mp_number*uu;
mp_number*vv;
mp_number*ww;

/*:351*//*372:*/
#line 8790 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number st;
mp_number ct;
mp_number sf;
mp_number cf;

/*:372*//*389:*/
#line 9401 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number bbmin[mp_y_code+1];
mp_number bbmax[mp_y_code+1];


/*:389*//*435:*/
#line 10657 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number half_cos[8];
mp_number d_cos[8];

/*:435*//*451:*/
#line 10982 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number cur_x;
mp_number cur_y;

/*:451*//*546:*/
#line 13356 "../../../source/texk/web2c/mplibdir/mp.w"

integer spec_offset;

/*:546*//*548:*/
#line 13518 "../../../source/texk/web2c/mplibdir/mp.w"

mp_knot spec_p1;
mp_knot spec_p2;

/*:548*//*607:*/
#line 15708 "../../../source/texk/web2c/mplibdir/mp.w"

unsigned int tol_step;

/*:607*//*608:*/
#line 15775 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number*bisect_stack;
integer bisect_ptr;

/*:608*//*613:*/
#line 15862 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number cur_t;
mp_number cur_tt;
integer time_to_go;
mp_number max_t;

/*:613*//*617:*/
#line 16012 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number delx;
mp_number dely;
integer tol;
integer uv;
integer xy;
integer three_l;
mp_number appr_t;
mp_number appr_tt;

/*:617*//*626:*/
#line 16246 "../../../source/texk/web2c/mplibdir/mp.w"

integer serial_no;

/*:626*//*637:*/
#line 16444 "../../../source/texk/web2c/mplibdir/mp.w"

boolean fix_needed;
boolean watch_coefs;
mp_value_node dep_final;

/*:637*//*670:*/
#line 17578 "../../../source/texk/web2c/mplibdir/mp.w"

mp_node cur_mod_;

/*:670*//*677:*/
#line 17640 "../../../source/texk/web2c/mplibdir/mp.w"

in_state_record*input_stack;
integer input_ptr;
integer max_in_stack;
in_state_record cur_input;
int stack_size;

/*:677*//*682:*/
#line 17747 "../../../source/texk/web2c/mplibdir/mp.w"

integer in_open;
integer in_open_max;
unsigned int open_parens;
void**input_file;
integer*line_stack;
char**inext_stack;
char**iname_stack;
char**iarea_stack;
mp_string*mpx_name;

/*:682*//*688:*/
#line 17876 "../../../source/texk/web2c/mplibdir/mp.w"

mp_node*param_stack;
integer param_ptr;
integer max_param_stack;

/*:688*//*694:*/
#line 17931 "../../../source/texk/web2c/mplibdir/mp.w"

integer file_ptr;

/*:694*//*722:*/
#line 18531 "../../../source/texk/web2c/mplibdir/mp.w"

#define tex_flushing 7 
integer scanner_status;
mp_sym warning_info;

integer warning_line;
mp_node warning_info_node;

/*:722*//*733:*/
#line 18897 "../../../source/texk/web2c/mplibdir/mp.w"

boolean force_eof;

/*:733*//*765:*/
#line 19712 "../../../source/texk/web2c/mplibdir/mp.w"

mp_sym bg_loc;
mp_sym eg_loc;

/*:765*//*769:*/
#line 19766 "../../../source/texk/web2c/mplibdir/mp.w"

int expand_depth_count;
int expand_depth;

/*:769*//*814:*/
#line 20841 "../../../source/texk/web2c/mplibdir/mp.w"

mp_node cond_ptr;
integer if_limit;
quarterword cur_if;
integer if_line;

/*:814*//*829:*/
#line 21139 "../../../source/texk/web2c/mplibdir/mp.w"

mp_loop_data*loop_ptr;

/*:829*//*848:*/
#line 21586 "../../../source/texk/web2c/mplibdir/mp.w"

char*cur_name;
char*cur_area;
char*cur_ext;

/*:848*//*851:*/
#line 21614 "../../../source/texk/web2c/mplibdir/mp.w"

integer area_delimiter;

integer ext_delimiter;
boolean quoted_filename;

/*:851*//*871:*/
#line 21952 "../../../source/texk/web2c/mplibdir/mp.w"

boolean log_opened;
char*log_name;

/*:871*//*899:*/
#line 22371 "../../../source/texk/web2c/mplibdir/mp.w"

readf_index max_read_files;
void**rd_file;
char**rd_fname;
readf_index read_files;
write_index max_write_files;
void**wr_file;
char**wr_fname;
write_index write_files;

/*:899*//*905:*/
#line 22514 "../../../source/texk/web2c/mplibdir/mp.w"

mp_value cur_exp;

/*:905*//*931:*/
#line 23442 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number max_c[mp_proto_dependent+1];
mp_value_node max_ptr[mp_proto_dependent+1];
mp_value_node max_link[mp_proto_dependent+1];


/*:931*//*934:*/
#line 23483 "../../../source/texk/web2c/mplibdir/mp.w"

int var_flag;

/*:934*//*990:*/
#line 27253 "../../../source/texk/web2c/mplibdir/mp.w"

mp_string eof_line;

/*:990*//*1003:*/
#line 28441 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number txx;
mp_number txy;
mp_number tyx;
mp_number tyy;
mp_number tx;
mp_number ty;

/*:1003*//*1061:*/
#line 30576 "../../../source/texk/web2c/mplibdir/mp.w"

mp_run_data run_data;

/*:1061*//*1129:*/
#line 32200 "../../../source/texk/web2c/mplibdir/mp.w"

quarterword last_add_type;


/*:1129*//*1139:*/
#line 32458 "../../../source/texk/web2c/mplibdir/mp.w"

mp_sym start_sym;

/*:1139*//*1148:*/
#line 32588 "../../../source/texk/web2c/mplibdir/mp.w"

boolean long_help_seen;

/*:1148*//*1156:*/
#line 32745 "../../../source/texk/web2c/mplibdir/mp.w"

void*tfm_file;
char*metric_file_name;

/*:1156*//*1165:*/
#line 33004 "../../../source/texk/web2c/mplibdir/mp.w"

#define TFM_ITEMS 257
eight_bits bc;
eight_bits ec;
mp_node tfm_width[TFM_ITEMS];
mp_node tfm_height[TFM_ITEMS];
mp_node tfm_depth[TFM_ITEMS];
mp_node tfm_ital_corr[TFM_ITEMS];
boolean char_exists[TFM_ITEMS];
int char_tag[TFM_ITEMS];
int char_remainder[TFM_ITEMS];
char*header_byte;
int header_last;
int header_size;
four_quarters*lig_kern;
short nl;
mp_number*kern;
short nk;
four_quarters exten[TFM_ITEMS];
short ne;
mp_number*param;
short np;
short nw;
short nh;
short nd;
short ni;
short skip_table[TFM_ITEMS];
boolean lk_started;
integer bchar;
short bch_label;
short ll;
short lll;
short label_loc[257];
eight_bits label_char[257];
short label_ptr;

/*:1165*//*1195:*/
#line 33715 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number perturbation;
integer excess;

/*:1195*//*1203:*/
#line 33864 "../../../source/texk/web2c/mplibdir/mp.w"

mp_node dimen_head[5];

/*:1203*//*1209:*/
#line 34011 "../../../source/texk/web2c/mplibdir/mp.w"

mp_number max_tfm_dimen;
integer tfm_changed;

/*:1209*//*1223:*/
#line 34321 "../../../source/texk/web2c/mplibdir/mp.w"

void*tfm_infile;

/*:1223*//*1225:*/
#line 34337 "../../../source/texk/web2c/mplibdir/mp.w"

font_number font_max;
size_t font_mem_size;
font_data*font_info;
char**font_enc_name;
boolean*font_ps_name_fixed;
size_t next_fmem;
font_number last_fnum;
integer*font_dsize;
char**font_name;
char**font_ps_name;
font_number last_ps_fnum;
eight_bits*font_bc;
eight_bits*font_ec;
int*char_base;
int*width_base;
int*height_base;
int*depth_base;
mp_node*font_sizes;

/*:1225*//*1245:*/
#line 34659 "../../../source/texk/web2c/mplibdir/mp.w"

integer ten_pow[10];
integer scaled_out;

/*:1245*//*1253:*/
#line 34928 "../../../source/texk/web2c/mplibdir/mp.w"

char*first_file_name;
char*last_file_name;
integer first_output_code;
integer last_output_code;

integer total_shipped;

/*:1253*//*1261:*/
#line 35001 "../../../source/texk/web2c/mplibdir/mp.w"

mp_node last_pending;


/*:1261*//*1277:*/
#line 35351 "../../../source/texk/web2c/mplibdir/mp.w"

psout_data ps;
svgout_data svg;
pngout_data png;

/*:1277*//*1280:*/
#line 35378 "../../../source/texk/web2c/mplibdir/mp.w"

void*mem_file;

/*:1280*/
#line 149 "../../../source/texk/web2c/mplibdir/mp.w"

}MP_instance;
/*14:*/
#line 374 "../../../source/texk/web2c/mplibdir/mp.w"

/*867:*/
#line 21908 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_str_scan_file(MP mp,mp_string s);

/*:867*//*869:*/
#line 21928 "../../../source/texk/web2c/mplibdir/mp.w"

extern void mp_ptr_scan_file(MP mp,char*s);

/*:869*/
#line 375 "../../../source/texk/web2c/mplibdir/mp.w"



/*:14*//*89:*/
#line 1623 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_print(MP mp,const char*s);
void mp_printf(MP mp,const char*ss,...);
void mp_print_ln(MP mp);
void mp_print_char(MP mp,ASCII_code k);
void mp_print_str(MP mp,mp_string s);
void mp_print_nl(MP mp,const char*s);
void mp_print_two(MP mp,mp_number x,mp_number y);

/*:89*//*99:*/
#line 1853 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_print_int(MP mp,integer n);
void mp_print_pointer(MP mp,void*n);

/*:99*//*114:*/
#line 2037 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_normalize_selector(MP mp);

/*:114*//*119:*/
#line 2070 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_jump_out(MP mp);

/*:119*//*140:*/
#line 2437 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_overflow(MP mp,const char*s,integer n);


/*:140*//*142:*/
#line 2468 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_confusion(MP mp,const char*s);

/*:142*//*160:*/
#line 2691 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_new_randoms(MP mp);

/*:160*//*177:*/
#line 3024 "../../../source/texk/web2c/mplibdir/mp.w"


#define mp_snprintf(...) (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

/*:177*//*184:*/
#line 3204 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_free_node(MP mp,mp_node p,size_t siz);
void mp_free_symbolic_node(MP mp,mp_node p);
void mp_free_value_node(MP mp,mp_node p);

/*:184*//*335:*/
#line 7811 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_make_choices(MP mp,mp_knot knots);

/*:335*//*859:*/
#line 21758 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_pack_file_name(MP mp,const char*n,const char*a,const char*e);

/*:859*//*876:*/
#line 22001 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_pack_job_name(MP mp,const char*s);

/*:876*//*878:*/
#line 22023 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_prompt_file_name(MP mp,const char*s,const char*e);

/*:878*//*1095:*/
#line 31292 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_grow_internals(MP mp,int l);

/*:1095*//*1229:*/
#line 34420 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_reallocate_fonts(MP mp,font_number l);


/*:1229*//*1248:*/
#line 34683 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_open_output_file(MP mp);
char*mp_get_output_file_name(MP mp);
char*mp_set_output_file_name(MP mp,integer c);

/*:1248*//*1251:*/
#line 34909 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_store_true_output_filename(MP mp,int c);

/*:1251*//*1259:*/
#line 34989 "../../../source/texk/web2c/mplibdir/mp.w"

boolean mp_has_font_size(MP mp,font_number f);

/*:1259*/
#line 151 "../../../source/texk/web2c/mplibdir/mp.w"

/*8:*/
#line 269 "../../../source/texk/web2c/mplibdir/mp.w"


#if DEBUG
#define debug_number(A) printf("%d: %s=%.32f (%d)\n", __LINE__, #A, number_to_double(A), number_to_scaled(A))
#else
#line 274 "../../../source/texk/web2c/mplibdir/mp.w"
#define debug_number(A) 
#endif
#line 276 "../../../source/texk/web2c/mplibdir/mp.w"
#if DEBUG> 1
void do_debug_printf(MP mp,const char*prefix,const char*fmt,...);
#  define debug_printf(a1,a2,a3) do_debug_printf(mp, "", a1,a2,a3)
#  define FUNCTION_TRACE1(a1) do_ debug_printf(mp, "FTRACE: ", a1)
#  define FUNCTION_TRACE2(a1,a2) do_debug_printf(mp, "FTRACE: ", a1,a2)
#  define FUNCTION_TRACE3(a1,a2,a3) do_debug_printf(mp, "FTRACE: ", a1,a2,a3)
#  define FUNCTION_TRACE3X(a1,a2,a3) (void)mp
#  define FUNCTION_TRACE4(a1,a2,a3,a4) do_debug_printf(mp, "FTRACE: ", a1,a2,a3,a4)
#else
#line 285 "../../../source/texk/web2c/mplibdir/mp.w"
#  define debug_printf(a1,a2,a3) 
#  define FUNCTION_TRACE1(a1) (void)mp
#  define FUNCTION_TRACE2(a1,a2) (void)mp
#  define FUNCTION_TRACE3(a1,a2,a3) (void)mp
#  define FUNCTION_TRACE3X(a1,a2,a3) (void)mp
#  define FUNCTION_TRACE4(a1,a2,a3,a4) (void)mp
#endif
#line 292 "../../../source/texk/web2c/mplibdir/mp.w"

/*:8*//*40:*/
#line 890 "../../../source/texk/web2c/mplibdir/mp.w"

#define xchr(A) mp->xchr[(A)]
#define xord(A) mp->xord[(A)]

/*:40*//*73:*/
#line 1360 "../../../source/texk/web2c/mplibdir/mp.w"

#define update_terminal()  (mp->flush_file)(mp,mp->term_out)      
#define clear_terminal()      
#define wake_up_terminal() (mp->flush_file)(mp,mp->term_out)


/*:73*//*88:*/
#line 1606 "../../../source/texk/web2c/mplibdir/mp.w"

#define mp_fputs(b,f) (mp->write_ascii_file)(mp,f,b)
#define wterm(A)     mp_fputs((A), mp->term_out)
#define wterm_chr(A) { unsigned char ss[2]; ss[0]= (A); ss[1]= '\0'; wterm((char *)ss);}
#define wterm_cr     mp_fputs("\n", mp->term_out)
#define wterm_ln(A)  { wterm_cr; mp_fputs((A), mp->term_out); }
#define wlog(A)      mp_fputs((A), mp->log_file)
#define wlog_chr(A)  { unsigned char ss[2]; ss[0]= (A); ss[1]= '\0'; wlog((char *)ss);}
#define wlog_cr      mp_fputs("\n", mp->log_file)
#define wlog_ln(A)   { wlog_cr; mp_fputs((A), mp->log_file); }


/*:88*//*178:*/
#line 3049 "../../../source/texk/web2c/mplibdir/mp.w"

#define NODE_BODY                       \
  mp_variable_type type;                \
  mp_name_type_type name_type;          \
  unsigned short has_number;  \
  struct mp_node_data *link

typedef struct mp_node_data{
NODE_BODY;
mp_value_data data;
}mp_node_data;
typedef struct mp_node_data*mp_symbolic_node;

/*:178*//*197:*/
#line 4178 "../../../source/texk/web2c/mplibdir/mp.w"

#define internal_value(A) mp->internal[(A)].v.data.n
#define set_internal_from_number(A,B) do { \
  number_clone (internal_value ((A)),(B));\
} while (0)
#define internal_string(A) (mp_string)mp->internal[(A)].v.data.str
#define set_internal_string(A,B) mp->internal[(A)].v.data.str= (B)
#define internal_name(A) mp->internal[(A)].intname
#define set_internal_name(A,B) mp->internal[(A)].intname= (B)
#define internal_type(A) (mp_variable_type)mp->internal[(A)].v.type
#define set_internal_type(A,B) mp->internal[(A)].v.type= (B)
#define set_internal_from_cur_exp(A) do { \
  if (internal_type ((A)) == mp_string_type) { \
      add_str_ref (cur_exp_str ()); \
      set_internal_string ((A), cur_exp_str ()); \
  } else { \
      set_internal_from_number ((A), cur_exp_value_number ()); \
  } \
} while (0)



/*:197*//*241:*/
#line 5341 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_node_data*mp_token_node;

/*:241*//*257:*/
#line 5837 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_value_node_data{
NODE_BODY;
mp_value_data data;
mp_number subscript_;
mp_sym hashloc_;
mp_node parent_;
mp_node attr_head_;
mp_node subscr_head_;
}mp_value_node_data;

/*:257*//*268:*/
#line 6118 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_pair_node_data{
NODE_BODY;
mp_node x_part_;
mp_node y_part_;
}mp_pair_node_data;
typedef struct mp_pair_node_data*mp_pair_node;

/*:268*//*273:*/
#line 6195 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_transform_node_data{
NODE_BODY;
mp_node tx_part_;
mp_node ty_part_;
mp_node xx_part_;
mp_node yx_part_;
mp_node xy_part_;
mp_node yy_part_;
}mp_transform_node_data;
typedef struct mp_transform_node_data*mp_transform_node;

/*:273*//*276:*/
#line 6259 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_color_node_data{
NODE_BODY;
mp_node red_part_;
mp_node green_part_;
mp_node blue_part_;
}mp_color_node_data;
typedef struct mp_color_node_data*mp_color_node;

/*:276*//*279:*/
#line 6307 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_cmykcolor_node_data{
NODE_BODY;
mp_node cyan_part_;
mp_node magenta_part_;
mp_node yellow_part_;
mp_node black_part_;
}mp_cmykcolor_node_data;
typedef struct mp_cmykcolor_node_data*mp_cmykcolor_node;

/*:279*//*459:*/
#line 11147 "../../../source/texk/web2c/mplibdir/mp.w"

#define mp_fraction mp_number
#define mp_angle mp_number
#define new_number(A) (((math_data *)(mp->math))->allocate)(mp, &(A), mp_scaled_type)
#define new_fraction(A) (((math_data *)(mp->math))->allocate)(mp, &(A), mp_fraction_type)
#define new_angle(A) (((math_data *)(mp->math))->allocate)(mp, &(A), mp_angle_type)
#define free_number(A) (((math_data *)(mp->math))->free)(mp, &(A))

/*:459*//*462:*/
#line 11279 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_fill_node_data{
NODE_BODY;
halfword color_model_;
mp_number red;
mp_number green;
mp_number blue;
mp_number black;
mp_string pre_script_;
mp_string post_script_;
mp_knot path_p_;
mp_knot pen_p_;
unsigned char ljoin;
mp_number miterlim;
}mp_fill_node_data;
typedef struct mp_fill_node_data*mp_fill_node;

/*:462*//*466:*/
#line 11360 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_stroked_node_data{
NODE_BODY;
halfword color_model_;
mp_number red;
mp_number green;
mp_number blue;
mp_number black;
mp_string pre_script_;
mp_string post_script_;
mp_knot path_p_;
mp_knot pen_p_;
unsigned char ljoin;
mp_number miterlim;
unsigned char lcap;
mp_node dash_p_;
mp_number dash_scale;
}mp_stroked_node_data;
typedef struct mp_stroked_node_data*mp_stroked_node;


/*:466*//*472:*/
#line 11560 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_text_node_data{
NODE_BODY;
halfword color_model_;
mp_number red;
mp_number green;
mp_number blue;
mp_number black;
mp_string pre_script_;
mp_string post_script_;
mp_string text_p_;
halfword font_n_;
mp_number width;
mp_number height;
mp_number depth;
mp_number tx;
mp_number ty;
mp_number txx;
mp_number txy;
mp_number tyx;
mp_number tyy;
}mp_text_node_data;
typedef struct mp_text_node_data*mp_text_node;

/*:472*//*476:*/
#line 11661 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_start_clip_node_data{
NODE_BODY;
mp_knot path_p_;
}mp_start_clip_node_data;
typedef struct mp_start_clip_node_data*mp_start_clip_node;
typedef struct mp_start_bounds_node_data{
NODE_BODY;
mp_knot path_p_;
}mp_start_bounds_node_data;
typedef struct mp_start_bounds_node_data*mp_start_bounds_node;
typedef struct mp_stop_clip_node_data{
NODE_BODY;
}mp_stop_clip_node_data;
typedef struct mp_stop_clip_node_data*mp_stop_clip_node;
typedef struct mp_stop_bounds_node_data{
NODE_BODY;
}mp_stop_bounds_node_data;
typedef struct mp_stop_bounds_node_data*mp_stop_bounds_node;


/*:476*//*480:*/
#line 11769 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_dash_node_data{
NODE_BODY;
mp_number start_x;
mp_number stop_x;
mp_number dash_y;
mp_node dash_info_;
}mp_dash_node_data;

/*:480*//*485:*/
#line 11824 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_edge_header_node_data{
NODE_BODY;
mp_number start_x;
mp_number stop_x;
mp_number dash_y;
mp_node dash_info_;
mp_number minx;
mp_number miny;
mp_number maxx;
mp_number maxy;
mp_node bblast_;
int bbtype;
mp_node list_;
mp_node obj_tail_;
halfword ref_count_;
}mp_edge_header_node_data;
typedef struct mp_edge_header_node_data*mp_edge_header_node;

/*:485*//*812:*/
#line 20824 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_if_node_data{
NODE_BODY;
int if_line_field_;
}mp_if_node_data;
typedef struct mp_if_node_data*mp_if_node;

/*:812*/
#line 152 "../../../source/texk/web2c/mplibdir/mp.w"

#endif
#line 154 "../../../source/texk/web2c/mplibdir/mp.w"

/*:4*/
