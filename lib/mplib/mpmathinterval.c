/*1:*/
#line 22 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

#include <w2c/config.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <math.h> 
#include "mpmathinterval.h" 
#define ROUND(a) floor((a)+0.5)
#define MPFI_ROUNDING MPFI_RNDN
#define MPFR_ROUNDING MPFR_RNDN
#define E_STRING "2.7182818284590452353602874713526624977572470936999595749669676277240766303535"
#define PI_STRING "3.1415926535897932384626433832795028841971693993751058209749445923078164062862"
#define fraction_multiplier 4096
#define angle_multiplier 16 \

#define mpfi_negative_p(a) (mpfi_is_strictly_neg((a) ) > 0) 
#define mpfi_positive_p(a) (mpfi_is_strictly_pos((a) ) > 0) 
#define mpfi_overlaps_zero_p(a) (mpfi_cmp_ui(a,0) ==0) 
#define mpfi_nonnegative_p(a) (mpfi_is_nonneg((a) ) > 0) 
#define checkZero(dec) if(mpfi_has_zero(dec) ) { \
mpfi_set_d(dec,0.0) ; \
} \

#define mpfi_zero_p(a) mpfi_has_zero(a)  \

#define unity 1
#define two 2
#define three 3
#define four 4
#define half_unit 0.5
#define three_quarter_unit 0.75
#define coef_bound ((7.0/3.0) *fraction_multiplier) 
#define fraction_threshold 0.04096
#define half_fraction_threshold (fraction_threshold/2) 
#define scaled_threshold 0.000122
#define half_scaled_threshold (scaled_threshold/2) 
#define near_zero_angle (0.0256*angle_multiplier) 
#define p_over_v_threshold 0x80000
#define equation_threshold 0.001
#define tfm_warn_threshold 0.0625
#define warning_limit pow(2.0,52.0) 
#define epsilon pow(2.0,-173.0) 
#define epsilonf pow(2.0,-52.0) 
#define EL_GORDO "1E1000000"
#define one_third_EL_GORDO (EL_GORDO/3.0)  \

#define MAX_PRECISION 1000.0
#define DEF_PRECISION 34.0 \

#define odd(A) (abs(A) %2==1)  \

#define halfp(A) (integer) ((unsigned) (A) >>1)  \

#define set_cur_cmd(A) mp->cur_mod_->type= (A) 
#define set_cur_mod(A) mpfi_set((mpfi_ptr) (mp->cur_mod_->data.n.data.num) ,A)  \

#define too_precise(a) (a> precision_bits) 
#define fraction_half (fraction_multiplier/2) 
#define fraction_one (1*fraction_multiplier) 
#define fraction_two (2*fraction_multiplier) 
#define fraction_three (3*fraction_multiplier) 
#define fraction_four (4*fraction_multiplier)  \

#define no_crossing {mpfi_set(ret->data.num,fraction_one_plus_mpfi_t) ;goto RETURN;}
#define one_crossing {mpfi_set(ret->data.num,fraction_one_mpfi_t) ;goto RETURN;}
#define zero_crossing {mpfi_set(ret->data.num,zero) ;goto RETURN;} \


#line 30 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"


/*:1*//*2:*/
#line 32 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

/*5:*/
#line 74 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

#define DEBUG 0
static void mp_interval_scan_fractional_token(MP mp,int n);
static void mp_interval_scan_numeric_token(MP mp,int n);
static void mp_interval_ab_vs_cd(MP mp,mp_number*ret,mp_number a,mp_number b,mp_number c,mp_number d);
static void mp_ab_vs_cd(MP mp,mp_number*ret,mp_number a,mp_number b,mp_number c,mp_number d);
static void mp_interval_crossing_point(MP mp,mp_number*ret,mp_number a,mp_number b,mp_number c);
static void mp_interval_number_modulo(mp_number*a,mp_number b);
static void mp_interval_print_number(MP mp,mp_number n);
static char*mp_interval_number_tostring(MP mp,mp_number n);
static void mp_interval_slow_add(MP mp,mp_number*ret,mp_number x_orig,mp_number y_orig);
static void mp_interval_square_rt(MP mp,mp_number*ret,mp_number x_orig);
static void mp_interval_sin_cos(MP mp,mp_number z_orig,mp_number*n_cos,mp_number*n_sin);
static void mp_init_randoms(MP mp,int seed);
static void mp_number_angle_to_scaled(mp_number*A);
static void mp_number_fraction_to_scaled(mp_number*A);
static void mp_number_scaled_to_fraction(mp_number*A);
static void mp_number_scaled_to_angle(mp_number*A);
static void mp_interval_m_unif_rand(MP mp,mp_number*ret,mp_number x_orig);
static void mp_interval_m_norm_rand(MP mp,mp_number*ret);
static void mp_interval_m_exp(MP mp,mp_number*ret,mp_number x_orig);
static void mp_interval_m_log(MP mp,mp_number*ret,mp_number x_orig);
static void mp_interval_pyth_sub(MP mp,mp_number*r,mp_number a,mp_number b);
static void mp_interval_pyth_add(MP mp,mp_number*r,mp_number a,mp_number b);
static void mp_interval_n_arg(MP mp,mp_number*ret,mp_number x,mp_number y);
static void mp_interval_velocity(MP mp,mp_number*ret,mp_number st,mp_number ct,mp_number sf,mp_number cf,mp_number t);
static void mp_set_interval_from_int(mp_number*A,int B);
static void mp_set_interval_from_boolean(mp_number*A,int B);
static void mp_set_interval_from_scaled(mp_number*A,int B);
static void mp_set_interval_from_addition(mp_number*A,mp_number B,mp_number C);
static void mp_set_interval_from_substraction(mp_number*A,mp_number B,mp_number C);
static void mp_set_interval_from_div(mp_number*A,mp_number B,mp_number C);
static void mp_set_interval_from_mul(mp_number*A,mp_number B,mp_number C);
static void mp_set_interval_from_int_div(mp_number*A,mp_number B,int C);
static void mp_set_interval_from_int_mul(mp_number*A,mp_number B,int C);
static void mp_set_interval_from_of_the_way(MP mp,mp_number*A,mp_number t,mp_number B,mp_number C);
static void mp_number_negate(mp_number*A);
static void mp_number_add(mp_number*A,mp_number B);
static void mp_number_substract(mp_number*A,mp_number B);
static void mp_number_half(mp_number*A);
static void mp_number_halfp(mp_number*A);
static void mp_number_double(mp_number*A);
static void mp_number_add_scaled(mp_number*A,int B);
static void mp_number_multiply_int(mp_number*A,int B);
static void mp_number_divide_int(mp_number*A,int B);
static void mp_interval_abs(mp_number*A);
static void mp_number_clone(mp_number*A,mp_number B);
static void mp_number_swap(mp_number*A,mp_number*B);
static int mp_round_unscaled(mp_number x_orig);
static int mp_number_to_int(mp_number A);
static int mp_number_to_scaled(mp_number A);
static int mp_number_to_boolean(mp_number A);
static double mp_number_to_double(mp_number A);
static int mp_number_odd(mp_number A);
static int mp_number_equal(mp_number A,mp_number B);
static int mp_number_greater(mp_number A,mp_number B);
static int mp_number_less(mp_number A,mp_number B);
static int mp_number_nonequalabs(mp_number A,mp_number B);
static void mp_number_floor(mp_number*i);
static void mp_interval_fraction_to_round_scaled(mp_number*x);
static void mp_interval_number_make_scaled(MP mp,mp_number*r,mp_number p,mp_number q);
static void mp_interval_number_make_fraction(MP mp,mp_number*r,mp_number p,mp_number q);
static void mp_interval_number_take_fraction(MP mp,mp_number*r,mp_number p,mp_number q);
static void mp_interval_number_take_scaled(MP mp,mp_number*r,mp_number p,mp_number q);
static void mp_new_number(MP mp,mp_number*n,mp_number_type t);
static void mp_free_number(MP mp,mp_number*n);
static void mp_set_interval_from_double(mp_number*A,double B);
static void mp_free_interval_math(MP mp);
static void mp_interval_set_precision(MP mp);
static void mp_check_mpfi_t(MP mp,mpfi_t dec);
static int interval_number_check(mpfi_t dec);
static char*mp_intervalnumber_tostring(mpfi_t n);
static void init_interval_constants(void);
static void free_interval_constants(void);
static mpfr_prec_t precision_digits_to_bits(double i);
static double precision_bits_to_digits(mpfr_prec_t i);

static void mp_interval_m_get_left_endpoint(MP mp,mp_number*ret,mp_number x_orig);
static void mp_interval_m_get_right_endpoint(MP mp,mp_number*ret,mp_number x_orig);
static void mp_interval_m_interval_set(MP mp,mp_number*ret,mp_number a,mp_number b);


static int mpfi_remainder(mpfi_t ROP,mpfi_t OP1,mpfi_t OP2);
static int mpfi_remainder_1(mpfi_t ROP,mpfi_t OP1,mpfr_t OP2);
static int mpfi_remainder_2(mpfi_t ROP,mpfi_t OP1,mpfi_t OP2);

/*:5*//*10:*/
#line 471 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static mpfi_t zero;
static mpfi_t one;
static mpfi_t minusone;
static mpfi_t two_mpfi_t;
static mpfi_t three_mpfi_t;
static mpfi_t four_mpfi_t;
static mpfi_t fraction_multiplier_mpfi_t;
static mpfi_t angle_multiplier_mpfi_t;
static mpfi_t fraction_one_mpfi_t;
static mpfi_t fraction_one_plus_mpfi_t;
static mpfi_t PI_mpfi_t;
static mpfi_t epsilon_mpfi_t;
static mpfi_t EL_GORDO_mpfi_t;
static boolean initialized= false;


/*:10*//*26:*/
#line 1136 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_make_fraction(MP mp,mpfi_t ret,mpfi_t p,mpfi_t q);

/*:26*//*28:*/
#line 1158 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_take_fraction(MP mp,mpfi_t ret,mpfi_t p,mpfi_t q);

/*:28*//*32:*/
#line 1199 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static void mp_wrapup_numeric_token(MP mp,unsigned char*start,unsigned char*stop);

/*:32*/
#line 33 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
;

/*:2*//*6:*/
#line 172 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

int interval_number_check(mpfi_t dec)
{
int test= false;
if(mpfi_nan_p(dec)!=0||mpfi_is_empty(dec)!=0){
test= true;
mpfi_set_d(dec,0.0);










}
return test;
}
void mp_check_mpfi_t(MP mp,mpfi_t dec)
{
mp->arith_error= interval_number_check(dec);
}



/*:6*//*7:*/
#line 201 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static double precision_bits;
mpfr_prec_t precision_digits_to_bits(double i)
{
return i/log10(2);
}
double precision_bits_to_digits(mpfr_prec_t d)
{
return d*log10(2);
}


/*:7*//*8:*/
#line 215 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
























































int mpfi_remainder(mpfi_t r,mpfi_t x,mpfi_t y){

int test,ret_val;
mpfr_t m,yd;
mpfr_inits2(precision_bits,m,yd,(mpfr_ptr)0);
test= mpfi_diam(yd,y);
if(test==0&&mpfr_zero_p(yd)> 0){
mpfi_get_fr(m,y);
ret_val= mpfi_remainder_1(r,x,m);
}else{
ret_val= mpfi_remainder_2(r,x,y);
}
mpfr_clears(m,yd,(mpfr_ptr)0);
return ret_val;
}

static int mpfi_remainder_1(mpfi_t r,mpfi_t x,mpfr_t m){
int ret_val;
ret_val= -1;
if(mpfi_is_strictly_neg(x)> 0){

mpfi_neg(x,x);
ret_val= mpfi_remainder_1(r,x,m);
mpfi_neg(r,r);
return ret_val;
}else{
mpfr_t a,b;
mpfr_inits2(precision_bits,a,b,(mpfr_ptr)0);
mpfi_get_left(a,x);mpfi_get_right(b,x);
if(mpfr_sgn(a)<0){


mpfi_t l1,l2,ret1,ret2;
mpfr_t one,zero;
mpfi_inits2(precision_bits,ret1,ret2,l1,l2,(mpfi_ptr)0);
mpfr_inits2(precision_bits,one,zero,(mpfr_ptr)0);
mpfr_set_si(one,1,MPFR_RNDN);mpfr_set_si(zero,0,MPFR_RNDN);
mpfr_neg(a,a,MPFR_RNDN);
mpfi_interv_fr(l1,one,a);mpfi_interv_fr(l2,zero,b);
mpfi_remainder_1(ret1,l1,m);mpfi_remainder_1(ret2,l2,m);
ret_val= mpfi_union(r,ret1,ret2);
mpfi_clears(ret1,ret2,l1,l2,(mpfi_ptr)0);
mpfr_clears(one,zero,(mpfr_ptr)0);
}else{

mpfr_t d,abs_m,rem_a,rem_b,zero,one,abs_m_1;
mpfr_inits2(precision_bits,d,abs_m,rem_a,rem_b,zero,one,abs_m_1,(mpfr_ptr)0);
mpfr_sub(d,b,a,MPFR_RNDN);
mpfr_abs(abs_m,m,MPFR_RNDN);


mpfr_fmod(rem_a,a,m,MPFR_RNDN);mpfr_fmod(rem_b,b,m,MPFR_RNDN);
if(mpfr_less_p(d,abs_m)&&mpfr_lessequal_p(rem_a,rem_b)){

ret_val= mpfi_interv_fr(r,rem_a,rem_b);
}else{

mpfr_set_si(one,1,MPFR_RNDN);mpfr_set_si(zero,0,MPFR_RNDN);mpfr_set_si(zero,0,MPFR_RNDN);
mpfr_sub(abs_m_1,abs_m,one,MPFR_RNDN);
ret_val= mpfi_interv_fr(r,zero,abs_m_1);
}
mpfr_clears(d,abs_m,rem_a,rem_b,zero,one,abs_m_1,(mpfr_ptr)0);
}
mpfr_clears(a,b,(mpfr_ptr)0);
}
return ret_val;
}

static int mpfi_remainder_2(mpfi_t r,mpfi_t x,mpfi_t m){
int ret_val;
if(mpfi_is_strictly_neg(x)> 0){

mpfi_neg(x,x);
ret_val= mpfi_remainder_2(r,x,m);
mpfi_neg(r,r);
return ret_val;
}else{
mpfr_t a,b,n1,m1;
mpfr_t one,zero;
mpfr_inits2(precision_bits,one,zero,(mpfr_ptr)0);
mpfr_set_si(one,1,MPFR_RNDN);mpfr_set_si(zero,0,MPFR_RNDN);
mpfr_inits2(precision_bits,a,b,(mpfr_ptr)0);
mpfi_get_left(a,x);mpfi_get_right(b,x);
mpfi_get_left(m1,m);mpfi_get_right(n1,m);
if(mpfr_sgn(a)<0){

mpfi_t l1,l2,ret1,ret2;
mpfi_inits2(precision_bits,ret1,ret2,l1,l2,(mpfi_ptr)0);
mpfr_neg(a,a,MPFR_RNDN);
mpfi_interv_fr(l1,one,a);mpfi_interv_fr(l2,zero,b);
mpfi_remainder_2(ret1,l1,m);mpfi_remainder_2(ret2,l2,m);
ret_val= mpfi_union(r,ret1,ret2);
mpfi_clears(ret1,ret2,l1,l2,(mpfi_ptr)0);
}else{
int test;
mpfr_t yd;
mpfr_inits2(precision_bits,yd,(mpfr_ptr)0);
test= mpfi_diam(yd,m);
if(test==0&&mpfr_zero_p(yd)> 0){

mpfr_t m2;
mpfr_inits2(precision_bits,m2,(mpfr_ptr)0);
mpfi_get_fr(m2,m);
ret_val= mpfi_remainder_1(r,x,m2);
mpfr_clears(m2,(mpfi_ptr)0);
}else if(mpfr_sgn(n1)<=0){

mpfi_neg(m,m);
ret_val= mpfi_remainder_2(r,x,m);
}else if(mpfr_sgn(m1)<=0){

mpfr_t r1;
mpfi_t l1;
mpfi_inits2(precision_bits,l1,(mpfi_ptr)0);
mpfr_inits2(precision_bits,r1,(mpfr_ptr)0);
mpfr_neg(m1,m1,MPFR_RNDN);

if(mpfr_greater_p(m1,n1)){
mpfr_set(r1,m1,MPFR_RNDN);
}else{
mpfr_set(r1,n1,MPFR_RNDN);
}
mpfi_interv_fr(l1,one,r1);
ret_val= mpfi_remainder_2(r,x,l1);
mpfr_clears(r1,(mpfr_ptr)0);
mpfi_clears(l1,(mpfi_ptr)0);
}else{
mpfr_t d;
mpfr_inits2(precision_bits,d,(mpfr_ptr)0);
mpfr_sub(d,b,a,MPFR_RNDN);
if(mpfr_greaterequal_p(d,n1)!=0){

mpfr_sub(n1,n1,one,MPFR_RNDN);
ret_val= mpfi_interv_fr(r,zero,n1);
}else if(mpfr_greaterequal_p(d,m1)!=0){

mpfi_t r1,l1,l2;
mpfi_inits2(precision_bits,l1,l2,(mpfi_ptr)0);
mpfr_sub(d,d,one,MPFR_RNDN);
mpfi_interv_fr(l1,zero,d);
mpfi_interv_fr(l2,d,n1);
mpfi_remainder_2(r1,x,l2);
ret_val= mpfi_union(r,l1,r1);
mpfi_clears(r1,l1,l2,(mpfi_ptr)0);
}else if(mpfr_greater_p(m1,b)!=0){

ret_val= mpfi_set(r,x);
}else if(mpfr_greater_p(n1,b)!=0){

mpfi_t l1;
mpfi_inits2(precision_bits,l1,(mpfi_ptr)0);
mpfi_interv_fr(l1,zero,b);
ret_val= mpfi_set(r,l1);
mpfi_clears(l1,(mpfi_ptr)0);
}else{

mpfi_t l1;
mpfi_inits2(precision_bits,l1,(mpfi_ptr)0);
mpfr_sub(n1,n1,one,MPFR_RNDN);
mpfi_interv_fr(l1,zero,n1);
ret_val= mpfi_set(r,l1);
mpfi_clears(l1,(mpfi_ptr)0);
}
mpfr_clears(d,(mpfr_ptr)0);
}
}
mpfr_clears(one,zero,(mpfr_ptr)0);
}
return ret_val;
}


/*:8*//*11:*/
#line 488 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void init_interval_constants(void){
if(!initialized){
mpfi_inits2(precision_bits,one,minusone,zero,two_mpfi_t,three_mpfi_t,four_mpfi_t,fraction_multiplier_mpfi_t,
fraction_one_mpfi_t,fraction_one_plus_mpfi_t,angle_multiplier_mpfi_t,PI_mpfi_t,
epsilon_mpfi_t,EL_GORDO_mpfi_t,(mpfi_ptr)0);
mpfi_set_si(one,1);
mpfi_set_si(minusone,-1);
mpfi_set_si(zero,0);
mpfi_set_si(two_mpfi_t,two);
mpfi_set_si(three_mpfi_t,three);
mpfi_set_si(four_mpfi_t,four);
mpfi_set_si(fraction_multiplier_mpfi_t,fraction_multiplier);
mpfi_set_si(fraction_one_mpfi_t,fraction_one);
mpfi_set_si(fraction_one_plus_mpfi_t,(fraction_one+1));
mpfi_set_si(angle_multiplier_mpfi_t,angle_multiplier);
mpfi_set_str(PI_mpfi_t,PI_STRING,10);
mpfi_set_d(epsilon_mpfi_t,epsilon);
mpfi_set_str(EL_GORDO_mpfi_t,EL_GORDO,10);
initialized= true;
}
}
void free_interval_constants(void){





}

/*:11*//*12:*/
#line 525 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void*mp_initialize_interval_math(MP mp){
math_data*math= (math_data*)mp_xmalloc(mp,1,sizeof(math_data));
precision_bits= precision_digits_to_bits(MAX_PRECISION);
init_interval_constants();

math->allocate= mp_new_number;
math->free= mp_free_number;
mp_new_number(mp,&math->precision_default,mp_scaled_type);
mpfi_set_d(math->precision_default.data.num,DEF_PRECISION);
mp_new_number(mp,&math->precision_max,mp_scaled_type);
mpfi_set_d(math->precision_max.data.num,MAX_PRECISION);
mp_new_number(mp,&math->precision_min,mp_scaled_type);

mpfi_set_d(math->precision_min.data.num,2.0);

mp_new_number(mp,&math->epsilon_t,mp_scaled_type);
mpfi_set(math->epsilon_t.data.num,epsilon_mpfi_t);
mp_new_number(mp,&math->inf_t,mp_scaled_type);
mpfi_set(math->inf_t.data.num,EL_GORDO_mpfi_t);
mp_new_number(mp,&math->warning_limit_t,mp_scaled_type);
mpfi_set_d(math->warning_limit_t.data.num,warning_limit);
mp_new_number(mp,&math->one_third_inf_t,mp_scaled_type);
mpfi_div(math->one_third_inf_t.data.num,math->inf_t.data.num,three_mpfi_t);
mp_new_number(mp,&math->unity_t,mp_scaled_type);
mpfi_set(math->unity_t.data.num,one);
mp_new_number(mp,&math->two_t,mp_scaled_type);
mpfi_set_si(math->two_t.data.num,two);
mp_new_number(mp,&math->three_t,mp_scaled_type);
mpfi_set_si(math->three_t.data.num,three);
mp_new_number(mp,&math->half_unit_t,mp_scaled_type);
mpfi_set_d(math->half_unit_t.data.num,half_unit);
mp_new_number(mp,&math->three_quarter_unit_t,mp_scaled_type);
mpfi_set_d(math->three_quarter_unit_t.data.num,three_quarter_unit);
mp_new_number(mp,&math->zero_t,mp_scaled_type);

mpfi_set_d(math->zero_t.data.num,0.0);

mp_new_number(mp,&math->arc_tol_k,mp_fraction_type);
{
mpfi_div_si(math->arc_tol_k.data.num,one,4096);

}
mp_new_number(mp,&math->fraction_one_t,mp_fraction_type);
mpfi_set_si(math->fraction_one_t.data.num,fraction_one);
mp_new_number(mp,&math->fraction_half_t,mp_fraction_type);
mpfi_set_si(math->fraction_half_t.data.num,fraction_half);
mp_new_number(mp,&math->fraction_three_t,mp_fraction_type);
mpfi_set_si(math->fraction_three_t.data.num,fraction_three);
mp_new_number(mp,&math->fraction_four_t,mp_fraction_type);
mpfi_set_si(math->fraction_four_t.data.num,fraction_four);

mp_new_number(mp,&math->three_sixty_deg_t,mp_angle_type);
mpfi_set_si(math->three_sixty_deg_t.data.num,360*angle_multiplier);
mp_new_number(mp,&math->one_eighty_deg_t,mp_angle_type);
mpfi_set_si(math->one_eighty_deg_t.data.num,180*angle_multiplier);

mp_new_number(mp,&math->one_k,mp_scaled_type);
mpfi_set_d(math->one_k.data.num,1.0/64);
mp_new_number(mp,&math->sqrt_8_e_k,mp_scaled_type);
{
mpfi_set_d(math->sqrt_8_e_k.data.num,112428.82793/65536.0);

}
mp_new_number(mp,&math->twelve_ln_2_k,mp_fraction_type);
{
mpfi_set_d(math->twelve_ln_2_k.data.num,139548959.6165/65536.0);

}
mp_new_number(mp,&math->coef_bound_k,mp_fraction_type);
mpfi_set_d(math->coef_bound_k.data.num,coef_bound);
mp_new_number(mp,&math->coef_bound_minus_1,mp_fraction_type);
mpfi_set_d(math->coef_bound_minus_1.data.num,coef_bound-1/65536.0);
mp_new_number(mp,&math->twelvebits_3,mp_scaled_type);
{
mpfi_set_d(math->twelvebits_3.data.num,1365/65536.0);

}
mp_new_number(mp,&math->twentysixbits_sqrt2_t,mp_fraction_type);
{
mpfi_set_d(math->twentysixbits_sqrt2_t.data.num,94906265.62/65536.0);

}
mp_new_number(mp,&math->twentyeightbits_d_t,mp_fraction_type);
{
mpfi_set_d(math->twentyeightbits_d_t.data.num,35596754.69/65536.0);

}
mp_new_number(mp,&math->twentysevenbits_sqrt2_d_t,mp_fraction_type);
{
mpfi_set_d(math->twentysevenbits_sqrt2_d_t.data.num,25170706.63/65536.0);

}

mp_new_number(mp,&math->fraction_threshold_t,mp_fraction_type);
mpfi_set_d(math->fraction_threshold_t.data.num,fraction_threshold);
mp_new_number(mp,&math->half_fraction_threshold_t,mp_fraction_type);
mpfi_set_d(math->half_fraction_threshold_t.data.num,half_fraction_threshold);
mp_new_number(mp,&math->scaled_threshold_t,mp_scaled_type);
mpfi_set_d(math->scaled_threshold_t.data.num,scaled_threshold);
mp_new_number(mp,&math->half_scaled_threshold_t,mp_scaled_type);
mpfi_set_d(math->half_scaled_threshold_t.data.num,half_scaled_threshold);
mp_new_number(mp,&math->near_zero_angle_t,mp_angle_type);
mpfi_set_d(math->near_zero_angle_t.data.num,near_zero_angle);
mp_new_number(mp,&math->p_over_v_threshold_t,mp_fraction_type);
mpfi_set_d(math->p_over_v_threshold_t.data.num,p_over_v_threshold);
mp_new_number(mp,&math->equation_threshold_t,mp_scaled_type);
mpfi_set_d(math->equation_threshold_t.data.num,equation_threshold);
mp_new_number(mp,&math->tfm_warn_threshold_t,mp_scaled_type);
mpfi_set_d(math->tfm_warn_threshold_t.data.num,tfm_warn_threshold);

math->from_int= mp_set_interval_from_int;
math->from_boolean= mp_set_interval_from_boolean;
math->from_scaled= mp_set_interval_from_scaled;
math->from_double= mp_set_interval_from_double;
math->from_addition= mp_set_interval_from_addition;
math->from_substraction= mp_set_interval_from_substraction;
math->from_oftheway= mp_set_interval_from_of_the_way;
math->from_div= mp_set_interval_from_div;
math->from_mul= mp_set_interval_from_mul;
math->from_int_div= mp_set_interval_from_int_div;
math->from_int_mul= mp_set_interval_from_int_mul;
math->negate= mp_number_negate;
math->add= mp_number_add;
math->substract= mp_number_substract;
math->half= mp_number_half;
math->halfp= mp_number_halfp;
math->do_double= mp_number_double;
math->abs= mp_interval_abs;
math->clone= mp_number_clone;
math->swap= mp_number_swap;
math->add_scaled= mp_number_add_scaled;
math->multiply_int= mp_number_multiply_int;
math->divide_int= mp_number_divide_int;
math->to_boolean= mp_number_to_boolean;
math->to_scaled= mp_number_to_scaled;
math->to_double= mp_number_to_double;
math->to_int= mp_number_to_int;
math->odd= mp_number_odd;
math->equal= mp_number_equal;
math->less= mp_number_less;
math->greater= mp_number_greater;
math->nonequalabs= mp_number_nonequalabs;
math->round_unscaled= mp_round_unscaled;
math->floor_scaled= mp_number_floor;
math->fraction_to_round_scaled= mp_interval_fraction_to_round_scaled;
math->make_scaled= mp_interval_number_make_scaled;
math->make_fraction= mp_interval_number_make_fraction;
math->take_fraction= mp_interval_number_take_fraction;
math->take_scaled= mp_interval_number_take_scaled;
math->velocity= mp_interval_velocity;
math->n_arg= mp_interval_n_arg;
math->m_log= mp_interval_m_log;
math->m_exp= mp_interval_m_exp;
math->m_unif_rand= mp_interval_m_unif_rand;
math->m_norm_rand= mp_interval_m_norm_rand;
math->pyth_add= mp_interval_pyth_add;
math->pyth_sub= mp_interval_pyth_sub;
math->fraction_to_scaled= mp_number_fraction_to_scaled;
math->scaled_to_fraction= mp_number_scaled_to_fraction;
math->scaled_to_angle= mp_number_scaled_to_angle;
math->angle_to_scaled= mp_number_angle_to_scaled;
math->init_randoms= mp_init_randoms;
math->sin_cos= mp_interval_sin_cos;
math->slow_add= mp_interval_slow_add;
math->sqrt= mp_interval_square_rt;
math->print= mp_interval_print_number;
math->tostring= mp_interval_number_tostring;
math->modulo= mp_interval_number_modulo;
math->ab_vs_cd= mp_ab_vs_cd;
math->crossing_point= mp_interval_crossing_point;
math->scan_numeric= mp_interval_scan_numeric_token;
math->scan_fractional= mp_interval_scan_fractional_token;
math->free_math= mp_free_interval_math;
math->set_precision= mp_interval_set_precision;

math->m_get_left_endpoint= mp_interval_m_get_left_endpoint;
math->m_get_right_endpoint= mp_interval_m_get_right_endpoint;
math->m_interval_set= mp_interval_m_interval_set;

return(void*)math;
}

void mp_interval_set_precision(MP mp){
double d= mpfi_get_d(internal_value(mp_number_precision).data.num);
precision_bits= precision_digits_to_bits(d);
}

void mp_free_interval_math(MP mp){
free_number(((math_data*)mp->math)->three_sixty_deg_t);
free_number(((math_data*)mp->math)->one_eighty_deg_t);
free_number(((math_data*)mp->math)->fraction_one_t);
free_number(((math_data*)mp->math)->zero_t);
free_number(((math_data*)mp->math)->half_unit_t);
free_number(((math_data*)mp->math)->three_quarter_unit_t);
free_number(((math_data*)mp->math)->unity_t);
free_number(((math_data*)mp->math)->two_t);
free_number(((math_data*)mp->math)->three_t);
free_number(((math_data*)mp->math)->one_third_inf_t);
free_number(((math_data*)mp->math)->inf_t);
free_number(((math_data*)mp->math)->warning_limit_t);
free_number(((math_data*)mp->math)->one_k);
free_number(((math_data*)mp->math)->sqrt_8_e_k);
free_number(((math_data*)mp->math)->twelve_ln_2_k);
free_number(((math_data*)mp->math)->coef_bound_k);
free_number(((math_data*)mp->math)->coef_bound_minus_1);
free_number(((math_data*)mp->math)->fraction_threshold_t);
free_number(((math_data*)mp->math)->half_fraction_threshold_t);
free_number(((math_data*)mp->math)->scaled_threshold_t);
free_number(((math_data*)mp->math)->half_scaled_threshold_t);
free_number(((math_data*)mp->math)->near_zero_angle_t);
free_number(((math_data*)mp->math)->p_over_v_threshold_t);
free_number(((math_data*)mp->math)->equation_threshold_t);
free_number(((math_data*)mp->math)->tfm_warn_threshold_t);
free_interval_constants();
free(mp->math);
}

/*:12*//*14:*/
#line 745 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_new_number(MP mp,mp_number*n,mp_number_type t){
(void)mp;
n->data.num= mp_xmalloc(mp,1,sizeof(mpfi_t));
mpfi_init2((mpfi_ptr)(n->data.num),precision_bits);

mpfi_set_d((mpfi_ptr)(n->data.num),0.0);
n->type= t;
}

/*:14*//*15:*/
#line 757 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_free_number(MP mp,mp_number*n){
(void)mp;
if(n->data.num){
mpfi_clear(n->data.num);
n->data.num= NULL;
}
n->type= mp_nan_type;
}

/*:15*//*16:*/
#line 769 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_set_interval_from_int(mp_number*A,int B){
mpfi_set_si(A->data.num,B);
}
void mp_set_interval_from_boolean(mp_number*A,int B){
mpfi_set_si(A->data.num,B);
}
void mp_set_interval_from_scaled(mp_number*A,int B){
mpfi_set_si(A->data.num,B);
mpfi_div_si(A->data.num,A->data.num,65536);
}
void mp_set_interval_from_double(mp_number*A,double B){
mpfi_set_d(A->data.num,B);
}
void mp_set_interval_from_addition(mp_number*A,mp_number B,mp_number C){
mpfi_add(A->data.num,B.data.num,C.data.num);
}
void mp_set_interval_from_substraction(mp_number*A,mp_number B,mp_number C){
mpfi_sub(A->data.num,B.data.num,C.data.num);
}
void mp_set_interval_from_div(mp_number*A,mp_number B,mp_number C){
mpfi_div(A->data.num,B.data.num,C.data.num);
}
void mp_set_interval_from_mul(mp_number*A,mp_number B,mp_number C){
mpfi_mul(A->data.num,B.data.num,C.data.num);
}
void mp_set_interval_from_int_div(mp_number*A,mp_number B,int C){
mpfi_div_si(A->data.num,B.data.num,C);
}
void mp_set_interval_from_int_mul(mp_number*A,mp_number B,int C){
mpfi_mul_si(A->data.num,B.data.num,C);
}
void mp_set_interval_from_of_the_way(MP mp,mp_number*A,mp_number t,mp_number B,mp_number C){
mpfi_t c,r1;
mpfi_init2(c,precision_bits);
mpfi_init2(r1,precision_bits);
mpfi_sub(c,B.data.num,C.data.num);
mp_interval_take_fraction(mp,r1,c,t.data.num);
mpfi_sub(A->data.num,B.data.num,r1);
mpfi_clear(c);
mpfi_clear(r1);
mp_check_mpfi_t(mp,A->data.num);
}
void mp_number_negate(mp_number*A){
mpfi_t c;
mpfi_init2(c,precision_bits);
mpfi_neg(c,A->data.num);
mpfi_set((mpfi_ptr)A->data.num,c);
mpfi_clear(c);
}
void mp_number_add(mp_number*A,mp_number B){
mpfi_add(A->data.num,A->data.num,B.data.num);
}
void mp_number_substract(mp_number*A,mp_number B){
mpfi_sub(A->data.num,A->data.num,B.data.num);
}
void mp_number_half(mp_number*A){
mpfi_div_si(A->data.num,A->data.num,2);
}
void mp_number_halfp(mp_number*A){
mpfi_div_si(A->data.num,A->data.num,2);
}
void mp_number_double(mp_number*A){
mpfi_mul_si(A->data.num,A->data.num,2);
}
void mp_number_add_scaled(mp_number*A,int B){
mpfi_add_d(A->data.num,A->data.num,B/65536.0);
}
void mp_number_multiply_int(mp_number*A,int B){
mpfi_mul_si(A->data.num,A->data.num,B);
}
void mp_number_divide_int(mp_number*A,int B){
mpfi_div_si(A->data.num,A->data.num,B);
}
void mp_interval_abs(mp_number*A){
mpfi_abs(A->data.num,A->data.num);
}
void mp_number_clone(mp_number*A,mp_number B){

mpfi_round_prec(A->data.num,precision_bits);
mpfi_set(A->data.num,(mpfi_ptr)B.data.num);
}
void mp_number_swap(mp_number*A,mp_number*B){
mpfi_swap(A->data.num,B->data.num);
}
void mp_number_fraction_to_scaled(mp_number*A){
A->type= mp_scaled_type;
mpfi_div(A->data.num,A->data.num,fraction_multiplier_mpfi_t);
}
void mp_number_angle_to_scaled(mp_number*A){
A->type= mp_scaled_type;
mpfi_div(A->data.num,A->data.num,angle_multiplier_mpfi_t);
}
void mp_number_scaled_to_fraction(mp_number*A){
A->type= mp_fraction_type;
mpfi_mul(A->data.num,A->data.num,fraction_multiplier_mpfi_t);
}
void mp_number_scaled_to_angle(mp_number*A){
A->type= mp_angle_type;
mpfi_mul(A->data.num,A->data.num,angle_multiplier_mpfi_t);
}


/*:16*//*18:*/
#line 878 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

int mp_number_to_scaled(mp_number A){
double v= mpfi_get_d(A.data.num);
return(int)(v*65536.0);
}

/*:18*//*19:*/
#line 888 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

int mp_number_to_int(mp_number A){
int32_t result= 0;
double temp;



if(mpfi_bounded_p(A.data.num)){
temp= ROUND(mpfi_get_d(A.data.num));
if((INT_MIN<=temp)&&(temp<=INT_MAX)){
result= (int)(temp);
}
}
return result;
}
int mp_number_to_boolean(mp_number A){
int32_t result= 0;
double temp;



if(mpfi_bounded_p(A.data.num)){
temp= ROUND(mpfi_get_d(A.data.num));
if((INT_MIN<=temp)&&(temp<=INT_MAX)){
result= (int)(temp);
}
}
return result;
}
double mp_number_to_double(mp_number A){
double res= 0.0;
if(mpfi_bounded_p(A.data.num)){
res= mpfi_get_d(A.data.num);
}
return res;
}
int mp_number_odd(mp_number A){
return odd(mp_number_to_int(A));
}
int mp_number_equal(mp_number A,mp_number B){

mpfr_t lA,rA,lB,rB;
int la,ra,lb,rb;
mpfr_inits2(precision_bits,lA,rA,lB,rB,(mpfr_ptr)0);
la= mpfi_get_left(lA,A.data.num);
lb= mpfi_get_left(lB,B.data.num);
ra= mpfi_get_right(rA,A.data.num);
rb= mpfi_get_right(rB,B.data.num);
return((la==0&&lb==0&&mpfr_equal_p(lA,lB)!=0)&&(ra==0&&rb==0&&mpfr_equal_p(rA,rB)!=0));
}
int mp_number_greater(mp_number A,mp_number B){

return mpfi_cmp(A.data.num,B.data.num)> 0;
}
int mp_number_less(mp_number A,mp_number B){

return mpfi_cmp(A.data.num,B.data.num)<0;
}
int mp_number_nonequalabs(mp_number A,mp_number B){
mpfi_t a,b;
int temp;
mpfi_abs(a,A.data.num);
mpfi_abs(b,B.data.num);

temp= mpfi_cmp(a,b);
return!(temp==0);
}

/*:19*//*22:*/
#line 978 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

char*mp_intervalnumber_tostring(mpfi_t n){





char*str= NULL,*buffer= NULL;
mpfr_exp_t exp= 0;
int neg= 0;
mpfr_t nn;
mpfr_init2(nn,precision_bits);
mpfi_mid(nn,n);
if((str= mpfr_get_str(NULL,&exp,10,0,nn,MPFR_ROUNDING))> 0){
int numprecdigits= precision_bits_to_digits(precision_bits);
if(*str=='-'){
neg= 1;
}
while(strlen(str)> 0&&*(str+strlen(str)-1)=='0'){
*(str+strlen(str)-1)= '\0';
}
buffer= malloc(strlen(str)+13+numprecdigits+1);





if(buffer){
int i= 0,j= 0;
if(neg){
buffer[i++]= '-';
j= 1;
}
if(strlen(str+j)==0){
buffer[i++]= '0';
}else{

if(exp<=numprecdigits&&exp> -6){
if(exp> 0){
buffer[i++]= str[j++];
while(--exp> 0){
buffer[i++]= (str[j]?str[j++]:'0');
}
if(str[j]){
buffer[i++]= '.';
while(str[j]){
buffer[i++]= str[j++];
}
}
}else{
int absexp;
buffer[i++]= '0';
buffer[i++]= '.';
absexp= -exp;
while(absexp--> 0){
buffer[i++]= '0';
}
while(str[j]){
buffer[i++]= str[j++];
}
}
}else{
buffer[i++]= str[j++];
if(str[j]){
buffer[i++]= '.';
while(str[j]){
buffer[i++]= str[j++];
}
}
{
char msg[256];
int k= 0;
mp_snprintf(msg,256,"%s%d",(exp> 0?"+":""),(int)(exp> 0?(exp-1):(exp-1)));
buffer[i++]= 'E';
while(msg[k]){
buffer[i++]= msg[k++];
}
}
}
}
buffer[i++]= '\0';
}
mpfr_free_str(str);
}
return buffer;
}
char*mp_interval_number_tostring(MP mp,mp_number n){
return mp_intervalnumber_tostring(n.data.num);
}


/*:22*//*23:*/
#line 1069 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_print_number(MP mp,mp_number n){
char*str= mp_interval_number_tostring(mp,n);
mp_print(mp,str);
free(str);
}




/*:23*//*24:*/
#line 1083 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_slow_add(MP mp,mp_number*ret,mp_number A,mp_number B){
mpfi_add(ret->data.num,A.data.num,B.data.num);
}

/*:24*//*25:*/
#line 1126 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_make_fraction(MP mp,mpfi_t ret,mpfi_t p,mpfi_t q){
mpfi_div(ret,p,q);
mp_check_mpfi_t(mp,ret);
mpfi_mul(ret,ret,fraction_multiplier_mpfi_t);
}
void mp_interval_number_make_fraction(MP mp,mp_number*ret,mp_number p,mp_number q){
mp_interval_make_fraction(mp,ret->data.num,p.data.num,q.data.num);
}

/*:25*//*27:*/
#line 1149 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_take_fraction(MP mp,mpfi_t ret,mpfi_t p,mpfi_t q){
mpfi_mul(ret,p,q);
mpfi_div(ret,ret,fraction_multiplier_mpfi_t);
}
void mp_interval_number_take_fraction(MP mp,mp_number*ret,mp_number p,mp_number q){
mp_interval_take_fraction(mp,ret->data.num,p.data.num,q.data.num);
}

/*:27*//*29:*/
#line 1171 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_number_take_scaled(MP mp,mp_number*ret,mp_number p_orig,mp_number q_orig){
mpfi_mul(ret->data.num,p_orig.data.num,q_orig.data.num);
}


/*:29*//*30:*/
#line 1183 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_number_make_scaled(MP mp,mp_number*ret,mp_number p_orig,mp_number q_orig){
mpfi_div(ret->data.num,p_orig.data.num,q_orig.data.num);
mp_check_mpfi_t(mp,ret->data.num);
}

/*:30*//*39:*/
#line 1210 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_wrapup_numeric_token(MP mp,unsigned char*start,unsigned char*stop){
int invalid= 0;
mpfi_t result;
size_t l= stop-start+1;
unsigned long lp,lpbit;
char*buf= mp_xmalloc(mp,l+1,1);
char*bufp= buf;
buf[l]= '\0';
mpfi_init2(result,precision_bits);
(void)strncpy(buf,(const char*)start,l);
invalid= mpfi_set_str(result,buf,10);

lp= (unsigned long)l;

if((*bufp=='-')||(*bufp=='+')||(*bufp=='0')||(*bufp=='.')){lp--;bufp++;}

lp= strchr(bufp,'.')?lp-1:lp;

bufp= buf+l-1;
while(*bufp=='0'){bufp--;lp= (((lp==0)||(lp==1))?1:lp-1);}

lp= lp> 0?lp:1;

lpbit= (unsigned long)ceil(lp/log10(2)+1);
free(buf);
bufp= NULL;
if(invalid==0){
set_cur_mod(result);

if(too_precise(lpbit)){
if(mpfi_positive_p((mpfi_ptr)(internal_value(mp_warning_check).data.num))&&
(mp->scanner_status!=tex_flushing)){
char msg[256];
const char*hlp[]= {"Continue and I'll try to cope",
"with that value; but it might be dangerous.",
"(Set warningcheck:=0 to suppress this message.)",
NULL};
mp_snprintf(msg,256,"Required precision is too high (%d vs. numberprecision = %f, required precision=%d bits vs internal precision=%f bits)",(unsigned int)lp,mpfi_get_d(internal_value(mp_number_precision).data.num),(int)lpbit,precision_bits);
;
mp_error(mp,msg,hlp,true);
}
}
}else if(mp->scanner_status!=tex_flushing){
const char*hlp[]= {"I could not handle this number specification",
"probably because it is out of range. Error:",
"",
NULL};
hlp[2]= strerror(errno);
mp_error(mp,"Enormous number has been reduced.",hlp,false);
;
set_cur_mod((mpfi_ptr)(((math_data*)(mp->math))->inf_t.data.num));
}
set_cur_cmd((mp_variable_type)mp_numeric_token);
mpfi_clear(result);
}

/*:39*//*40:*/
#line 1267 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static void find_exponent(MP mp){
if(mp->buffer[mp->cur_input.loc_field]=='e'||
mp->buffer[mp->cur_input.loc_field]=='E'){
mp->cur_input.loc_field++;
if(!(mp->buffer[mp->cur_input.loc_field]=='+'||
mp->buffer[mp->cur_input.loc_field]=='-'||
mp->char_class[mp->buffer[mp->cur_input.loc_field]]==digit_class)){
mp->cur_input.loc_field--;
return;
}
if(mp->buffer[mp->cur_input.loc_field]=='+'||
mp->buffer[mp->cur_input.loc_field]=='-'){
mp->cur_input.loc_field++;
}
while(mp->char_class[mp->buffer[mp->cur_input.loc_field]]==digit_class){
mp->cur_input.loc_field++;
}
}
}
void mp_interval_scan_fractional_token(MP mp,int n){
unsigned char*start= &mp->buffer[mp->cur_input.loc_field-1];
unsigned char*stop;
while(mp->char_class[mp->buffer[mp->cur_input.loc_field]]==digit_class){
mp->cur_input.loc_field++;
}
find_exponent(mp);
stop= &mp->buffer[mp->cur_input.loc_field-1];
mp_wrapup_numeric_token(mp,start,stop);
}


/*:40*//*41:*/
#line 1301 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_scan_numeric_token(MP mp,int n){
unsigned char*start= &mp->buffer[mp->cur_input.loc_field-1];
unsigned char*stop;
while(mp->char_class[mp->buffer[mp->cur_input.loc_field]]==digit_class){
mp->cur_input.loc_field++;
}
if(mp->buffer[mp->cur_input.loc_field]=='.'&&
mp->buffer[mp->cur_input.loc_field+1]!='.'){
mp->cur_input.loc_field++;
while(mp->char_class[mp->buffer[mp->cur_input.loc_field]]==digit_class){
mp->cur_input.loc_field++;
}
}
find_exponent(mp);
stop= &mp->buffer[mp->cur_input.loc_field-1];
mp_wrapup_numeric_token(mp,start,stop);
}

/*:41*//*43:*/
#line 1354 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_velocity(MP mp,mp_number*ret,mp_number st,mp_number ct,mp_number sf,
mp_number cf,mp_number t){
mpfi_t acc,num,denom;
mpfi_t r1,r2;
mpfi_t arg1,arg2;
mpfi_t i16,fone,fhalf,ftwo,sqrtfive;
mpfi_inits2(precision_bits,acc,num,denom,r1,r2,arg1,arg2,i16,fone,fhalf,ftwo,sqrtfive,(mpfi_ptr)0);
mpfi_set_si(i16,16);
mpfi_set_si(fone,fraction_one);
mpfi_set_si(fhalf,fraction_half);
mpfi_set_si(ftwo,fraction_two);
mpfi_set_si(sqrtfive,5);
mpfi_sqrt(sqrtfive,sqrtfive);
mpfi_div(arg1,sf.data.num,i16);
mpfi_sub(arg1,st.data.num,arg1);
mpfi_div(arg2,st.data.num,i16);
mpfi_sub(arg2,sf.data.num,arg2);
mp_interval_take_fraction(mp,acc,arg1,arg2);

mpfi_set(arg1,acc);
mpfi_sub(arg2,ct.data.num,cf.data.num);
mp_interval_take_fraction(mp,acc,arg1,arg2);

mpfi_sqrt(arg1,two_mpfi_t);
mpfi_mul(arg1,arg1,fone);
mp_interval_take_fraction(mp,r1,acc,arg1);
mpfi_add(num,ftwo,r1);

mpfi_sub(arg1,sqrtfive,one);
mpfi_mul(arg1,arg1,fhalf);
mpfi_mul(arg1,arg1,three_mpfi_t);

mpfi_sub(arg2,three_mpfi_t,sqrtfive);
mpfi_mul(arg2,arg2,fhalf);
mpfi_mul(arg2,arg2,three_mpfi_t);
mp_interval_take_fraction(mp,r1,ct.data.num,arg1);
mp_interval_take_fraction(mp,r2,cf.data.num,arg2);

mpfi_set_si(denom,fraction_three);
mpfi_add(denom,denom,r1);
mpfi_add(denom,denom,r2);


if(mpfi_cmp(t.data.num,one)!=0){
mpfi_div(num,num,t.data.num);
}
mpfi_set(r2,num);
mpfi_div(r2,r2,four_mpfi_t);

if(mpfi_cmp(denom,r2)<0){
mpfi_set_si(ret->data.num,fraction_four);
}else{
mp_interval_make_fraction(mp,ret->data.num,num,denom);
}
mpfi_clears(acc,num,denom,r1,r2,arg1,arg2,i16,fone,fhalf,ftwo,sqrtfive,(mpfi_ptr)0);
mp_check_mpfi_t(mp,ret->data.num);
}


/*:43*//*44:*/
#line 1419 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_ab_vs_cd(MP mp,mp_number*ret,mp_number a_orig,mp_number b_orig,mp_number c_orig,mp_number d_orig){
mpfi_t q,r,test;
mpfi_t a,b,c,d;
int cmp= 0;
(void)mp;
mpfi_inits2(precision_bits,q,r,test,a,b,c,d,(mpfi_ptr)0);
mpfi_set(a,(mpfi_ptr)a_orig.data.num);
mpfi_set(b,(mpfi_ptr)b_orig.data.num);
mpfi_set(c,(mpfi_ptr)c_orig.data.num);
mpfi_set(d,(mpfi_ptr)d_orig.data.num);

mpfi_mul(q,a,b);
mpfi_mul(r,c,d);
cmp= mpfi_cmp(q,r);
if(cmp==0){
mpfi_set(ret->data.num,zero);
goto RETURN;
}
if(cmp> 0){
mpfi_set(ret->data.num,one);
goto RETURN;
}
if(cmp<0){
mpfi_set(ret->data.num,minusone);
goto RETURN;
}


/*45:*/
#line 1492 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

if(mpfi_negative_p(a)){
mpfi_neg(a,a);
mpfi_neg(b,b);
}
if(mpfi_negative_p(c)){
mpfi_neg(c,c);
mpfi_neg(d,d);
}
if(!mpfi_positive_p(d)){
if(!mpfi_negative_p(b)){
if((mpfi_zero_p(a)||mpfi_zero_p(b))&&(mpfi_zero_p(c)||mpfi_zero_p(d)))
mpfi_set(ret->data.num,zero);
else
mpfi_set(ret->data.num,one);
goto RETURN;
}
if(mpfi_zero_p(d)){
if(mpfi_zero_p(a))
mpfi_set(ret->data.num,zero);
else
mpfi_set(ret->data.num,minusone);
goto RETURN;
}
mpfi_set(q,a);
mpfi_set(a,c);
mpfi_set(c,q);
mpfi_neg(q,b);
mpfi_neg(b,d);
mpfi_set(d,q);
}else if(!mpfi_positive_p(b)){
if(mpfi_negative_p(b)&&mpfi_positive_p(a)){
mpfi_set(ret->data.num,minusone);
goto RETURN;
}
if(mpfi_zero_p(c))
mpfi_set(ret->data.num,zero);
else
mpfi_set(ret->data.num,minusone);
goto RETURN;
}

/*:45*/
#line 1448 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
;
while(1){
mpfi_div(q,a,d);
mpfi_div(r,c,b);
cmp= mpfi_cmp(q,r);
if(cmp){
if(cmp> 1){
mpfi_set(ret->data.num,one);
}else{
mpfi_set(ret->data.num,minusone);
}
goto RETURN;
}
mpfi_remainder(q,a,d);
mpfi_remainder(r,c,b);
if(mpfi_zero_p(r)){
if(mpfi_zero_p(q)){
mpfi_set(ret->data.num,zero);
}else{
mpfi_set(ret->data.num,one);
}
goto RETURN;
}
if(mpfi_zero_p(q)){
mpfi_set(ret->data.num,minusone);
goto RETURN;
}
mpfi_set(a,b);
mpfi_set(b,q);
mpfi_set(c,d);
mpfi_set(d,r);
}
RETURN:
#if DEBUG
fprintf(stdout,"\n%f = ab_vs_cd(%f,%f,%f,%f)",mp_number_to_double(*ret),
mp_number_to_double(a_orig),mp_number_to_double(b_orig),
mp_number_to_double(c_orig),mp_number_to_double(d_orig));
#endif
#line 1486 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
 mp_check_mpfi_t(mp,ret->data.num);
mpfi_clears(q,r,test,a,b,c,d,(mpfi_ptr)0);
return;
}


/*:44*//*46:*/
#line 1567 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static void mp_interval_crossing_point(MP mp,mp_number*ret,mp_number aa,mp_number bb,mp_number cc){
mpfi_t a,b,c;
double d;
mpfi_t x,xx,x0,x1,x2;
mpfi_t scratch;
mpfi_inits2(precision_bits,a,b,c,x,xx,x0,x1,x2,scratch,(mpfi_ptr)0);
mpfi_set(a,(mpfi_ptr)aa.data.num);
mpfi_set(b,(mpfi_ptr)bb.data.num);
mpfi_set(c,(mpfi_ptr)cc.data.num);
if(mpfi_negative_p(a))
zero_crossing;
if(!mpfi_negative_p(c)){
if(!mpfi_negative_p(b)){
if(mpfi_positive_p(c)){
no_crossing;
}else if(mpfi_zero_p(a)&&mpfi_zero_p(b)){
no_crossing;
}else{
one_crossing;
}
}
if(mpfi_zero_p(a))
zero_crossing;
}else if(mpfi_zero_p(a)){
if(!mpfi_positive_p(b))
zero_crossing;
}


d= epsilonf;
mpfi_set(x0,a);
mpfi_sub(x1,a,b);
mpfi_sub(x2,b,c);
do{

mpfi_add(x,x1,x2);
mpfi_div(x,x,two_mpfi_t);
mpfi_add_d(x,x,1E-12);
mpfi_sub(scratch,x1,x0);

if(mpfi_cmp(scratch,x0)> 0){
mpfi_set(x2,x);
mpfi_add(x0,x0,x0);
d+= d;
}else{
mpfi_add(xx,scratch,x);

if(mpfi_cmp(xx,x0)> 0){
mpfi_set(x2,x);
mpfi_add(x0,x0,x0);
d+= d;
}else{
mpfi_sub(x0,x0,xx);

if(!(mpfi_cmp(x,x0)> 0)){
mpfi_add(scratch,x,x2);

if(!(mpfi_cmp(scratch,x0)> 0))
no_crossing;
}
mpfi_set(x1,x);
d= d+d+epsilonf;
}
}
}while(d<fraction_one);
mpfi_set_d(scratch,d);
mpfi_sub(ret->data.num,scratch,fraction_one_mpfi_t);
RETURN:
#if DEBUG
fprintf(stdout,"\n%f = crossing_point(%f,%f,%f)",mp_number_to_double(*ret),
mp_number_to_double(aa),mp_number_to_double(bb),mp_number_to_double(cc));
#endif
#line 1640 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
 mpfi_clears(a,b,c,x,xx,x0,x1,x2,scratch,(mpfi_ptr)0);
mp_check_mpfi_t(mp,ret->data.num);
return;
}


/*:46*//*48:*/
#line 1651 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

int mp_round_unscaled(mp_number x_orig){
double xx= mp_number_to_double(x_orig);
int x= (int)ROUND(xx);
return x;
}

/*:48*//*49:*/
#line 1660 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_number_floor(mp_number*i){

mpfr_t le,re;
mpfr_inits2(precision_bits,le,re,(mpfr_ptr)0);
mpfi_get_left(le,i->data.num);mpfi_get_right(re,i->data.num);
mpfr_rint_floor(le,le,MPFR_RNDD);mpfr_rint_floor(re,re,MPFR_RNDD);
mpfi_interv_fr(i->data.num,le,re);
mpfr_clears(le,re,(mpfr_ptr)0);
}

/*:49*//*50:*/
#line 1672 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_fraction_to_round_scaled(mp_number*x_orig){
x_orig->type= mp_scaled_type;
mpfi_div(x_orig->data.num,x_orig->data.num,fraction_multiplier_mpfi_t);
}



/*:50*//*52:*/
#line 1686 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_square_rt(MP mp,mp_number*ret,mp_number x_orig){
if(!mpfi_nonnegative_p((mpfi_ptr)x_orig.data.num)){
/*53:*/
#line 1697 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

{
if(mpfi_negative_p((mpfi_ptr)x_orig.data.num)){
char msg[256];
const char*hlp[]= {
"Since I don't take square roots of negative numbers,",
"I'm zeroing this one. Proceed, with fingers crossed.",
NULL};
char*xstr= mp_interval_number_tostring(mp,x_orig);
mp_snprintf(msg,256,"Square root of %s has been replaced by 0",xstr);
free(xstr);
;
mp_error(mp,msg,hlp,true);
}else if(mpfi_overlaps_zero_p((mpfi_ptr)x_orig.data.num)){
char msg[256];
const char*hlp[]= {
"Since I don't take square roots of intervals that contains negative and positive numbers,",
"I'm zeroing this one. Proceed, with fingers crossed.",
NULL};
char*xstr= mp_interval_number_tostring(mp,x_orig);
mp_snprintf(msg,256,"Square root of interval  [a,b] with a<0 and b>0 that contains %s has been replaced by 0",xstr);
free(xstr);
mp_error(mp,msg,hlp,true);
}

mpfi_set_d(ret->data.num,0.0);
return;
}


/*:53*/
#line 1689 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
;
}else{
mpfi_sqrt(ret->data.num,x_orig.data.num);
}
mp_check_mpfi_t(mp,ret->data.num);
}


/*:52*//*54:*/
#line 1729 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_pyth_add(MP mp,mp_number*ret,mp_number a_orig,mp_number b_orig){
mpfi_t a,b,asq,bsq;
mpfi_inits2(precision_bits,a,b,asq,bsq,(mpfi_ptr)0);
mpfi_set(a,(mpfi_ptr)a_orig.data.num);
mpfi_set(b,(mpfi_ptr)b_orig.data.num);


mpfi_sqr(asq,a);
mpfi_sqr(bsq,b);
mpfi_add(a,asq,bsq);
mpfi_sqrt(ret->data.num,a);
mp_check_mpfi_t(mp,ret->data.num);
mpfi_clears(a,b,asq,bsq,(mpfi_ptr)0);
}

/*:54*//*55:*/
#line 1747 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_pyth_sub(MP mp,mp_number*ret,mp_number a_orig,mp_number b_orig){
mpfi_t a,b,asq,bsq;
mpfi_inits2(precision_bits,a,b,asq,bsq,(mpfi_ptr)0);
mpfi_set(a,(mpfi_ptr)a_orig.data.num);
mpfi_set(b,(mpfi_ptr)b_orig.data.num);

if(!(mpfi_cmp(a,b)> 0)){
/*56:*/
#line 1769 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

{

if(mpfi_cmp(a,b)<0){
char msg[256];
const char*hlp[]= {
"Since I don't take square roots of negative numbers,",
"I'm zeroing this one. Proceed, with fingers crossed.",
NULL};
char*astr= mp_interval_number_tostring(mp,a_orig);
char*bstr= mp_interval_number_tostring(mp,b_orig);
mp_snprintf(msg,256,"Pythagorean subtraction %s+-+%s has been replaced by 0",astr,bstr);
free(astr);
free(bstr);
;
mp_error(mp,msg,hlp,true);
}

mpfi_set_d(a,0.0);
}


/*:56*/
#line 1755 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
;
}else{


mpfi_sqr(asq,a);
mpfi_sqr(bsq,b);
mpfi_sub(a,asq,bsq);
mpfi_sqrt(a,a);
}
mpfi_set(ret->data.num,a);
mp_check_mpfi_t(mp,ret->data.num);
}


/*:55*//*57:*/
#line 1793 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_m_interval_set(MP mp,mp_number*ret,mp_number a,mp_number b){
mpfi_t ret_val;
mpfi_init2(ret_val,precision_bits);
mpfi_interv_fr(ret_val,a.data.num,b.data.num);
mpfi_set(ret->data.num,ret_val);
mpfi_clear(ret_val);
}



/*:57*//*58:*/
#line 1806 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_m_get_left_endpoint(MP mp,mp_number*ret,mp_number x_orig){
mpfr_t ret_val;
mpfr_init2(ret_val,precision_bits);
mpfi_get_left(ret_val,x_orig.data.num);
mpfi_set_fr(ret->data.num,ret_val);
mpfr_clear(ret_val);
}

void mp_interval_m_get_right_endpoint(MP mp,mp_number*ret,mp_number x_orig){
mpfr_t ret_val;
mpfr_init2(ret_val,precision_bits);
mpfi_get_right(ret_val,x_orig.data.num);
mpfi_set_fr(ret->data.num,ret_val);
mpfr_clear(ret_val);
}


/*:58*//*59:*/
#line 1827 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_m_log(MP mp,mp_number*ret,mp_number x_orig){
if(!mpfi_positive_p((mpfi_ptr)x_orig.data.num)){
/*60:*/
#line 1839 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

{
char msg[256];
const char*hlp[]= {
"Since I don't take logs of non-positive numbers,",
"I'm zeroing this one. Proceed, with fingers crossed.",
NULL};
char*xstr= mp_interval_number_tostring(mp,x_orig);
mp_snprintf(msg,256,"Logarithm of %s has been replaced by 0",xstr);
free(xstr);
;
mp_error(mp,msg,hlp,true);

mpfi_set_d(ret->data.num,0.0);
}


/*:60*/
#line 1830 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
;
}else{
mpfi_log(ret->data.num,x_orig.data.num);
mp_check_mpfi_t(mp,ret->data.num);
mpfi_mul_si(ret->data.num,ret->data.num,256);
}
mp_check_mpfi_t(mp,ret->data.num);
}

/*:59*//*61:*/
#line 1859 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_m_exp(MP mp,mp_number*ret,mp_number x_orig){
mpfi_t temp;
mpfi_init2(temp,precision_bits);
mpfi_div_si(temp,x_orig.data.num,256);
mpfi_exp(ret->data.num,temp);
mp_check_mpfi_t(mp,ret->data.num);
mpfi_clear(temp);
}


/*:61*//*62:*/
#line 1873 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_n_arg(MP mp,mp_number*ret,mp_number x_orig,mp_number y_orig){
if(mpfi_zero_p((mpfi_ptr)x_orig.data.num)&&mpfi_zero_p((mpfi_ptr)y_orig.data.num)){
/*63:*/
#line 1896 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

{
const char*hlp[]= {
"The `angle' between two identical points is undefined.",
"I'm zeroing this one. Proceed, with fingers crossed.",
NULL};
mp_error(mp,"angle(0,0) is taken as zero",hlp,true);
;

mpfi_set_d(ret->data.num,0.0);
}


/*:63*/
#line 1876 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"
;
}else{
mpfi_t atan2val,oneeighty_angle;
mpfi_init2(atan2val,precision_bits);
mpfi_init2(oneeighty_angle,precision_bits);
ret->type= mp_angle_type;
mpfi_set_si(oneeighty_angle,180*angle_multiplier);
mpfi_div(oneeighty_angle,oneeighty_angle,PI_mpfi_t);


mpfi_atan2(atan2val,y_orig.data.num,x_orig.data.num);
mpfi_mul(ret->data.num,atan2val,oneeighty_angle);
checkZero((mpfi_ptr)ret->data.num);
mpfi_clear(atan2val);
mpfi_clear(oneeighty_angle);
}
mp_check_mpfi_t(mp,ret->data.num);
}


/*:62*//*65:*/
#line 1915 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_sin_cos(MP mp,mp_number z_orig,mp_number*n_cos,mp_number*n_sin){
mpfi_t rad;
mpfi_t one_eighty;
mpfi_init2(rad,precision_bits);
mpfi_init2(one_eighty,precision_bits);
mpfi_set_si(one_eighty,180*16);
mpfi_mul(rad,z_orig.data.num,PI_mpfi_t);
mpfi_div(rad,rad,one_eighty);

mpfi_sin(n_sin->data.num,rad);
mpfi_cos(n_cos->data.num,rad);

mpfi_mul(n_cos->data.num,n_cos->data.num,fraction_multiplier_mpfi_t);
mpfi_mul(n_sin->data.num,n_sin->data.num,fraction_multiplier_mpfi_t);
mp_check_mpfi_t(mp,n_cos->data.num);
mp_check_mpfi_t(mp,n_sin->data.num);
mpfi_clear(rad);
mpfi_clear(one_eighty);
}

/*:65*//*66:*/
#line 1939 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

#define KK 100                     
#define LL  37                     
#define MM (1L<<30)                
#define mod_diff(x,y) (((x)-(y))&(MM-1)) 

static long ran_x[KK];

static void ran_array(long aa[],int n)


{
register int i,j;
for(j= 0;j<KK;j++)aa[j]= ran_x[j];
for(;j<n;j++)aa[j]= mod_diff(aa[j-KK],aa[j-LL]);
for(i= 0;i<LL;i++,j++)ran_x[i]= mod_diff(aa[j-KK],aa[j-LL]);
for(;i<KK;i++,j++)ran_x[i]= mod_diff(aa[j-KK],ran_x[i-LL]);
}




#define QUALITY 1009 
static long ran_arr_buf[QUALITY];
static long ran_arr_dummy= -1,ran_arr_started= -1;
static long*ran_arr_ptr= &ran_arr_dummy;

#define TT  70   
#define is_odd(x)  ((x)&1)          

static void ran_start(long seed)

{
register int t,j;
long x[KK+KK-1];
register long ss= (seed+2)&(MM-2);
for(j= 0;j<KK;j++){
x[j]= ss;
ss<<= 1;if(ss>=MM)ss-= MM-2;
}
x[1]++;
for(ss= seed&(MM-1),t= TT-1;t;){
for(j= KK-1;j> 0;j--)x[j+j]= x[j],x[j+j-1]= 0;
for(j= KK+KK-2;j>=KK;j--)
x[j-(KK-LL)]= mod_diff(x[j-(KK-LL)],x[j]),
x[j-KK]= mod_diff(x[j-KK],x[j]);
if(is_odd(ss)){
for(j= KK;j> 0;j--)x[j]= x[j-1];
x[0]= x[KK];
x[LL]= mod_diff(x[LL],x[KK]);
}
if(ss)ss>>= 1;else t--;
}
for(j= 0;j<LL;j++)ran_x[j+KK-LL]= x[j];
for(;j<KK;j++)ran_x[j-LL]= x[j];
for(j= 0;j<10;j++)ran_array(x,KK+KK-1);
ran_arr_ptr= &ran_arr_started;
}

#define ran_arr_next() (*ran_arr_ptr>=0? *ran_arr_ptr++: ran_arr_cycle())
static long ran_arr_cycle(void)
{
if(ran_arr_ptr==&ran_arr_dummy)
ran_start(314159L);
ran_array(ran_arr_buf,QUALITY);
ran_arr_buf[KK]= -1;
ran_arr_ptr= ran_arr_buf+1;
return ran_arr_buf[0];
}




/*:66*//*67:*/
#line 2014 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_init_randoms(MP mp,int seed){
int j,jj,k;
int i;
j= abs(seed);
while(j>=fraction_one){
j= j/2;
}
k= 1;
for(i= 0;i<=54;i++){
jj= k;
k= j-k;
j= jj;
if(k<0)
k+= fraction_one;
mpfi_set_si(mp->randoms[(i*21)%55].data.num,j);
}
mp_new_randoms(mp);
mp_new_randoms(mp);
mp_new_randoms(mp);

ran_start((unsigned long)seed);

}

/*:67*//*68:*/
#line 2039 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

void mp_interval_number_modulo(mp_number*a,mp_number b){
mpfi_remainder(a->data.num,a->data.num,b.data.num);
}

/*:68*//*69:*/
#line 2046 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static void mp_next_unif_random(MP mp,mp_number*ret){
mp_number rop;
unsigned long int op;
float flt_op;
(void)mp;
mp_new_number(mp,&rop,mp_scaled_type);
op= (unsigned)ran_arr_next();
flt_op= op/(MM*1.0);
mpfi_set_d((mpfi_ptr)(rop.data.num),flt_op);
mp_number_clone(ret,rop);
free_number(rop);
}



/*:69*//*70:*/
#line 2064 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static void mp_next_random(MP mp,mp_number*ret){
if(mp->j_random==0)
mp_new_randoms(mp);
else
mp->j_random= mp->j_random-1;
mp_number_clone(ret,mp->randoms[mp->j_random]);
}

/*:70*//*71:*/
#line 2080 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static void mp_interval_m_unif_rand(MP mp,mp_number*ret,mp_number x_orig){





mp_number y;
mp_number x,abs_x;
mp_number u;
char*r;mpfr_exp_t e;
mpfr_t ret_val;
double ret_d;
mpfr_init2(ret_val,precision_bits);
new_fraction(y);
new_number(x);
new_number(abs_x);
new_number(u);
mp_number_clone(&x,x_orig);
mp_number_clone(&abs_x,x);
mp_interval_abs(&abs_x);
mp_next_unif_random(mp,&u);
mpfi_mul(y.data.num,abs_x.data.num,u.data.num);
free_number(u);
if(mp_number_equal(y,abs_x)){
mp_number_clone(ret,((math_data*)mp->math)->zero_t);
}else if(mp_number_greater(x,((math_data*)mp->math)->zero_t)){
mp_number_clone(ret,y);
}else{
mp_number_clone(ret,y);
mp_number_negate(ret);
}
r= mpfr_get_str(NULL,
&e,
10,
0,
ret_val,
MPFR_ROUNDING
);
mpfr_free_str(r);
free_number(abs_x);
free_number(x);
free_number(y);
ret_d= mpfr_get_d(ret_val,MPFR_ROUNDING);
mpfi_set_d(ret->data.num,ret_d);
}



/*:71*//*72:*/
#line 2133 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static void mp_interval_m_norm_rand(MP mp,mp_number*ret){
mp_number ab_vs_cd;
mp_number abs_x;
mp_number u;
mp_number r;
mp_number la,xa;
new_number(ab_vs_cd);
new_number(la);
new_number(xa);
new_number(abs_x);
new_number(u);
new_number(r);

do{
do{
mp_number v;
new_number(v);
mp_next_random(mp,&v);
mp_number_substract(&v,((math_data*)mp->math)->fraction_half_t);
mp_interval_number_take_fraction(mp,&xa,((math_data*)mp->math)->sqrt_8_e_k,v);
free_number(v);
mp_next_random(mp,&u);
mp_number_clone(&abs_x,xa);
mp_interval_abs(&abs_x);
}while(!mp_number_less(abs_x,u));
mp_interval_number_make_fraction(mp,&r,xa,u);
mp_number_clone(&xa,r);
mp_interval_m_log(mp,&la,u);
mp_set_interval_from_substraction(&la,((math_data*)mp->math)->twelve_ln_2_k,la);
mp_interval_ab_vs_cd(mp,&ab_vs_cd,((math_data*)mp->math)->one_k,la,xa,xa);
}while(mp_number_less(ab_vs_cd,((math_data*)mp->math)->zero_t));
mp_number_clone(ret,xa);
free_number(ab_vs_cd);
free_number(r);
free_number(abs_x);
free_number(la);
free_number(xa);
free_number(u);
}



/*:72*//*73:*/
#line 2180 "../../../source/texk/web2c/mplibdir/mpmathinterval.w"

static void mp_interval_ab_vs_cd(MP mp,mp_number*ret,mp_number a_orig,mp_number b_orig,mp_number c_orig,mp_number d_orig){
mpfi_t a,b,c,d;
mpfi_t ab,cd;

int cmp= 0;
(void)mp;
mpfi_inits2(precision_bits,a,b,c,d,ab,cd,(mpfi_ptr)0);
mpfi_set(a,(mpfi_ptr)a_orig.data.num);
mpfi_set(b,(mpfi_ptr)b_orig.data.num);
mpfi_set(c,(mpfi_ptr)c_orig.data.num);
mpfi_set(d,(mpfi_ptr)d_orig.data.num);

mpfi_mul(ab,a,b);
mpfi_mul(cd,c,d);

mpfi_set(ret->data.num,zero);
cmp= mpfi_cmp(ab,cd);
if(cmp){
if(cmp> 0)
mpfi_set(ret->data.num,one);
else
mpfi_set(ret->data.num,minusone);
}
mp_check_mpfi_t(mp,ret->data.num);
mpfi_clears(a,b,c,d,ab,cd,(mpfi_ptr)0);
return;
}

/*:73*/
