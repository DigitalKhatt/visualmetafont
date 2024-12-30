%% Copyright © 2015-2020 Amine Anane. http://digitalkhatt/license
 %
 % This file is part of DigitalKhatt.
 %
 % DigitalKhatt is free software: you can redistribute it and/or modify
 % it under the terms of the GNU Affero General Public License as published by
 % the Free Software Foundation, either version 3 of the License, or
 % (at your option) any later version.
 %
 % DigitalKhatt is distributed in the hope that it will be useful,
 % but WITHOUT ANY WARRANTY; without even the implied warranty of
 % MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 % GNU Affero General Public License for more details.
 %
 % You should have received a copy of the GNU Affero General Public License
 % along with DigitalKhatt.  If not, see <https://www.gnu.org/licenses/>.
 %
%%

%mpost -numbersystem double -mem mpguifont
%math_mode

string newline; newline = char 10;

def drawoptions(text t) =
  def _op_ = t enddef;
enddef;

if known MPGUI:
	drawoptions(withcolor false);
else:
  numeric params;params[0]:=0;params[1]:=0;params[2]:=0;
  drawoptions();
fi

def fill expr c = addto currentpicture contour c _op_ enddef;
def fillc expr c = addto currentpicture contour c _op_ if _addedpostscript <> "" : withpostscript  _addedpostscript fi enddef;

newinternal string _addedpostscript;
def addwithpostscript(expr t) =
  _addedpostscript := _addedpostscript  & t & ";";  
enddef;

vardef round primary u =
 if numeric u: floor(u+.5)
 elseif pair u: (round xpart u, round ypart u)
 else: u fi enddef;

newinternal nextunicode, lefttatweel,righttatweel, leftbearing, rightbearing,  nuqta, which_in_baseline, totalglyphs, savepicture, number_of_anchors; 
newinternal string charname,originalglyph,coloredglyph;
newinternal showAnchors,digitscale;
pair lefttatweel_const, righttatweel_const;
newinternal minLeft,maxLeft,minRight,maxRight, maxLii, maxRii;
newinternal charlt,charrt,glyphtype;

%For parametric kashida
newinternal leftVerticalRatio,leftVerticalRatioNeg,rightVerticalRatio,rightVerticalRatioNeg;

digitscale := 0.80;

nextunicode := 58000; % 57343; %hex"E000"-1; % the first code for aya numbers

string nametable.familyName;
string nametable.copyright;

def MarkAnchor = 1 enddef;
def EntryAnchor = 2 enddef;
def ExitAnchor = 3 enddef;
def EntryAnchorRTL = 4 enddef;
def ExitAnchorRTL = 5 enddef;
def Anchor = 6 enddef;

def GlyphTypeBase = 1 enddef;
def GlyphTypeLigature = 2 enddef;
def GlyphTypeMark = 3 enddef;
def GlyphTypeComponent = 4 enddef;
def GlyphTypeColored = 5 enddef;
def GlyphTypeTemp = 6 enddef;

def generateAlternate(suffix macroname)(suffix params) =
beginchar(alternatechar,983040,-1,-1,-1);charlt:=params0;charrt:=params1;macroname(params);endchar;
enddef;

primarydef w ?? d = if known w : w else : d fi enddef;

def makechardef(suffix name)(expr c,w_sharp,g_type)(text t) =    
  vardef name$(suffix params) =    
    interim lefttatweel := params0 ?? 0;
    interim righttatweel := params1 ?? 0;   	  
    t         
    originalglyph := str name;   
  enddef;  
  expandafter vardef scantokens(str name & "_")(expr _lt,_rt) =    
    save params;
    params0 := _lt;
    params1 :=_rt;   	  
    name$(params);    
  enddef;
  beginchar(name,c,w_sharp,g_type,-1);
    charlt := params0;
    charrt := params1;   	   
    name$(params);
  endchar; 
enddef;
delimiters begindefchar enddefchar;

def defchar(suffix name)(expr c,w_sharp,g_type,d_sharp) =  
  makechardef(name)(c,w_sharp,g_type) begindefchar
enddef;

def beginchar(suffix name)(expr c,w_sharp,g_type,d_sharp) =
 clearxy;
 begingroup
 originalglyph := ""; coloredglyph:="";
 glyphtype:= if g_type = -1 : GlyphTypeBase else: g_type fi;
 charcode:=if known c: byte c else: -1 fi; 
 if (charcode = -1) and (unknown name): 
  charcode := incr nextunicode; name:= charcode; 
 else :  
  if charcode = -1 : 
    charcode := name; 
  else:
    name := charcode;
  fi;
 fi;
 charname := str name;
 %save leftanchor,rightanchor;
 numeric param[];
 pair leftanchor,rightanchor;
 charlt := charrt := 0; 
 leftbearing := 0;
 rightbearing := 0;
 lefttatweel_const := righttatweel_const := (0,0);
 charwd:=w_sharp;  
 charic:=0;  clearit; %clearpen; %clearxy; 
 number_of_anchors := 0;
 left_tatweeltension := 0;
 right_tatweeltension := 0;  
 left_verticalextratio := 0;
 right_verticalextratio := 0;
 which_in_baseline := 3;
 _addedpostscript := "";
 savepicture := 0;  
 save anchors,minA,maxA;
 string anchors[]anchorname;
 pair anchors[]pairvalue;
 numeric anchors[]anctype; 
 tr_ := identity;
 pair leftdeltas[][][];
 pair rightdeltas[][][];
 pair defdeltas[][][];

 leftVerticalRatio := 10;
 leftVerticalRatioNeg := 0;
 rightVerticalRatioNeg := 0;
 rightVerticalRatio := 10; 
 minLeft := minRight := -1;
 maxLeft := maxRight := 6;
 maxLii := maxRii := 20;
enddef;
transform tr_;

vardef currentwidth = xpart (urcorner currentpicture) - xpart (ulcorner currentpicture) enddef;

def endchar =
  
	%scantokens extra_endchar;
	%if proofing>0: makebox(proofrule); fi
  
	chardx:=w;     % desired width of the character in pixels
  
  if which_in_baseline <> 3 :
    if (which_in_baseline = 4) :
        if known leftanchor :
            tr_ := tr_ shifted (- xpart leftanchor,floor ypart leftanchor - ypart leftanchor);        
        elseif known rightanchor :
            tr_ := tr_ shifted (floor xpart rightanchor - xpart rightanchor,floor ypart rightanchor - ypart rightanchor);            
        fi
    elseif (which_in_baseline = 5) :
      ; tr_ := tr_ shifted -(xpart leftanchor,ypart rightanchor); 
    elseif (known rightanchor) and (known leftanchor) :      
      tr_ := tr_ shifted -leftanchor;
    elseif  (known leftanchor)  :      
      tr_ := tr_ shifted -leftanchor;
    elseif  (known rightanchor) :      
      tr_ := tr_ shifted (0,- ypart rightanchor); 
    fi;
  elseif known leftanchor :
    tr_ := tr_ shifted (- xpart leftanchor,0); 
    %tr_ := tr_ shifted -leftanchor;
  elseif known rightanchor :
  %   tr_ := tr_ shifted (floor xpart rightanchor - xpart rightanchor,ypart tr_ - floor ypart tr_ + ypart rightanchor - floor ypart rightanchor);   
  fi;
  %elseif which_in_baseline = 2 :
  %  if (known leftanchor) :      
  %    tr_ := tr_ shifted -(xpart leftanchor,0);    
  %  fi;
  %fi;
  
  if charwd = -1: 
    if (known rightanchor) and (known leftanchor) :
      charwd:= xpart rightanchor - xpart leftanchor;      
    elseif  (known leftanchor)  :
      charwd:= xpart (urcorner currentpicture) - xpart (leftanchor) + rightbearing; % - residu;      
    elseif  (known rightanchor) :
      numeric tnum_;
      tnum_ = round (- xpart(ulcorner currentpicture) + leftbearing);
      charwd:= xpart (rightanchor) + tnum_;      
      tr_ := tr_ shifted (tnum_,0);
    else:
      charwd:= xpart (urcorner currentpicture) - xpart (ulcorner currentpicture) + leftbearing + rightbearing;       
      tr_ := tr_ shifted (-xpart (ulcorner currentpicture) + leftbearing,0);
    fi;
  fi;	
  
  currentpicture := currentpicture transformed tr_;
  
  charht := ypart (ulcorner currentpicture);if charht < 0 : charht := 0; fi ;  
  chardp := ypart (llcorner currentpicture);if chardp > 0 : chardp := 0; fi ;   
  
  if known leftanchor :
    %leftanchor := leftanchor transformed tr_;
    defAnchor(rtlcursive,leftanchor,ExitAnchorRTL); 
  fi
  if known rightanchor :  
    %rightanchor := rightanchor transformed tr_;
    defAnchor(rtlcursive,rightanchor,EntryAnchorRTL); 
  fi  

  if savepicture = 1 :
	  expandafter picture scantokens charname pic; 
	  scantokens charname pic := currentpicture;
  fi;

  % Access to array is sequential and becomes very slow when its size increases
  % glyphs[charcode] := currentpicture;
  % glyphs[charcode].charname := charname;
  % glyphs[charcode].charwd := charwd;
  % glyphs[charcode].matrix := tr_;
  % glyphs[charcode].leftanchor := leftanchor;
  % glyphs[charcode].rightanchor := rightanchor;
  
  for i = 0 upto (number_of_anchors - 1): 
    anchors[i].pairvalue := anchors[i].pairvalue transformed tr_;
    if showAnchors <> 0 :
      setlabel(anchors[i].anchorname,anchors[i].pairvalue);      
    fi
  endfor;
  
	shipit;
	endgroup 
enddef;

pair tmp_pair_params_[];
numeric tmp_num_params_[];

def drawcomp(suffix charcode)(expr T) =  
	begingroup		
    draw charcode.pic transformed T withprescript "begincomponent" withpostscript str charcode & "," & decimal xpart T & "," & decimal ypart T;	
    draw (0,0) transformed T withprescript "endcomponent";  	
	endgroup
enddef;

def controlledPath (expr numpath,numpoint)(text name,content) =
  def name = content; enddef;
enddef;

def setCursiveAnchor(suffix pname)(expr pvalue)(expr anchortype) =
  defAnchor(pname,pvalue,anchortype); 
enddef;
def setMarkAnchor(suffix pname)(expr pvalue) =
  defAnchor(pname,pvalue,MarkAnchor); 
enddef;
def setAnchor(suffix pname)(expr pvalue) =
  defAnchor(pname,pvalue,Anchor);
enddef;
def defAnchor(suffix pname)(expr pvalue)(expr anchortype) =
  numeric iid_;
  if known anchors.pname :
    if anchors[anchors.pname].anctype =  anchortype:
      iid_ := anchors.pname;
    fi    
  fi
  if not known iid_ :
    anchors.pname := number_of_anchors; iid_ := number_of_anchors; number_of_anchors := incr number_of_anchors;
  fi
  anchors[iid_].anchorname := str pname;
  anchors[iid_].pairvalue := pvalue;
  anchors[iid_].anctype := anchortype;
  
enddef;

def saveAnchors(expr table, fileName) =
  %anchors[charcode].name  := pname;
  %anchors[charcode].pairvalue  := pvalue;  
  pair _temp;  
  
  for iii = 1 upto totalglyphs :    
    numglyph := listeglyphs[iii];
    if known glyphs[numglyph].charname : 
      %write table & ".anchors[" & ditto & glyphs[numglyph].charname & ditto & "] = {" to fileName;
      write table & ".anchors[" & decimal numglyph & "] = {" to fileName;
      if (known glyphs[numglyph].number_of_anchors) : 
        for i = 1 upto glyphs[numglyph].number_of_anchors :
          if glyphs[numglyph].anchors[i].type = 1:
            _temp := glyphs[numglyph].anchors[i].pairvalue transformed glyphs[numglyph].matrix;
            write "  [" & ditto & glyphs[numglyph].anchors[i].name & ditto & "] = {" & decimal xpart _temp & "," & decimal ypart _temp & "}," to fileName;
          fi;
        endfor
       fi;
      if known glyphs[numglyph].leftanchor : 
        _temp := glyphs[numglyph].leftanchor;
        write "  [" & ditto & "leftanchor" & ditto & "] = {" & decimal xpart _temp & "," & decimal ypart _temp & "}," to fileName;
      fi;
      if known glyphs[numglyph].rightanchor : 
        _temp := glyphs[numglyph].rightanchor;
        write "  [" & ditto & "rightanchor" & ditto & "] = {" & decimal xpart _temp & "," & decimal ypart _temp & "}," to fileName;
      fi;
        
      write "}" to fileName;    
    fi;
    
  endfor;
  
  write EOF to fileName;
enddef;
string EOF;
EOF = char 0; 


bpppix_ := 1;
pt := 1;
 
warningcheck := 0;

if unknown scale_factor :
	scale_factor := 1 ;
fi ;


mode:=lowres;
%mag	:= 0.01 * 0.99626 * scale_factor	;

mode_setup;	

prologues := 3;
%outputformat := "svg";
%outputtemplate := "%j-%c.svg";
fontmaking:=0;
tracingonline:=1;

% picture glyphs[];
%string glyphs[]charname;
%transform glyphs[]matrix;
%pair glyphs[]leftanchor,glyphs[]rightanchor;
%string glyphs[]anchors[]name;
%pair glyphs[]anchors[]pairvalue;
pair glyphs[]points[];
pair points[];
pair points[]a;
pair points[]b;
pair points[]c;
pair points[]d;
pair points[]e;
pair rctr[];
pair lctr[];

pair tmpp[];
numeric ldir[];
numeric rdir[];
numeric mdir[];
numeric ltens[];
numeric rtens[];
numeric mtens[];

secondarydef p endlink q =
if ypart p < ypart q :
	p 
	.. controls  (p - (residu,0)) ..  (xpart p -residu, 1/2[ypart p,ypart q]) 
	.. controls  (q - (residu,0)) .. q 
else:
	p 
	.. controls  (p + (residu,0)) ..  (xpart p +residu, 1/2[ypart p,ypart q]) 
	.. controls  (q + (residu,0)) .. q 
fi
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef setdef(suffix $)(expr b) = if not known $ : $ := b fi enddef;
secondarydef p leftjoin q =
   hide(
    setdef(x.lft.offset,nuqta + 50);
    setdef(y.lft.offset,0);
    setdef(x.lft.ratio.2 ,44);
    setdef(x.lft.ratio.1.top ,11);
    setdef(y.lft.ratio.1.top ,30);
    setdef(x.lft.ratio.1.bot ,x.lft.ratio.1.top);
    setdef(y.lft.ratio.1.bot ,y.lft.ratio.1.top);

    tatweel_ :=  lefttatweel *nuqta;  

    if tatweel_ > 0 :
       z.lft.bot =  round(-(tatweel_ ,tatweel_ / leftVerticalRatio));    
    else:
       z.lft.bot =  round(-(tatweel_,if leftVerticalRatioNeg <> 0 : tatweel_ / leftVerticalRatioNeg else : 0 fi ));  
    fi   
    z.lft.top = z.lft.bot + ljoinvector;

    ratio1_ := (100 - x.lft.ratio.2)/100;;  
    first_ratio_bot_x_ := x.lft.ratio.1.bot/100;
    first_ratio_bot_y_ := y.lft.ratio.1.bot /100;
    first_ratio_top_x_ := x.lft.ratio.1.top/100;
    first_ratio_top_y_ := y.lft.ratio.1.top/100;

    hight.bot_ :=  ypart p - y.lft.bot;
    width.bot_ :=  -x.lft.bot +xpart p;
    hight.top_ :=  ypart q - y.lft.top;
    width.top_ :=  -x.lft.top +xpart q;
    
    z_l1 := p - (width.bot_ * first_ratio_bot_x_,hight.bot_* first_ratio_bot_y_);    
    z_l2 :=  (x.lft.bot + width.bot_  * ratio1_,y.lft.bot);  
    z_l3 :=  (x.lft.top + width.top_  * ratio1_,y.lft.top); 
    z_l4 :=  q - (width.top_  *first_ratio_top_x_,hight.top_ * first_ratio_top_y_);
    leftanchor = z.lft.bot;    
  )

  p  ..  controls  z_l1 and z_l2 ..  
  z.lft.bot .. controls  ( z.lft.bot - (residu,0)) .. 
  (1/2[ z.lft.bot,z.lft.top] - (residu,0)) .. controls  (z.lft.top - (residu,0)) .. 
  z.lft.top  .. controls z_l3  and z_l4 .. 
 q
enddef;
secondarydef p rightjoin q =
   hide(    
    setdef(x.rt.offset,nuqta);
    setdef(y.rt.offset,0);
    setdef(y.rt.rbearing,0);
    
    setdef(x.rt.ratio.2 ,44);
    setdef(x.rt.ratio.1.top ,11);
    setdef(y.rt.ratio.1.top ,30);
    setdef(x.rt.ratio.1.bot ,x.rt.ratio.1.top);
    setdef(y.rt.ratio.1.bot ,y.rt.ratio.1.top);

    tatweel_ := righttatweel*nuqta;
    if known x.rt.rbearing :
       x.rt.bot := round(x.rt.rbearing + tatweel_);
    else:
      if xpart q > xpart p : 
          x.rt.bot := round(xpart q + x.rt.offset + tatweel_);
      else :
          x.rt.bot := round(xpart p + x.rt.offset + tatweel_);
      fi
    fi
    
    if tatweel_ > 0 :           
         y.rt.bot :=  round(y.rt.rbearing - tatweel_ / rightVerticalRatio);
    else:           
         y.rt.bot :=  round(y.rt.rbearing if rightVerticalRatioNeg <> 0 : - tatweel_ / rightVerticalRatioNeg fi);
    fi

    z.rt.top = z.rt.bot + rjoinvector;
    ratio1_ := (100 - x.rt.ratio.2)/100;
    first_ratio_bot_x_ := x.rt.ratio.1.bot/100;
    first_ratio_bot_y_ := y.rt.ratio.1.bot /100;
    first_ratio_top_x_ :=  x.rt.ratio.1.top/100;
    first_ratio_top_y_ := y.rt.ratio.1.top /100;
    hight.bot_ :=  ypart q - y.rt.bot;
    width.bot_ :=  x.rt.bot -xpart q;
    hight.top_ :=  ypart p - y.rt.top;
    width.top_ :=  x.rt.top -xpart p;

    z_r1 := p + (width.top_  * first_ratio_top_x_, - hight.top_* first_ratio_top_y_);  
    z_r2 := (x.rt.top -  width.top_ * ratio1_  , y.rt.top); 
    z_r3 := (x.rt.bot -  width.bot_ * ratio1_ , y.rt.bot);
    z_r4 := q + (width.bot_  * first_ratio_bot_x_, - hight.bot_* first_ratio_bot_y_); 

   rightanchor =  z.rt.bot;
  )
  p  .. controls z_r1 and z_r2 .. % .. tension atleast 1 and atleast 1 ..  
 z.rt.top  .. controls  ( z.rt.top + (residu,0)) ..  
(1/2[ z.rt.top, z.rt.bot] + (residu,0)) 	.. controls  ( z.rt.bot + (residu,0)) ..
 z.rt.bot   .. controls z_r3 and z_r4 .. % {left} .. tension atleast 1 and atleast 1 ..
q
enddef;
secondarydef p setranchor q =
  hide(z.rt.rbearing=q;)
  p
enddef;
let rightjoinii = rightjoin;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef tpoint(expr p,q,r,s,t) =
  points12 := t[p,q];
  points23 := t[q,r];
  points34 := t[r,s];
  points123 := t[points12,points23];
  points234 := t[points23,points34];
  t[points123,points234]
enddef;
pair z_l[],z_r[];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
secondarydef p link q =
hide(
  if left_tatweeltension = 0: left_tatweeltension := 1;fi;
  if right_tatweeltension = 0: right_tatweeltension := 1;fi;
  if left_verticalextratio = 0: left_verticalextratio := 12;fi;
  if right_verticalextratio = 0: right_verticalextratio := 6;fi;
)
if ypart p < ypart q :
  hide(   
    z_0:= p; 
    z_0:= round z_0;    
    z_1 := z_0 + joinvector; 
    leftanchor := z_0; 
  )
	z_0 
	.. controls  (z_0 - (residu,0)) ..  (1/2[z_0,z_1] - (residu,0)) %(xpart z_0 -residu, 1/2[ypart z_0,ypart z_1]) 
	.. controls  (z_1 - (residu,0)) .. z_1 .. tension atleast left_tatweeltension .. q
else:
	hide(    
    z_4:= q;    
    z_4 := round z_4;   
    z_3 := z_4 + joinvector; 
    rightanchor := z_4;
  )
	p .. tension atleast right_tatweeltension .. z_3
	.. controls  (z_3 + (residu,0)) ..  (1/2[z_4,z_3] + (residu,0)) % (xpart z_3 +residu, 1/2[ypart z_3,ypart z_4]) 
	.. controls  (z_4 + (residu,0)) .. z_4
fi
enddef;
vardef definekashida(suffix kashida_macro)(expr p,q, upTimeBegin, bottomTimeBegin, rightPath,pLeft,pRight,exact ) =
path ret_;
which_in_baseline := 4;
path sinna__;
pair shift__;

if not rightPath :  
    sinna__ := pathpart image(kashida_macro(righttatweel,0));     
    length_ := length   sinna__;   
    sinna__ := cutKashida(sinna__,upTimeBegin,bottomTimeBegin,rightPath,exact,_bottomTime,_upTime);        
    finalPoint := if (_bottomTime < pLeft) : pLeft else : length_ - floor(_bottomTime) + pLeft fi;       
    shift__ := p - point finalPoint of sinna__;
    sinna__ := sinna__ shifted shift__;     
    rightanchor := rightanchor shifted shift__;
    %rightanchor := point  0 of sinna__ ;
    %rightanchor := rightanchor + (0,-5);
    sinna__ := subpath(finalPoint, finalPoint + length sinna__  - 2 ) of sinna__;
    rightfactor := factor;
else:
    sinna__ := pathpart image(kashida_macro(lefttatweel,0));      
    length_ := length   sinna__;
    sinna__ := cutKashida(sinna__,upTimeBegin,bottomTimeBegin,rightPath,exact,_bottomTime,_upTime); 
    finalPoint := if (_upTime < pRight) : pRight - floor(_upTime) else : length_ - floor(_upTime) + pRight fi;
    shift__ := p - point finalPoint of sinna__;                
    sinna__ := sinna__ shifted shift__;
    leftanchor := leftanchor shifted shift__;
    sinna__ := subpath(finalPoint, finalPoint + length sinna__  - 2 ) of sinna__;
    leftfactor := factor;
fi
ret_ := subpath(1,infinity) of sinna__;
ret_
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
secondarydef p kashida_iii q =

hide(
path kashida_;
boolean rightPath_;

if ypart p < ypart q :
    rightPath_:= true;
else :
    rightPath_ := false;
fi
kashida_ := definekashida(kashida.iii_)(p,q,3, 0, rightPath_,3,7,false );
addwithpostscript( "kashida_iii="  & decimal length kashida_ );
)

p ..  kashida_ .. q
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
def defaultkashida(expr kashidanumber) =
string name_;
name_ := "kashida_" & kashidanumber;
expandafter secondarydef expandafter p scantokens(name_) q =

hide(
path kashida_;
boolean rightPath_;


if ypart p < ypart q :
    rightPath_:= true;
else :
    rightPath_ := false;
fi
kashida_ := definekashida(scantokens("kashida."& kashidanumber & "_"))(p,q,5, 0, rightPath_,3,8,true );
addwithpostscript( "kashida_" & kashidanumber & "="  & decimal length kashida_ );
)

p ..  kashida_ .. q
enddef;
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%secondarydef p kashida_ii q =

%hide(
%path kashida_;
%boolean rightPath_;


%if ypart p < ypart q :
%    rightPath_:= true;
%else :
%    rightPath_ := false;
%fi
%kashida_ := definekashida(kashida.ii_)(p,q,5, 0, rightPath_,3,8,true );
%addwithpostscript( "kashida_ii="  & decimal length kashida_ );
%)
%
%p ..  kashida_ .. q
%enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef cutKashida(expr kashida, upTimeBegin, bottomTimeBegin, rightPath,exact)(suffix bottomTime,upTime) =
    path p, ret;
    pair bottomDispl, upDispl, bottomPoint, upPoint;
    
    l_ := 1residu ;

    p := kashida;

    if not exact :    
        upTime := upTimeBegin  + directiontime right of subpath(upTimeBegin,infinity) of p;
        bottomTime := bottomTimeBegin  + directiontime left of subpath(bottomTimeBegin,infinity) of p;	
    else:
        upTime := upTimeBegin;
        bottomTime := bottomTimeBegin;
    fi;
    


    upPoint := point upTime of  p;
    bottomPoint := point bottomTime of  p;

   

    if not rightPath :

        

        bottomDispl := bottomPoint - l_* unitvector direction bottomTime of p;
        upDispl := upPoint +  l_* unitvector direction upTime of p;

        if bottomTime > upTime : upTime := upTime + length p; fi
          

        ret:= subpath(bottomTime,upTime) of p  .. controls   upDispl .. 1/2[upDispl,bottomDispl] .. controls bottomDispl ..   cycle ;
        %ret:= subpath(bottomTime,upTime) of p  -- 1/2[upPoint,upDispl] ..   1/2[bottomPoint,bottomDispl] --   cycle ;
        
        rightanchor := bottomPoint;

        %draw ret;
    else:        

        bottomDispl := bottomPoint + l_* unitvector direction bottomTime of p;
        upDispl := upPoint -   l_* unitvector direction upTime of p;

        if upTime > bottomTime : bottomTime := bottomTime + length p; fi
           

        ret:= subpath(upTime,bottomTime) of p .. controls   bottomDispl .. 1/2[bottomDispl,upDispl] .. controls upDispl ..   cycle ;
        %ret:= subpath(upTime,bottomTime) of p --  1/2[bottomPoint,bottomDispl] ..    1/2[upPoint,upDispl] --  cycle ;
        
        
        leftanchor := bottomPoint;
    fi;
    ret
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
def cutGlyph(expr bottomPoint, upPoint, rightPath) =

path _p;
pair bottomDispl, upDispl;
numeric bottomTime, upTime;

l_ := residu;

_p := pathpart currentpicture;

currentpicture:=nullpicture;

(bottomTime,whatever) = _p intersectiontimes bottomPoint;
(upTime,whatever) = _p intersectiontimes upPoint; 

if rightPath :
  bottomDispl := bottomPoint - l_* unitvector direction bottomTime of _p;
  upDispl := upPoint +  l_* unitvector direction upTime of _p;
  fill subpath(bottomTime,upTime) of _p .. controls   upDispl .. 1/2[upDispl,bottomDispl] .. controls bottomDispl ..   cycle ;
else:
  bottomDispl := bottomPoint + l_* unitvector direction bottomTime of _p;
  upDispl := upPoint -   l_* unitvector direction upTime of _p;
  fill subpath(0,bottomTime) of _p .. controls   bottomDispl .. 1/2[bottomDispl,upDispl] .. controls upDispl ..  subpath(upTime,length _p) of _p .. cycle ;
fi;

enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef diratpoint expr pt of p =
  numeric time_;
  (time_,whatever) = p intersectiontimes pt;
  if time_ < 0: 
    errmessage("The point does not belong to the path"); origin
  else: 
    direction time_ of p
  fi
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
newinternal numeric residu, lineheight, left_verticalextratio, left_verticalconst,right_verticalextratio, right_verticalconst, left_tatweeltension, right_tatweeltension,penwidth;

pair joinvector,ljoinvector,rjoinvector;

def rotatepen primary a =
+ (penwidth,0 ) rotated a
enddef;


left_verticalextratio := right_verticalextratio := 12;
left_verticalconst := right_verticalconst := 0;

residu :=5;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef component expr index of pic =
save i_,p;
i_ := 1;
path p;
for item within pic:
p := pathpart item;
exitif i_ = index;

i_ := i_ + 1;

endfor
p
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef bldm(expr def,delta) =
factor_ := (1 + lefttatweel) / 21;
def + factor_ * delta
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef brdm(expr def,delta) =
factor_ := (1 + righttatweel) / 21;
def + factor_ * delta
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef bldmvi(expr def,delta) =
factor_ := (lefttatweel - minLeft) / (maxLeft - minLeft);
def + factor_ * delta
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef brdmvi(expr def,delta) =
factor_ := (1 + righttatweel) / (maxRight - minRight);
def + factor_ * delta
enddef;
vardef bldmv(expr def,dmax,dmin) =
if lefttatweel < 0 :
  factor_ := lefttatweel  /  minLeft;
  def + factor_ * dmin
else:
   factor_ := lefttatweel / maxLii;
   def + factor_ * dmax
fi
enddef;
vardef brdmv(expr def,dmax,dmin) =
if righttatweel < 0 :
  factor_ := righttatweel  /  minRight;
  def + factor_ * dmin
else:
   factor_ := righttatweel / maxRii;
   def + factor_ * dmax
fi
enddef;
vardef liii[](expr dft,dmax,dmin) =
if params@ < 0 :
  factor_ := params@  /  (minA@ ?? -1);
  dft + factor_ * dmin
else:
   factor_ := params@ / (maxA@ ?? 20);
   dft + factor_ * dmax
fi
enddef;
vardef leii[](expr dft,dmax) =
_maxA := maxA@ ?? 20;
_minA := minA@ ?? 0;
factor_ := (params@ - _minA) / (_maxA - _minA);
dft + factor_ * dmax
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef interpolatepoint(expr min,max,factor_) =
%factor := if factor_ > 1 : 1 else :  factor_ fi; 
xdelta_ := xpart max - xpart min;
ydelta_ := ypart max - ypart min;
(xpart min + xdelta_ * factor_ ,ypart min + ydelta_ * factor_)
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef interpolatepath(expr minpath,maxpath,factor_) =
path out;

out := interpolatepoint(point 0 of minpath,point 0 of maxpath,factor_);


for j=0 upto length minpath - 1 :
out := out & interpolatepoint(point j of minpath,point j of maxpath,factor_)
 ..  controls interpolatepoint(postcontrol j of minpath,postcontrol j of maxpath,factor_)
 and  interpolatepoint(precontrol j + 1 of minpath,precontrol j + 1 of maxpath,factor_) 
..   interpolatepoint(point j + 1 of minpath, point j + 1  of maxpath,factor_);
endfor
out

enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef interpolatepic(expr defpic,deltapic,tatweel) =
save currentpicture;
picture currentpicture;
currentpicture := nullpicture;
wd_max := xpart (urcorner deltapic) - xpart (ulcorner deltapic);
wd_min := xpart (urcorner defpic) - xpart (ulcorner defpic);

g_diff := urcorner deltapic - urcorner defpic;


factor := tatweel * nuqta /(wd_max - wd_min);


path minpath, maxpath;
picture out;

i_ := 1;

for item within defpic:
minpath := pathpart item;
maxpath := component i_ of deltapic;

fill interpolatepath(minpath,maxpath,factor) & cycle ;

i_ := i_ + 1;
endfor
currentpicture
enddef;
pair g_diff;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef interpolatemm(expr minpic,maxpic,default,tatweel) =
save defaultpic_;
picture result;

numeric _tatweel;
_tatweel := default + tatweel;

if _tatweel < 0 : _tatweel := 0; fi

result:=  interpolatepic(minpic,maxpic,_tatweel);
result
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef interpolate(expr minpic,defaultpic, maxpic,tatweel) =
save defaultpic_;
picture result;

if tatweel >= 0 :
result:=  interpolatepic(defaultpic,maxpic,tatweel);
else:
result:=  interpolatepic(defaultpic,minpic, tatweel);
fi
result
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef calcDeltasPoint(expr path,pathIndex, pointIndex,type) =
    pair out,leftAdjust,rightAdjust,deftAdjust;
    leftAdjust := if known leftdeltas[pathIndex][pointIndex][type] : leftfactor * leftdeltas[pathIndex][pointIndex][type] else : (0,0) fi;
    rightAdjust := if known rightdeltas[pathIndex][pointIndex][type] : rightfactor * rightdeltas[pathIndex][pointIndex][type] else : (0,0) fi;
    deftAdjust := if known defdeltas[pathIndex][pointIndex][type] : defdeltas[pathIndex][pointIndex][type] else : (0,0) fi;
    if type = 0 :
        out := point pointIndex of path;
    elseif type = 1:
        out := precontrol pointIndex of path;
    else :
        out := postcontrol pointIndex of path;
    fi
    out := out + deftAdjust + leftAdjust + rightAdjust;
    out
enddef;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
vardef calcDeltasPic(expr pic) =
    save currentpicture;
    picture currentpicture;
    currentpicture := nullpicture;
    path _path, _newpath;
    numeric _pathIndex;
    _pathIndex := 0;
    for item within pic:
        _path := pathpart item;
        _newpath := calcDeltasPoint(_path,_pathIndex,0,0);
        for j=0 upto length _path - 2 :
            _newpath := _newpath ..  controls calcDeltasPoint(_path,_pathIndex,j,2) and  calcDeltasPoint(_path,_pathIndex,j+1,1) ..  calcDeltasPoint(_path,_pathIndex,j+1,0);
        endfor
        fill _newpath ..  controls calcDeltasPoint(_path,_pathIndex,length _path - 1,2) and  calcDeltasPoint(_path,_pathIndex,0,1) .. cycle withpostscript  _addedpostscript;
        _pathIndex := _pathIndex + 1;
    endfor
    currentpicture
enddef;
vardef getWidth(expr pic) =
  xpart (urcorner pic) - xpart (ulcorner pic)
enddef;
vardef getDigitName(expr digit) =
  if digit = 0 : "zeroindic"
  elseif digit = 1 : "oneindic"
  elseif digit = 2 : "twoindic"
  elseif digit = 3 : "threeindic"
  elseif digit = 4 : "fourindic"
  elseif digit = 5 : "fiveindic"
  elseif digit = 6 : "sixindic"
  elseif digit = 7 : "sevenindic"
  elseif digit = 8 : "eightindic"
  elseif digit = 9 : "nineindic"
  else : ""
  fi
enddef;

vardef genAyaNumber(suffix ayaname)(expr ayaNumber, maxWidth) =

  save ayapic,position,digitheight,digittrans,digitname,digitvalue,digitkern,width_;
  string digitname[];
  picture ayapic,digitspic[]; 
  transform digittrans[];
  numeric  digitvalue[],width_[];
  ayapic := ayaname.pic;
  digitheight := 110;
  digitkern := 40;
  width_0:=getWidth(ayapic);
  drawcomp(ayaname,identity);
  if ayaNumber < 10 :
    digitname.1 := getDigitName(ayaNumber);
    digitspic.1 := scantokens(digitname.1)pic;
    position := width_0/2 - getWidth(digitspic.1)/2;
    digittrans.1 := identity shifted (position,digitheight);
    drawcomp(scantokens digitname.1,digittrans.1);
  elseif ayaNumber < 100 : 
    digitvalue.1 := ayaNumber mod 10;
    digitvalue.2 := ayaNumber div 10;
    digitname.1 := getDigitName(digitvalue.1);
    digitname.2 := getDigitName(digitvalue.2);
    digitspic.1 := scantokens(digitname.1)pic;
    digitspic.2 := scantokens(digitname.2)pic;
    position := width_0/2 - (getWidth(digitspic.1) + getWidth(digitspic.2) + digitkern)/2;
    digittrans.2 := identity shifted (position,digitheight);
    digittrans.1 := identity shifted (position + getWidth(digitspic.2) + digitkern,digitheight);
    drawcomp(scantokens digitname.1,digittrans.1);
    drawcomp(scantokens digitname.2,digittrans.2);
  else:
    digitvalue.1 := ayaNumber mod 10;
    digitvalue.2 := (ayaNumber div 10) mod 10;
    digitvalue.3 := ayaNumber div 100;
    digitname.1 := getDigitName(digitvalue.1);
    digitname.2 := getDigitName(digitvalue.2);
    digitname.3 := getDigitName(digitvalue.3);
    digitspic.1 := scantokens(digitname.1)pic;
    digitspic.2 := scantokens(digitname.2)pic;
    digitspic.3 := scantokens(digitname.3)pic;
    position := width_0/2 - (getWidth(digitspic.1) + getWidth(digitspic.2) + getWidth(digitspic.3) + 2digitkern)/2;
    picture _pic;numeric _scale;
    _pic := image(
     draw digitspic.3;
     draw digitspic.2 shifted (getWidth(digitspic.3) + digitkern,0);
     draw digitspic.1 shifted (getWidth(digitspic.3) + getWidth(digitspic.2) + 2*digitkern,0);
    );
    width_1:=getWidth(_pic);
    if width_1 > maxWidth :
      _scale := maxWidth / width_1 ;
      _pic := _pic xscaled _scale;
      position := (width_0 - getWidth(_pic))/2;         
    fi
    draw _pic shifted (position,digitheight);
  fi


enddef;
vardef convRGB(expr rgb)=
  (hex substring (0,2) of rgb/255,hex substring (2,4) of rgb/255,hex substring (4,6) of rgb/255)
enddef;	
vardef tint(expr srgb,factor)=
  save currentR,currentG,blue,currentB,newR,newG,newB,inv;
  currentR := redpart srgb ** 2.2;
  currentG := greenpart srgb ** 2.2;
  currentB := bluepart srgb ** 2.2;
  newR := currentR + (1 - currentR) * factor;
  newG := currentG + (1 - currentG) * factor;
  newB := currentB + (1 - currentB) * factor;
  inv := 1/2.2;
  (newR ** inv,newG ** inv,newB ** inv)
enddef;	
vardef directionatpoint expr pt of p =
 save x_,y_; (x_,y_)=p intersectiontimes pt;
 if x_<0: errmessage("The paths don't intersect"); origin
 else: direction x_ of p fi enddef;
vardef split expr gt of p =  
  save t,start,endt,_pt; numeric t,start,endt; pair _pt[];
  start:= floor(gt);
  t := gt - start; 
  if t = 0 :
    start := gt - 1;
    endt := gt + 1;
    _pt2.5 := point gt of p;show _pt2.5;
    subpath(0,start) of p .. controls postcontrol start of p  and round(precontrol gt of p) .. 
    (round(xpart _pt2.5),round(ypart _pt2.5)) .. controls round(postcontrol gt of p) and precontrol endt of p .. 
    subpath(endt,length p-1) of p 
    if cycle p :  .. controls postcontrol length p-1 of p and precontrol 0 of p ..  cycle fi
  else :     
     % Casteljau 
     endt := ceiling(gt);
    _pt1 := point start of p;
    _pt2 := postcontrol start of p;
    _pt2.5 := point gt of p;
    _pt3 := precontrol endt of p;
    _pt4 := point endt of p;

    _pt12 := (1-t) * _pt1 + t * _pt2;
    _pt23 := (1-t) * _pt2 + t * _pt3;
    _pt34 := (1-t) * _pt3 + t * _pt4;
    _pt1223 := round((1-t) * _pt12 + t * _pt23);
    _pt2334 := round((1-t) * _pt23 + t * _pt34);
    
    subpath(0,start) of p .. controls _pt12  and _pt1223 .. 
    (round(xpart _pt2.5),round(ypart _pt2.5)) .. controls _pt2334 and _pt34 .. 
    subpath(endt,length p-1) of p 
    if cycle p :  .. controls postcontrol length p-1 of p and precontrol 0 of p ..  cycle fi
  fi
enddef;

vardef roundwidth expr pic =
  save p,ledge,redge,currentpicture,x_,y_,tint,cp;
  path p,ledge,redge,cp;
  picture currentpicture;
  pair intpoint;
  currentpicture := nullpicture;
  ledge := llcorner pic -- ulcorner pic;
  redge := lrcorner pic -- urcorner pic;
  for item within pic:
    p := pathpart item;
    tint := xpart(p intersectiontimes ledge);
    cp:= if tint < 0 : p else: split tint of p fi;
    tint := xpart(p intersectiontimes redge);
    cp:= if tint < 0 : cp else: split tint of cp fi;
    fill cp;
  endfor
  currentpicture
enddef;
