/*2:*/
#line 35 "../../../source/texk/web2c/mplibdir/mpxout.w"

#include <w2c/config.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdarg.h> 
#include <assert.h> 
#include <setjmp.h> 
#include <errno.h>  



#ifndef WIN32
#include <sys/types.h> 
#include <unistd.h> 
#endif
#line 51 "../../../source/texk/web2c/mplibdir/mpxout.w"

#ifdef WIN32
#include <io.h> 
#include <process.h> 
#else
#line 56 "../../../source/texk/web2c/mplibdir/mpxout.w"
#if HAVE_SYS_WAIT_H
# include <sys/wait.h> 
#endif
#line 59 "../../../source/texk/web2c/mplibdir/mpxout.w"
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#line 62 "../../../source/texk/web2c/mplibdir/mpxout.w"
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
#line 65 "../../../source/texk/web2c/mplibdir/mpxout.w"
#endif
#line 66 "../../../source/texk/web2c/mplibdir/mpxout.w"

#ifdef WIN32
#include <direct.h> 
#else
#line 70 "../../../source/texk/web2c/mplibdir/mpxout.w"
#if HAVE_DIRENT_H
# include <dirent.h> 
#else
#line 73 "../../../source/texk/web2c/mplibdir/mpxout.w"
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h> 
# endif
#line 77 "../../../source/texk/web2c/mplibdir/mpxout.w"
# if HAVE_SYS_DIR_H
#  include <sys/dir.h> 
# endif
#line 80 "../../../source/texk/web2c/mplibdir/mpxout.w"
# if HAVE_NDIR_H
#  include <ndir.h> 
# endif
#line 83 "../../../source/texk/web2c/mplibdir/mpxout.w"
#endif
#line 84 "../../../source/texk/web2c/mplibdir/mpxout.w"
#endif
#line 85 "../../../source/texk/web2c/mplibdir/mpxout.w"
#if HAVE_SYS_STAT_H
#include <sys/stat.h> 
#endif
#line 88 "../../../source/texk/web2c/mplibdir/mpxout.w"
#include <ctype.h> 
#include <time.h> 
#include <math.h> 
#define trunc(x)   ((integer) (x))
#define fabs(x)    ((x)<0?(-(x)):(x))
#define floor(x)   ((integer) (fabs(x)))
#ifndef PI
#define PI  3.14159265358979323846
#endif
#line 97 "../../../source/texk/web2c/mplibdir/mpxout.w"
#include "avl.h"
#include "mpxout.h"
#define true 1
#define false 0 \

#define MAXINT 0x7FFFFF \

#define incr(A) (A) = (A) +1
#define decr(A) (A) = (A) -1 \

#define mpx_jump_out longjmp(mpx->jump_buf,1)  \

#define xfree(A) do{mpx_xfree(A) ;A= NULL;}while(0) 
#define xrealloc(P,A,B) mpx_xrealloc(mpx,P,A,B) 
#define xmalloc(A,B) mpx_xmalloc(mpx,A,B) 
#define xstrdup(A) mpx_xstrdup(mpx,A)  \

#define max_size_test 0x7FFFFFFF \

#define VERBATIM_TEX 1
#define B_TEX 2
#define FIRST_VERBATIM_TEX 3 \

#define virtual_space 1000000
#define max_fonts 1000
#define max_fnums 3000
#define max_widths (256*max_fonts) 
#define line_length 79
#define stack_size 100
#define font_tolerance 0.00001 \
 \

#define bad_dvi(A) mpx_abort(mpx,"Bad DVI file: "A"!") 
#define bad_dvi_two(A,B) mpx_abort(mpx,"Bad DVI file: "A"!",B)  \
 \

#define printable(c) (isprint(c) &&c<128&&c!='"') 
#define xchr(A) (A)  \

#define id_byte 2 \

#define set_char_0 0
#define set1 128
#define set_rule 132
#define put1 133
#define put_rule 137
#define nop 138
#define bop 139
#define eop 140
#define push 141
#define pop 142
#define right1 143
#define w0 147
#define w1 148
#define x0 152
#define x1 153
#define down1 157
#define y0 161
#define y1 162
#define z0 166
#define z1 167
#define fnt_num_0 171
#define fnt1 235
#define xxx1 239
#define xxx4 242
#define fnt_def1 243
#define pre 247
#define post 248
#define post_post 249
#define undefined_commands 250:case 251:case 252:case 253:case 254:case 255 \

#define char_width(A,B) mpx->width[mpx->info_base[(A) ]+(B) ]
#define start_cmd(A,B) mpx->cmd_ptr[mpx->info_base[(A) ]+(B) ] \

#define font_warn(A,B) mpx_warn(mpx,"%s %s",A,mpx->font_name[(B) ]) 
#define font_error(A,B) mpx_error(mpx,"%s %s",A,mpx->font_name[(B) ]) 
#define font_abort(A,B) mpx_abort(mpx,"%s %s",A,mpx->font_name[(B) ])  \
 \

#define special 0
#define normal 1
#define initial 2 \

#define four_cases(A) (A) :case(A) +1:case(A) +2:case(A) +3
#define eight_cases(A) four_cases((A) ) :case four_cases((A) +4) 
#define sixteen_cases(A) eight_cases((A) ) :case eight_cases((A) +8) 
#define thirty_two_cases(A) sixteen_cases((A) ) :case sixteen_cases((A) +16) 
#define sixty_four_cases(A) thirty_two_cases((A) ) :case thirty_two_cases((A) +32)  \

#define max_named_colors 100 \

#define color_warn(A) mpx_warn(mpx,A) 
#define color_warn_two(A,B) mpx_warn(mpx,"%s%s",A,B)  \

#define XXX_BUF 256 \

#define max_color_stack_depth 10 \

#define SHIFTS 100
#define MAXCHARS 256 \

#define is_specchar(c) (!mpx->gflag&&(c) <=2) 
#define LWscale 0.03
#define YCORR 12.0 \

#define test_redo_search do{ \
if(deff==NULL)  \
deff= mpx_fsearch(mpx,cname,mpx_specchar_format) ; \
}while(0)  \

#define Speed ((float) (PI/4.0) )  \

#define dbname "trfonts.map"
#define adjname "trchars.adj" \

#define TMPNAME_EXT(a,b) {strcpy(a,tmpname) ;strcat(a,b) ;} \

#define split_command(a,b) mpx_do_split_command(mpx,a,&b,' ') 
#define split_pipes(a,b) mpx_do_split_command(mpx,a,&b,'|')  \

#define ERRLOG "mpxerr.log"
#define MPXLOG "makempx.log" \


#line 99 "../../../source/texk/web2c/mplibdir/mpxout.w"


/*:2*//*3:*/
#line 111 "../../../source/texk/web2c/mplibdir/mpxout.w"

typedef signed int web_integer;
typedef signed int web_boolean;
/*5:*/
#line 137 "../../../source/texk/web2c/mplibdir/mpxout.w"

/*8:*/
#line 168 "../../../source/texk/web2c/mplibdir/mpxout.w"

enum mpx_history_states{
mpx_spotless= 0,
mpx_cksum_trouble,
mpx_warning_given,
mpx_fatal_error
};


/*:8*//*131:*/
#line 2181 "../../../source/texk/web2c/mplibdir/mpxout.w"

typedef struct named_color_record{
const char*name;
const char*value;
}named_color_record;

/*:131*//*165:*/
#line 2596 "../../../source/texk/web2c/mplibdir/mpxout.w"

typedef struct{
char*name;
int num;
}avl_entry;

/*:165*//*190:*/
#line 3199 "../../../source/texk/web2c/mplibdir/mpxout.w"

typedef struct{
char*name;
char*mac;
}spec_entry;

/*:190*/
#line 138 "../../../source/texk/web2c/mplibdir/mpxout.w"

typedef struct mpx_data{
int mode;
/*9:*/
#line 177 "../../../source/texk/web2c/mplibdir/mpxout.w"

int history;

/*:9*//*11:*/
#line 188 "../../../source/texk/web2c/mplibdir/mpxout.w"

char*banner;
char*mpname;
FILE*mpfile;
char*mpxname;
FILE*mpxfile;
FILE*errfile;
int lnno;

/*:11*//*16:*/
#line 244 "../../../source/texk/web2c/mplibdir/mpxout.w"

jmp_buf jump_buf;

/*:16*//*23:*/
#line 375 "../../../source/texk/web2c/mplibdir/mpxout.w"

int texcnt;
int verbcnt;
char*bb,*tt,*aa;
char*buf;
unsigned bufsize;

/*:23*//*40:*/
#line 773 "../../../source/texk/web2c/mplibdir/mpxout.w"

FILE*dvi_file;
FILE*tfm_file;
FILE*vf_file;

/*:40*//*44:*/
#line 817 "../../../source/texk/web2c/mplibdir/mpxout.w"

char*cur_name;

/*:44*//*45:*/
#line 825 "../../../source/texk/web2c/mplibdir/mpxout.w"

int b0,b1,b2,b3;

/*:45*//*47:*/
#line 844 "../../../source/texk/web2c/mplibdir/mpxout.w"

web_boolean vf_reading;
unsigned char cmd_buf[(virtual_space+1)];
unsigned int buf_ptr;

/*:47*//*55:*/
#line 1019 "../../../source/texk/web2c/mplibdir/mpxout.w"

web_integer font_num[(max_fnums+1)];
web_integer internal_num[(max_fnums+1)];
web_boolean local_only[(max_fnums+1)];
char*font_name[(max_fonts+1)];
double font_scaled_size[(max_fonts+1)];
double font_design_size[(max_fonts+1)];
web_integer font_check_sum[(max_fonts+1)];
web_integer font_bc[(max_fonts+1)];
web_integer font_ec[(max_fonts+1)];
web_integer info_base[(max_fonts+1)];
web_integer width[(max_widths+1)];

web_integer fbase[(max_fonts+1)];
web_integer ftop[(max_fonts+1)];
web_integer cmd_ptr[(max_widths+1)];
unsigned int nfonts;
unsigned int vf_ptr;
unsigned int info_ptr;
unsigned int n_cmds;
unsigned int cur_fbase,cur_ftop;


/*:55*//*63:*/
#line 1135 "../../../source/texk/web2c/mplibdir/mpxout.w"

double dvi_per_fix;

/*:63*//*67:*/
#line 1190 "../../../source/texk/web2c/mplibdir/mpxout.w"

web_integer in_width[256];
web_integer tfm_check_sum;

/*:67*//*87:*/
#line 1499 "../../../source/texk/web2c/mplibdir/mpxout.w"

int state;
int print_col;

/*:87*//*93:*/
#line 1585 "../../../source/texk/web2c/mplibdir/mpxout.w"

web_integer h;
web_integer v;
double conv;
double mag;

/*:93*//*95:*/
#line 1613 "../../../source/texk/web2c/mplibdir/mpxout.w"

boolean font_used[(max_fonts+1)];
boolean fonts_used;
boolean rules_used;
web_integer str_h1;
web_integer str_v;
web_integer str_h2;
web_integer str_f;
double str_scale;


/*:95*//*107:*/
#line 1767 "../../../source/texk/web2c/mplibdir/mpxout.w"

web_integer pic_dp;web_integer pic_ht;web_integer pic_wd;

/*:107*//*111:*/
#line 1825 "../../../source/texk/web2c/mplibdir/mpxout.w"

web_integer w;web_integer x;web_integer y;web_integer z;

web_integer hstack[(stack_size+1)];
web_integer vstack[(stack_size+1)];
web_integer wstack[(stack_size+1)];
web_integer xstack[(stack_size+1)];
web_integer ystack[(stack_size+1)];
web_integer zstack[(stack_size+1)];
web_integer stk_siz;
double dvi_scale;

/*:111*//*124:*/
#line 2107 "../../../source/texk/web2c/mplibdir/mpxout.w"

web_integer k;web_integer p;
web_integer numerator;web_integer denominator;

/*:124*//*128:*/
#line 2166 "../../../source/texk/web2c/mplibdir/mpxout.w"

char*dviname;

/*:128*//*132:*/
#line 2189 "../../../source/texk/web2c/mplibdir/mpxout.w"

named_color_record named_colors[(max_named_colors+1)];

web_integer num_named_colors;

/*:132*//*142:*/
#line 2356 "../../../source/texk/web2c/mplibdir/mpxout.w"

web_integer color_stack_depth;
char*color_stack[(max_color_stack_depth+1)];

/*:142*//*155:*/
#line 2524 "../../../source/texk/web2c/mplibdir/mpxout.w"

int next_specfnt[(max_fnums+1)];
int shiftchar[SHIFTS];
float shifth[SHIFTS];
float shiftv[SHIFTS];
int shiftptr;
int shiftbase[(max_fnums+1)];
int specfnt;
int*specf_tail;
float cursize;
unsigned int curfont;
float Xslant;
float Xheight;
float sizescale;
int gflag;
float unit;

/*:155*//*158:*/
#line 2562 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx_file_finder find_file;

/*:158*//*169:*/
#line 2661 "../../../source/texk/web2c/mplibdir/mpxout.w"

char*arg_tail;

/*:169*//*174:*/
#line 2774 "../../../source/texk/web2c/mplibdir/mpxout.w"

avl_tree trfonts;

/*:174*//*179:*/
#line 2960 "../../../source/texk/web2c/mplibdir/mpxout.w"

avl_tree charcodes[(max_fnums+1)];

/*:179*//*182:*/
#line 3042 "../../../source/texk/web2c/mplibdir/mpxout.w"

boolean graphics_used;
float dmp_str_h1;
float dmp_str_v;
float dmp_str_h2;
float str_size;


/*:182*//*189:*/
#line 3194 "../../../source/texk/web2c/mplibdir/mpxout.w"

avl_tree spec_tab;

/*:189*//*197:*/
#line 3347 "../../../source/texk/web2c/mplibdir/mpxout.w"

float gx;
float gy;

/*:197*//*210:*/
#line 3840 "../../../source/texk/web2c/mplibdir/mpxout.w"

char tex[15];
int debug;
const char*progname;

/*:210*//*222:*/
#line 4113 "../../../source/texk/web2c/mplibdir/mpxout.w"

char*maincmd;

/*:222*/
#line 141 "../../../source/texk/web2c/mplibdir/mpxout.w"

}mpx_data;

/*:5*/
#line 114 "../../../source/texk/web2c/mplibdir/mpxout.w"

/*20:*/
#line 292 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_xfree(void*x);
static void*mpx_xrealloc(MPX mpx,void*p,size_t nmem,size_t size);
static void*mpx_xmalloc(MPX mpx,size_t nmem,size_t size);
static char*mpx_xstrdup(MPX mpX,const char*s);


/*:20*//*96:*/
#line 1628 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_prepare_font_use(MPX mpx);

/*:96*//*100:*/
#line 1653 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_first_use(MPX mpx,int f);

/*:100*//*134:*/
#line 2204 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_def_named_color(MPX mpx,const char*n,const char*v);

/*:134*//*159:*/
#line 2565 "../../../source/texk/web2c/mplibdir/mpxout.w"

static char*mpx_find_file(MPX mpx,const char*nam,const char*mode,int ftype);

/*:159*//*162:*/
#line 2580 "../../../source/texk/web2c/mplibdir/mpxout.w"

static FILE*mpx_fsearch(MPX mpx,const char*nam,int format);

/*:162*//*183:*/
#line 3052 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_slant_and_ht(MPX mpx);

/*:183*//*212:*/
#line 3866 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_erasetmp(MPX mpx);

/*:212*/
#line 115 "../../../source/texk/web2c/mplibdir/mpxout.w"


/*:3*//*7:*/
#line 159 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_initialize(MPX mpx){
memset(mpx,0,sizeof(struct mpx_data));
/*10:*/
#line 180 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->history= mpx_spotless;

/*:10*//*24:*/
#line 382 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->bufsize= 1000;

/*:24*//*48:*/
#line 849 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->vf_reading= false;
mpx->buf_ptr= virtual_space;

/*:48*//*56:*/
#line 1042 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->nfonts= 0;mpx->info_ptr= 0;mpx->font_name[0]= 0;
mpx->vf_ptr= max_fnums;
mpx->cur_fbase= 0;mpx->cur_ftop= 0;

/*:56*//*88:*/
#line 1503 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->state= initial;
mpx->print_col= 0;

/*:88*//*92:*/
#line 1572 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->state= initial;

/*:92*//*135:*/
#line 2212 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->num_named_colors= 0;
mpx_def_named_color(mpx,"Apricot","(1.0, 0.680006, 0.480006)");
mpx_def_named_color(mpx,"Aquamarine","(0.180006, 1.0, 0.7)");
mpx_def_named_color(mpx,"Bittersweet","(0.760012, 0.0100122, 0.0)");
mpx_def_named_color(mpx,"Black","(0.0, 0.0, 0.0)");
mpx_def_named_color(mpx,"Blue","(0.0, 0.0, 1.0)");
mpx_def_named_color(mpx,"BlueGreen","(0.15, 1.0, 0.669994)");
mpx_def_named_color(mpx,"BlueViolet","(0.1, 0.05, 0.960012)");
mpx_def_named_color(mpx,"BrickRed","(0.719994, 0.0, 0.0)");
mpx_def_named_color(mpx,"Brown","(0.4, 0.0, 0.0)");
mpx_def_named_color(mpx,"BurntOrange","(1.0, 0.489988, 0.0)");
mpx_def_named_color(mpx,"CadetBlue","(0.380006, 0.430006, 0.769994)");
mpx_def_named_color(mpx,"CarnationPink","(1.0, 0.369994, 1.0)");
mpx_def_named_color(mpx,"Cerulean","(0.0600122, 0.889988, 1.0)");
mpx_def_named_color(mpx,"CornflowerBlue","(0.35, 0.869994, 1.0)");
mpx_def_named_color(mpx,"Cyan","(0.0, 1.0, 1.0)");
mpx_def_named_color(mpx,"Dandelion","(1.0, 0.710012, 0.160012)");
mpx_def_named_color(mpx,"DarkOrchid","(0.6, 0.2, 0.8)");
mpx_def_named_color(mpx,"Emerald","(0.0, 1.0, 0.5)");
mpx_def_named_color(mpx,"ForestGreen","(0.0, 0.880006, 0.0)");
mpx_def_named_color(mpx,"Fuchsia","(0.45, 0.00998169, 0.919994)");
mpx_def_named_color(mpx,"Goldenrod","(1.0, 0.9, 0.160012)");
mpx_def_named_color(mpx,"Gray","(0.5, 0.5, 0.5)");
mpx_def_named_color(mpx,"Green","(0.0, 1.0, 0.0)");
mpx_def_named_color(mpx,"GreenYellow","(0.85, 1.0, 0.310012)");
mpx_def_named_color(mpx,"JungleGreen","(0.0100122, 1.0, 0.480006)");
mpx_def_named_color(mpx,"Lavender","(1.0, 0.519994, 1.0)");
mpx_def_named_color(mpx,"LimeGreen","(0.5, 1.0, 0.0)");
mpx_def_named_color(mpx,"Magenta","(1.0, 0.0, 1.0)");
mpx_def_named_color(mpx,"Mahogany","(0.65, 0.0, 0.0)");
mpx_def_named_color(mpx,"Maroon","(0.680006, 0.0, 0.0)");
mpx_def_named_color(mpx,"Melon","(1.0, 0.539988, 0.5)");
mpx_def_named_color(mpx,"MidnightBlue","(0.0, 0.439988, 0.569994)");
mpx_def_named_color(mpx,"Mulberry","(0.640018, 0.0800061, 0.980006)");
mpx_def_named_color(mpx,"NavyBlue","(0.0600122, 0.460012, 1.0)");
mpx_def_named_color(mpx,"OliveGreen","(0.0, 0.6, 0.0)");
mpx_def_named_color(mpx,"Orange","(1.0, 0.389988, 0.130006)");
mpx_def_named_color(mpx,"OrangeRed","(1.0, 0.0, 0.5)");
mpx_def_named_color(mpx,"Orchid","(0.680006, 0.360012, 1.0)");
mpx_def_named_color(mpx,"Peach","(1.0, 0.5, 0.3)");
mpx_def_named_color(mpx,"Periwinkle","(0.430006, 0.45, 1.0)");
mpx_def_named_color(mpx,"PineGreen","(0.0, 0.75, 0.160012)");
mpx_def_named_color(mpx,"Plum","(0.5, 0.0, 1.0)");
mpx_def_named_color(mpx,"ProcessBlue","(0.0399878, 1.0, 1.0)");
mpx_def_named_color(mpx,"Purple","(0.55, 0.139988, 1.0)");
mpx_def_named_color(mpx,"RawSienna","(0.55, 0.0, 0.0)");
mpx_def_named_color(mpx,"Red","(1.0, 0.0, 0.0)");
mpx_def_named_color(mpx,"RedOrange","(1.0, 0.230006, 0.130006)");
mpx_def_named_color(mpx,"RedViolet","(0.590018, 0.0, 0.660012)");
mpx_def_named_color(mpx,"Rhodamine","(1.0, 0.180006, 1.0)");
mpx_def_named_color(mpx,"RoyalBlue","(0.0, 0.5, 1.0)");
mpx_def_named_color(mpx,"RoyalPurple","(0.25, 0.1, 1.0)");
mpx_def_named_color(mpx,"RubineRed","(1.0, 0.0, 0.869994)");
mpx_def_named_color(mpx,"Salmon","(1.0, 0.469994, 0.619994)");
mpx_def_named_color(mpx,"SeaGreen","(0.310012, 1.0, 0.5)");
mpx_def_named_color(mpx,"Sepia","(0.3, 0.0, 0.0)");
mpx_def_named_color(mpx,"SkyBlue","(0.380006, 1.0, 0.880006)");
mpx_def_named_color(mpx,"SpringGreen","(0.739988, 1.0, 0.239988)");
mpx_def_named_color(mpx,"Tan","(0.860012, 0.580006, 0.439988)");
mpx_def_named_color(mpx,"TealBlue","(0.119994, 0.980006, 0.640018)");
mpx_def_named_color(mpx,"Thistle","(0.880006, 0.410012, 1.0)");
mpx_def_named_color(mpx,"Turquoise","(0.15, 1.0, 0.8)");
mpx_def_named_color(mpx,"Violet","(0.210012, 0.119994, 1.0)");
mpx_def_named_color(mpx,"VioletRed","(1.0, 0.189988, 1.0)");
mpx_def_named_color(mpx,"White","(1.0, 1.0, 1.0)");
mpx_def_named_color(mpx,"WildStrawberry","(1.0, 0.0399878, 0.610012)");
mpx_def_named_color(mpx,"Yellow","(1.0, 1.0, 0.0)");
mpx_def_named_color(mpx,"YellowGreen","(0.560012, 1.0, 0.260012)");
mpx_def_named_color(mpx,"YellowOrange","(1.0, 0.580006, 0.0)");

/*:135*//*143:*/
#line 2362 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->color_stack_depth= 0;

/*:143*//*156:*/
#line 2541 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->shiftptr= 0;
mpx->specfnt= (max_fnums+1);
mpx->specf_tail= &(mpx->specfnt);
mpx->unit= 0.0;
mpx->lnno= 0;
mpx->gflag= 0;
mpx->h= 0;mpx->v= 0;

/*:156*//*161:*/
#line 2577 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->find_file= mpx_find_file;

/*:161*/
#line 162 "../../../source/texk/web2c/mplibdir/mpxout.w"

}

/*:7*//*12:*/
#line 199 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_printf(MPX mpx,const char*header,const char*msg,va_list ap){
fprintf(mpx->errfile,"makempx %s: %s:",header,mpx->mpname);
if(mpx->lnno!=0)
fprintf(mpx->errfile,"%d:",mpx->lnno);
fprintf(mpx->errfile," ");
(void)vfprintf(mpx->errfile,msg,ap);
fprintf(mpx->errfile,"\n");
}

/*:12*//*13:*/
#line 209 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_report(MPX mpx,const char*msg,...){
va_list ap;
if(mpx->debug==0)return;
va_start(ap,msg);
mpx_printf(mpx,"debug",msg,ap);
va_end(ap);
if(mpx->history<mpx_warning_given)
mpx->history= mpx_cksum_trouble;
}

/*:13*//*14:*/
#line 220 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_warn(MPX mpx,const char*msg,...){
va_list ap;
va_start(ap,msg);
mpx_printf(mpx,"warning",msg,ap);
va_end(ap);
if(mpx->history<mpx_warning_given)
mpx->history= mpx_cksum_trouble;
}

/*:14*//*15:*/
#line 230 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_error(MPX mpx,const char*msg,...){
va_list ap;
va_start(ap,msg);
mpx_printf(mpx,"error",msg,ap);
va_end(ap);
mpx->history= mpx_warning_given;
}

/*:15*//*17:*/
#line 248 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_abort(MPX mpx,const char*msg,...){
va_list ap;
va_start(ap,msg);
fprintf(stderr,"fatal: ");
(void)vfprintf(stderr,msg,ap);
va_end(ap);
va_start(ap,msg);
mpx_printf(mpx,"fatal",msg,ap);
va_end(ap);
mpx->history= mpx_fatal_error;
mpx_erasetmp(mpx);
mpx_jump_out;
}

/*:17*//*19:*/
#line 274 "../../../source/texk/web2c/mplibdir/mpxout.w"

static FILE*mpx_xfopen(MPX mpx,const char*fname,const char*fmode){
FILE*f= fopen(fname,fmode);
if(f==NULL)
mpx_abort(mpx,"File open error for %s in mode %s",fname,fmode);
return f;
}
static void mpx_fclose(MPX mpx,FILE*file){
(void)mpx;
(void)fclose(file);
}

/*:19*//*21:*/
#line 304 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_xfree(void*x){
if(x!=NULL)free(x);
}
static void*mpx_xrealloc(MPX mpx,void*p,size_t nmem,size_t size){
void*w;
if((max_size_test/size)<nmem){
mpx_abort(mpx,"Memory size overflow");
}
w= realloc(p,(nmem*size));
if(w==NULL)mpx_abort(mpx,"Out of Memory");
return w;
}
static void*mpx_xmalloc(MPX mpx,size_t nmem,size_t size){
void*w;
if((max_size_test/size)<nmem){
mpx_abort(mpx,"Memory size overflow");
}
w= malloc(nmem*size);
if(w==NULL)mpx_abort(mpx,"Out of Memory");
return w;
}
static char*mpx_xstrdup(MPX mpx,const char*s){
char*w;
if(s==NULL)
return NULL;
w= strdup(s);
if(w==NULL)mpx_abort(mpx,"Out of Memory");
return w;
}
/*:21*//*22:*/
#line 338 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_newer(char*source,char*target){
struct stat source_stat,target_stat;
#if HAVE_SYS_STAT_H
if(stat(target,&target_stat)<0)return 0;
if(stat(source,&source_stat)<0)return 1;
#if HAVE_STRUCT_STAT_ST_MTIM
if(source_stat.st_mtim.tv_sec> target_stat.st_mtim.tv_sec||
(source_stat.st_mtim.tv_sec==target_stat.st_mtim.tv_sec&&
source_stat.st_mtim.tv_nsec>=target_stat.st_mtim.tv_nsec))
return 0;
#else
#line 350 "../../../source/texk/web2c/mplibdir/mpxout.w"
 if(source_stat.st_mtime>=target_stat.st_mtime)
return 0;
#endif
#line 353 "../../../source/texk/web2c/mplibdir/mpxout.w"
#endif
#line 354 "../../../source/texk/web2c/mplibdir/mpxout.w"
 return 1;
}



/*:22*//*25:*/
#line 387 "../../../source/texk/web2c/mplibdir/mpxout.w"

static char*mpx_getline(MPX mpx,FILE*mpfile){
int c;
unsigned loc= 0;
if(feof(mpfile))
return NULL;
if(mpx->buf==NULL)
mpx->buf= xmalloc(mpx->bufsize,1);
while((c= getc(mpfile))!=EOF&&c!='\n'&&c!='\r'){
mpx->buf[loc++]= (char)c;
if(loc==mpx->bufsize){
char*temp= mpx->buf;
unsigned n= mpx->bufsize+(mpx->bufsize>>4);
if(n> MAXINT)
mpx_abort(mpx,"Line is too long");
mpx->buf= xmalloc(n,1);
memcpy(mpx->buf,temp,mpx->bufsize);
free(temp);
mpx->bufsize= n;
}
}
mpx->buf[loc]= 0;
if(c=='\r'){
c= getc(mpfile);
if(c!='\n')
ungetc(c,mpfile);
}
mpx->lnno++;
return mpx->buf;
}


/*:25*//*26:*/
#line 422 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_match_str(const char*s,const char*t){
while(*t!=0){
if(*s!=*t)
return 0;
s++;
t++;
}
if((*s>='a'&&*s<='z')||(*s>='A'&&*s<='Z')||*s=='_')
return 0;
return 1;
}


/*:26*//*27:*/
#line 452 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_getbta(MPX mpx,char*s){
int ok= 1;
mpx->bb= s;
if(s==NULL){
mpx->tt= NULL;
mpx->aa= NULL;
return 0;
}
for(mpx->tt= mpx->bb;*(mpx->tt)!=0;mpx->tt++){
switch(*(mpx->tt)){
case'"':
case'%':
mpx->aa= mpx->tt+1;
return 1;
case'b':
if(ok&&mpx_match_str(mpx->tt,"btex")){
mpx->aa= mpx->tt+4;
return 1;
}else{
ok= 0;
}
break;
case'e':
if(ok&&mpx_match_str(mpx->tt,"etex")){
mpx->aa= mpx->tt+4;
return 1;
}else{
ok= 0;
}
break;
case'v':
if(ok&&mpx_match_str(mpx->tt,"verbatimtex")){
mpx->aa= mpx->tt+11;
return 1;
}else{
ok= 0;
}
break;
default:
if((*(mpx->tt)>='a'&&*(mpx->tt)<='z')||
(*(mpx->tt)>='A'&&*(mpx->tt)<='Z')||
(*(mpx->tt)=='_'))
ok= 0;
else
ok= 1;
}
}
mpx->aa= mpx->tt;
return 0;
}

/*:27*//*28:*/
#line 504 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_copy_mpto(MPX mpx,FILE*outfile,int textype){
char*s;
char*t;
char c;
char*res= NULL;
t= NULL;
do{
if(mpx->aa==NULL||*mpx->aa==0){
if((mpx->aa= mpx_getline(mpx,mpx->mpfile))==NULL){
mpx_error(mpx,"btex section does not end");
return;
}
}
if(mpx_getbta(mpx,mpx->aa)&&*(mpx->tt)=='e'){
s= mpx->tt;
}else{
if(mpx->tt==NULL){
mpx_error(mpx,"btex section does not end");
return;
}else if(*(mpx->tt)=='b'){
mpx_error(mpx,"btex in TeX mode");
return;
}else if(*(mpx->tt)=='v'){
mpx_error(mpx,"verbatimtex in TeX mode");
return;
}
s= mpx->aa;
}
c= *s;
*s= 0;
if(res==NULL){
res= xmalloc(strlen(mpx->bb)+2,1);
res= strncpy(res,mpx->bb,(strlen(mpx->bb)+1));
}else{
res= xrealloc(res,strlen(res)+strlen(mpx->bb)+2,1);
res= strncat(res,mpx->bb,strlen(mpx->bb));
}
if(c=='\0')
res= strcat(res,"\n");
*s= c;
}while(*(mpx->tt)!='e');
s= res;
if(textype==B_TEX){

for(s= res+strlen(res)-1;
s>=res&&(*s==' '||*s=='\t'||*s=='\r'||*s=='\n');s--);
t= s;
*(++s)= '\0';
}else{
t= s;
}
if(textype==B_TEX||textype==FIRST_VERBATIM_TEX){

for(s= res;
s<(res+strlen(res))&&(*s==' '||*s=='\t'||*s=='\r'
||*s=='\n');s++);
for(;*t!='\n'&&t> s;t--);
}
fprintf(outfile,"%s",s);
if(textype==B_TEX){


if((t!=s||*t!='%')&&mpx->mode==mpx_tex_mode)
fprintf(outfile,"%%");
}
free(res);
}


/*:28*//*29:*/
#line 576 "../../../source/texk/web2c/mplibdir/mpxout.w"

static const char*mpx_predoc[]= {"",".po 0\n"};
static const char*mpx_postdoc[]= {"\\end{document}\n",""};
static const char*mpx_pretex1[]= {
"\\gdef\\mpxshipout{\\shipout\\hbox\\bgroup%\n"
"  \\setbox0=\\hbox\\bgroup}%\n"
"\\gdef\\stopmpxshipout{\\egroup"
"  \\dimen0=\\ht0 \\advance\\dimen0\\dp0\n"
"  \\dimen1=\\ht0 \\dimen2=\\dp0\n"
"  \\setbox0=\\hbox\\bgroup\n"
"    \\box0\n"
"    \\ifnum\\dimen0>0 \\vrule width1sp height\\dimen1 depth\\dimen2 \n"
"    \\else \\vrule width1sp height1sp depth0sp\\relax\n"
"    \\fi\\egroup\n"
"  \\ht0=0pt \\dp0=0pt \\box0 \\egroup}\n"
"\\mpxshipout%% line %d %s\n",".lf %d %s\n"};
static const char*mpx_pretex[]= {"\\mpxshipout%% line %d %s\n",".bp\n.lf %d %s\n"};
static const char*mpx_posttex[]= {"\n\\stopmpxshipout\n","\n"};
static const char*mpx_preverb1[]= {"",".lf %d %s\n"};
static const char*mpx_preverb[]= {"%% line %d %s\n",".lf %d %s\n"};
static const char*mpx_postverb[]= {"\n","\n"};

/*:29*//*30:*/
#line 598 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_mpto(MPX mpx,char*tmpname,char*mptexpre){
FILE*outfile;
int verbatim_written= 0;
int mode= mpx->mode;
char*mpname= mpx->mpname;
if(mode==mpx_tex_mode){
TMPNAME_EXT(mpx->tex,".tex");
}else{
TMPNAME_EXT(mpx->tex,".i");
}
outfile= mpx_xfopen(mpx,mpx->tex,"wb");
if(mode==mpx_tex_mode){
FILE*fr;
if((fr= fopen(mptexpre,"r"))!=NULL){
size_t i;
char buf[512];
while((i= fread((void*)buf,1,512,fr))> 0){
fwrite((void*)buf,1,i,outfile);
}
mpx_fclose(mpx,fr);
}
}
mpx->mpfile= mpx_xfopen(mpx,mpname,"r");
fprintf(outfile,"%s",mpx_predoc[mode]);
while(mpx_getline(mpx,mpx->mpfile)!=NULL)
/*31:*/
#line 632 "../../../source/texk/web2c/mplibdir/mpxout.w"

{
mpx->aa= mpx->buf;
while(mpx_getbta(mpx,mpx->aa)){
if(*(mpx->tt)=='%'){
break;
}else if(*(mpx->tt)=='"'){
do{
if(!mpx_getbta(mpx,mpx->aa))
mpx_error(mpx,"string does not end");
}while(*(mpx->tt)!='"');
}else if(*(mpx->tt)=='b'){
if(mpx->texcnt++==0)
fprintf(outfile,mpx_pretex1[mode],mpx->lnno,mpname);
else
fprintf(outfile,mpx_pretex[mode],mpx->lnno,mpname);
mpx_copy_mpto(mpx,outfile,B_TEX);
fprintf(outfile,"%s",mpx_posttex[mode]);
}else if(*(mpx->tt)=='v'){
if(mpx->verbcnt++==0&&mpx->texcnt==0)
fprintf(outfile,mpx_preverb1[mode],mpx->lnno,mpname);
else
fprintf(outfile,mpx_preverb[mode],mpx->lnno,mpname);
if(!verbatim_written)
mpx_copy_mpto(mpx,outfile,FIRST_VERBATIM_TEX);
else
mpx_copy_mpto(mpx,outfile,VERBATIM_TEX);
verbatim_written= 1;
fprintf(outfile,"%s",mpx_postverb[mode]);
}else{
mpx_error(mpx,"unmatched etex");
}
}
}

/*:31*/
#line 624 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
fprintf(outfile,"%s",mpx_postdoc[mode]);
mpx_fclose(mpx,mpx->mpfile);
mpx_fclose(mpx,outfile);
mpx->lnno= 0;
}

/*:30*//*37:*/
#line 723 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_open_mpxfile(MPX mpx){
mpx->mpxfile= mpx_xfopen(mpx,mpx->mpxname,"wb");
}

/*:37*//*41:*/
#line 779 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_open_dvi_file(MPX mpx){
mpx->dvi_file= fopen(mpx->dviname,"rb");
if(mpx->dvi_file==NULL)
mpx_abort(mpx,"DVI generation failed");
}

/*:41*//*42:*/
#line 787 "../../../source/texk/web2c/mplibdir/mpxout.w"

static web_boolean mpx_open_tfm_file(MPX mpx){
mpx->tfm_file= mpx_fsearch(mpx,mpx->cur_name,mpx_tfm_format);
if(mpx->tfm_file==NULL)
mpx_abort(mpx,"Cannot find TFM %s",mpx->cur_name);
free(mpx->cur_name);
return true;
}

/*:42*//*43:*/
#line 799 "../../../source/texk/web2c/mplibdir/mpxout.w"

static web_boolean mpx_open_vf_file(MPX mpx){
if(mpx->vf_file)
fclose(mpx->vf_file);
mpx->vf_file= mpx_fsearch(mpx,mpx->cur_name,mpx_vf_format);
if(mpx->vf_file){
free(mpx->cur_name);
return true;
}
return false;
}

/*:43*//*46:*/
#line 831 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_read_tfm_word(MPX mpx){
mpx->b0= getc(mpx->tfm_file);
mpx->b1= getc(mpx->tfm_file);
mpx->b2= getc(mpx->tfm_file);
mpx->b3= getc(mpx->tfm_file);
}

/*:46*//*49:*/
#line 857 "../../../source/texk/web2c/mplibdir/mpxout.w"

static web_integer mpx_get_byte(MPX mpx){
unsigned char b;
/*50:*/
#line 908 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->vf_reading){
b= (unsigned char)getc(mpx->vf_file);
}else if(mpx->buf_ptr==virtual_space){
b= (unsigned char)getc(mpx->dvi_file);
}else{
b= mpx->cmd_buf[mpx->buf_ptr];
incr(mpx->buf_ptr);
}

/*:50*/
#line 860 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
return b;
}

static web_integer mpx_signed_byte(MPX mpx){
unsigned char b;
/*50:*/
#line 908 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->vf_reading){
b= (unsigned char)getc(mpx->vf_file);
}else if(mpx->buf_ptr==virtual_space){
b= (unsigned char)getc(mpx->dvi_file);
}else{
b= mpx->cmd_buf[mpx->buf_ptr];
incr(mpx->buf_ptr);
}

/*:50*/
#line 866 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
return(b<128?b:(b-256));
}

static web_integer mpx_get_two_bytes(MPX mpx){
unsigned char a,b;
a= 0;b= 0;
/*51:*/
#line 918 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->vf_reading){
a= (unsigned char)getc(mpx->vf_file);
b= (unsigned char)getc(mpx->vf_file);
}else if(mpx->buf_ptr==virtual_space){
a= (unsigned char)getc(mpx->dvi_file);
b= (unsigned char)getc(mpx->dvi_file);
}else if(mpx->buf_ptr+2> mpx->n_cmds){
mpx_abort(mpx,"Error detected while interpreting a virtual font");

}else{
a= mpx->cmd_buf[mpx->buf_ptr];
b= mpx->cmd_buf[mpx->buf_ptr+1];
mpx->buf_ptr+= 2;
}

/*:51*/
#line 873 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
return(a*(int)(256)+b);
}

static web_integer mpx_signed_pair(MPX mpx){
unsigned char a,b;
a= 0;b= 0;
/*51:*/
#line 918 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->vf_reading){
a= (unsigned char)getc(mpx->vf_file);
b= (unsigned char)getc(mpx->vf_file);
}else if(mpx->buf_ptr==virtual_space){
a= (unsigned char)getc(mpx->dvi_file);
b= (unsigned char)getc(mpx->dvi_file);
}else if(mpx->buf_ptr+2> mpx->n_cmds){
mpx_abort(mpx,"Error detected while interpreting a virtual font");

}else{
a= mpx->cmd_buf[mpx->buf_ptr];
b= mpx->cmd_buf[mpx->buf_ptr+1];
mpx->buf_ptr+= 2;
}

/*:51*/
#line 880 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(a<128)return(a*256+b);
else return((a-256)*256+b);
}

static web_integer mpx_get_three_bytes(MPX mpx){
unsigned char a,b,c;
a= 0;b= 0;c= 0;
/*52:*/
#line 934 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->vf_reading){
a= (unsigned char)getc(mpx->vf_file);
b= (unsigned char)getc(mpx->vf_file);
c= (unsigned char)getc(mpx->vf_file);
}else if(mpx->buf_ptr==virtual_space){
a= (unsigned char)getc(mpx->dvi_file);
b= (unsigned char)getc(mpx->dvi_file);
c= (unsigned char)getc(mpx->dvi_file);
}else if(mpx->buf_ptr+3> mpx->n_cmds){
mpx_abort(mpx,"Error detected while interpreting a virtual font");

}else{
a= mpx->cmd_buf[mpx->buf_ptr];
b= mpx->cmd_buf[mpx->buf_ptr+1];
c= mpx->cmd_buf[mpx->buf_ptr+2];
mpx->buf_ptr+= 3;
}

/*:52*/
#line 888 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
return((a*(int)(256)+b)*256+c);
}

static web_integer mpx_signed_trio(MPX mpx){
unsigned char a,b,c;
a= 0;b= 0;c= 0;
/*52:*/
#line 934 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->vf_reading){
a= (unsigned char)getc(mpx->vf_file);
b= (unsigned char)getc(mpx->vf_file);
c= (unsigned char)getc(mpx->vf_file);
}else if(mpx->buf_ptr==virtual_space){
a= (unsigned char)getc(mpx->dvi_file);
b= (unsigned char)getc(mpx->dvi_file);
c= (unsigned char)getc(mpx->dvi_file);
}else if(mpx->buf_ptr+3> mpx->n_cmds){
mpx_abort(mpx,"Error detected while interpreting a virtual font");

}else{
a= mpx->cmd_buf[mpx->buf_ptr];
b= mpx->cmd_buf[mpx->buf_ptr+1];
c= mpx->cmd_buf[mpx->buf_ptr+2];
mpx->buf_ptr+= 3;
}

/*:52*/
#line 895 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(a<128)return((a*(int)(256)+b)*256+c);
else return(((a-(int)(256))*256+b)*256+c);
}

static web_integer mpx_signed_quad(MPX mpx){
unsigned char a,b,c,d;
a= 0;b= 0;c= 0;d= 0;
/*53:*/
#line 953 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->vf_reading){
a= (unsigned char)getc(mpx->vf_file);
b= (unsigned char)getc(mpx->vf_file);
c= (unsigned char)getc(mpx->vf_file);
d= (unsigned char)getc(mpx->vf_file);
}else if(mpx->buf_ptr==virtual_space){
a= (unsigned char)getc(mpx->dvi_file);
b= (unsigned char)getc(mpx->dvi_file);
c= (unsigned char)getc(mpx->dvi_file);
d= (unsigned char)getc(mpx->dvi_file);
}else if(mpx->buf_ptr+4> mpx->n_cmds){
mpx_abort(mpx,"Error detected while interpreting a virtual font");

}else{
a= mpx->cmd_buf[mpx->buf_ptr];
b= mpx->cmd_buf[mpx->buf_ptr+1];
c= mpx->cmd_buf[mpx->buf_ptr+2];
d= mpx->cmd_buf[mpx->buf_ptr+3];
mpx->buf_ptr+= 4;
}

/*:53*/
#line 903 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(a<128)return(((a*(int)(256)+b)*256+c)*256+d);
else return((((a-256)*(int)(256)+b)*256+c)*256+d);
}

/*:49*//*57:*/
#line 1050 "../../../source/texk/web2c/mplibdir/mpxout.w"
/*89:*/
#line 1510 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_print_char(MPX mpx,unsigned char c){
web_integer l;
if(printable(c))l= 1;
else if(c<10)l= 5;
else if(c<100)l= 6;
else l= 7;
if(mpx->print_col+l> line_length-2){
if(mpx->state==normal){
fprintf(mpx->mpxfile,"\"");mpx->state= special;
}
fprintf(mpx->mpxfile,"\n");
mpx->print_col= 0;
}
/*90:*/
#line 1527 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->state==normal){
if(printable(c)){
fprintf(mpx->mpxfile,"%c",xchr(c));
}else{
fprintf(mpx->mpxfile,"\"&char%d",c);
mpx->print_col+= 2;
}
}else{
if(mpx->state==special){
fprintf(mpx->mpxfile,"&");
incr(mpx->print_col);
}
if(printable(c)){
fprintf(mpx->mpxfile,"\"%c",xchr(c));
incr(mpx->print_col);
}else{
fprintf(mpx->mpxfile,"char%d",c);
}
}
mpx->print_col+= l;
if(printable(c))
mpx->state= normal;
else
mpx->state= special

/*:90*/
#line 1524 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}

/*:89*//*91:*/
#line 1556 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_end_char_string(MPX mpx,web_integer l){
while(mpx->state> special){
fprintf(mpx->mpxfile,"\"");
incr(mpx->print_col);
decr(mpx->state);
}
if(mpx->print_col+l> line_length){
fprintf(mpx->mpxfile,"\n ");mpx->print_col= 0;
}
mpx->state= initial;
}

/*:91*/
#line 1050 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_print_font(MPX mpx,web_integer f){
if((f<0)||(f>=(int)mpx->nfonts)){
bad_dvi("Undefined font");
}else{
char*s= mpx->font_name[f];
while(*s){
mpx_print_char(mpx,(unsigned char)*s);
s++;
}
}
}

/*:57*//*59:*/
#line 1079 "../../../source/texk/web2c/mplibdir/mpxout.w"
/*64:*/
#line 1144 "../../../source/texk/web2c/mplibdir/mpxout.w"

static web_integer mpx_match_font(MPX mpx,unsigned ff,web_boolean exact){
unsigned f;
for(f= 0;f<mpx->nfonts;f++){
if(f!=ff){
/*65:*/
#line 1172 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(strcmp(mpx->font_name[f],mpx->font_name[ff]))
continue

/*:65*/
#line 1149 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(exact){
if(fabs(mpx->font_scaled_size[f]-mpx->font_scaled_size[ff])<=font_tolerance){
if(!mpx->vf_reading){
if(mpx->local_only[f]){
mpx->font_num[f]= mpx->font_num[ff];mpx->local_only[f]= false;
}else if(mpx->font_num[f]!=mpx->font_num[ff]){
continue;
}
}
break;
}
}else if(mpx->info_base[f]!=max_widths){
break;
}
}
}
if(f<mpx->nfonts){
/*66:*/
#line 1176 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(fabs(mpx->font_design_size[f]-mpx->font_design_size[ff])> font_tolerance){
font_error("Inconsistent design sizes given for ",ff);

}else if(mpx->font_check_sum[f]!=mpx->font_check_sum[ff]){
font_warn("Checksum mismatch for ",ff);

}

/*:66*/
#line 1167 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}
return(web_integer)f;
}

/*:64*/
#line 1079 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_define_font(MPX mpx,web_integer e){
unsigned i;
web_integer n;
web_integer k;
web_integer x;
if(mpx->nfonts==max_fonts)
mpx_abort(mpx,"DVItoMP capacity exceeded (max fonts=%d)!",max_fonts);

/*60:*/
#line 1097 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->vf_ptr==mpx->nfonts)
mpx_abort(mpx,"DVItoMP capacity exceeded (max font numbers=%d)",max_fnums);

if(mpx->vf_reading){
mpx->font_num[mpx->nfonts]= 0;i= mpx->vf_ptr;decr(mpx->vf_ptr);
}else{
i= mpx->nfonts;
}
mpx->font_num[i]= e

/*:60*/
#line 1088 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
/*61:*/
#line 1108 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->font_check_sum[mpx->nfonts]= mpx_signed_quad(mpx);
/*62:*/
#line 1124 "../../../source/texk/web2c/mplibdir/mpxout.w"

x= mpx_signed_quad(mpx);
k= 1;
while(mpx->x> 040000000){
x= x/2;k= k+k;
}
mpx->font_scaled_size[mpx->nfonts]= x*k/1048576.0;
if(mpx->vf_reading)
mpx->font_design_size[mpx->nfonts]= mpx_signed_quad(mpx)*mpx->dvi_per_fix/1048576.0;
else mpx->font_design_size[mpx->nfonts]= mpx_signed_quad(mpx)/1048576.0;

/*:62*/
#line 1110 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
n= mpx_get_byte(mpx);
n= n+mpx_get_byte(mpx);
mpx->font_name[mpx->nfonts]= xmalloc((size_t)(n+1),1);
for(k= 0;k<n;k++)
mpx->font_name[mpx->nfonts][k]= (char)mpx_get_byte(mpx);
mpx->font_name[mpx->nfonts][k]= 0

/*:61*/
#line 1089 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
mpx->internal_num[i]= mpx_match_font(mpx,mpx->nfonts,true);
if(mpx->internal_num[i]==(int)mpx->nfonts){
mpx->info_base[mpx->nfonts]= max_widths;
mpx->local_only[mpx->nfonts]= mpx->vf_reading;incr(mpx->nfonts);
}
}

/*:59*//*68:*/
#line 1203 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_in_TFM(MPX mpx,web_integer f){

web_integer k;
int lh;
int nw;
unsigned int wp;
/*69:*/
#line 1220 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx_read_tfm_word(mpx);lh= mpx->b2*(int)(256)+mpx->b3;
mpx_read_tfm_word(mpx);
mpx->font_bc[f]= mpx->b0*(int)(256)+mpx->b1;
mpx->font_ec[f]= mpx->b2*(int)(256)+mpx->b3;
if(mpx->font_ec[f]<mpx->font_bc[f])mpx->font_bc[f]= mpx->font_ec[f]+1;
if(mpx->info_ptr+(unsigned int)mpx->font_ec[f]-(unsigned int)mpx->font_bc[f]+1> max_widths)
mpx_abort(mpx,"DVItoMP capacity exceeded (width table size=%d)!",max_widths);

wp= mpx->info_ptr+(unsigned int)mpx->font_ec[f]-(unsigned int)mpx->font_bc[f]+1;
mpx_read_tfm_word(mpx);nw= mpx->b0*256+mpx->b1;
if((nw==0)||(nw> 256))
font_abort("Bad TFM file for ",f);

for(k= 1;k<=3+lh;k++){
if(feof(mpx->tfm_file))
font_abort("Bad TFM file for ",f);

mpx_read_tfm_word(mpx);
if(k==4){
if(mpx->b0<128)
mpx->tfm_check_sum= ((mpx->b0*(int)(256)+mpx->b1)*256+mpx->b2)*256+mpx->b3;
else
mpx->tfm_check_sum= (((mpx->b0-256)*(int)(256)+mpx->b1)*256+mpx->b2)*256+mpx->b3;
}
if(k==5){
if(mpx->mode==mpx_troff_mode){
mpx->font_design_size[f]= (((mpx->b0*(int)(256)+mpx->b1)*256+mpx->b2)*256+mpx->b3)/(65536.0*16);
}
}
}

/*:69*/
#line 1210 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
/*70:*/
#line 1252 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(wp> 0){
for(k= (int)mpx->info_ptr;k<=(int)wp-1;k++){
mpx_read_tfm_word(mpx);
if(mpx->b0> nw)
font_abort("Bad TFM file for ",f);

mpx->width[k]= mpx->b0;
}
}

/*:70*/
#line 1211 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
/*71:*/
#line 1268 "../../../source/texk/web2c/mplibdir/mpxout.w"

for(k= 0;k<=nw-1;k++){
mpx_read_tfm_word(mpx);
if(mpx->b0> 127)mpx->b0= mpx->b0-256;
mpx->in_width[k]= ((mpx->b0*0400+mpx->b1)*0400+mpx->b2)*0400+mpx->b3;
}

/*:71*/
#line 1212 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
/*74:*/
#line 1291 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->in_width[0]!=0)
font_abort("Bad TFM file for ",f);

mpx->info_base[f]= (int)(mpx->info_ptr-(unsigned int)mpx->font_bc[f]);
if(wp> 0){
for(k= (int)mpx->info_ptr;k<=(int)wp-1;k++){
mpx->width[k]= mpx->in_width[mpx->width[k]];
}
}


/*:74*/
#line 1213 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
mpx->fbase[f]= 0;mpx->ftop[f]= 0;
mpx->info_ptr= wp;
mpx_fclose(mpx,mpx->tfm_file);
return;
}

/*:68*//*75:*/
#line 1311 "../../../source/texk/web2c/mplibdir/mpxout.w"

/*115:*/
#line 1911 "../../../source/texk/web2c/mplibdir/mpxout.w"

static web_integer mpx_first_par(MPX mpx,unsigned int o){
switch(o){
case sixty_four_cases(set_char_0):
case sixty_four_cases(set_char_0+64):
return(web_integer)(o-set_char_0);
break;
case set1:case put1:case fnt1:case xxx1:case fnt_def1:
return mpx_get_byte(mpx);
break;
case set1+1:case put1+1:case fnt1+1:case xxx1+1:case fnt_def1+1:
return mpx_get_two_bytes(mpx);
break;
case set1+2:case put1+2:case fnt1+2:case xxx1+2:case fnt_def1+2:
return mpx_get_three_bytes(mpx);
break;
case right1:case w1:case x1:case down1:case y1:case z1:
return mpx_signed_byte(mpx);
break;
case right1+1:case w1+1:case x1+1:case down1+1:case y1+1:case z1+1:
return mpx_signed_pair(mpx);
break;
case right1+2:case w1+2:case x1+2:case down1+2:case y1+2:case z1+2:
return mpx_signed_trio(mpx);
break;
case set1+3:case set_rule:case put1+3:case put_rule:
case right1+3:case w1+3:case x1+3:case down1+3:case y1+3:case z1+3:
case fnt1+3:case xxx1+3:case fnt_def1+3:
return mpx_signed_quad(mpx);
break;
case nop:case bop:case eop:case push:case pop:case pre:case post:
case post_post:case undefined_commands:
return 0;
break;
case w0:return mpx->w;break;
case x0:return mpx->x;break;
case y0:return mpx->y;break;
case z0:return mpx->z;break;
case sixty_four_cases(fnt_num_0):
return(web_integer)(o-fnt_num_0);
break;
}
return 0;
}

/*:115*/
#line 1312 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_in_VF(MPX mpx,web_integer f){

web_integer p;
boolean was_vf_reading;
web_integer c;
web_integer limit;
web_integer w;
was_vf_reading= mpx->vf_reading;mpx->vf_reading= true;
/*76:*/
#line 1344 "../../../source/texk/web2c/mplibdir/mpxout.w"

p= mpx_get_byte(mpx);
if(p!=pre)
font_abort("Bad VF file for ",f);
p= mpx_get_byte(mpx);
if(p!=202)
font_abort("Bad VF file for ",f);
p= mpx_get_byte(mpx);
while(p--> 0)
(void)mpx_get_byte(mpx);
mpx->tfm_check_sum= mpx_signed_quad(mpx);
(void)mpx_signed_quad(mpx);

/*:76*/
#line 1321 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
/*77:*/
#line 1357 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->ftop[f]= (web_integer)mpx->vf_ptr;
if(mpx->vf_ptr==mpx->nfonts)
mpx_abort(mpx,"DVItoMP capacity exceeded (max font numbers=%d)",max_fnums);

decr(mpx->vf_ptr);
mpx->info_base[f]= (web_integer)mpx->info_ptr;
limit= max_widths-mpx->info_base[f];
mpx->font_bc[f]= limit;mpx->font_ec[f]= 0

/*:77*/
#line 1322 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
p= mpx_get_byte(mpx);
while(p>=fnt_def1){
if(p> fnt_def1+3)
font_abort("Bad VF file for ",f);
mpx_define_font(mpx,mpx_first_par(mpx,(unsigned int)p));
p= mpx_get_byte(mpx);
}
while(p<=242){
if(feof(mpx->vf_file))
font_abort("Bad VF file for ",f);
/*78:*/
#line 1367 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(p==242){
p= mpx_signed_quad(mpx);c= mpx_signed_quad(mpx);w= mpx_signed_quad(mpx);
if(c<0)
font_abort("Bad VF file for ",f);
}else{
c= mpx_get_byte(mpx);w= mpx_get_three_bytes(mpx);
}
if(c>=limit)
mpx_abort(mpx,"DVItoMP capacity exceeded (max widths=%d)",max_widths);

if(c<mpx->font_bc[f])mpx->font_bc[f]= c;
if(c> mpx->font_ec[f])mpx->font_ec[f]= c;
char_width(f,c)= w

/*:78*/
#line 1333 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
/*79:*/
#line 1382 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->n_cmds+(unsigned int)p>=virtual_space)
mpx_abort(mpx,"DVItoMP capacity exceeded (virtual font space=%d)",virtual_space);

start_cmd(f,c)= (web_integer)mpx->n_cmds;
while(p> 0){
mpx->cmd_buf[mpx->n_cmds]= (unsigned char)mpx_get_byte(mpx);
incr(mpx->n_cmds);decr(p);
}
mpx->cmd_buf[mpx->n_cmds]= eop;
incr(mpx->n_cmds)

/*:79*/
#line 1334 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
p= mpx_get_byte(mpx);
}
if(p==post){
/*80:*/
#line 1397 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->fbase[f]= (web_integer)(mpx->vf_ptr+1);
mpx->info_ptr= (unsigned int)(mpx->info_base[f]+mpx->font_ec[f]+1)


/*:80*/
#line 1338 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
mpx->vf_reading= was_vf_reading;
return;
}
}

/*:75*//*81:*/
#line 1411 "../../../source/texk/web2c/mplibdir/mpxout.w"

static web_integer mpx_select_font(MPX mpx,web_integer e){
int f;
int ff;
web_integer k;
/*82:*/
#line 1440 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->cur_ftop<=mpx->nfonts)
mpx->cur_ftop= mpx->nfonts;
mpx->font_num[mpx->cur_ftop]= e;
k= (web_integer)mpx->cur_fbase;
while((mpx->font_num[k]!=e)||mpx->local_only[k])incr(k);
if(k==(int)mpx->cur_ftop)
mpx_abort(mpx,"Undefined font selected");
f= mpx->internal_num[k]

/*:82*/
#line 1417 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(mpx->info_base[f]==max_widths){
ff= mpx_match_font(mpx,(unsigned)f,false);
if(ff<(int)mpx->nfonts){
/*83:*/
#line 1450 "../../../source/texk/web2c/mplibdir/mpxout.w"

{
mpx->font_bc[f]= mpx->font_bc[ff];
mpx->font_ec[f]= mpx->font_ec[ff];
mpx->info_base[f]= mpx->info_base[ff];
mpx->fbase[f]= mpx->fbase[ff];
mpx->ftop[f]= mpx->ftop[ff];
}

/*:83*/
#line 1421 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}else{
/*84:*/
#line 1463 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->cur_name= xstrdup(mpx->font_name[f])

/*:84*/
#line 1423 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(mpx_open_vf_file(mpx)){
mpx_in_VF(mpx,f);
}else{
if(!mpx_open_tfm_file(mpx))
font_abort("No TFM file found for ",f);

mpx_in_TFM(mpx,f);
}
/*85:*/
#line 1466 "../../../source/texk/web2c/mplibdir/mpxout.w"

{
if((mpx->font_check_sum[f]!=0)&&(mpx->tfm_check_sum!=0)&&
(mpx->font_check_sum[f]!=mpx->tfm_check_sum)){
font_warn("Checksum mismatch for ",f);

}
}

/*:85*/
#line 1433 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}
/*99:*/
#line 1647 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->font_used[f]= false;

/*:99*/
#line 1435 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}
return f;
}

/*:81*//*94:*/
#line 1591 "../../../source/texk/web2c/mplibdir/mpxout.w"
/*103:*/
#line 1672 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_finish_last_char(MPX mpx){
double m,x,y;

if(mpx->str_f>=0){
if(mpx->mode==mpx_tex_mode){
m= mpx->str_scale*mpx->font_scaled_size[mpx->str_f]*
mpx->mag/mpx->font_design_size[mpx->str_f];
x= mpx->conv*mpx->str_h1;
y= mpx->conv*(-mpx->str_v);
if((fabs(x)>=4096.0)||(fabs(y)>=4096.0)||(m>=4096.0)||(m<0)){
mpx_warn(mpx,"text is out of range");
mpx_end_char_string(mpx,60);
}else{
mpx_end_char_string(mpx,40);
}
fprintf(mpx->mpxfile,",_n%d,%1.5f,%1.4f,%1.4f,",mpx->str_f,m,x,y);
/*154:*/
#line 2494 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->color_stack_depth> 0){
fprintf(mpx->mpxfile," withcolor %s\n",mpx->color_stack[mpx->color_stack_depth]);
}


/*:154*/
#line 1689 "../../../source/texk/web2c/mplibdir/mpxout.w"

fprintf(mpx->mpxfile,");\n");
}else{
m= mpx->str_size/mpx->font_design_size[mpx->str_f];
x= mpx->dmp_str_h1*mpx->unit;
y= YCORR-mpx->dmp_str_v*mpx->unit;
if(fabs(x)>=4096.0||fabs(y)>=4096.0||m>=4096.0||m<0){
mpx_warn(mpx,"text out of range ignored");
mpx_end_char_string(mpx,67);
}else{
mpx_end_char_string(mpx,47);
}
fprintf(mpx->mpxfile,"), _n%d",mpx->str_f);
fprintf(mpx->mpxfile,",%.5f,%.4f,%.4f)",(m*1.00375),(x/100.0),y);
mpx_slant_and_ht(mpx);
fprintf(mpx->mpxfile,";\n");
}
mpx->str_f= -1;
}
}

/*:103*/
#line 1591 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_set_char(MPX mpx,web_integer f,web_integer c){
if((c<mpx->font_bc[f])||(c> mpx->font_ec[f]))
mpx_abort(mpx,"attempt to typeset invalid character %d",c);

if((mpx->h!=mpx->str_h2)||(mpx->v!=mpx->str_v)||
(f!=mpx->str_f)||(mpx->dvi_scale!=mpx->str_scale)){
if(mpx->str_f>=0){
mpx_finish_last_char(mpx);
}else if(!mpx->fonts_used){
/*98:*/
#line 1643 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx_prepare_font_use(mpx)


/*:98*/
#line 1601 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}
if(!mpx->font_used[f])
/*102:*/
#line 1666 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx_first_use(mpx,f);

/*:102*/
#line 1604 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
fprintf(mpx->mpxfile,"_s(");mpx->print_col= 3;
mpx->str_scale= mpx->dvi_scale;mpx->str_f= f;
mpx->str_v= mpx->v;mpx->str_h1= mpx->h;
}
mpx_print_char(mpx,(unsigned char)c);
mpx->str_h2= (web_integer)(mpx->h+/*72:*/
#line 1285 "../../../source/texk/web2c/mplibdir/mpxout.w"

floor(mpx->dvi_scale*mpx->font_scaled_size[f]*char_width(f,c))

/*:72*/
#line 1610 "../../../source/texk/web2c/mplibdir/mpxout.w"
);
}

/*:94*//*97:*/
#line 1631 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_prepare_font_use(MPX mpx){
unsigned k;
for(k= 0;k<mpx->nfonts;k++)
mpx->font_used[k]= false;
mpx->fonts_used= true;
fprintf(mpx->mpxfile,"string _n[];\n");
fprintf(mpx->mpxfile,"vardef _s(expr _t,_f,_m,_x,_y)(text _c)=\n");
fprintf(mpx->mpxfile,
"  addto _p also _t infont _f scaled _m shifted (_x,_y) _c; enddef;\n");
}

/*:97*//*101:*/
#line 1656 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_first_use(MPX mpx,int f){
mpx->font_used[f]= true;
fprintf(mpx->mpxfile,"_n%d=",f);
mpx->print_col= 6;
mpx_print_font(mpx,f);
mpx_end_char_string(mpx,1);
fprintf(mpx->mpxfile,";\n");
}

/*:101*//*104:*/
#line 1712 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_set_rule(MPX mpx,web_integer ht,web_integer wd){
double xx1,yy1,xx2,yy2,ww;

if(wd==1){
/*106:*/
#line 1762 "../../../source/texk/web2c/mplibdir/mpxout.w"

{
mpx->pic_wd= mpx->h;mpx->pic_dp= mpx->v;mpx->pic_ht= ht-mpx->v;
}

/*:106*/
#line 1717 "../../../source/texk/web2c/mplibdir/mpxout.w"

}else if((ht> 0)||(wd> 0)){
if(mpx->str_f>=0)
mpx_finish_last_char(mpx);
if(!mpx->rules_used){
mpx->rules_used= true;
fprintf(mpx->mpxfile,
"interim linecap:=0;\n"
"vardef _r(expr _a,_w)(text _t) =\n"
"  addto _p doublepath _a withpen pencircle scaled _w _t enddef;");
}
/*105:*/
#line 1739 "../../../source/texk/web2c/mplibdir/mpxout.w"

xx1= mpx->conv*mpx->h;
yy1= mpx->conv*(-mpx->v);
if(wd> ht){
xx2= xx1+mpx->conv*wd;
ww= mpx->conv*ht;
yy1= yy1+0.5*ww;
yy2= yy1;
}else{
yy2= yy1+mpx->conv*ht;
ww= mpx->conv*wd;
xx1= xx1+0.5*ww;
xx2= xx1;
}

/*:105*/
#line 1729 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if((fabs(xx1)>=4096.0)||(fabs(yy1)>=4096.0)||
(fabs(xx2)>=4096.0)||(fabs(yy2)>=4096.0)||(ww>=4096.0))
mpx_warn(mpx,"hrule or vrule is out of range");
fprintf(mpx->mpxfile,"_r((%1.4f,%1.4f)..(%1.4f,%1.4f), %1.4f,",xx1,yy1,xx2,yy2,ww);
/*154:*/
#line 2494 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx->color_stack_depth> 0){
fprintf(mpx->mpxfile," withcolor %s\n",mpx->color_stack[mpx->color_stack_depth]);
}


/*:154*/
#line 1734 "../../../source/texk/web2c/mplibdir/mpxout.w"

fprintf(mpx->mpxfile,");\n");
}
}

/*:104*//*108:*/
#line 1774 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_start_picture(MPX mpx){
mpx->fonts_used= false;
mpx->rules_used= false;
mpx->graphics_used= false;
mpx->str_f= -1;
mpx->str_v= 0;
mpx->str_h2= 0;
mpx->str_scale= 1.0;
mpx->dmp_str_v= 0.0;
mpx->dmp_str_h2= 0.0;
mpx->str_size= 0.0;
fprintf(mpx->mpxfile,
"begingroup save %s_p,_r,_s,_n; picture _p; _p=nullpicture;\n",
(mpx->mode==mpx_tex_mode?"":"_C,_D,"));
}

static void mpx_stop_picture(MPX mpx){
double w,h,dd;
if(mpx->str_f>=0)
mpx_finish_last_char(mpx);
if(mpx->mode==mpx_tex_mode){
/*109:*/
#line 1801 "../../../source/texk/web2c/mplibdir/mpxout.w"

dd= -mpx->pic_dp*mpx->conv;
w= mpx->conv*mpx->pic_wd;
h= mpx->conv*mpx->pic_ht;
fprintf(mpx->mpxfile,
"setbounds _p to (0,%1.4f)--(%1.4f,%1.4f)--\n"
" (%1.4f,%1.4f)--(0,%1.4f)--cycle;\n",dd,w,dd,w,h,h)

/*:109*/
#line 1796 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}
fprintf(mpx->mpxfile,"_p endgroup\n");
}

/*:108*//*113:*/
#line 1845 "../../../source/texk/web2c/mplibdir/mpxout.w"
/*137:*/
#line 2294 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_xxx(MPX mpx,web_integer p)
{
unsigned char buf[(XXX_BUF+1)];
web_integer l,r,m,k,len;
boolean found;
int bufsiz= XXX_BUF;
len= 0;
while((p> 0)&&(len<bufsiz)){
buf[len]= (unsigned char)mpx_get_byte(mpx);
decr(p);incr(len);
}
/*138:*/
#line 2325 "../../../source/texk/web2c/mplibdir/mpxout.w"

if((len<=5)
||(buf[0]!='c')
||(buf[1]!='o')
||(buf[2]!='l')
||(buf[3]!='o')
||(buf[4]!='r')
||(buf[5]!=' ')
)goto XXXX;

/*:138*/
#line 2306 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(p> 0){
color_warn("long \"color\" special ignored");
goto XXXX;
}
if(/*140:*/
#line 2343 "../../../source/texk/web2c/mplibdir/mpxout.w"

(len==9)&&
(buf[6]=='p')&&
(buf[7]=='o')&&
(buf[8]=='p')

/*:140*/
#line 2311 "../../../source/texk/web2c/mplibdir/mpxout.w"
){
/*144:*/
#line 2367 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx_finish_last_char(mpx);
if(mpx->color_stack_depth> 0){
free(mpx->color_stack[mpx->color_stack_depth]);
decr(mpx->color_stack_depth);
}else{
color_warn("color stack underflow");
}

/*:144*/
#line 2312 "../../../source/texk/web2c/mplibdir/mpxout.w"

}else if(/*139:*/
#line 2335 "../../../source/texk/web2c/mplibdir/mpxout.w"

(len>=11)&&
(buf[6]=='p')&&
(buf[7]=='u')&&
(buf[8]=='s')&&
(buf[9]=='h')&&
(buf[10]==' ')

/*:139*/
#line 2313 "../../../source/texk/web2c/mplibdir/mpxout.w"
){
/*145:*/
#line 2378 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx_finish_last_char(mpx);
if(mpx->color_stack_depth>=max_color_stack_depth)
mpx_abort(mpx,"color stack overflow");
incr(mpx->color_stack_depth);


l= 11;
while((l<len-1)&&(buf[l]==' '))incr(l);
if(/*146:*/
#line 2397 "../../../source/texk/web2c/mplibdir/mpxout.w"

(l+4<len)
&&(buf[l]=='r')
&&(buf[l+1]=='g')
&&(buf[l+2]=='b')
&&(buf[l+3]==' ')

/*:146*/
#line 2387 "../../../source/texk/web2c/mplibdir/mpxout.w"
){
/*147:*/
#line 2404 "../../../source/texk/web2c/mplibdir/mpxout.w"

l= l+4;
while((l<len)&&(buf[l]==' '))incr(l);
while((len> l)&&(buf[len-1]==' '))decr(len);
mpx->color_stack[mpx->color_stack_depth]= xmalloc((size_t)(len-l+3),1);
k= 0;
/*152:*/
#line 2447 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->color_stack[mpx->color_stack_depth][k]= '(';
incr(k);
while(l<len){
if(buf[l]==' '){
mpx->color_stack[mpx->color_stack_depth][k]= ',';
while((l<len)&&(buf[l]==' '))incr(l);
incr(k);
}else{
mpx->color_stack[mpx->color_stack_depth][k]= (char)buf[l];
incr(l);
incr(k);
}
}
mpx->color_stack[mpx->color_stack_depth][k]= ')';
mpx->color_stack[mpx->color_stack_depth][k+1]= 0;

/*:152*/
#line 2410 "../../../source/texk/web2c/mplibdir/mpxout.w"


/*:147*/
#line 2388 "../../../source/texk/web2c/mplibdir/mpxout.w"

}else if(/*150:*/
#line 2429 "../../../source/texk/web2c/mplibdir/mpxout.w"

(l+5<len)
&&(buf[l]=='c')
&&(buf[l+1]=='m')
&&(buf[l+2]=='y')
&&(buf[l+3]=='k')
&&(buf[l+4]==' ')

/*:150*/
#line 2389 "../../../source/texk/web2c/mplibdir/mpxout.w"
){
/*151:*/
#line 2437 "../../../source/texk/web2c/mplibdir/mpxout.w"

l= l+5;
while((l<len)&&(buf[l]==' '))incr(l);

while((len> l)&&(buf[len-1]==' '))decr(len);
mpx->color_stack[mpx->color_stack_depth]= xmalloc((size_t)(len-l+7),1);
strcpy(mpx->color_stack[mpx->color_stack_depth],"cmyk");
k= 4;
/*152:*/
#line 2447 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->color_stack[mpx->color_stack_depth][k]= '(';
incr(k);
while(l<len){
if(buf[l]==' '){
mpx->color_stack[mpx->color_stack_depth][k]= ',';
while((l<len)&&(buf[l]==' '))incr(l);
incr(k);
}else{
mpx->color_stack[mpx->color_stack_depth][k]= (char)buf[l];
incr(l);
incr(k);
}
}
mpx->color_stack[mpx->color_stack_depth][k]= ')';
mpx->color_stack[mpx->color_stack_depth][k+1]= 0;

/*:152*/
#line 2445 "../../../source/texk/web2c/mplibdir/mpxout.w"


/*:151*/
#line 2390 "../../../source/texk/web2c/mplibdir/mpxout.w"

}else if(/*148:*/
#line 2412 "../../../source/texk/web2c/mplibdir/mpxout.w"

(l+5<len)
&&(buf[l]=='g')
&&(buf[l+1]=='r')
&&(buf[l+2]=='a')
&&(buf[l+3]=='y')
&&(buf[l+4]==' ')

/*:148*/
#line 2391 "../../../source/texk/web2c/mplibdir/mpxout.w"
){
/*149:*/
#line 2420 "../../../source/texk/web2c/mplibdir/mpxout.w"

l= l+5;
while((l<len)&&(buf[l]==' '))incr(l);
while((len> l)&&(buf[len-1]==' '))decr(len);
mpx->color_stack[mpx->color_stack_depth]= xmalloc((size_t)(len-l+9),1);
strcpy(mpx->color_stack[mpx->color_stack_depth],"white*");
k= 6;
/*152:*/
#line 2447 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->color_stack[mpx->color_stack_depth][k]= '(';
incr(k);
while(l<len){
if(buf[l]==' '){
mpx->color_stack[mpx->color_stack_depth][k]= ',';
while((l<len)&&(buf[l]==' '))incr(l);
incr(k);
}else{
mpx->color_stack[mpx->color_stack_depth][k]= (char)buf[l];
incr(l);
incr(k);
}
}
mpx->color_stack[mpx->color_stack_depth][k]= ')';
mpx->color_stack[mpx->color_stack_depth][k+1]= 0;

/*:152*/
#line 2427 "../../../source/texk/web2c/mplibdir/mpxout.w"


/*:149*/
#line 2392 "../../../source/texk/web2c/mplibdir/mpxout.w"

}else{
/*153:*/
#line 2467 "../../../source/texk/web2c/mplibdir/mpxout.w"

for(k= l;k<=len-1;k++){
buf[k-l]= xchr(buf[k]);
}
buf[len-l]= 0;

l= 1;r= mpx->num_named_colors;
found= false;
while((l<=r)&&!found){
m= (l+r)/2;k= strcmp((char*)(buf),mpx->named_colors[m].name);
if(k==0){
mpx->color_stack[mpx->color_stack_depth]= xstrdup(mpx->named_colors[m].value);
found= true;
}else if(k<0){
r= m-1;
}else{
l= m+1;
}
}
if(!found){
color_warn_two("non-hardcoded color \"%s\" in \"color push\" command",buf);
mpx->color_stack[mpx->color_stack_depth]= xstrdup((char*)(buf));
}

/*:153*/
#line 2394 "../../../source/texk/web2c/mplibdir/mpxout.w"

}

/*:145*/
#line 2314 "../../../source/texk/web2c/mplibdir/mpxout.w"

}else{
color_warn("unknown \"color\" special ignored");
goto XXXX;
}
XXXX:
for(k= 1;k<=p;k++)(void)mpx_get_byte(mpx);
}

/*:137*/
#line 1845 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_push(MPX mpx){
if(mpx->stk_siz==stack_size)
mpx_abort(mpx,"DVItoMP capacity exceeded (stack size=%d)",stack_size);

mpx->hstack[mpx->stk_siz]= mpx->h;
mpx->vstack[mpx->stk_siz]= mpx->v;mpx->wstack[mpx->stk_siz]= mpx->w;
mpx->xstack[mpx->stk_siz]= mpx->x;
mpx->ystack[mpx->stk_siz]= mpx->y;mpx->zstack[mpx->stk_siz]= mpx->z;
incr(mpx->stk_siz);
}

static void mpx_do_pop(MPX mpx){
if(mpx->stk_siz==0)
bad_dvi("attempt to pop empty stack");
else{
decr(mpx->stk_siz);
mpx->h= mpx->hstack[mpx->stk_siz];
mpx->v= mpx->vstack[mpx->stk_siz];mpx->w= mpx->wstack[mpx->stk_siz];
mpx->x= mpx->xstack[mpx->stk_siz];
mpx->y= mpx->ystack[mpx->stk_siz];mpx->z= mpx->zstack[mpx->stk_siz];
}
}

/*:113*//*114:*/
#line 1875 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_dvi_commands(MPX mpx);
static void mpx_set_virtual_char(MPX mpx,web_integer f,web_integer c){
double old_scale;
unsigned old_buf_ptr;
unsigned old_fbase,old_ftop;
if(mpx->fbase[f]==0)
mpx_do_set_char(mpx,f,c);
else{
old_fbase= mpx->cur_fbase;old_ftop= mpx->cur_ftop;
mpx->cur_fbase= (unsigned int)mpx->fbase[f];
mpx->cur_ftop= (unsigned int)mpx->ftop[f];
old_scale= mpx->dvi_scale;
mpx->dvi_scale= mpx->dvi_scale*mpx->font_scaled_size[f];
old_buf_ptr= mpx->buf_ptr;
mpx->buf_ptr= (unsigned int)start_cmd(f,c);
mpx_do_push(mpx);
mpx_do_dvi_commands(mpx);
mpx_do_pop(mpx);
mpx->buf_ptr= old_buf_ptr;
mpx->dvi_scale= old_scale;
mpx->cur_fbase= old_fbase;
mpx->cur_ftop= old_ftop;
}
}

/*:114*//*116:*/
#line 1958 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_dvi_commands(MPX mpx){
unsigned int o;
web_integer p,q;
web_integer cur_font;
if((mpx->cur_fbase<mpx->cur_ftop)&&(mpx->buf_ptr<virtual_space))
cur_font= mpx_select_font(mpx,mpx->font_num[mpx->cur_ftop-1]);
else
cur_font= max_fnums+1;
mpx->w= 0;mpx->x= 0;mpx->y= 0;mpx->z= 0;
while(true){
/*118:*/
#line 1976 "../../../source/texk/web2c/mplibdir/mpxout.w"

{
o= (unsigned int)mpx_get_byte(mpx);
p= mpx_first_par(mpx,o);
if(feof(mpx->dvi_file))
bad_dvi("the DVI file ended prematurely");

if(o<set1+4){
if(cur_font> max_fnums){
if(mpx->vf_reading)
mpx_abort(mpx,"no font selected for character %d in virtual font",p);
else
bad_dvi_two("no font selected for character %d",p);
}

mpx_set_virtual_char(mpx,cur_font,p);
mpx->h+= /*73:*/
#line 1288 "../../../source/texk/web2c/mplibdir/mpxout.w"

floor(mpx->dvi_scale*mpx->font_scaled_size[cur_font]*char_width(cur_font,p))

/*:73*/
#line 1992 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}else{
switch(o){
case four_cases(put1):
mpx_set_virtual_char(mpx,cur_font,p);
break;
case set_rule:
q= (web_integer)trunc(mpx_signed_quad(mpx)*mpx->dvi_scale);
mpx_do_set_rule(mpx,(web_integer)trunc(p*mpx->dvi_scale),q);
mpx->h+= q;
break;
case put_rule:
q= (web_integer)trunc(mpx_signed_quad(mpx)*mpx->dvi_scale);
mpx_do_set_rule(mpx,(web_integer)trunc(p*mpx->dvi_scale),q);
break;
/*119:*/
#line 2017 "../../../source/texk/web2c/mplibdir/mpxout.w"

case four_cases(xxx1):
mpx_do_xxx(mpx,p);
break;
case pre:case post:case post_post:
bad_dvi("preamble or postamble within a page!");

break;

/*:119*//*120:*/
#line 2026 "../../../source/texk/web2c/mplibdir/mpxout.w"

case nop:
break;
case bop:
bad_dvi("bop occurred before eop");

break;
case eop:
return;
break;
case push:
mpx_do_push(mpx);
break;
case pop:
mpx_do_pop(mpx);
break;

/*:120*//*121:*/
#line 2043 "../../../source/texk/web2c/mplibdir/mpxout.w"

case four_cases(right1):
mpx->h+= trunc(p*mpx->dvi_scale);
break;
case w0:case four_cases(w1):
mpx->w= (web_integer)trunc(p*mpx->dvi_scale);mpx->h+= mpx->w;
break;
case x0:case four_cases(x1):
mpx->x= (web_integer)trunc(p*mpx->dvi_scale);mpx->h+= mpx->x;
break;
case four_cases(down1):
mpx->v+= trunc(p*mpx->dvi_scale);
break;
case y0:case four_cases(y1):
mpx->y= (web_integer)trunc(p*mpx->dvi_scale);mpx->v+= mpx->y;
break;
case z0:case four_cases(z1):
mpx->z= (web_integer)trunc(p*mpx->dvi_scale);mpx->v+= mpx->z;
break;

/*:121*//*122:*/
#line 2063 "../../../source/texk/web2c/mplibdir/mpxout.w"

case sixty_four_cases(fnt_num_0):case four_cases(fnt1):
cur_font= mpx_select_font(mpx,p);
break;
case four_cases(fnt_def1):
mpx_define_font(mpx,p);
break;

/*:122*/
#line 2008 "../../../source/texk/web2c/mplibdir/mpxout.w"

case undefined_commands:
bad_dvi_two("undefined command %d",o);

break;
}
}
}

/*:118*/
#line 1969 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}
}

/*:116*//*123:*/
#line 2075 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_dvitomp(MPX mpx,char*dviname){
int k;
mpx->dviname= dviname;
mpx_open_dvi_file(mpx);
/*125:*/
#line 2111 "../../../source/texk/web2c/mplibdir/mpxout.w"

{
int p;
p= mpx_get_byte(mpx);
if(p!=pre)
bad_dvi("First byte isn""t start of preamble!");

p= mpx_get_byte(mpx);
if(p!=id_byte)
mpx_warn(mpx,"identification in byte 1 should be %d!",id_byte);

/*126:*/
#line 2136 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->numerator= mpx_signed_quad(mpx);mpx->denominator= mpx_signed_quad(mpx);
if((mpx->numerator<=0)||(mpx->denominator<=0))
bad_dvi("bad scale ratio in preamble");

mpx->mag= mpx_signed_quad(mpx)/1000.0;
if(mpx->mag<=0.0)
bad_dvi("magnification isn't positive");

mpx->conv= (mpx->numerator/254000.0)*(72.0/mpx->denominator)*mpx->mag;
mpx->dvi_per_fix= (254000.0/mpx->numerator)*(mpx->denominator/72.27)/1048576.0;

/*:126*/
#line 2122 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
p= mpx_get_byte(mpx);
while(p> 0){
decr(p);
(void)mpx_get_byte(mpx);
}
}

/*:125*/
#line 2080 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
mpx_open_mpxfile(mpx);
if(mpx->banner!=NULL)
fprintf(mpx->mpxfile,"%s\n",mpx->banner);
while(true){
/*127:*/
#line 2148 "../../../source/texk/web2c/mplibdir/mpxout.w"

do{
int p;
k= mpx_get_byte(mpx);
if((k>=fnt_def1)&&(k<fnt_def1+4)){
p= mpx_first_par(mpx,(unsigned int)k);
mpx_define_font(mpx,p);k= nop;
}
}while(k==nop);
if(k==post)
break;
if(k!=bop)
bad_dvi("missing bop");



/*:127*/
#line 2085 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
for(k= 0;k<=10;k++)
(void)mpx_signed_quad(mpx);
/*112:*/
#line 1837 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->dvi_scale= 1.0;
mpx->stk_siz= 0;
mpx->h= 0;mpx->v= 0;
mpx->Xslant= 0.0;mpx->Xheight= 0.0

/*:112*/
#line 2088 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
mpx_start_picture(mpx);
mpx_do_dvi_commands(mpx);
if(mpx->stk_siz!=0)
bad_dvi("stack not empty at end of page");

mpx_stop_picture(mpx);
fprintf(mpx->mpxfile,"mpxbreak\n");
}
if(mpx->dvi_file)
mpx_fclose(mpx,mpx->dvi_file);
if(mpx->history<=mpx_cksum_trouble)
return 0;
else
return mpx->history;
}

/*:123*//*133:*/
#line 2196 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_def_named_color(MPX mpx,const char*n,const char*v){
mpx->num_named_colors++;
assert(mpx->num_named_colors<max_named_colors);
mpx->named_colors[mpx->num_named_colors].name= n;
mpx->named_colors[mpx->num_named_colors].value= v;
}

/*:133*//*160:*/
#line 2568 "../../../source/texk/web2c/mplibdir/mpxout.w"

static char*mpx_find_file(MPX mpx,const char*nam,const char*mode,int ftype){
(void)mpx;
if(mode[0]!='r'||(!access(nam,R_OK))||ftype){
return strdup(nam);
}
return NULL;
}

/*:160*//*163:*/
#line 2583 "../../../source/texk/web2c/mplibdir/mpxout.w"

static FILE*mpx_fsearch(MPX mpx,const char*nam,int format){
FILE*f= NULL;
char*fname= (mpx->find_file)(mpx,nam,"r",format);
if(fname){
f= fopen(fname,"rb");
mpx_report(mpx,"%p = fopen(%s,\"rb\")",f,fname);
}
return f;
}

/*:163*//*166:*/
#line 2602 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_comp_name(void*p,const void*pa,const void*pb){
(void)p;
return strcmp(((const avl_entry*)pa)->name,
((const avl_entry*)pb)->name);
}
static void*destroy_avl_entry(void*pa){
avl_entry*p;
p= (avl_entry*)pa;
free(p->name);
free(p);
return NULL;
}
static void*copy_avl_entry(const void*pa){
const avl_entry*p;
avl_entry*q;
p= (const avl_entry*)pa;
q= malloc(sizeof(avl_entry));
if(q!=NULL){
q->name= strdup(p->name);
q->num= p->num;
}
return(void*)q;
}


/*:166*//*167:*/
#line 2628 "../../../source/texk/web2c/mplibdir/mpxout.w"

static avl_tree mpx_avl_create(MPX mpx){
avl_tree t;
t= avl_create(mpx_comp_name,
copy_avl_entry,
destroy_avl_entry,
malloc,free,NULL);
if(t==NULL)
mpx_abort(mpx,"Memory allocation failure");
return t;
}

/*:167*//*168:*/
#line 2645 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_avl_probe(MPX mpx,avl_tree tab,avl_entry*p){
avl_entry*r= (avl_entry*)avl_find(p,tab);
if(r==NULL){
if(avl_ins(p,tab,avl_false)<0)
mpx_abort(mpx,"Memory allocation failure");
}
}


/*:168*//*170:*/
#line 2664 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_get_int(MPX mpx,char*s){
register int i,d,neg;
if(s==NULL)
goto BAD;
for(neg= 0;;s++){
if(*s=='-')
neg= !neg;
else if(*s!=' '&&*s!='\t')
break;
}
if(i= *s-'0',0> i||i> 9)
goto BAD;
while(d= *++s-'0',0<=d&&d<=9)
i= 10*i+d;
mpx->arg_tail= s;
return neg?-i:i;
BAD:
mpx->arg_tail= NULL;
return 0;
}

/*:170*//*171:*/
#line 2691 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_get_int_map(MPX mpx,char*s){
register int i;
if(s==NULL)
goto BAD;
i= (int)strtol(s,&(mpx->arg_tail),0);
if(s==mpx->arg_tail)
goto BAD;
return i;
BAD:
mpx->arg_tail= NULL;
return 0;
}

/*:171*//*172:*/
#line 2710 "../../../source/texk/web2c/mplibdir/mpxout.w"

static float mpx_get_float(MPX mpx,char*s){
register int d,neg,digits;
register float x,y;
digits= 0;
neg= 0;x= 0.0;
if(s!=NULL){
for(neg= 0;;s++){
if(*s=='-')
neg= !neg;
else if(*s!=' '&&*s!='\t')
break;
}
x= 0.0;
while(d= *s-'0',0<=d&&d<=9){
x= (float)10.0*x+(float)d;
digits++;
s++;
}
if(*s=='.'){
y= 1.0;
while(d= *++s-'0',0<=d&&d<=9){
y/= (float)10.0;
x+= y*(float)d;
digits++;
}
}
}
if(digits==0){
mpx->arg_tail= NULL;
return 0.0;
}
mpx->arg_tail= s;
return neg?-x:x;
}

/*:172*//*173:*/
#line 2752 "../../../source/texk/web2c/mplibdir/mpxout.w"

static float mpx_get_float_map(MPX mpx,char*s){
if(s!=NULL){
while(isspace((unsigned char)*s))
s++;
while(!isspace((unsigned char)*s)&&*s)
s++;
}
mpx->arg_tail= s;
return 0;
}


/*:173*//*175:*/
#line 2777 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_read_fmap(MPX mpx,const char*dbase){
FILE*fin;
avl_entry*tmp;
char*nam;
char*buf;
mpx->nfonts= 0;
fin= mpx_fsearch(mpx,dbase,mpx_trfontmap_format);
if(fin==NULL)
mpx_abort(mpx,"Cannot find %s",dbase);

mpx->trfonts= mpx_avl_create(mpx);
while((buf= mpx_getline(mpx,fin))!=NULL){
if(mpx->nfonts==(max_fnums+1))
mpx_abort(mpx,"Need to increase max_fnums");
nam= buf;
while(*buf&&*buf!='\t')
buf++;
if(nam==buf)
continue;
tmp= xmalloc(sizeof(avl_entry),1);
tmp->name= xmalloc(1,(size_t)(buf-nam)+1);
strncpy(tmp->name,nam,(unsigned int)(buf-nam));
tmp->name[(buf-nam)]= '\0';
tmp->num= (int)mpx->nfonts++;
avl_code_t ret = avl_ins(tmp, mpx->trfonts, avl_false);
assert(ret > 0);
//VMF assert(avl_ins(tmp,mpx->trfonts,avl_false)> 0);
if(*buf){
buf++;
while(*buf=='\t')buf++;
while(*buf&&*buf!='\t')buf++;
while(*buf=='\t')buf++;
if(*buf)
nam= buf;
while(*buf)buf++;
}
mpx->font_name[tmp->num]= xstrdup(nam);
mpx->font_num[tmp->num]= -1;
}
mpx_fclose(mpx,fin);
}


/*:175*//*176:*/
#line 2833 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_read_char_adj(MPX mpx,const char*adjfile){
FILE*fin;
char buf[200];
avl_entry tmp,*p;
unsigned int i;

fin= mpx_fsearch(mpx,adjfile,mpx_trcharadj_format);
if(fin==NULL)
mpx_abort(mpx,"Cannot find %s",adjfile);

for(i= 0;i<mpx->nfonts;i++)
mpx->shiftbase[i]= 0;
while(fgets(buf,200,fin)!=NULL){
if(mpx->shiftptr==SHIFTS-1)
mpx_abort(mpx,"Need to increase SHIFTS");
if(buf[0]!=' '&&buf[0]!='\t'){
for(i= 0;buf[i]!='\0';i++)
if(buf[i]=='\n')
buf[i]= '\0';
mpx->shiftchar[mpx->shiftptr++]= -1;
tmp.name= buf;
p= (avl_entry*)avl_find(&tmp,mpx->trfonts);
if(p==NULL)
mpx_abort(mpx,"%s refers to unknown font %s",adjfile,buf);
assert(p);
mpx->shiftbase[p->num]= mpx->shiftptr;

}else{
mpx->shiftchar[mpx->shiftptr]= mpx_get_int(mpx,buf);
mpx->shifth[mpx->shiftptr]= mpx_get_float(mpx,mpx->arg_tail);
mpx->shiftv[mpx->shiftptr]= -mpx_get_float(mpx,mpx->arg_tail);
if(mpx->arg_tail==NULL)
mpx_abort(mpx,"Bad shift entry : \"%s\"",buf);
mpx->shiftptr++;
}
}
mpx->shiftchar[mpx->shiftptr++]= -1;
mpx_fclose(mpx,fin);
}

/*:176*//*177:*/
#line 2882 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_read_desc(MPX mpx){
const char*const k1[]= {
"res","hor","vert","unitwidth","paperwidth",
"paperlength","biggestfont","spare2","encoding",
NULL
};
const char*const g1[]= {
"family","paperheight","postpro","prepro",
"print","image_generator","broken",
NULL
};
char cmd[200];
FILE*fp;
int i,n;

fp= mpx_fsearch(mpx,"DESC",mpx_desc_format);
if(fp==NULL)
mpx_abort(mpx,"Cannot find DESC");
while(fscanf(fp,"%199s",cmd)!=EOF){
if(*cmd=='#'){
while((i= getc(fp))!=EOF&&i!='\n');
continue;
}
if(strcmp(cmd,"fonts")==0){
if(fscanf(fp,"%d",&n)!=1)
return;
for(i= 0;i<n;i++)
if(fscanf(fp,"%*s")==EOF)
return;
}else if(strcmp(cmd,"sizes")==0){
while(fscanf(fp,"%d",&n)==1&&n!=0);
}else if(strcmp(cmd,"styles")==0||
strcmp(cmd,"papersize")==0){
mpx->gflag++;
while((i= getc(fp))!=EOF&&i!='\n');
}else if(strcmp(cmd,"sizescale")==0){
if(fscanf(fp,"%d",&n)==1)
mpx->sizescale= (float)n;
mpx->gflag++;
}else if(strcmp(cmd,"charset")==0){
return;
}else{
for(i= 0;k1[i];i++)
if(strcmp(cmd,k1[i])==0){
if(fscanf(fp,"%*s")==EOF)
return;
break;
}
if(k1[i]==0)
for(i= 0;g1[i];i++)
if(strcmp(cmd,g1[i])==0){
if(fscanf(fp,"%*s")==EOF)
return;
mpx->gflag= 1;
break;
}
}
}
}


/*:177*//*180:*/
#line 2963 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_scan_desc_line(MPX mpx,int f,char*lin){
static int lastcode;
avl_entry*tmp;
char*s,*t;
t= lin;
while(*lin!=' '&&*lin!='\t'&&*lin!='\0')
lin++;
if(lin==t)
return 1;
s= xmalloc((size_t)(lin-t+1),1);
strncpy(s,t,(size_t)(lin-t));
*(s+(lin-t))= '\0';
while(*lin==' '||*lin=='\t')
lin++;
if(*lin=='"'){
if(lastcode<MAXCHARS){
tmp= xmalloc(sizeof(avl_entry),1);
tmp->name= s;
tmp->num= lastcode;
mpx_avl_probe(mpx,mpx->charcodes[f],tmp);
}
}else{
(void)mpx_get_float_map(mpx,lin);
(void)mpx_get_int(mpx,mpx->arg_tail);
lastcode= mpx_get_int_map(mpx,mpx->arg_tail);
if(mpx->arg_tail==NULL)
return 0;
if(lastcode<MAXCHARS){
tmp= xmalloc(sizeof(avl_entry),1);
tmp->name= s;
tmp->num= lastcode;
mpx_avl_probe(mpx,mpx->charcodes[f],tmp);
}
}
return 1;
}

/*:180*//*181:*/
#line 3004 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_read_fontdesc(MPX mpx,char*nam){
char buf[200];
avl_entry tmp,*p;
FILE*fin;
int f;

if(mpx->unit==0.0)
mpx_abort(mpx,"Resolution is not set soon enough");
tmp.name= nam;
p= (avl_entry*)avl_find(&tmp,mpx->trfonts);
if(p==NULL)
mpx_abort(mpx,"Font was not in map file");
assert(p);
f= p->num;
fin= mpx_fsearch(mpx,nam,mpx_fontdesc_format);
if(fin==NULL)
mpx_abort(mpx,"Cannot find %s",nam);
for(;;){
if(fgets(buf,200,fin)==NULL)
mpx_abort(mpx,"Description file for %s ends unexpectedly",nam);
if(strncmp(buf,"special",7)==0){
*(mpx->specf_tail)= f;
mpx->next_specfnt[f]= (max_fnums+1);
mpx->specf_tail= &(mpx->next_specfnt[f]);
}else if(strncmp(buf,"charset",7)==0)
break;
}
mpx->charcodes[f]= mpx_avl_create(mpx);
while(fgets(buf,200,fin)!=NULL)
if(mpx_scan_desc_line(mpx,f,buf)==0)
mpx_abort(mpx,"%s has a bad line in its description file: %s",nam,buf);
mpx_fclose(mpx,fin);
return f;
}

/*:181*//*184:*/
#line 3055 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_slant_and_ht(MPX mpx){
int i= 0;
fprintf(mpx->mpxfile,"(");
if(mpx->Xslant!=0.0){
fprintf(mpx->mpxfile," slanted%.5f",mpx->Xslant);
i++;
}
if(mpx->Xheight!=mpx->cursize&&mpx->Xheight!=0.0&&mpx->cursize!=0.0){
fprintf(mpx->mpxfile," yscaled%.4f",mpx->Xheight/mpx->cursize);
i++;
}
fprintf(mpx->mpxfile,")");
}


/*:184*//*185:*/
#line 3073 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_set_num_char(MPX mpx,int f,int c){
float hh,vv;
int i;

hh= (float)mpx->h;
vv= (float)mpx->v;
for(i= mpx->shiftbase[f];mpx->shiftchar[i]>=0&&i<SHIFTS;i++)
if(mpx->shiftchar[i]==c){
hh+= (mpx->cursize/mpx->unit)*mpx->shifth[i];
vv+= (mpx->cursize/mpx->unit)*mpx->shiftv[i];
break;
}
if(hh-mpx->dmp_str_h2>=1.0||mpx->dmp_str_h2-hh>=1.0||
vv-mpx->dmp_str_v>=1.0||mpx->dmp_str_v-vv>=1.0||
f!=mpx->str_f||mpx->cursize!=mpx->str_size){
if(mpx->str_f>=0)
mpx_finish_last_char(mpx);
else if(!mpx->fonts_used)
mpx_prepare_font_use(mpx);
if(!mpx->font_used[f])
mpx_first_use(mpx,f);
fprintf(mpx->mpxfile,"_s((");
mpx->print_col= 3;
mpx->str_f= f;
mpx->dmp_str_v= vv;
mpx->dmp_str_h1= hh;
mpx->str_size= mpx->cursize;
}
mpx_print_char(mpx,(unsigned char)c);
mpx->dmp_str_h2= hh+(float)char_width(f,c);
}

/*:185*//*186:*/
#line 3108 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_set_string(MPX mpx,char*cname){
float hh;

if(!*cname)
return;
hh= (float)mpx->h;
mpx_set_num_char(mpx,(int)mpx->curfont,*cname);
hh+= (float)char_width(mpx->curfont,(int)*cname);
while(*++cname){
mpx_print_char(mpx,(unsigned char)*cname);
hh+= (float)char_width(mpx->curfont,(int)*cname);
}
mpx->h= (web_integer)floor(hh+0.5);
mpx_finish_last_char(mpx);
}

/*:186*//*188:*/
#line 3142 "../../../source/texk/web2c/mplibdir/mpxout.w"

static char*mpx_copy_spec_char(MPX mpx,char*cname){
FILE*deff;
int c;
char*s,*t;
char specintro[]= "vardef ";
unsigned k= 0;
if(strcmp(cname,"ao")==0){
deff= mpx_fsearch(mpx,"ao.x",mpx_specchar_format);
test_redo_search;
}else if(strcmp(cname,"lh")==0){
deff= mpx_fsearch(mpx,"lh.x",mpx_specchar_format);
test_redo_search;
}else if(strcmp(cname,"~=")==0){
deff= mpx_fsearch(mpx,"twiddle",mpx_specchar_format);
test_redo_search;
}else{
deff= mpx_fsearch(mpx,cname,mpx_specchar_format);
}
if(deff==NULL)
mpx_abort(mpx,"No vardef in charlib/%s",cname);

while(k<(unsigned)strlen(specintro)){
if((c= getc(deff))==EOF)
mpx_abort(mpx,"No vardef in charlib/%s",cname);
putc(c,mpx->mpxfile);
if(c==specintro[k])
k++;
else
k= 0;
}
s= xmalloc(mpx->bufsize,1);
t= s;
while((c= getc(deff))!='('){
if(c==EOF)
mpx_abort(mpx,"vardef in charlib/%s has no arguments",cname);
putc(c,mpx->mpxfile);
*t++= (char)c;
}
putc(c,mpx->mpxfile);
*t++= '\0';
while((c= getc(deff))!=EOF);
putc(c,mpx->mpxfile);
return s;
}


/*:188*//*191:*/
#line 3205 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_set_char(MPX mpx,char*cname){
int f,c;
avl_entry tmp,*p;
spec_entry*sp;

if(*cname==' '||*cname=='\t')
return;
f= (int)mpx->curfont;
tmp.name= cname;
p= avl_find(&tmp,mpx->charcodes[f]);
if(p==NULL){
for(f= mpx->specfnt;f!=(max_fnums+1);f= mpx->next_specfnt[f]){
p= avl_find(&tmp,mpx->charcodes[f]);
if(p!=NULL)
goto OUT_LABEL;
}
mpx_abort(mpx,"There is no character %s",cname);
}
OUT_LABEL:
assert(p);
c= p->num;
if(!is_specchar(c)){
mpx_set_num_char(mpx,f,c);
}else{
if(mpx->str_f>=0)
mpx_finish_last_char(mpx);
if(!mpx->fonts_used)
mpx_prepare_font_use(mpx);
if(!mpx->font_used[f])
mpx_first_use(mpx,f);
if(mpx->spec_tab)
mpx->spec_tab= mpx_avl_create(mpx);
sp= xmalloc(sizeof(spec_entry),1);
sp->name= cname;
sp->mac= NULL;
{
spec_entry*r= (spec_entry*)avl_find(sp,mpx->spec_tab);
if(r==NULL){
if(avl_ins(sp,mpx->spec_tab,avl_false)<0)
mpx_abort(mpx,"Memory allocation failure");
}
}
if(sp->mac==NULL){
sp->mac= mpx_copy_spec_char(mpx,cname);
}
fprintf(mpx->mpxfile,"_s(%s(_n%d)",sp->mac,f);
fprintf(mpx->mpxfile,",%.5f,%.4f,%.4f)",
(mpx->cursize/mpx->font_design_size[f])*1.00375,
(double)(((float)mpx->h*mpx->unit)/100.0),YCORR-(float)mpx->v*mpx->unit);
mpx_slant_and_ht(mpx);
fprintf(mpx->mpxfile,";\n");
}
}

/*:191*//*192:*/
#line 3265 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_font_def(MPX mpx,int n,char*nam){
int f;
unsigned k;
avl_entry tmp,*p;
tmp.name= nam;
p= (avl_entry*)avl_find(&tmp,mpx->trfonts);
if(p==NULL)
mpx_abort(mpx,"Font %s was not in map file",nam);
assert(p);
f= p->num;
if(mpx->charcodes[f]==NULL){
mpx_read_fontdesc(mpx,nam);
mpx->cur_name= xstrdup(mpx->font_name[f]);
if(!mpx_open_tfm_file(mpx))
font_abort("No TFM file found for ",f);

mpx_in_TFM(mpx,f);
}
for(k= 0;k<mpx->nfonts;k++)
if(mpx->font_num[k]==n)
mpx->font_num[k]= -1;
mpx->font_num[f]= n;
/*99:*/
#line 1647 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->font_used[f]= false;

/*:99*/
#line 3288 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}



/*:192*//*193:*/
#line 3299 "../../../source/texk/web2c/mplibdir/mpxout.w"

static float mpx_b_eval(const float*xx,float t){
float zz[4];
register int i,j;
for(i= 0;i<=3;i++)
zz[i]= xx[i];
for(i= 3;i> 0;i--)
for(j= 0;j<i;j++)
zz[j]+= t*(zz[j+1]-zz[j]);
return zz[0];
}


/*:193*//*194:*/
#line 3316 "../../../source/texk/web2c/mplibdir/mpxout.w"

static const float xx[]= {1.0,1.0,(float)0.8946431597,(float)0.7071067812};
static const float yy[]= {0.0,(float)0.2652164899,(float)0.5195704026,(float)0.7071067812};

/*:194*//*195:*/
#line 3320 "../../../source/texk/web2c/mplibdir/mpxout.w"

static float mpx_circangle(float t){
float ti;
ti= (float)floor(t);
t-= ti;
return(float)atan(mpx_b_eval(yy,t)/
mpx_b_eval(xx,t))+ti*Speed;
}


/*:195*//*196:*/
#line 3333 "../../../source/texk/web2c/mplibdir/mpxout.w"

static float mpx_circtime(float a){
int i;
float t;
t= a/Speed;
for(i= 2;--i>=0;)
t+= (a-mpx_circangle(t))/Speed;
return t;
}



/*:196*//*198:*/
#line 3351 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_prepare_graphics(MPX mpx){

fprintf(mpx->mpxfile,"vardef _D(expr _d)expr _q =\n");
fprintf(mpx->mpxfile,
" addto _p doublepath _q withpen pencircle scaled _d; enddef;\n");
mpx->graphics_used= true;
}


/*:198*//*199:*/
#line 3366 "../../../source/texk/web2c/mplibdir/mpxout.w"

static char*mpx_do_line(MPX mpx,char*s){
float dh,dv;

fprintf(mpx->mpxfile,"(%.3f,%.3f)",mpx->gx*mpx->unit,mpx->gy*mpx->unit);
dh= mpx_get_float(mpx,s);
dv= mpx_get_float(mpx,mpx->arg_tail);
if(mpx->arg_tail==NULL)
return NULL;
mpx->gx+= dh;
mpx->gy-= dv;
fprintf(mpx->mpxfile,"--\n");
return mpx->arg_tail;
}


/*:199*//*200:*/
#line 3389 "../../../source/texk/web2c/mplibdir/mpxout.w"

static char*mpx_spline_seg(MPX mpx,char*s){
float dh1,dv1,dh2,dv2;

dh1= mpx_get_float(mpx,s);
dv1= mpx_get_float(mpx,mpx->arg_tail);
if(mpx->arg_tail==NULL)
mpx_abort(mpx,"Missing spline increments");
s= mpx->arg_tail;
fprintf(mpx->mpxfile,"(%.3f,%.3f)",(mpx->gx+.5*dh1)*mpx->unit,
(mpx->gy-.5*dv1)*mpx->unit);
mpx->gx+= dh1;
mpx->gy-= dv1;
dh2= mpx_get_float(mpx,s);
dv2= mpx_get_float(mpx,mpx->arg_tail);
if(mpx->arg_tail==NULL)
return NULL;
fprintf(mpx->mpxfile,"..\ncontrols (%.3f,%.3f) and (%.3f,%.3f)..\n",
(mpx->gx-dh1/6.0)*mpx->unit,(mpx->gy+dv1/6.0)*mpx->unit,
(mpx->gx+dh2/6.0)*mpx->unit,(mpx->gy-dv2/6.0)*mpx->unit);
return s;
}


/*:200*//*201:*/
#line 3415 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_ellipse(MPX mpx,float a,float b){

fprintf(mpx->mpxfile,"makepath(pencircle xscaled %.3f\n yscaled %.3f",
a*mpx->unit,b*mpx->unit);
fprintf(mpx->mpxfile," shifted (%.3f,%.3f));\n",(mpx->gx+.5*a)*mpx->unit,
mpx->gy*mpx->unit);
mpx->gx+= a;
}


/*:201*//*202:*/
#line 3429 "../../../source/texk/web2c/mplibdir/mpxout.w"

static
void mpx_do_arc(MPX mpx,float cx,float cy,float ax,float ay,float bx,float by){
float t1,t2;

t1= mpx_circtime((float)atan2(ay,ax));
t2= mpx_circtime((float)atan2(by,bx));
if(t2<t1)
t2+= (float)8.0;
fprintf(mpx->mpxfile,"subpath (%.5f,%.5f) of\n",t1,t2);
fprintf(mpx->mpxfile,
" makepath(pencircle scaled %.3f shifted (%.3f,%.3f));\n",
2.0*sqrt(ax*ax+ay*ay)*mpx->unit,cx*mpx->unit,cy*mpx->unit);
mpx->gx= cx+bx;
mpx->gy= cy+by;
}



/*:202*//*203:*/
#line 3450 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_do_graphic(MPX mpx,char*s){
float h1,v1,h2,v2;

mpx_finish_last_char(mpx);



if(s[0]=='F'&&s[1]=='d')
return;
mpx->gx= (float)mpx->h;
mpx->gy= (float)YCORR/mpx->unit-((float)mpx->v);
if(!mpx->graphics_used)
mpx_prepare_graphics(mpx);
fprintf(mpx->mpxfile,"D(%.4f) ",LWscale*mpx->cursize);
switch(*s++){
case'c':
h1= mpx_get_float(mpx,s);
if(mpx->arg_tail==NULL)
mpx_abort(mpx,"Bad argument in %s",s-2);
mpx_do_ellipse(mpx,h1,h1);
break;
case'e':
h1= mpx_get_float(mpx,s);
v1= mpx_get_float(mpx,mpx->arg_tail);
if(mpx->arg_tail==NULL)
mpx_abort(mpx,"Bad argument in %s",s-2);
mpx_do_ellipse(mpx,h1,v1);
break;
case'A':
fprintf(mpx->mpxfile,"reverse ");

case'a':
h1= mpx_get_float(mpx,s);
v1= mpx_get_float(mpx,mpx->arg_tail);
h2= mpx_get_float(mpx,mpx->arg_tail);
v2= mpx_get_float(mpx,mpx->arg_tail);
if(mpx->arg_tail==NULL)
mpx_abort(mpx,"Bad argument in %s",s-2);
mpx_do_arc(mpx,mpx->gx+h1,mpx->gy-v1,-h1,v1,h2,-v2);
break;
case'l':
case'p':
while(s!=NULL)
s= mpx_do_line(mpx,s);
fprintf(mpx->mpxfile,";\n");
break;
case'q':
do
s= mpx_spline_seg(mpx,s);
while(s!=NULL);
fprintf(mpx->mpxfile,";\n");
break;
case'~':
fprintf(mpx->mpxfile,"(%.3f,%.3f)--",mpx->gx*mpx->unit,mpx->gy*mpx->unit);
do
s= mpx_spline_seg(mpx,s);
while(s!=NULL);
fprintf(mpx->mpxfile,"--(%.3f,%.3f);\n",mpx->gx*mpx->unit,mpx->gy*mpx->unit);
break;
default:
mpx_abort(mpx,"Unknown drawing function %s",s-2);
}
mpx->h= (int)floor(mpx->gx+.5);
mpx->v= (int)floor(YCORR/mpx->unit+.5-mpx->gy);
}



/*:203*//*204:*/
#line 3521 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_change_font(MPX mpx,int f){
for(mpx->curfont= 0;mpx->curfont<mpx->nfonts;mpx->curfont++)
if(mpx->font_num[mpx->curfont]==f)
return;
mpx_abort(mpx,"Bad font setting");
}


/*:204*//*205:*/
#line 3533 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_do_x_cmd(MPX mpx,char*s0)
{
float x;
int n;
char*s;

s= s0;
while(*s==' '||*s=='\t')
s++;
switch(*s++){
case'r':
if(mpx->unit!=0.0)
mpx_abort(mpx,"Attempt to reset resolution");
while(*s!=' '&&*s!='\t')
s++;
mpx->unit= mpx_get_float(mpx,s);
if(mpx->unit<=0.0)
mpx_abort(mpx,"Bad resolution: x %s",s0);
mpx->unit= (float)72.0/mpx->unit;
break;
case'f':
while(*s!=' '&&*s!='\t')
s++;
n= mpx_get_int(mpx,s);
if(mpx->arg_tail==NULL)
mpx_abort(mpx,"Bad font def: x %s",s0);
s= mpx->arg_tail;
while(*s==' '||*s=='\t')
s++;
mpx_do_font_def(mpx,n,s);
break;
case's':
return 0;
case'H':
while(*s!=' '&&*s!='\t')
s++;
mpx->Xheight= mpx_get_float(mpx,s);







if(mpx->sizescale!=0.0){
if(mpx->unit!=0.0)
mpx->Xheight*= mpx->unit;
else
mpx->Xheight/= mpx->sizescale;
}
if(mpx->Xheight==mpx->cursize)
mpx->Xheight= 0.0;
break;
case'S':
while(*s!=' '&&*s!='\t')
s++;
mpx->Xslant= mpx_get_float(mpx,s)*((float)PI/(float)180.0);
x= (float)cos(mpx->Xslant);
if(-1e-4<x&&x<1e-4)
mpx_abort(mpx,"Excessive slant");
mpx->Xslant= (float)sin(mpx->Xslant)/x;
break;
default:
;
}
return 1;
}


/*:205*//*206:*/
#line 3616 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_do_page(MPX mpx,FILE*trf){
char*buf;
char a,*c,*cc;

mpx->h= mpx->v= 0;
while((buf= mpx_getline(mpx,trf))!=NULL){
mpx->lnno++;
c= buf;
while(*c!='\0'){
switch(*c){
case' ':
case'\t':
case'w':
c++;
break;
case's':
mpx->cursize= mpx_get_float(mpx,c+1);








if(mpx->sizescale!=0.0){
if(mpx->unit!=0.0)
mpx->cursize*= mpx->unit;
else
mpx->cursize/= mpx->sizescale;
}
goto iarg;
case'f':
mpx_change_font(mpx,mpx_get_int(mpx,c+1));
goto iarg;
case'c':
if(c[1]=='\0')
mpx_abort(mpx,"Bad c command in troff output");
cc= c+2;
goto set;
case'C':
cc= c;
do
cc++;
while(*cc!=' '&&*cc!='\t'&&*cc!='\0');
goto set;
case'N':
mpx_set_num_char(mpx,(int)mpx->curfont,mpx_get_int(mpx,c+1));
goto iarg;
case'H':
mpx->h= mpx_get_int(mpx,c+1);
goto iarg;
case'V':
mpx->v= mpx_get_int(mpx,c+1);
goto iarg;
case'h':
mpx->h+= mpx_get_int(mpx,c+1);
goto iarg;
case'v':
mpx->v+= mpx_get_int(mpx,c+1);
goto iarg;
case'0':
case'1':
case'2':
case'3':
case'4':
case'5':
case'6':
case'7':
case'8':
case'9':
if(c[1]<'0'||c[1]> '9'||c[2]=='\0')
mpx_abort(mpx,"Bad nnc command in troff output");
mpx->h+= 10*(c[0]-'0')+c[1]-'0';
c++;
cc= c+2;
goto set;
case'p':
return 1;
case'n':
(void)mpx_get_int(mpx,c+1);
(void)mpx_get_int(mpx,mpx->arg_tail);
goto iarg;
case'D':
mpx_do_graphic(mpx,c+1);
goto eoln;
case'x':
if(!mpx_do_x_cmd(mpx,c+1))
return 0;
goto eoln;
case'#':
goto eoln;
case'F':

goto eoln;
case'm':

goto eoln;
case'u':



mpx_abort(mpx,"Bad command in troff output\n"
"change the DESC file for your GROFF PostScript device, remove tcommand");
case't':

cc= c;
do
cc++;
while(*cc!=' '&&*cc!='\t'&&*cc!='\0');
a= *cc;
*cc= '\0';
mpx_set_string(mpx,++c);
c= cc;
*c= a;
continue;
default:
mpx_abort(mpx,"Bad command in troff output");
}
continue;
set:
a= *cc;
*cc= '\0';
mpx_set_char(mpx,++c);
c= cc;
*c= a;
continue;
iarg:
c= mpx->arg_tail;
}
eoln:;
}
return 0;
}


/*:206*//*207:*/
#line 3758 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int mpx_dmp(MPX mpx,char*infile){
int more;
FILE*trf= mpx_xfopen(mpx,infile,"r");
mpx_read_desc(mpx);
mpx_read_fmap(mpx,dbname);
if(!mpx->gflag)
mpx_read_char_adj(mpx,adjname);
mpx_open_mpxfile(mpx);
if(mpx->banner!=NULL)
fprintf(mpx->mpxfile,"%s\n",mpx->banner);
if(mpx_do_page(mpx,trf)){
do{
/*112:*/
#line 1837 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx->dvi_scale= 1.0;
mpx->stk_siz= 0;
mpx->h= 0;mpx->v= 0;
mpx->Xslant= 0.0;mpx->Xheight= 0.0

/*:112*/
#line 3771 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
mpx_start_picture(mpx);
more= mpx_do_page(mpx,trf);
mpx_stop_picture(mpx);
fprintf(mpx->mpxfile,"mpxbreak\n");
}while(more);
}
mpx_fclose(mpx,trf);
if(mpx->history<=mpx_cksum_trouble)
return 0;
else
return mpx->history;
}


/*:207*//*208:*/
#line 3827 "../../../source/texk/web2c/mplibdir/mpxout.w"


#define TEXERR "mpxerr.tex"
#define DVIERR "mpxerr.dvi"
#define TROFF_INERR "mpxerr.i"
#define TROFF_OUTERR "mpxerr.t"

/*:208*//*209:*/
#line 3834 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_rename(MPX mpx,const char*a,const char*b){
mpx_report(mpx,"renaming %s to %s",a,b);
rename(a,b);
}

/*:209*//*211:*/
#line 3846 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_default_erasetmp(MPX mpx){
char*wrk;
char*p;
if(mpx->mode==mpx_tex_mode){
wrk= xstrdup(mpx->tex);
p= strrchr(wrk,'.');
*p= '\0';strcat(wrk,".aux");remove(wrk);
*p= '\0';strcat(wrk,".pdf");remove(wrk);
*p= '\0';strcat(wrk,".toc");remove(wrk);
*p= '\0';strcat(wrk,".idx");remove(wrk);
*p= '\0';strcat(wrk,".ent");remove(wrk);
*p= '\0';strcat(wrk,".out");remove(wrk);
*p= '\0';strcat(wrk,".nav");remove(wrk);
*p= '\0';strcat(wrk,".snm");remove(wrk);
*p= '\0';strcat(wrk,".tui");remove(wrk);
free(wrk);
}
}

/*:211*//*213:*/
#line 3869 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_cleandir(MPX mpx,char*cur_path){
char*wrk,*p;
#ifdef _WIN32
struct _finddata_t c_file;
intptr_t hFile;
#else
#line 3876 "../../../source/texk/web2c/mplibdir/mpxout.w"
 struct dirent*entry;
DIR*d;
#endif
#line 3879 "../../../source/texk/web2c/mplibdir/mpxout.w"
 wrk= xstrdup(mpx->tex);
p= strrchr(wrk,'.');
*p= '\0';

#ifdef _WIN32
strcat(cur_path,"/*");
if((hFile= _findfirst(cur_path,&c_file))==-1L){
mpx_default_erasetmp(mpx);
}else{
if(strstr(c_file.name,wrk)==c_file.name)
remove(c_file.name);
while(_findnext(hFile,&c_file)!=-1L){
if(strstr(c_file.name,wrk)==c_file.name)
remove(c_file.name);
}
_findclose(hFile);
}
#else
#line 3897 "../../../source/texk/web2c/mplibdir/mpxout.w"
 if((d= opendir(cur_path))==NULL){
mpx_default_erasetmp(mpx);
}else{
while((entry= readdir(d))!=NULL){
if(strstr(entry->d_name,wrk)==entry->d_name)
remove(entry->d_name);
}
closedir(d);
}
#endif 
#line 3907 "../../../source/texk/web2c/mplibdir/mpxout.w"
free(wrk);
}


/*:213*//*214:*/
#line 3918 "../../../source/texk/web2c/mplibdir/mpxout.w"

#ifdef WIN32
#define GETCWD _getcwd
#else
#line 3922 "../../../source/texk/web2c/mplibdir/mpxout.w"
#define GETCWD getcwd
#endif
#line 3924 "../../../source/texk/web2c/mplibdir/mpxout.w"
 static void mpx_erasetmp(MPX mpx){
char cur_path[1024];
if(mpx->debug)
return;
if(mpx->tex[0]!='\0'){
remove(mpx->tex);
if(GETCWD(cur_path,1020)==NULL){
mpx_default_erasetmp(mpx);
}else{
mpx_cleandir(mpx,cur_path);
}
}
}


/*:214*//*215:*/
#line 3943 "../../../source/texk/web2c/mplibdir/mpxout.w"

static char*mpx_print_command(MPX mpx,int cmdlength,char**cmdline){
char*s,*t;
int i;
size_t l;
(void)mpx;
l= 0;
for(i= 0;i<cmdlength;i++){
l+= strlen(cmdline[i])+1;
}
s= xmalloc((size_t)l,1);t= s;
for(i= 0;i<cmdlength;i++){
if(i> 0)*t++= ' ';
t= strcpy(t,cmdline[i]);
t+= strlen(cmdline[i]);
}
return s;
}

/*:215*//*216:*/
#line 3965 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int do_spawn(MPX mpx,char*icmd,char**options){
#ifndef WIN32
pid_t child;
#endif
#line 3970 "../../../source/texk/web2c/mplibdir/mpxout.w"
 int retcode= -1;
char*cmd= xmalloc(strlen(icmd)+1,1);
if(icmd[0]!='"'){
strcpy(cmd,icmd);
}else{
strncpy(cmd,icmd+1,strlen(icmd)-2);
cmd[strlen(icmd)-2]= 0;
}
#ifndef WIN32
child= fork();
if(child<0)
mpx_abort(mpx,"fork failed: %s",strerror(errno));
if(child==0){
if(execvp(cmd,options))
mpx_abort(mpx,"exec failed: %s",strerror(errno));
}else{
if(wait(&retcode)==child){
retcode= (WIFEXITED(retcode)?WEXITSTATUS(retcode):-1);
}else{
mpx_abort(mpx,"wait failed: %s",strerror(errno));
}
}
#else
#line 3993 "../../../source/texk/web2c/mplibdir/mpxout.w"
 retcode= _spawnvp(_P_WAIT,cmd,(const char*const*)options);
#endif
#line 3995 "../../../source/texk/web2c/mplibdir/mpxout.w"
 xfree(cmd);
return retcode;
}

/*:216*//*217:*/
#line 3999 "../../../source/texk/web2c/mplibdir/mpxout.w"

#ifdef WIN32
#define nuldev "nul"
#else
#line 4003 "../../../source/texk/web2c/mplibdir/mpxout.w"
#define nuldev "/dev/null"
#endif
#line 4005 "../../../source/texk/web2c/mplibdir/mpxout.w"
 static int mpx_run_command(MPX mpx,char*inname,char*outname,int count,char**cmdl){
char*s;
int retcode;
int sav_o,sav_i;
FILE*fr,*fw;

if(count<1||cmdl==NULL||cmdl[0]==NULL)
return-1;

s= mpx_print_command(mpx,count,cmdl);
mpx_report(mpx,"running command %s",s);
free(s);

fr= mpx_xfopen(mpx,(inname?inname:nuldev),"r");
fw= mpx_xfopen(mpx,(outname?outname:nuldev),"wb");
/*219:*/
#line 4036 "../../../source/texk/web2c/mplibdir/mpxout.w"

#ifdef WIN32
#define DUP _dup
#define DUPP _dup2
#else
#line 4041 "../../../source/texk/web2c/mplibdir/mpxout.w"
#define DUP dup
#define DUPP dup2
#endif
#line 4044 "../../../source/texk/web2c/mplibdir/mpxout.w"
 sav_i= DUP(fileno(stdin));
sav_o= DUP(fileno(stdout));
DUPP(fileno(fr),fileno(stdin));
DUPP(fileno(fw),fileno(stdout))

/*:219*/
#line 4020 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
retcode= do_spawn(mpx,cmdl[0],cmdl);
/*220:*/
#line 4049 "../../../source/texk/web2c/mplibdir/mpxout.w"

DUPP(sav_i,fileno(stdin));
close(sav_i);
DUPP(sav_o,fileno(stdout));
close(sav_o)

/*:220*/
#line 4022 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
mpx_fclose(mpx,fr);
mpx_fclose(mpx,fw);
return retcode;
}

/*:217*//*221:*/
#line 4063 "../../../source/texk/web2c/mplibdir/mpxout.w"

static int
mpx_do_split_command(MPX mpx,char*maincmd,char***cmdline_addr,char target){
char*piece;
char*cmd;
char**cmdline;
size_t i;
int ret= 0;
int in_string= 0;
if(strlen(maincmd)==0)
return 0;
i= sizeof(char*)*(strlen(maincmd)+1);
cmdline= xmalloc(i,1);
memset(cmdline,0,i);
*cmdline_addr= cmdline;

i= 0;
while(maincmd[i]==' ')
i++;
cmd= xstrdup(maincmd);
piece= cmd;
for(;i<=strlen(maincmd);i++){
if(in_string==1){
if(cmd[i]=='"'){
in_string= 0;
}
}else if(in_string==2){
if(cmd[i]=='\''){
in_string= 0;
}
}else{
if(cmd[i]=='"'){
in_string= 1;
}else if(cmd[i]=='\''){
in_string= 2;
}else if(cmd[i]==target){
cmd[i]= 0;
cmdline[ret++]= piece;
while(i<strlen(maincmd)&&cmd[(i+1)]==' ')
i++;
piece= cmd+i+1;
}
}
}
if(*piece){
cmdline[ret++]= piece;
}
return ret;
}

/*:221*//*223:*/
#line 4116 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_command_cleanup(MPX mpx,char**cmdline){
(void)mpx;
xfree(cmdline[0]);
xfree(cmdline);
}



/*:223*//*224:*/
#line 4125 "../../../source/texk/web2c/mplibdir/mpxout.w"

static void mpx_command_error(MPX mpx,int cmdlength,char**cmdline){
char*s= mpx_print_command(mpx,cmdlength,cmdline);
mpx_command_cleanup(mpx,cmdline);
mpx_abort(mpx,"Command failed: %s; see mpxerr.log",s);
}



/*:224*//*226:*/
#line 4154 "../../../source/texk/web2c/mplibdir/mpxout.w"

int mpx_makempx(mpx_options*mpxopt){
MPX mpx;
char**cmdline,**cmdbits;
char infile[15];
int retcode,i;
char tmpname[]= "mpXXXXXX";
int cmdlength= 1;
int cmdbitlength= 1;
if(!mpxopt->debug){
/*229:*/
#line 4345 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(mpx_newer(mpxopt->mpname,mpxopt->mpxname))
return 0


/*:229*/
#line 4164 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
}
mpx= malloc(sizeof(struct mpx_data));
if(mpx==NULL||mpxopt->cmd==NULL||mpxopt->mpname==NULL||mpxopt->mpxname==NULL)
return mpx_fatal_error;
mpx_initialize(mpx);
if(mpxopt->banner!=NULL)
mpx->banner= mpxopt->banner;
mpx->mode= mpxopt->mode;
mpx->debug= mpxopt->debug;
if(mpxopt->find_file!=NULL)
mpx->find_file= mpxopt->find_file;
if(mpxopt->cmd!=NULL)
mpx->maincmd= xstrdup(mpxopt->cmd);
mpx->mpname= xstrdup(mpxopt->mpname);
mpx->mpxname= xstrdup(mpxopt->mpxname);
/*18:*/
#line 263 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(setjmp(mpx->jump_buf)!=0){
int h= mpx->history;
xfree(mpx->buf);
xfree(mpx->maincmd);
xfree(mpx->mpname);
xfree(mpx->mpxname);
xfree(mpx);
return h;
}

/*:18*/
#line 4180 "../../../source/texk/web2c/mplibdir/mpxout.w"
;

if(mpx->debug){
mpx->errfile= stderr;
}else{
mpx->errfile= mpx_xfopen(mpx,MPXLOG,"wb");
}
mpx->progname= "makempx";
/*230:*/
#line 4352 "../../../source/texk/web2c/mplibdir/mpxout.w"

 /*@-bufferoverflowhigh@*/ 
#ifdef HAVE_MKSTEMP
i= mkstemp(tmpname);
if(i==-1){
sprintf(tmpname,"mp%06d",(int)(time(NULL)%1000000));
}else{
close(i);
remove(tmpname);
}
#else
#line 4363 "../../../source/texk/web2c/mplibdir/mpxout.w"
#ifdef HAVE_MKTEMP
{
char*tmpstring= mktemp(tmpname);
if((tmpstring==NULL)||strlen(tmpname)==0){
sprintf(tmpname,"mp%06d",(int)(time(NULL)%1000000));
}else{


if(tmpstring!=tmpname){
i= strlen(tmpstring);
if(i> 8)i= 8;
strncpy(tmpname,tmpstring,i);
}
}
}
#else
#line 4379 "../../../source/texk/web2c/mplibdir/mpxout.w"
 sprintf(tmpname,"mp%06d",(int)(time(NULL)%1000000));
#endif
#line 4381 "../../../source/texk/web2c/mplibdir/mpxout.w"
#endif
#line 4382 "../../../source/texk/web2c/mplibdir/mpxout.w"
 /*@+bufferoverflowhigh@*/ /*:230*/
#line 4188 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(mpxopt->mptexpre==NULL)
mpxopt->mptexpre= xstrdup("mptexpre.tex");
/*32:*/
#line 667 "../../../source/texk/web2c/mplibdir/mpxout.w"

mpx_mpto(mpx,tmpname,mpxopt->mptexpre)

/*:32*/
#line 4191 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(mpxopt->cmd==NULL)
goto DONE;
if(mpx->mode==mpx_tex_mode){
/*227:*/
#line 4284 "../../../source/texk/web2c/mplibdir/mpxout.w"

{
char log[15];
mpx->maincmd= xrealloc(mpx->maincmd,strlen(mpx->maincmd)+strlen(mpx->tex)+2,1);
strcat(mpx->maincmd," ");
strcat(mpx->maincmd,mpx->tex);
cmdlength= split_command(mpx->maincmd,cmdline);

retcode= mpx_run_command(mpx,NULL,NULL,cmdlength,cmdline);

TMPNAME_EXT(log,".log");
if(!retcode){
TMPNAME_EXT(infile,".dvi");
remove(log);
}else{
mpx_rename(mpx,mpx->tex,TEXERR);
mpx_rename(mpx,log,ERRLOG);
mpx_command_error(mpx,cmdlength,cmdline);
}
mpx_command_cleanup(mpx,cmdline);
}

/*:227*/
#line 4195 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(mpx_dvitomp(mpx,infile)){
mpx_rename(mpx,infile,DVIERR);
if(!mpx->debug)
remove(mpx->mpxname);
mpx_abort(mpx,"Dvi conversion failed: %s %s\n",
DVIERR,mpx->mpxname);
}
}else if(mpx->mode==mpx_troff_mode){
/*228:*/
#line 4306 "../../../source/texk/web2c/mplibdir/mpxout.w"

{
char*cur_in,*cur_out;
char tmp_a[15],tmp_b[15];
TMPNAME_EXT(tmp_a,".t");
TMPNAME_EXT(tmp_b,".tmp");
cur_in= mpx->tex;
cur_out= tmp_a;


cmdbitlength= split_pipes(mpx->maincmd,cmdbits);
cmdline= NULL;

for(i= 0;i<cmdbitlength;i++){
if(cmdline!=NULL)free(cmdline);
cmdlength= split_command(cmdbits[i],cmdline);
retcode= mpx_run_command(mpx,cur_in,cur_out,cmdlength,cmdline);

if(retcode){
mpx_rename(mpx,mpx->tex,TROFF_INERR);
mpx_command_error(mpx,cmdlength,cmdline);
}
if(i<cmdbitlength-1){
if(i%2==0){
cur_in= tmp_a;
cur_out= tmp_b;
}else{
cur_in= tmp_b;
cur_out= tmp_a;
}
}
}
if(tmp_a!=cur_out){remove(tmp_a);}
if(tmp_b!=cur_out){remove(tmp_b);}
strcpy(infile,cur_out);
}

/*:228*/
#line 4204 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(mpx_dmp(mpx,infile)){
mpx_rename(mpx,infile,TROFF_OUTERR);
mpx_rename(mpx,mpx->tex,TROFF_INERR);
if(!mpx->debug)
remove(mpx->mpxname);
mpx_abort(mpx,"Troff conversion failed: %s %s\n",
TROFF_OUTERR,mpx->mpxname);
}
}
mpx_fclose(mpx,mpx->mpxfile);
if(!mpx->debug)
mpx_fclose(mpx,mpx->errfile);
if(!mpx->debug){
remove(MPXLOG);
remove(ERRLOG);
remove(infile);
}
mpx_erasetmp(mpx);
DONE:
retcode= mpx->history;
mpx_xfree(mpx->buf);
mpx_xfree(mpx->maincmd);
for(i= 0;i<(int)mpx->nfonts;i++)
mpx_xfree(mpx->font_name[i]);
free(mpx);
if(retcode==mpx_cksum_trouble)
retcode= 0;
return retcode;
}
int mpx_run_dvitomp(mpx_options*mpxopt){
MPX mpx;
int retcode,i;
mpx= malloc(sizeof(struct mpx_data));
if(mpx==NULL||mpxopt->mpname==NULL||mpxopt->mpxname==NULL)
return mpx_fatal_error;
mpx_initialize(mpx);
if(mpxopt->banner!=NULL)
mpx->banner= mpxopt->banner;
mpx->mode= mpxopt->mode;
mpx->debug= mpxopt->debug;
if(mpxopt->find_file!=NULL)
mpx->find_file= mpxopt->find_file;
mpx->mpname= xstrdup(mpxopt->mpname);
mpx->mpxname= xstrdup(mpxopt->mpxname);
/*18:*/
#line 263 "../../../source/texk/web2c/mplibdir/mpxout.w"

if(setjmp(mpx->jump_buf)!=0){
int h= mpx->history;
xfree(mpx->buf);
xfree(mpx->maincmd);
xfree(mpx->mpname);
xfree(mpx->mpxname);
xfree(mpx);
return h;
}

/*:18*/
#line 4249 "../../../source/texk/web2c/mplibdir/mpxout.w"
;
if(mpx->debug){
mpx->errfile= stderr;
}else{
mpx->errfile= mpx_xfopen(mpx,MPXLOG,"wb");
}
mpx->progname= "dvitomp";
if(mpx_dvitomp(mpx,mpx->mpname)){
if(!mpx->debug)
remove(mpx->mpxname);
mpx_abort(mpx,"Dvi conversion failed: %s %s\n",
DVIERR,mpx->mpxname);
}
mpx_fclose(mpx,mpx->mpxfile);
if(!mpx->debug)
mpx_fclose(mpx,mpx->errfile);
if(!mpx->debug){
remove(MPXLOG);
remove(ERRLOG);
}
mpx_erasetmp(mpx);
retcode= mpx->history;
mpx_xfree(mpx->buf);
for(i= 0;i<(int)mpx->nfonts;i++)
mpx_xfree(mpx->font_name[i]);
free(mpx);
if(retcode==mpx_cksum_trouble)
retcode= 0;
return retcode;
}


/*:226*/
