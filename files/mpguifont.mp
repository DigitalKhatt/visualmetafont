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

%if known MPGUI:
	input mfplain.mp;
%fi

vardef round primary u =
 if numeric u: floor(u+.5)
 elseif pair u: (round xpart u, round ypart u)
 else: u fi enddef;

newinternal nextunicode, lefttatweel,righttatweel, leftbearing, rightbearing,  nuqta, which_in_baseline, leftbearingnumber, totalglyphs, ignore_exp_parameters, savepicture, number_of_anchors; 
newinternal string charname,originalglyph;
pair lefttatweel_const, righttatweel_const;

leftbearingnumber := 0;

nextunicode := 58000; % 57343; %hex"E000"-1; % the first code for aya numbers
string charmacroname;

def MarkAnchor = 1 enddef;
def EntryAnchor = 2 enddef;
def ExitAnchor = 3 enddef;
def EntryAnchorRTL = 4 enddef;
def ExitAnchorRTL = 5 enddef;

% vardef first.hah.medi.pluslt_[]@#=@ enddef;

% vardef hah.medi.pluslt_[]_(expr lt,rt) = hah.medi_(lt,rt);enddef;

def makechardef(suffix name)(expr c,w_sharp)(text t) =
  charmacroname:=str name & "_";  
  expandafter vardef scantokens charmacroname(expr lt,rt) = %endgroup gobbled true  5 = x.12345678255; x.12345678255 := whatever;
	if(ignore_exp_parameters <> 0) :
    ignore_exp_parameters := 0; 
	else:
		charlt := lefttatweel := lt;
    charrt := righttatweel :=rt;   
	fi
    %if charwd = -1: charwd := w_sharp; fi;
    t         
    originalglyph := str name;
  %gobble begingroup 
  enddef;
  beginchar(name,c,w_sharp,-1,-1);
  scantokens charmacroname(0,0);
  endchar; 
enddef;
delimiters begindefchar enddefchar;

def defchar(suffix name)(expr c,w_sharp,h_sharp,d_sharp) =  
  makechardef(name)(c,w_sharp) begindefchar
enddef;

def beginchar(suffix name)(expr c,w_sharp,h_sharp,d_sharp) =  
 begingroup
 originalglyph := "";
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
 leftbearing := 100;
 rightbearing := 0;
 lefttatweel_const := righttatweel_const := (0,0);
 charwd:=w_sharp;      charht:=h_sharp;       chardp:=d_sharp;
 w:=charwd; h:=charht; d:=chardp;
 charic:=0; clearxy; clearit; %clearpen; 
 number_of_anchors := 0;
 left_tatweeltension := 0;
 right_tatweeltension := 0;  
 left_verticalextratio := 0;
 right_verticalextratio := 0;
 which_in_baseline := 3;
 savepicture := 0; 

 string anchors[]anchorname;
 pair anchors[]pairvalue;
 numeric anchors[]anctype; 
 tr_ := identity;
enddef;
transform tr_;

vardef currentwidth = xpart (urcorner currentpicture) - xpart (ulcorner currentpicture) enddef;

def endchar =
	%scantokens extra_endchar;
	%if proofing>0: makebox(proofrule); fi
  
	chardx:=w;     % desired width of the character in pixels
  
  if which_in_baseline <> 3 :
    if (known rightanchor) and (known leftanchor) :      
      tr_ := tr_ shifted -leftanchor;
    elseif  (known leftanchor)  :      
      tr_ := tr_ shifted -leftanchor;
    elseif  (known rightanchor) :      
      tr_ := tr_ shifted (0,- ypart rightanchor); 
    fi;
  elseif known leftanchor :
    %tr_ := tr_ shifted (- xpart leftanchor,0); 
    tr_ := tr_ shifted -leftanchor;
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
      charwd:= xpart (urcorner currentpicture) - xpart (leftanchor) + rightbearing - residu;      
    elseif  (known rightanchor) :
      %charwd:= xpart (urcorner currentpicture) - xpart (ulcorner currentpicture) + leftbearing - residu;   
      charwd:= xpart (rightanchor) - xpart (ulcorner currentpicture) + leftbearing;
      %currentpicture := currentpicture shifted (-xpart (ulcorner currentpicture) + leftbearing,0);
      tr_ := tr_ shifted (-xpart (ulcorner currentpicture) + leftbearing,0);
    else:
      charwd:= xpart (urcorner currentpicture) - xpart (ulcorner currentpicture) + leftbearing + rightbearing; 
      %currentpicture := currentpicture shifted (-xpart (ulcorner currentpicture) + leftbearing,0);
      tr_ := tr_ shifted (-xpart (ulcorner currentpicture) + leftbearing,0);
    fi;
  fi;	
  
  currentpicture := currentpicture transformed tr_;
  
  charht := ypart (ulcorner currentpicture);if charht < 0 : charht := 0; fi ;  
  chardp := ypart (llcorner currentpicture);if chardp > 0 : chardp := 0; fi ; 
  
  %if unknown glyphs[charcode] :
  %  listeglyphs[incr totalglyphs] := charcode;
  %fi
  
  leftanchor := leftanchor transformed tr_;
  rightanchor := rightanchor transformed tr_;

  if savepicture = 1 :
	expandafter picture scantokens charname pic; 
	scantokens charname pic := currentpicture;
  fi;
  
  % glyphs[charcode] := currentpicture;
  % glyphs[charcode].charname := charname;
  % glyphs[charcode].charwd := charwd;
  % glyphs[charcode].matrix := tr_;
  % glyphs[charcode].leftanchor := leftanchor;
  % glyphs[charcode].rightanchor := rightanchor;
  
  for i = 0 upto (number_of_anchors - 1): 
    anchors[i].pairvalue := anchors[i].pairvalue transformed tr_;
  endfor;
  
	shipit;
	endgroup 
enddef;

def drawcomponent(suffix charcode)(expr x,y,xx,xy,yx,yy) =  
	begingroup
		if unknown MPGUI :
			transform T_;
			xpart T_ = x; ypart T_ = y; xxpart T_ =  xx; xypart T_ = xy; yxpart T_ = yx; yypart T_ = yy;
			draw glyphs[charcode] transformed T_ withprescript "begin component " & decimal charcode withpostscript "end component " & decimal charcode;
		fi ;
	endgroup
enddef;

def drawcomp(suffix charcode)(expr T) =  
	begingroup		
    draw scantokens str charcode pic transformed T withprescript "begin component " & decimal charcode withpostscript "end component " & decimal charcode;		
	endgroup
enddef;

def controlledPath (expr numpath,numpoint)(text name,content) =
  def name = content; enddef;
enddef;

def setCursiveAnchor(suffix pname)(expr pvalue)(expr anchortype) =
  anchors[number_of_anchors].anchorname := str pname;  
  anchors[number_of_anchors].pairvalue := pvalue; 
  anchors[number_of_anchors].anctype := anchortype;  
  number_of_anchors := incr number_of_anchors;
enddef;
def setMarkAnchor(suffix pname)(expr pvalue) =
  anchors[number_of_anchors].anchorname := str pname;  
  anchors[number_of_anchors].pairvalue := pvalue; 
  anchors[number_of_anchors].anctype := MarkAnchor;  
  number_of_anchors := incr number_of_anchors;
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
outputformat := "svg";
outputtemplate := "automedinaoutput\%j-%c.svg";
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

secondarydef p join q =
hide(
  if left_tatweeltension = 0: left_tatweeltension := 1;fi;
  if right_tatweeltension = 0: right_tatweeltension := 1;fi;
  if left_verticalextratio = 0: left_verticalextratio := 12;fi;
  if right_verticalextratio = 0: right_verticalextratio := 12;fi;
)
if ypart p < ypart q :
  hide(
    left_elongation := -lefttatweel * nuqta;
    z_0:= lefttatweel_const + (left_elongation,left_elongation/left_verticalextratio); 
    z_0:= round z_0;    
    z_1 := z_0 + (0,lineheight);    % pendirection
    leftanchor := z_0; 
  )
	p .. tension atleast left_tatweeltension .. z_0
	.. controls  (z_0 - (residu,0)) ..  (1/2[z_0,z_1] - (residu,0)) %(xpart z_0 -residu, 1/2[ypart z_0,ypart z_1]) 
	.. controls  (z_1 - (residu,0)) .. z_1 .. tension atleast left_tatweeltension .. q
else:
	hide(
    right_elongation := righttatweel * nuqta;
    z_4:= q + righttatweel_const + (right_elongation, -right_elongation/right_verticalextratio); 
    z_5 := p;
    z_6 := q;
    if (known leftanchor) :
      if which_in_baseline = 1 :
        z_4 := (xpart z_4, ypart leftanchor);  
      elseif which_in_baseline = 2 :
         %z_4 := (xpart z_4, -ypart leftanchor + ypart z_4);          
         z_4 := (xpart z_4, 0);
      fi
      
    fi
    z_4 := round z_4;   
    z_3 := z_4 + (0,lineheight); % pendirection; 
    rightanchor := z_4;
  )
	z_5 .. tension atleast right_tatweeltension .. z_3
	.. controls  (z_3 + (residu,0)) ..  (1/2[z_4,z_3] + (residu,0)) % (xpart z_3 +residu, 1/2[ypart z_3,ypart z_4]) 
	.. controls  (z_4 + (residu,0)) .. z_4 .. tension atleast right_tatweeltension .. z_6
fi
enddef;
def cutGlyph(expr bottomPoint, upPoint, rightPath) =

path p;
pair bottomDispl, upDispl;
numeric bottomTime, upTime;

l_ := residu;

p := pathpart currentpicture;

currentpicture:=nullpicture;

(bottomTime,whatever) = p intersectiontimes bottomPoint;
(upTime,whatever) = p intersectiontimes upPoint; 

if rightPath :
  bottomDispl := bottomPoint - l_* unitvector direction bottomTime of p;
  upDispl := upPoint +  l_* unitvector direction upTime of p;
  fill subpath(bottomTime,upTime) of p .. controls   upDispl .. 1/2[upDispl,bottomDispl] .. controls bottomDispl ..   cycle ;
else:
  bottomDispl := bottomPoint + l_* unitvector direction bottomTime of p;
  upDispl := upPoint -   l_* unitvector direction upTime of p;
  fill subpath(0,bottomTime) of p .. controls   bottomDispl .. 1/2[bottomDispl,upDispl] .. controls upDispl ..  subpath(upTime,length p) of p .. cycle ;
fi;

enddef;

vardef diratpoint expr pt of p =
  numeric time_;
  (time_,whatever) = p intersectiontimes pt;
  if time_ < 0: 
    errmessage("The point does not belong to the path"); origin
  else: 
    direction time_ of p
  fi
enddef;

newinternal numeric residu, lineheight, left_verticalextratio, left_verticalconst,right_verticalextratio, right_verticalconst, left_tatweeltension, right_tatweeltension,
penangle, pendirection, penwidth;

pair pendirection;

def rotatepen primary a =
+ (penwidth,0 ) rotated a
enddef;


left_verticalextratio := right_verticalextratio := 12;
left_verticalconst := right_verticalconst := 0;



%penangle := 85;
residu :=5;
lineheight := 82;
penwidth := 85;
adjacent := penwidth+-+lineheight;
%sind penangle * penwidth = lineheight;
%penwidth = lineheight++aaaa;
penangle := angle (adjacent,lineheight);
pendirection := (penwidth,0) rotated penangle;
nuqta := 1.4penwidth;

%penwidth = lineheight++(lineheight/2);

%pair pendirection;

%pendirection := (lineheight/2,lineheight);
%penangle := angle pendirection;

input myfontbase.mp;
input ayah.mp;


