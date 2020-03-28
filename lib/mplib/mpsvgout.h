/*3:*/
// #line 80 "../../../source/texk/web2c/mplibdir/svgout.w"

#ifndef MPSVGOUT_H
#define MPSVGOUT_H 1
#include "mplib.h"
#include "mpmp.h"
#include "mplibps.h"
typedef struct svgout_data_struct{
/*6:*/
// #line 113 "../../../source/texk/web2c/mplibdir/svgout.w"

size_t file_offset;

/*:6*//*12:*/
// #line 165 "../../../source/texk/web2c/mplibdir/svgout.w"

char*buf;
unsigned loc;
unsigned bufsize;

/*:12*//*20:*/
// #line 299 "../../../source/texk/web2c/mplibdir/svgout.w"

int level;

/*:20*//*27:*/
// #line 386 "../../../source/texk/web2c/mplibdir/svgout.w"

integer dx;
integer dy;

/*:27*//*56:*/
// #line 1159 "../../../source/texk/web2c/mplibdir/svgout.w"

int clipid;

/*:56*/
// #line 87 "../../../source/texk/web2c/mplibdir/svgout.w"

}svgout_data_struct;
/*4:*/
// #line 92 "../../../source/texk/web2c/mplibdir/svgout.w"

void mp_svg_backend_initialize(MP mp);
void mp_svg_backend_free(MP mp);

/*:4*//*60:*/
// #line 1209 "../../../source/texk/web2c/mplibdir/svgout.w"

int mp_svg_gr_ship_out(mp_edge_object*hh,int prologues,int standalone);

/*:60*/
// #line 89 "../../../source/texk/web2c/mplibdir/svgout.w"

#endif

/*:3*/
