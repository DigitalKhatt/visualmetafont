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

numeric fine,u,linewidth;
u := 22; %dx;
fine := 0.4u;
linewidth := 2u;

fine.breadth:=fine;
pickup pencircle scaled fine; fine:=fine-eps;
fine.nib:=savepen; breadth_[fine.nib]:=fine;

color mycolor[];
	
mycolor1 := (.625,0,0) ;
mycolor1 := .5[red,green];
mycolor1 := (hex "a4"/255,hex "bb"/255,hex "90"/255);

mycolor2 := (hex "d4"/255,hex "c3"/255,hex "7b"/255);
mycolor3 := (hex "d2"/255,hex "b7"/255,hex "66"/255); %#d2b766s
mycolor4 := red;

mycolor4 := (hex "a2"/255,hex "20"/255,hex "22"/255); %#ae2022

mycolor2 := 1.1mycolor2;
mycolor1 := 1.3mycolor1;

def ayashape = 

	path temppath[];
	picture pic[];
	transform Reflection;
	
	pickup fine.nib;
	
	%leaf 
	path feuille_haut, feuille_haut_m;
	
	
	
	z1 = (0,36u);
	z2 = z1 + (17u,-12u);
	z3 = z1 + (28u,-19u);
	z4 = z3 + (-4u,-2u);
	z5 = z3 + (-9u,-1u);
	z6 = z5 + (-4u,-2u);
	z7 = z6 + (-8u,4.5u);
	z8 = z1 + (0,-21u);
	
	pos1(linewidth ,90);
	pos2(linewidth ,45);
	pos3(linewidth,0);
	pos4(linewidth,-90);
	pos5(linewidth,-90);
	pos6(linewidth,-90);
	pos7(linewidth,-90);
	pos8(linewidth,-90);
	
	feuille_haut := stroke z1e {dir -75} .. z2e .. {dir -20}z3e {dir -160}.. z4e .. {dir 160} z5e {dir -160} .. {dir -150} z6e {dir 120} .. {left}z7e .. z8e;
	feuille_haut_m := path_;
	
	% cicrcle inside leaf
	path circle_inside_leaf;
	z9 = (0,24.5u);
	circle_inside_leaf := fullcircle scaled 6u shifted z9 ; % withpen pencircle scaled 4bp;	
	
	% curve right to circle
	path curve_right_circle;
	z10 = z9 + (8u,-1u);
	z11 = z10 + (8u,-4u);
	
	curve_right_circle := z10 {(z11 - z10) rotated 10} .. {(z11 - z10) rotated -10 }z11;
	
	% drawing
	pair dep;
	
	dep := (0,0);
	
	Reflection = identity reflectedabout  (z1 + dep,dep);
	
	% inner and outer circles
	path innercircle,outercircle;
	
	innercircle := fullcircle scaled 48u;
	outercircle := fullcircle scaled 58u;
	
	
	
	fill outercircle withcolor mycolor2;
	fill innercircle withcolor mycolor1;
	draw outercircle withpen pencircle scaled linewidth;
	draw innercircle withpen pencircle scaled linewidth;
	%leaf
	picture leaf;
	leaf := image(
	
		fill  feuille_haut_m -- (feuille_haut_m transformed Reflection) -- cycle withcolor mycolor2;
		
		filldraw feuille_haut;
		filldraw feuille_haut transformed Reflection;

		fill circle_inside_leaf;
	
		draw curve_right_circle withpen pencircle scaled linewidth;
		draw curve_right_circle transformed Reflection withpen pencircle scaled linewidth ;
	);
	
	draw leaf;
	draw leaf reflectedabout ((0,0), (1,0)) ;
	
  picture ayapic;
	
	ayapic := currentpicture;
 
	
enddef;
def hizbshape =
	picture hizbpic;	
	hizbpic := currentpicture;
enddef;
%beginchar(hex "06DD",0,0,0);
%	ayashape; 
%	currentpicture := ayapic shifted(-xpart (llcorner ayapic),-ypart (llcorner ayapic) );	
%	
%	numeric _depth, _margin; _depth := -600;_margin := 50;
%	
%	currentpicture := currentpicture shifted (_margin,_depth);
%	
%	charwd := xpart (lrcorner currentpicture) - xpart (llcorner currentpicture) + 2_margin;
%	charht :=ypart (ulcorner currentpicture);
%	chardp :=ypart (llcorner currentpicture);
%	if chardp < 0 : chardp := 0; fi
%endchar;