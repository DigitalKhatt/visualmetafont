% Public domain.

newinternal superness;
	
vardef image(text t) =
  save currentpicture;
  picture currentpicture;
  currentpicture := nullpicture;
  t;
  currentpicture
enddef;

vardef mybuildcycle(text ll) =
  save ta_, tb_, k_, i_, pp_; path pp_[];
  k_=0;
  for q=ll: pp_[incr k_]=q; endfor
  i_=k_;
  for i=1 upto k_:
    (ta_[i], length pp_[i_]-tb_[i_]) =
      pp_[i] intersectiontimes reverse pp_[i_];
    if ta_[i]<0:
      errmessage("Paths "& decimal i &" and "& decimal i_ &" don't intersect");
    fi
    i_ := i;
  endfor
  
  for i=1 upto k_: 	
	subpath (ta_[i],tb_[i]) of pp_[i] .. 
	endfor
    cycle
enddef;

def shiftdef(suffix $)(expr delta) =
 vardef $ primary x = x+delta enddef enddef;

vardef stroke text t =
 forsuffixes e = l,,r: path_.e:=t; rpath_.e:= reverse (t); endfor
 if cycle path_.l:
  errmessage "Beware: `stroke' isn't intended for cycles"; fi
 rpathstroke_ := rpath_.l -- reverse rpath_.r -- cycle;
 path_.l -- reverse path_.r -- cycle 
enddef;



path path_,rpath_,rpath_.l,rpath_.r,rpathstroke_;

vardef circ_stroke text t =
 forsuffixes e = l,r: path_.e:=t; endfor
 if cycle path_.l:
  errmessage "Beware: `stroke' isn't intended for cycles"; fi
 path_.l .. tension 3 .. reverse path_.r .. tension 3 .. cycle enddef;
 
vardef circ_strokee text t =
 forsuffixes e = l,r: path_.e:=t; endfor
 if cycle path_.l:
  errmessage "Beware: `stroke' isn't intended for cycles"; fi
 path_.l ..  reverse path_.r  .. cycle enddef;

vardef super_arc.r(suffix $,$$) = % outside of super-ellipse
 pair center,corner;
 if y$=y$r: center=(x$$r,y$r); corner=(x$r,y$$r);
 else: center=(x$r,y$$r); corner=(x$$r,y$r); fi
 z$.r{corner-z$.r}...superness[center,corner]{z$$.r-z$.r}
  ...{z$$.r-corner}z$$.r enddef;

vardef super_arc.l(suffix $,$$) = % inside of super-ellipse
 pair center,corner;
 if y$=y$r: center=(x$$l,y$l); corner=(x$l,y$$l);
 else: center=(x$l,y$$l); corner=(x$$l,y$l); fi
 z$l{corner-z$l}...superness[center,corner]{z$$l-z$l}
  ...{z$$l-corner}z$$l enddef;

vardef pulled_super_arc.r(suffix $,$$)(expr superpull) =
 pair center,corner;
 if y$=y$r: center=(x$$r,y$r); corner=(x$r,y$$r);
 else: center=(x$r,y$$r); corner=(x$$r,y$r); fi
 z$r{corner-z$r}...superness[center,corner]{z$$r-z$r}
  ...{z$$r-corner}z$$r enddef;

vardef pulled_super_arc.l(suffix $,$$)(expr superpull) =
 pair center,corner,outer_point;
 if y$=y$r: center=(x$$l,y$l); corner=(x$l,y$$l);
  outer_point=superness[(x$$r,y$r),(x$r,y$$r)];
 else: center=(x$l,y$$l); corner=(x$$l,y$l);
  outer_point=superness[(x$r,y$$r),(x$$r,y$r)]; fi
 z$l{corner-z$l}
  ...superpull[superness[center,corner],outer_point]{z$$l-z$l}
  ...{z$$l-corner}z$$l enddef;

vardef pulled_arc@#(suffix $,$$) =
 pulled_super_arc@#($,$$)(superpull) enddef;

vardef serif_arc(suffix $,$$) =
 z${x$$-x$,0}...(.75[x$,x$$],.25[y$,y$$]){z$$-z$}...{0,y$$-y$}z$$ enddef;

vardef penpos@#(expr b,d) =
 if known b: if b<=0: errmessage "bad penpos"; fi fi
 (x@#r-x@#l,y@#r-y@#l)=(b,0) rotated d;
 x@#=.5(x@#l+x@#r); y@#=.5(y@#l+y@#r) enddef;

newinternal currentbreadth;

vardef pos@#(expr b,d) =
	if known b: 
		if b <= currentbreadth: 
			errmessage "bad pos"; 
		fi 
	fi
	(x@#r-x@#l,y@#r-y@#l)=(b-currentbreadth,0) rotated d;
	x@#=.5(x@#l+x@#r); y@#=.5(y@#l+y@#r) 
enddef;

def numeric_pickup_ primary q =
 currentpen:=pen_[q];
 pen_lft:=pen_lft_[q];  pen_rt:=pen_rt_[q];
 pen_top:=pen_top_[q];  pen_bot:=pen_bot_[q];
 currentpen_path:=pen_path_[q];
 if known breadth_[q]: currentbreadth:=breadth_[q]; fi 
enddef;

def yah_p_iso =
	numeric u_,y_;
	u_ := (charwd/(15*dx#))*dx;
	y_ := (charht/(8*dy#))*dy;
	x10 = x4 - u_ = x5; x3 = x6 = x7 + 2u_ = x4 - 8u_; x8 = x4 + 4u_ = x.r;
	y10 = y6 = y4 - 3y_ = y5 - 5y_ = y.r; y3 = y.rr; y7 = y8 = y.r - 3y_;
	z9 = 0.2[z10,z5]; z1 = 0.9[z6,z9]; z2 = 0.8[z3,z5];
	draw z8--z7{left}..z6..controls z1 and z2..z5..z4{(u_,-6y_)};
enddef;

vardef labels@#(text t) =
  forsuffixes $=t:
    setlabel(str$,z$); 
  endfor  
enddef;

vardef labelspoints@#(text t) =
  forsuffixes $=t:
    setlabel(str$,points$); 
  endfor  
enddef;

vardef setlabel(expr s,z) = % puts string s near point z
  save p; picture p;
  if known z:
    p = s infont defaultfont scaled (defaultscale/bp_per_pixel);
    draw p shifted z t_  withcolor black;   
  fi
enddef;
