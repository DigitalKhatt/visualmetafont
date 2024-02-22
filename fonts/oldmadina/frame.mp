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

input myfontbase.mp
	
numeric fine;

fine := 1pt;

fine.breadth:=fine;
pickup pencircle scaled fine; fine:=fine-eps;
fine.nib:=savepen; breadth_[fine.nib]:=fine;

numeric u;

u := 1mm;

color darkred ; darkred := (.625,0,0) ;
color color[];

color1 := (.625,0,0) ;
color1 := .5[red,green];
color1 := (hex "a4"/255,hex "bb"/255,hex "90"/255);

color2 := (hex "d4"/255,hex "c3"/255,hex "7b"/255);
color3 := (hex "d2"/255,hex "b7"/255,hex "66"/255); %#d2b766s


linewidth := 2u;

pickup fine.nib;

pos1(linewidth ,90);
pos2(linewidth,45);
pos3(linewidth,90);
pos4(linewidth,180);
pos5(linewidth,-90);
pos6(linewidth,-110);
pos7(linewidth,120);	
pos8(linewidth,90);

z1 = (0,43u);
z2 = z1 + (12u,-13u);
z3 = z1 + (20u,-17u);
z4 = z3 + (2u,2u);
z5 = z3 + (0,4u);
z6 = z5 + (-2.5u,-0.5u);
z7 = z3 + (1.5u,8u);	
z8  = z1 + (25u,0);

path feuille_haut;

feuille_haut := stroke z1e {(z2-z1) rotated -15} .. {(z2-z1) rotated -20}z2e {(z3-z2) rotated 20} .. {right} z3e .. z4e .. z5e .. {dir -150} z6e {dir 120}   ..{dir -30}z7e {dir 100}.. {z8-z7}z8e;


picture pic[];
pic1 := image(
	fill path_.l -- (x8l,y1l -37.5u) -- (x1l,y1l-37.5u) -- cycle withcolor color1;
	filldraw feuille_haut;
);

draw pic1;
draw pic1 reflectedabout (z1 + (0.01u,0),(0.01u,0)); 

path feuille_milieu;


%La feuille au milieu

z9 = z1 + (0,-6u);
z10 = z9 + (9u,-10u);
z11 = z10 + (11u,-6u);
z12 = z11 + (-5u,-4u);
z13 = z1 + (0,-44u);

pos9(linewidth ,90);
pos10(linewidth,45);
pos11(linewidth,0);
pos12(linewidth,-45);
pos13(linewidth,-90);



feuille_milieu := stroke z9e {(z10-z9) rotated -15} .. {(z10-z9) rotated -20} z10e {(z11-z10) rotated 20} .. {(z11-z10) rotated 15} z11e {(z12-z11) rotated 15} 
			.. {left} z12e	{down} .. {(z13-z12) rotated 15} z13e;
			
			
path feuille_milieu_m;
feuille_milieu_m := path_;

pic1 := image(
	fill path_.l -- (z13l + (-0,0)) -- cycle withcolor color2;	
	filldraw feuille_milieu;
);

draw pic1;
draw pic1 reflectedabout (z1 + (0.01u,0),(0.01u,0)); 		

pair shift[];

shift1 = (0u,-20u);
z8a = z8 shifted shift1;
z7a = z7 shifted shift1;
z6a = z6 shifted shift1;
z5a = z5 shifted shift1;
z4a = z4 shifted shift1;
z3a = z4a + (-6u,-4u);
z2a = z3a + (-13u,13u);
z1a = z2a + (4u,5u);
z0a = z1a + (4u,-5u);
z0.5a = (0,ypart z3a);


pos4a(linewidth,180);
pos5a(linewidth,-90);
pos6a(linewidth,-110);
pos7a(linewidth,120);	
pos8a(linewidth,90);
pos3a(linewidth,90);
pos2a(linewidth,0);
pos1a(linewidth,-90);	
pos0a(linewidth,-180);	
pos0.5a(linewidth,90);

path feuille_base,feuille_base_reverse,feuille_base_m;

feuille_base := stroke  z8a.e  {(z7a - z8a)} .. {dir -80}z7a.e {dir 150}.. {dir -60}z6a.e {dir 30} ..  z5a.e .. {down}z4a.e .. {left}z3a.e .. z2a.e{up} .. {right}z1a.e;
feuille_base_reverse := rpathstroke_;
feuille_base_m := path_;

path coeur,coeur_m;	

coeur := stroke z1a.e {right} .. z0a.e .. z0.5a.e {dir -135};
coeur_m := path_;

filldraw mybuildcycle(feuille_base,coeur_m);
filldraw mybuildcycle(feuille_base_reverse,feuille_milieu_m);




filldraw coeur;	
filldraw coeur reflectedabout (z1,(0,0));
filldraw feuille_base reflectedabout (z1,(0,0));

%le bas de la feuille d'en bas

path bordure_enbas,bordure_enbas_m;	

z1c = (x8,y4a - 2u);
z2c = (x8-9u ,0);
z3c = (0 ,y2c+10u);

pos1c(linewidth,90);
pos2c(linewidth,90);
pos3c(linewidth,90);	

bordure_enbas := stroke z1c.e {dir -110} .. {left} z2c.e .. z3c.e {dir 135};
bordure_enbas_m := path_;
tpath_1 := mybuildcycle(bordure_enbas,feuille_milieu_m);
%tpath_2 := mybuildcycle(bordure_enbas,coeur3);



filldraw tpath_1;
%filldraw tpath_2;
filldraw bordure_enbas reflectedabout (z1,(0,0));

%la forme sur le coeur

path formesurcoeur;

z1b = (0,y2a+2u);
z2b =  z1b + (4u,0);
z3b =  z2b + (3u,-3u);
z4b =  z3b + (-3u,-3u);
z5b =  z4b + (-2u,2u);
z6b =  (0,y5b);

pos1b(linewidth,90);
pos2b(linewidth,90);
pos3b(linewidth,0);	
pos4b(linewidth,-90);	
pos5b(linewidth,-90);
pos6b(linewidth,-90);

formesurcoeur := stroke z1b.e  .. {right} z2b.e .. z3b.e {down} .. z4b.e {left}.. z5b.e {dir 135} --- z6b.e;

filldraw formesurcoeur;






%la forme ovale au dessus du coeur

path petitcoeur;;

z1d = (x1b,y1b + 11u);
z2d = z1d + (2u,-2u);
z3d = z2d + (-2u,-4u);


pos1d(linewidth -0.5u,90);
pos2d(linewidth -0.5u ,0);
pos3d(linewidth -0.5u,-90);

petitcoeur := stroke z1d.e {right} .. {down} z2d.e .. z3d.e {dir -120};

filldraw petitcoeur;

%le cercle en haut a droite

path cercle;

temp := 1.9u;
z1f = z1 + (13u,-4u);
z2f = z1f + (temp,-temp);
z3f = z2f + (-temp,-temp);	
z4f = z3f + (-temp,temp);	


pos1f(linewidth -0.9u,90);
pos2f(linewidth -0.9u ,0);
pos3f(linewidth -0.9u,-90);
pos4f(linewidth -0.9u,-180);

cercle := stroke z1f.e {right} .. {down} z2f.e .. z3f.e {left} .. z4f.e {up} .. z1f.e;;

filldraw cercle;

%losange au milieu de la feuille en bas à droite

path losange;

z1g = z8a + (0,-6u);
z2g = z1g + (-1.8u,-4u);
z3g = z2g + (1.8u,-3u);

losange := z1g -- z2g -- z3g -- cycle;	

filldraw losange;	

%pickup fine.nib;




%picture pic;
%pic = currentpicture;
%for i=1 upto 1:
%	pic:=pic reflectedabout (z1 + (0.01u,0),(0.01u,0)); 
%	draw pic;
%endfor

%pic := currentpicture;
%for i=1 upto 1:
%	pic:=pic shifted (50u,0);
%	draw pic;
%endfor

%penlabels(l,m,r,ll,rr,range 1 thru 20);
