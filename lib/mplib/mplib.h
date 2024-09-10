/*3:*/
#line 89 "../../../source/texk/web2c/mplibdir/mp.w"

#ifndef MPLIB_H
#define MPLIB_H 1
#include <stdlib.h> 
#ifndef HAVE_BOOLEAN
//VMF
typedef unsigned char boolean;
#endif
#line 96 "../../../source/texk/web2c/mplibdir/mp.w"
/*2:*/
#line 78 "../../../source/texk/web2c/mplibdir/mp.w"

#define metapost_version "2.11"

/*:2*/
#line 96 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct MP_instance*MP;
/*19:*/
#line 423 "../../../source/texk/web2c/mplibdir/mp.w"

typedef enum{
mp_nan_type= 0,
mp_scaled_type,
mp_fraction_type,
mp_angle_type,
mp_double_type,
mp_binary_type,
mp_decimal_type,
mp_interval_type
}mp_number_type;
typedef union{
void*num;
double dval;
int val;
}mp_number_store;
typedef struct mp_number_data{
mp_number_store data;
mp_number_type type;
}mp_number_data;
typedef struct mp_number_data mp_number;
#define is_number(A) ((A).type != mp_nan_type)

typedef void(*convert_func)(mp_number*r);
typedef void(*m_log_func)(MP mp,mp_number*r,mp_number a);
typedef void(*m_exp_func)(MP mp,mp_number*r,mp_number a);
typedef void(*m_unif_rand_func)(MP mp,mp_number*ret,mp_number x_orig);
typedef void(*m_norm_rand_func)(MP mp,mp_number*ret);
typedef void(*pyth_add_func)(MP mp,mp_number*r,mp_number a,mp_number b);
typedef void(*pyth_sub_func)(MP mp,mp_number*r,mp_number a,mp_number b);
typedef void(*n_arg_func)(MP mp,mp_number*r,mp_number a,mp_number b);
typedef void(*velocity_func)(MP mp,mp_number*r,mp_number a,mp_number b,mp_number c,mp_number d,mp_number e);
typedef void(*ab_vs_cd_func)(MP mp,mp_number*r,mp_number a,mp_number b,mp_number c,mp_number d);
typedef void(*crossing_point_func)(MP mp,mp_number*r,mp_number a,mp_number b,mp_number c);
typedef void(*number_from_int_func)(mp_number*A,int B);
typedef void(*number_from_boolean_func)(mp_number*A,int B);
typedef void(*number_from_scaled_func)(mp_number*A,int B);
typedef void(*number_from_double_func)(mp_number*A,double B);
typedef void(*number_from_addition_func)(mp_number*A,mp_number B,mp_number C);
typedef void(*number_from_substraction_func)(mp_number*A,mp_number B,mp_number C);
typedef void(*number_from_div_func)(mp_number*A,mp_number B,mp_number C);
typedef void(*number_from_mul_func)(mp_number*A,mp_number B,mp_number C);
typedef void(*number_from_int_div_func)(mp_number*A,mp_number B,int C);
typedef void(*number_from_int_mul_func)(mp_number*A,mp_number B,int C);
typedef void(*number_from_oftheway_func)(MP mp,mp_number*A,mp_number t,mp_number B,mp_number C);
typedef void(*number_negate_func)(mp_number*A);
typedef void(*number_add_func)(mp_number*A,mp_number B);
typedef void(*number_substract_func)(mp_number*A,mp_number B);
typedef void(*number_modulo_func)(mp_number*A,mp_number B);
typedef void(*number_half_func)(mp_number*A);
typedef void(*number_halfp_func)(mp_number*A);
typedef void(*number_double_func)(mp_number*A);
typedef void(*number_abs_func)(mp_number*A);
typedef void(*number_clone_func)(mp_number*A,mp_number B);
typedef void(*number_swap_func)(mp_number*A,mp_number*B);
typedef void(*number_add_scaled_func)(mp_number*A,int b);
typedef void(*number_multiply_int_func)(mp_number*A,int b);
typedef void(*number_divide_int_func)(mp_number*A,int b);
typedef int(*number_to_int_func)(mp_number A);
typedef int(*number_to_boolean_func)(mp_number A);
typedef int(*number_to_scaled_func)(mp_number A);
typedef int(*number_round_func)(mp_number A);
typedef void(*number_floor_func)(mp_number*A);
typedef double(*number_to_double_func)(mp_number A);
typedef int(*number_odd_func)(mp_number A);
typedef int(*number_equal_func)(mp_number A,mp_number B);
typedef int(*number_less_func)(mp_number A,mp_number B);
typedef int(*number_greater_func)(mp_number A,mp_number B);
typedef int(*number_nonequalabs_func)(mp_number A,mp_number B);
typedef void(*make_scaled_func)(MP mp,mp_number*ret,mp_number A,mp_number B);
typedef void(*make_fraction_func)(MP mp,mp_number*ret,mp_number A,mp_number B);
typedef void(*take_fraction_func)(MP mp,mp_number*ret,mp_number A,mp_number B);
typedef void(*take_scaled_func)(MP mp,mp_number*ret,mp_number A,mp_number B);
typedef void(*sin_cos_func)(MP mp,mp_number A,mp_number*S,mp_number*C);
typedef void(*slow_add_func)(MP mp,mp_number*A,mp_number S,mp_number C);
typedef void(*sqrt_func)(MP mp,mp_number*ret,mp_number A);
typedef void(*init_randoms_func)(MP mp,int seed);
typedef void(*new_number_func)(MP mp,mp_number*A,mp_number_type t);
typedef void(*free_number_func)(MP mp,mp_number*n);
typedef void(*fraction_to_round_scaled_func)(mp_number*n);
typedef void(*print_func)(MP mp,mp_number A);
typedef char*(*tostring_func)(MP mp,mp_number A);
typedef void(*scan_func)(MP mp,int A);
typedef void(*mp_free_func)(MP mp);
typedef void(*set_precision_func)(MP mp);

typedef void(*m_get_left_endpoint_func)(MP mp,mp_number*r,mp_number a);
typedef void(*m_get_right_endpoint_func)(MP mp,mp_number*r,mp_number a);
typedef void(*m_interval_set_func)(MP mp,mp_number*r,mp_number a,mp_number b);

typedef struct math_data{
mp_number precision_default;
mp_number precision_max;
mp_number precision_min;
mp_number epsilon_t;
mp_number inf_t;
mp_number one_third_inf_t;
mp_number zero_t;
mp_number unity_t;
mp_number two_t;
mp_number three_t;
mp_number half_unit_t;
mp_number three_quarter_unit_t;
mp_number fraction_one_t;
mp_number fraction_half_t;
mp_number fraction_three_t;
mp_number fraction_four_t;
mp_number one_eighty_deg_t;
mp_number three_sixty_deg_t;
mp_number one_k;
mp_number sqrt_8_e_k;
mp_number twelve_ln_2_k;
mp_number coef_bound_k;
mp_number coef_bound_minus_1;
mp_number twelvebits_3;
mp_number arc_tol_k;
mp_number twentysixbits_sqrt2_t;
mp_number twentyeightbits_d_t;
mp_number twentysevenbits_sqrt2_d_t;
mp_number fraction_threshold_t;
mp_number half_fraction_threshold_t;
mp_number scaled_threshold_t;
mp_number half_scaled_threshold_t;
mp_number near_zero_angle_t;
mp_number p_over_v_threshold_t;
mp_number equation_threshold_t;
mp_number tfm_warn_threshold_t;
mp_number warning_limit_t;
new_number_func allocate;
free_number_func free;
number_from_int_func from_int;
number_from_boolean_func from_boolean;
number_from_scaled_func from_scaled;
number_from_double_func from_double;
number_from_addition_func from_addition;
number_from_substraction_func from_substraction;
number_from_div_func from_div;
number_from_mul_func from_mul;
number_from_int_div_func from_int_div;
number_from_int_mul_func from_int_mul;
number_from_oftheway_func from_oftheway;
number_negate_func negate;
number_add_func add;
number_substract_func substract;
number_half_func half;
number_modulo_func modulo;
number_halfp_func halfp;
number_double_func do_double;
number_abs_func abs;
number_clone_func clone;
number_swap_func swap;
number_add_scaled_func add_scaled;
number_multiply_int_func multiply_int;
number_divide_int_func divide_int;
number_to_int_func to_int;
number_to_boolean_func to_boolean;
number_to_scaled_func to_scaled;
number_to_double_func to_double;
number_odd_func odd;
number_equal_func equal;
number_less_func less;
number_greater_func greater;
number_nonequalabs_func nonequalabs;
number_round_func round_unscaled;
number_floor_func floor_scaled;
make_scaled_func make_scaled;
make_fraction_func make_fraction;
take_fraction_func take_fraction;
take_scaled_func take_scaled;
velocity_func velocity;
ab_vs_cd_func ab_vs_cd;
crossing_point_func crossing_point;
n_arg_func n_arg;
m_log_func m_log;
m_exp_func m_exp;
m_unif_rand_func m_unif_rand;
m_norm_rand_func m_norm_rand;
pyth_add_func pyth_add;
pyth_sub_func pyth_sub;
fraction_to_round_scaled_func fraction_to_round_scaled;
convert_func fraction_to_scaled;
convert_func scaled_to_fraction;
convert_func scaled_to_angle;
convert_func angle_to_scaled;
init_randoms_func init_randoms;
sin_cos_func sin_cos;
sqrt_func sqrt;
slow_add_func slow_add;
print_func print;
tostring_func tostring;
scan_func scan_numeric;
scan_func scan_fractional;
mp_free_func free_math;
set_precision_func set_precision;

m_get_left_endpoint_func m_get_left_endpoint;
m_get_right_endpoint_func m_get_right_endpoint;
m_interval_set_func m_interval_set;
}math_data;



/*:19*//*46:*/
#line 971 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_filetype{
mp_filetype_terminal= 0,
mp_filetype_error,
mp_filetype_program,
mp_filetype_log,
mp_filetype_postscript,
mp_filetype_bitmap,
mp_filetype_memfile,
mp_filetype_metrics,
mp_filetype_fontmap,
mp_filetype_font,
mp_filetype_encoding,
mp_filetype_text
};
typedef char*(*mp_file_finder)(MP,const char*,const char*,int);
typedef char*(*mp_script_runner)(MP,const char*,size_t);
typedef char*(*mp_text_maker)(MP,const char*,size_t,int);
typedef void*(*mp_file_opener)(MP,const char*,const char*,int);
typedef char*(*mp_file_reader)(MP,void*,size_t*);
typedef void(*mp_binfile_reader)(MP,void*,void**,size_t*);
typedef void(*mp_file_closer)(MP,void*);
typedef int(*mp_file_eoftest)(MP,void*);
typedef void(*mp_file_flush)(MP,void*);
typedef void(*mp_file_writer)(MP,void*,const char*);
typedef void(*mp_binfile_writer)(MP,void*,void*,size_t);

/*:46*//*78:*/
#line 1468 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct{
unsigned char*str;
size_t len;
int refs;
}mp_lstring;
typedef mp_lstring*mp_string;

/*:78*//*104:*/
#line 1926 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_interaction_mode{
mp_unspecified_mode= 0,
mp_batch_mode,
mp_nonstop_mode,
mp_scroll_mode,
mp_error_stop_mode
};

/*:104*//*110:*/
#line 2002 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_history_state{
mp_spotless= 0,
mp_warning_issued,
mp_error_message_issued,
mp_fatal_error_stop,
mp_system_error_stop
};

/*:110*//*124:*/
#line 2167 "../../../source/texk/web2c/mplibdir/mp.w"

typedef void(*mp_editor_cmd)(MP,char*,int);

/*:124*//*168:*/
#line 2879 "../../../source/texk/web2c/mplibdir/mp.w"

typedef enum{
mp_math_scaled_mode= 0,
mp_math_double_mode= 1,
mp_math_binary_mode= 2,
mp_math_decimal_mode= 3,
mp_math_interval_mode= 4
}mp_math_mode;

/*:168*//*303:*/
#line 7166 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_knot_data*mp_knot;
typedef struct mp_knot_data{
mp_number x_coord;
mp_number y_coord;
mp_number left_x;
mp_number left_y;
mp_number right_x;
mp_number right_y;
mp_knot next;
union{
struct{
unsigned short left_type;
unsigned short right_type;
}types;
mp_knot prev;
signed int info;
}data;
unsigned char originator;
}mp_knot_data;


/*:303*//*304:*/
#line 7190 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct mp_gr_knot_data*mp_gr_knot;
typedef struct mp_gr_knot_data{
double x_coord;
double y_coord;
double left_x;
double left_y;
double right_x;
double right_y;
mp_gr_knot next;
union{
struct{
unsigned short left_type;
unsigned short right_type;
}types;
mp_gr_knot prev;
signed int info;
}data;
unsigned char originator;
}mp_gr_knot_data;


/*:304*//*307:*/
#line 7301 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_knot_originator{
mp_program_code= 0,
mp_metapost_user
};

/*:307*//*892:*/
#line 22317 "../../../source/texk/web2c/mplibdir/mp.w"

typedef int(*mp_makempx_cmd)(MP mp,char*origname,char*mtxname);

/*:892*//*1058:*/
#line 30531 "../../../source/texk/web2c/mplibdir/mp.w"

#undef term_in
#undef term_out

typedef struct{
void*fptr;
char*data;
char*cur;
size_t size;
size_t used;
}mp_stream;
typedef struct{
mp_stream term_out;
mp_stream error_out;
mp_stream log_out;
mp_stream ship_out;
mp_stream term_in;
struct mp_edge_object*edges;
}mp_run_data;

/*:1058*//*1274:*/
#line 35342 "../../../source/texk/web2c/mplibdir/mp.w"

typedef void(*mp_backend_writer)(MP,void*);

/*:1274*/
#line 98 "../../../source/texk/web2c/mplibdir/mp.w"

typedef struct MP_options{
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
#line 100 "../../../source/texk/web2c/mplibdir/mp.w"

}MP_options;
/*22:*/
#line 736 "../../../source/texk/web2c/mplibdir/mp.w"

extern MP_options*mp_options(void);
extern MP mp_initialize(MP_options*opt);
extern int mp_status(MP mp);
extern boolean mp_finished(MP mp);
extern void*mp_userdata(MP mp);

/*:22*//*122:*/
#line 2139 "../../../source/texk/web2c/mplibdir/mp.w"

extern void mp_error(MP mp,const char*msg,const char**hlp,boolean deletions_allowed);
extern void mp_warn(MP mp,const char*msg);


/*:122*//*139:*/
#line 2433 "../../../source/texk/web2c/mplibdir/mp.w"

extern void mp_fatal_error(MP mp,const char*s);


/*:139*//*201:*/
#line 4232 "../../../source/texk/web2c/mplibdir/mp.w"

int mp_troff_mode(MP mp);

/*:201*//*234:*/
#line 4996 "../../../source/texk/web2c/mplibdir/mp.w"

double mp_get_numeric_value(MP mp,const char*s,size_t l);
int mp_get_boolean_value(MP mp,const char*s,size_t l);
char*mp_get_string_value(MP mp,const char*s,size_t l);
mp_knot mp_get_path_value(MP mp,const char*s,size_t l);

/*:234*//*382:*/
#line 9272 "../../../source/texk/web2c/mplibdir/mp.w"

int mp_close_path_cycle(MP mp,mp_knot p,mp_knot q);
int mp_close_path(MP mp,mp_knot q,mp_knot first);
mp_knot mp_create_knot(MP mp);
int mp_set_knot(MP mp,mp_knot p,double x,double y);
mp_knot mp_append_knot(MP mp,mp_knot p,double x,double y);
int mp_set_knot_curl(MP mp,mp_knot q,double value);
int mp_set_knot_left_curl(MP mp,mp_knot q,double value);
int mp_set_knot_right_curl(MP mp,mp_knot q,double value);
int mp_set_knotpair_curls(MP mp,mp_knot p,mp_knot q,double t1,double t2);
int mp_set_knotpair_tensions(MP mp,mp_knot p,mp_knot q,double t1,double t2);
int mp_set_knot_left_tension(MP mp,mp_knot p,double t1);
int mp_set_knot_right_tension(MP mp,mp_knot p,double t1);
int mp_set_knot_left_control(MP mp,mp_knot p,double t1,double t2);
int mp_set_knot_right_control(MP mp,mp_knot p,double t1,double t2);
int mp_set_knotpair_controls(MP mp,mp_knot p,mp_knot q,double x1,double y1,double x2,double y2);
int mp_set_knot_direction(MP mp,mp_knot q,double x,double y);
int mp_set_knotpair_directions(MP mp,mp_knot p,mp_knot q,double x1,double y1,double x2,double y2);
int mp_solve_path(MP mp,mp_knot first);
void mp_free_path(MP mp,mp_knot p);

/*:382*//*384:*/
#line 9309 "../../../source/texk/web2c/mplibdir/mp.w"

#define mp_knot_left_curl mp_knot_left_x
#define mp_knot_left_given mp_knot_left_x
#define mp_knot_left_tension mp_knot_left_y
#define mp_knot_right_curl mp_knot_right_x
#define mp_knot_right_given mp_knot_right_x
#define mp_knot_right_tension mp_knot_right_y
mp_number mp_knot_x_coord(MP mp,mp_knot p);
mp_number mp_knot_y_coord(MP mp,mp_knot p);
mp_number mp_knot_left_x(MP mp,mp_knot p);
mp_number mp_knot_left_y(MP mp,mp_knot p);
mp_number mp_knot_right_x(MP mp,mp_knot p);
mp_number mp_knot_right_y(MP mp,mp_knot p);
int mp_knot_right_type(MP mp,mp_knot p);
int mp_knot_left_type(MP mp,mp_knot p);
mp_knot mp_knot_next(MP mp,mp_knot p);
double mp_number_as_double(MP mp,mp_number n);


/*:384*//*1057:*/
#line 30508 "../../../source/texk/web2c/mplibdir/mp.w"

void mp_set_internal(MP mp,char*n,char*v,int isstring);

/*:1057*//*1066:*/
#line 30849 "../../../source/texk/web2c/mplibdir/mp.w"

extern mp_run_data*mp_rundata(MP mp);

/*:1066*//*1074:*/
#line 31009 "../../../source/texk/web2c/mplibdir/mp.w"

int mp_run(MP mp);
int mp_execute(MP mp,char*s,size_t l);
int mp_finish(MP mp);
char*mp_metapost_version(void);void mp_show_library_versions(void);

/*:1074*//*1235:*/
#line 34519 "../../../source/texk/web2c/mplibdir/mp.w"

double mp_get_char_dimension(MP mp,char*fname,int n,int t);


/*:1235*//*1291:*/
#line 35603 "../../../source/texk/web2c/mplibdir/mp.w"

int mp_memory_usage(MP mp);
int mp_hash_usage(MP mp);
int mp_param_usage(MP mp);
int mp_open_usage(MP mp);

/*:1291*/
#line 102 "../../../source/texk/web2c/mplibdir/mp.w"

/*205:*/
#line 4362 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_color_model{
mp_no_model= 1,
mp_grey_model= 3,
mp_rgb_model= 5,
mp_cmyk_model= 7,
mp_uninitialized_model= 9
};


/*:205*//*305:*/
#line 7212 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_knot_type{
mp_endpoint= 0,
mp_explicit,
mp_given,
mp_curl,
mp_open,
mp_end_cycle
};

/*:305*//*461:*/
#line 11256 "../../../source/texk/web2c/mplibdir/mp.w"

enum mp_graphical_object_code{
/*463:*/
#line 11296 "../../../source/texk/web2c/mplibdir/mp.w"

mp_fill_code= 1,

/*:463*//*467:*/
#line 11381 "../../../source/texk/web2c/mplibdir/mp.w"

mp_stroked_code= 2,

/*:467*//*473:*/
#line 11584 "../../../source/texk/web2c/mplibdir/mp.w"

mp_text_code= 3,

/*:473*//*477:*/
#line 11682 "../../../source/texk/web2c/mplibdir/mp.w"

mp_start_clip_code= 4,
mp_start_bounds_code= 5,
mp_stop_clip_code= 6,
mp_stop_bounds_code= 7,


/*:477*//*1265:*/
#line 35033 "../../../source/texk/web2c/mplibdir/mp.w"

mp_special_code= 8,

/*:1265*/
#line 11258 "../../../source/texk/web2c/mplibdir/mp.w"

mp_final_graphic
};

/*:461*/
#line 103 "../../../source/texk/web2c/mplibdir/mp.w"

#endif
#line 105 "../../../source/texk/web2c/mplibdir/mp.w"

/*:3*/
