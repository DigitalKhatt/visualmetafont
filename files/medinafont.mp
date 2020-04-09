beginchar(allah,-1,2000,602,-24);
%%backgroundimage:images/Allah.jpg,-150,-1325.56,1,0,0,1
%%beginparams
z1 = (357.639,59.0278);
z2 = (728.009,81.0185);
z3 = (719.907,-0.578704);
z4 = (347.801,-20.2546);

%%beginpaths

fill
(1.73611,39.3519) .. tension 2.70908 and 1.40034 ..
(71.7593,186.921) .. tension 1.18822 and 2.7276 ..
(237.847,287.616) .. tension 1.47366 and 1 ..
(248.383,342.154) .. tension 1 and 3.2949 ..
 {right}(272.725,395.513) .. tension 1.09193 and 1 ..
 {down}(277.199,387.731) .. tension 1 and 1 ..
 {down}(272.663,322.06) .. tension 1 and 1 ..
(280.342,175.875) .. tension 1.16852 and 0.898728 ..
 {right}z1 .. tension 1 and 1 ..
(470.811,90.4225) .. tension 0.791136 and 2.72112 ..
 {up}(535.843,175.245) .. tension 2.53865 and 1.23111 ..
 {up}(493.634,429.398) .. tension 2.02879 and 3.3752 ..
 {right}(515.522,486.09) .. tension 7.57161 and 1.2973 ..
(595.486,160.88) .. tension 1.10177 and 1.954 ..
 {right}z2 .. tension 1 and 1.63689 ..
 {up}(760.995,103.009) .. tension 9.93739 and 1 ..
(699.074,261.574) .. tension 1 and 1.36007 ..
 {up}(654.514,410.301) .. tension 1 and 4.58332 ..
 {right}(670.097,510.737) .. tension 1 and 1 ..
(678.343,502.935) .. tension 1 and 1 ..
(684.184,471.734) .. tension 0.876729 and 1.61148 ..
 {down}(791.088,148.727) .. tension 1.4891 and 1.40548 ..
 {left}z3 .. tension 1.80691 and 1.23383 ..
(581.048,67.2587) .. tension 1 and 8.22102 ..
 {left}(546.296,133.102) .. tension 21.6119 and 1 ..
(480.579,28.1872) .. tension 1.17827 and 1 ..
 {left}z4 .. tension 0.827491 and 1 ..
(248.383,73.1194) .. tension 1 and 4.43068 ..
 {left}(241.319,113.426) .. tension 6.65058 and 1 ..
(177.083,49.1898) .. tension 1.45947 and 0.75 ..
 {left}(45.1389,29.5139) .. tension 1 and 2.39869 ..
 {up} cycle
;
fill
(62.5,118.056) .. tension 3.83529 and 5.56227 ..
(239.005,135.995) .. tension 6.21562 and 4.22514 ..
 {left}(232.639,226.852) .. tension 10.9839 and 2.52398 ..
 {down} cycle
;

%%endpaths
%%beginbody

glyphs[allah].points[1] := z1;
glyphs[allah].points[2] := z2;
glyphs[allah].points[3] := z3;
glyphs[allah].points[4] := z4;

savepicture:=1;

endchar;
defchar(behshape_hah.init,-1,-1,-1,-1);
%%backgroundimage:images/yeh_hah.init.jpg,-2402,-1310,1,0,0,1
%%beginparams
lefttatweel := 1.14; % numeric
z1 = (187.843,35.3787);
z3 = (93.5904,268.97);
z4 = (241.056,269.778);
z5 = (707.795,178.635);
z6 = (48.3432,203.355);

%%beginverbatim

if left_verticalextratio = 0 : left_verticalextratio := 12; fi
 
lefttatweel := charlt + 0;
z2 = z1 + (0,lineheight) rotated -25;

%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension 1 and 7.16575 ..
 {up}(352.608,156.206) .. tension 11.4017 and 3.20171 ..
 {left}(113.878,210.009) .. tension 0.75 and 6.35213 ..
 {left}(49.4805,196.424) .. tension 1.39544 and 1 ..
 {up}z6 .. tension 1.74599 ..
z3 .. tension 1.6654 ..
(248.335,337.553) .. tension 1.24443 ..
(323.958,393.717) .. tension 1 and 1 ..
(395.945,468.028) .. tension 1.40212 ..
 {right}(517.665,566.349) .. tension 1.66263 and 1.66908 ..
 {down}(573.661,429.557) .. tension 1 and 7.2459 ..
 {left}(541.363,346.637) .. tension 3.99671 and 1 ..
 {up}(533,377) .. tension 1 and 2.29045 ..
 {left}(491.354,489.693) .. tension 1.61274 ..
(291.554,304.568) .. tension 7.56205 ..
 {down}z4 .. tension 3.69766 and 2 ..
(329.218,234.802) ..
(578.344,187.71) ..
 {right}(705.637,186.045) ..
 {down}z5 .. tension 3.46819 ..
 {left}(654.804,108.009) .. tension 2.71262 and 0.75 ..
(532.972,121.608) ..
(380,103) ..
(279,68.2) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(behmediyehfina,-1,1100,-1,-1);
%%backgroundimage:images/behmediyehfina.jpg,-2931.69,-1241.9,1,0,0,1
%%beginparams
z1 = (328.914,133.709);
z2 = (495,58.3396);
z3 = (486.942,126.926);
z4 = (678,245.755);
z6 = (617.289,78.7848);

%%beginverbatim

righttatweel_const := (52,-3);
z5 = z4 - (0,lineheight);
%%endverbatim

%%beginpaths

fill
(27,28) .. tension 1.15253 and 3.3611 ..
 {right}(106.688,250.075) .. tension 1 and 1 ..
 {down}(111.549,241.741) .. tension 5.63294 and 0.757542 ..
 {down}(59.4167,87.8056) .. tension 1 and 0.915494 ..
 {right}(255.164,-76.5994) .. tension 0.992248 and 7.28829 ..
 {up}(564.007,55.1932) .. tension 2.37563 and 1 ..
 {left}z2 .. tension 0.75 and 1.23646 ..
 {up}z1 .. tension 0.88586 and 1 ..
(413.934,303.418) .. tension 1 and 1.31753 ..
 {right}(507.37,355.997) .. tension 1.70457 and 0.784083 ..
 {right}z4 .. tension 1 and 1 ..
(826,294) .. tension 1 and 2.17307 ..
 {right}(886.823,394.929) .. tension 1 and 1 ..
 {down}(890.447,379.447) .. tension 1.77855 and 1 ..
 {down}(871.965,311.589) .. tension 4.24093 and 1.50218 ..
 {right}(957.755,277.778) .. tension 1 and 1 ..
 {up}(972.704,288.387) .. tension 1 and 1 ..
 {up}(964.506,371.335) .. tension 1 and 5.11817 ..
 {right}(990.548,450.907) .. tension 3.91941 and 1 ..
 {down}(1011.34,315.837) .. tension 5.70435 and 0.84063 ..
 {left}(940.394,192.419) .. tension 1 and 4.1596 ..
 {left}(831.887,240.162) .. tension 5.64608 and 1 ..
(816.454,220.332) ..
 {left}z5 ..
(562.771,206.155) .. tension 1 and 2.2613 ..
 {left}(482.184,281.911) .. tension 1.93794 and 2.48175 ..
 {down}(364.043,186.878) .. tension 1 and 1.07483 ..
 {right}z3 .. tension 1 and 2.9362 ..
 {right}(564.973,128.619) .. tension 1 and 1.87612 ..
 {down}z6 .. tension 1.67143 and 1.27101 ..
 {left}(243.127,-157.232) .. tension 0.907684 and 0.923566 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(lam_hah.init,-1,-1,-1,-1);
%%backgroundimage:images/lam_hah.jpg,-2275,-1322,1,0,0,1
%%beginparams
lefttatweel := 1.14; % numeric
z1 = (120.521,22.7438);
z3 = (99.2837,288.729);
z4 = (312.09,276.392);
z5 = (643.65,229.071);

%%beginverbatim

if left_verticalextratio = 0 : left_verticalextratio := 12; fi
 
lefttatweel := charlt + 0;
z2 = z1 + (0,lineheight) rotated -15;

%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension 1 and 7.16575 ..
 {up}(340.455,185.141) .. tension 10.8298 and 1.79081 ..
(229.926,211.323) .. tension 0.815564 and 3.20171 ..
 {left}(113.343,219.514) .. tension 0.75 and 6.35213 ..
 {left}(59.8623,197.764) .. tension 1.39544 and 1 ..
 {up}(53.7016,206.034) .. tension 1.74599 ..
z3 .. tension 2.3758 and 6.46414 ..
 {up}(296.226,318.129) .. tension 2.44495 and 2.44495 ..
(249.945,572.217) .. tension 1 and 7.35794 ..
(222.285,759.25) .. tension 1.66263 and 0.966451 ..
 {up}(194.557,803.303) .. tension 1.88324 and 7.2459 ..
 {right}(240.29,881.804) .. tension 22.6369 and 52.9346 ..
 {down}(301.251,788.925) .. tension 1.17354 and 4.55519 ..
 {down}(266.637,731.489) .. tension 4.87812 and 3.22481 ..
(321.065,451.85) .. tension 1.33352 and 3.40307 ..
 {down}(326.718,330.69) .. tension 2.33188 and 3.30683 ..
 {down}z4 .. tension 7.15165 and 1.05411 ..
(571.671,232.754) .. tension 1 and 1.14446 ..
 {right}(638.508,235.235) .. tension 2.71262 and 0.75 ..
 {down}z5 .. tension 7.62288 ..
 {left}(589.319,146.203) .. tension 2.71262 and 0.75 ..
 {left}(479.209,163.152) .. tension 1.01367 and 0.75 ..
(294.472,95.6429) .. tension 1 and 1 ..
 cycle
;

%%endpaths
enddefchar;
beginchar(space,32,500,0,0);
%%beginbody
% glyphname:space
%%begincomponents
%%endcomponents
endchar;
beginchar(space.ii,-1,-1,-1,-1);
%%beginbody
charwd := 0;
endchar;
beginchar(rubelhizb,1758,-1,-1,-1);
%%backgroundimage:images/hizb.jpg,-925.291,-1293.96,1,0,0,1
%%beginparams
z1 = (277.778,265.278);
%%beginbody

displacx := 30;
displacy := 90;
dir_ := 130;

z2 = z1 + (-displacx,displacy);
z3 = z1 + (displacx,displacy);

z4 = z1 + (0,1.5*displacy);


x5 := x1;
y5 := y1-65;

picture flower;
flower := image(
fill
z1 {dir dir_} ..
 {up}z2 .. tension 1 and 0.9 .. 
 {right}z4 .. tension 0.9 and 1 ..
 {down}z3.. 
 {dir (-dir_)} cycle
;
);

draw flower;

for i=1 upto 6 :
  draw flower rotatedaround (z5,i*51.42);
endfor

currentpicture :=  currentpicture rotatedaround (z5,25);

fill fullcircle scaled 65 shifted z5;

leftbearing :=0;
rightbearing :=100;



endchar;
beginchar(placeofsajdah,1769,-1,-1,-1);
%%backgroundimage:images/sajda.jpg,-1086.5,-1089.55,1,0,0,1

%%beginpaths

fill
(216.976,57.5783) .. tension 1 and 1 ..
 {up}(239.326,92.239) .. tension 1 and 1 ..
 {up}(231.626,330.504) .. tension 10.3117 and 0.75 ..
 {up}(148.276,406.314) .. tension 1.03636 and 1.34025 ..
(307.999,562.339) .. tension 1 and 5.21582 ..
 {right}(335.697,626.688) .. tension 7.853 and 1 ..
(372.178,563.172) .. tension 1.23371 and 1.00181 ..
 {down}(539.931,417.245) .. tension 0.762728 and 6.87108 ..
 {down}(471.644,347.801) .. tension 1 and 1.34752 ..
 {down}(481.288,135.513) .. tension 0.916134 and 2.79655 ..
 {down}(521.656,81.2328) .. tension 12.0083 and 5.3958 ..
(365.1,62.441) .. tension 5.4814 and 11.064 ..
 {up} cycle
;
fill
(338.381,571.47) .. tension 7.79178 and 1 ..
(301.33,533.158) .. tension 1 and 1 ..
 {down}(167.583,403.485) .. tension 0.777617 and 3.94323 ..
 {down}(247.221,339.662) .. tension 1 and 1 ..
 {down}(256.03,116.042) .. tension 1 and 1.41754 ..
 {down}(251.419,71.0595) .. tension 13.6448 and 2.41969 ..
(369.571,74.3808) .. tension 2.18139 and 10.3562 ..
 {up}(481.315,90.3108) .. tension 0.75 and 1 ..
 {up}(463.543,131.637) .. tension 2.66531 and 1 ..
 {up}(453.64,353.733) .. tension 3.40012 and 0.851637 ..
 {up}(517.216,410.317) .. tension 1 and 1.00683 ..
(375.719,536.485) .. tension 1 and 3.10649 ..
 {left} cycle
;
fill
(220.441,402.636) .. tension 1.86077 and 1 ..
(287.454,462.159) .. tension 1 and 2.92388 ..
 {right}(339.698,505.529) .. tension 2.39921 and 1 ..
(394.901,465.787) .. tension 1 and 1.7426 ..
 {down}(469.415,413.041) .. tension 0.978867 and 2.43577 ..
 {down}(417.361,370.486) .. tension 1 and 1 ..
 {down}(432.577,141.494) .. tension 3.59482 and 0.933838 ..
(358.91,126.914) .. tension 1 and 6.52764 ..
 {up}(292.399,127.183) .. tension 1 and 1 ..
 {up}(278.803,362.248) .. tension 3.25519 and 1.12185 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing:=0;
rightbearing:=100;
endchar;
beginchar(endofaya,1757,1,-1,-1);
%%backgroundimage:images/aya.jpg,-368.206,-1170.09,1,0,0,1
%%beginparams
z1 = (516.686,563.695);
z2 = (959.421,241.222);
z3 = (925.62,247.222);
z4 = (515.6,832.751);
z5 = (523.167,854.762);
z6 = (529.585,691.277);
z7 = (526.377,712.656);
z8 = (546.464,-281.019);
z9 = (545.667,-259.973);

%%beginverbatim

y3 := y2;
x4 := x1 + 8;
x5 := x4;
x9 := x8 := x6 := x7 := x5;

color bluecolor[];

	
%bluecolor1 := (136/255,199/255,230/255);
bluecolor1 := (221/255,231/255,241/255); %DDE7F1
bluecolor2 := (73/255,190/255,233/255); %49BEE9
%bluecolor3 := (127/255,205/255,241/255);
bluecolor3 := (220/255,240/255,255/255); % DCF0FF



%%endverbatim

%%beginpaths

controlledPath (13,0)(_outer)(
z1 .. tension 0.962423 and 0.773801 ..
 {right}(659.838,666.917) .. tension 1.4109 and 0.927449 ..
 {down}z2 .. tension 1.7294 and 1.42232 ..
 {up}z3 .. tension 1.14593 and 1 ..
(764.586,467.229) .. tension 1 and 5.3754 ..
 {up}(659.945,499.558) .. tension 1 and 9.93853 ..
 {up}(706.25,563.889) .. tension 4.21269 and 1.80814 ..
 {left}(649.036,582.547) .. tension 1 and 1 ..
 {down}(642.463,577.871) .. tension 2.6966 and 1 ..
 {down}(652.083,549.306) .. tension 1 and 1 ..
 {left}(585.423,483.883) .. tension 0.918294 and 1 ..
 {up} cycle
);
controlledPath (14,0)(_inner)(
(536.214,569.034) .. tension 1 and 1 ..
 {right}(586.264,504.791) .. tension 1.51708 and 1 ..
 {up}(628.376,545.428) .. tension 1 and 1 ..
 {left}(607.362,568.704) .. tension 1 and 2.02801 ..
 {left}(585.81,534.642) .. tension 1.59049 and 1 ..
(578.222,576.293) .. tension 1 and 1 ..
 {right}(623.071,599.922) .. tension 0.923118 and 2.54885 ..
 {down}(829.754,497.255) .. tension 1 and 2.16566 ..
 {left}(823.527,483.948) .. tension 3.75177 and 0.923179 ..
 {left}(741.223,541.088) .. tension 1 and 2.89856 ..
 {down}(700.459,517.333) .. tension 3.85631 and 0.75 ..
(799.011,479.183) .. tension 1.08099 and 1 ..
(903.666,377.039) .. tension 1 and 6.32556 ..
 {right}(914.147,361.056) .. tension 1 and 1 ..
 {up}(917.728,366.995) .. tension 2.72107 and 0.936863 ..
 {left}(644.29,647.184) .. tension 0.918795 and 1.44182 ..
 {down} cycle
);
controlledPath (15,0)(_flower)(
z5 .. tension 1 and 1 ..
(556.6,840.929) .. tension 1 and 2.2342 ..
(580.457,801.724) .. tension 1.86343 and 1 ..
(630.948,787.01) .. tension 1 and 1.72823 ..
(673.005,736.023) .. tension 1.6601 and 0.75 ..
(710.654,705.63) .. tension 1 and 1 ..
 {down}(726.128,642.386) .. tension 1 and 1 ..
(704.142,653.571) .. tension 1 and 1.08751 ..
 {left}(649.306,723.04) .. tension 1 and 2.53645 ..
 {left}(590.278,706.944) .. tension 1 and 1 ..
 {up}(572.917,714.583) .. tension 1.39584 and 3.46967 ..
 {up}(636.806,747.917) .. tension 1 and 1 ..
 {left}(587.964,772.247) .. tension 1 and 4.15927 ..
 {left}(537.5,753.472) .. tension 1 and 1 ..
 {up}(528.472,759.722) .. tension 1 and 1 ..
 {up}(554.861,798.611) .. tension 1 and 1 ..
 {left}z4 .. tension 1 and 1 ..
 {right} cycle
);
controlledPath (16,0)(_circle)(
z7 .. tension 1.20827 and 1.2275 ..
 {down}(593.171,650.559) .. tension 1.6671 and 1.30051 ..
 {up}(570.988,642.361) .. tension 1 and 1 ..
 {left}z6 .. tension 1 and 1 ..
 {right} cycle
);
controlledPath (17,0)(_bottom)(
z9 .. tension 1.20827 and 1 ..
(564.303,-248.494) .. tension 1 and 1.2275 ..
 {right}(581.597,-230.518) .. tension 1.6671 and 1 ..
 {right}(626.152,-236.076) .. tension 1 and 1 ..
(665.027,-207.369) .. tension 1 and 1 ..
(718.557,-183.738) .. tension 1 and 1 ..
 {up}(720.968,-159.626) .. tension 3.23302 and 2.76018 ..
 {down}(755.061,-145.319) .. tension 1 and 1 ..
(699.173,-218.796) .. tension 1 and 1 ..
(679.575,-224.663) .. tension 1 and 1.30051 ..
 {left}(638.879,-252.808) .. tension 1 and 1 ..
 {left}(605.241,-254.174) .. tension 1 and 1 ..
(571.671,-267.918) .. tension 1 and 1 ..
 {left}z8 .. tension 1 and 1 ..
 {right} cycle
);

%%endpaths
%%beginbody


path _paths[];

_paths0 := _outer;
_paths1 := _inner;
_paths2 := _flower;
_paths3 := _circle;
_paths4 := _bottom;

_paths0.1 := _paths0 reflectedabout (z2,z3);
_paths0.2 := _paths0  reflectedabout (z2,z3) reflectedabout (z4, z5);
_paths0.3 := _paths0 reflectedabout (z4, z5);


fill _paths0.1;
fill _paths1 reflectedabout (z2,z3) withcolor white;
fill _paths0.2;
fill _paths1 reflectedabout (z2,z3) reflectedabout (z4, z5) withcolor white;
fill _paths0.3;
fill _paths1 reflectedabout (z4, z5) withcolor white;


numeric x_[];
numeric y_[];
(x_1,y_1)=reverse _paths2  intersectiontimes _paths0;
(x_2,y_2)=_paths0 intersectiontimes _paths0 reflectedabout (z4, z5);
(x_3,y_3)=reverse (_paths2  reflectedabout (z4, z5)) intersectiontimes _paths0 reflectedabout (z4, z5);
(x_4,y_4)=reverse (_paths2  reflectedabout (z4, z5)) intersectiontimes reverse _paths2;

fill subpath (0,x_1) of reverse _paths2 .. subpath(y_1,x_2) of _paths0 
..  subpath(y_2,y_3) of _paths0 reflectedabout (z4, z5)  
..  subpath(x_3,x_4) of reverse (_paths2  reflectedabout (z4, z5)) 
.. cycle withcolor bluecolor3;

_paths8 := _paths2;
_paths2 := _paths3;

numeric x_[];
numeric y_[];
(x_1,y_1)=reverse _paths2  intersectiontimes _paths0;
(x_2,y_2)=_paths0 intersectiontimes _paths0 reflectedabout (z4, z5);
(x_3,y_3)=reverse (_paths2  reflectedabout (z4, z5)) intersectiontimes _paths0 reflectedabout (z4, z5);
(x_4,y_4)=reverse (_paths2  reflectedabout (z4, z5)) intersectiontimes reverse _paths2;

fill subpath (0,x_1) of reverse _paths2 .. subpath(y_1,x_2) of _paths0 
..  subpath(y_2,y_3) of _paths0 reflectedabout (z4, z5)  
..  subpath(x_3,x_4) of reverse (_paths2  reflectedabout (z4, z5)) 
.. cycle withcolor bluecolor2;

_paths2 := _paths8;

(x_5,y_5)= _paths4  intersectiontimes _paths0.1;
(x_6,y_6)=_paths0.1 intersectiontimes _paths0.2;
(x_7,y_7)=_paths0.2 intersectiontimes _paths4  reflectedabout (z4, z5);
(x_8,y_8)=(_paths4  reflectedabout (z4, z5)) intersectiontimes _paths4;


fill subpath (0,x_5) of  _paths4 .. subpath(y_5,x_6) of _paths0.1 
..  subpath(y_6 ,x_7) of _paths0.2
..  subpath(y_7,x_8) of _paths4  reflectedabout (z4, z5)
.. cycle withcolor bluecolor3;

%fill inside

(x_9,y_9)= reverse  _paths0  intersectiontimes reverse _paths0.1;
(x_10,y_10)= reverse  _paths0.1  intersectiontimes reverse _paths0.2;
(x_11,y_11)= reverse  _paths0.2  intersectiontimes reverse _paths0.3;
(x_12,y_12)= reverse  _paths0.3  intersectiontimes reverse _paths0;

fill subpath (y_9,x_10) of   reverse _paths0.1 
.. subpath(y_10,x_11) of  reverse _paths0.2
..  subpath(y_11 ,x_12) of reverse _paths0.3
..  subpath(y_12,x_9) of  reverse  _paths0
.. cycle withcolor bluecolor3;

fill _paths2 reflectedabout (z4, z5);
fill _paths3 reflectedabout (z4, z5);
fill _paths4 reflectedabout (z4, z5);


fill _paths0;
fill _paths1 withcolor white;
fill _paths2;
fill _paths3;
fill _paths4;

currentpicture := currentpicture shifted(-xpart (llcorner currentpicture),-ypart (llcorner currentpicture) );	
	
numeric _depth, _margin; _depth := -250;
leftbearing := 50;
rightbearing := 0;
	
currentpicture := currentpicture shifted (leftbearing,_depth);
	
charwd := xpart (lrcorner currentpicture) - xpart (llcorner currentpicture) + leftbearing + rightbearing;




endchar;
beginchar(one_dot,-1,-1,-1,-1);
%%backgroundimage:images/dots.jpg,-125.732,-297.828,1,0,0,1
%%beginparams
z1 = (0,0);

%%beginverbatim

_offset := 1/10;
_tension :=0.75;
interim leftbearing := 0;
interim rightbearing := 0;

z2 = z1 + (-penwidth,0 );
z3 = z2 + (0,penwidth );
z4 = z1 + (0,penwidth );

z11 = _offset[z1,z2];
z12 = _offset[z1,z4];

z21 = _offset[z2,z1];
z22 = _offset[z2,z3];

z31 = _offset[z3,z2];
z32 = _offset[z3,z4];

z41 = _offset[z4,z3];
z42 = _offset[z4,z1];


%%endverbatim

%%beginpaths

fill
z11 --
z21 {left} .. tension _tension ..
 {up}z22 --
z31 {up} .. tension _tension ..
 {right}z32 --
z41 {right} .. tension _tension ..
 {down}z42 --
z12 {down} .. tension _tension ..
 {left} cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture rotated -30;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
savepicture:=1;

endchar;
beginchar(two_dots,-1,-1,-1,-1);
%%backgroundimage:images/dots.jpg,-125.732,-297.828,1,0,0,1
%%beginparams
z1 = (106,25);
%%beginbody
interim leftbearing := 0;
interim rightbearing := 0;
drawcomp(one_dot,identity);
drawcomp(one_dot,identity shifted z1);
savepicture:=1;

endchar;
beginchar(thankyou,-1,1,-1,-1);
%%backgroundimage:images/thankyou.bmp,-217.731,-387.955,1,0,0,1

%%beginpaths

fill
(-29.0245,24.8383) .. tension 0.75 and 1 ..
(-24.5634,44.171) .. tension 1 and 1 ..
 {up}(-20.6088,59.3131) .. tension 1 and 1 ..
(-22.796,95.154) .. tension 0.800172 and 2.34588 ..
 {up}(-25.0849,123.328) .. tension 1.35674 and 0.75 ..
 {up}(-25.615,128.052) .. tension 1.67556 and 2.61612 ..
 {right}(-23.195,135.786) .. tension 3.47901 and 1 ..
(-18.7387,124.884) .. tension 1 and 2.2138 ..
 {down}(-14.8367,115.676) .. tension 1 and 1.2544 ..
 {left}(-15.6286,113.333) .. tension 1.14311 and 1.21907 ..
 {left}(-18.5622,115.747) .. tension 11.5584 and 1 ..
(-16.7112,84.9132) .. tension 1 and 1 ..
 {down}(-15.5286,73.9192) .. tension 0.798036 and 1.50375 ..
 {down}(-26.3406,26.9014) .. tension 1 and 0.794331 ..
 {right}(-18.9776,18.6985) .. tension 1 and 4.73142 ..
 {up}(-2.38306,31.2571) .. tension 4.53359 and 0.75 ..
 {up}(-4.87567,56.1011) .. tension 1.15175 and 1 ..
(5.75118,73.9071) .. tension 1 and 13.444 ..
 {up}(14.7913,82.3291) .. tension 1.90696 and 1 ..
 {up}(2.7288,96.5158) .. tension 1 and 1.2476 ..
(24.7917,119.54) .. tension 1 and 1 ..
 {right}(65.8633,134.238) .. tension 1 and 1 ..
 {down}(68.5195,129.215) .. tension 0.990918 and 3.04354 ..
 {down}(55.5873,107.669) .. tension 1 and 0.864831 ..
 {left}(54.5482,105.989) .. tension 1.0764 and 1 ..
 {up}(53.8231,108.034) .. tension 1.49994 and 1.26126 ..
 {up}(64.3135,125.433) .. tension 1 and 1 ..
 {left}(61.3721,126.109) .. tension 1.32518 and 16.8274 ..
 {down}(27.7339,113.596) .. tension 31.6087 and 1.33455 ..
 {right}(66.7911,89.3098) .. tension 0.918425 and 1.0613 ..
 {dir 93.6879}(75.4814,113.727) .. tension 1.15107 and 1 ..
 {up}(73.1768,147.016) .. tension 1 and 1 ..
 {up}(71.9509,150.48) .. tension 3.54826 and 2.91302 ..
 {right}(75.133,158.518) .. tension 7.05297 and 15.3822 ..
 {down}(83.3933,138.696) .. tension 0.75 and 1 ..
 {left}(82.7777,136.025) .. tension 1.59829 and 1.44478 ..
 {left}(79.147,140.042) .. tension 15.4518 and 1 ..
 {down}(80.2633,101.405) .. tension 1.1287 and 1 ..
 {left}(64.5147,80.3718) .. tension 1.18726 and 5.57748 ..
 {left}(32.1544,101.529) .. tension 3.35192 and 1 ..
 {down}(34.2078,93.6508) .. tension 1 and 1 ..
(29.7298,81.1652) .. tension 1.11285 and 4.86753 ..
 {down}(12.0031,70.8267) .. tension 3.14532 and 0.75 ..
 {down}(19.0923,58.0463) .. tension 1 and 1.83661 ..
 {left}(16.9118,51.7944) .. tension 0.803224 and 4.09756 ..
 {left}(3.06728,62.9003) .. tension 3.31311 and 1.85479 ..
 {down}(-2.45435,55.5868) .. tension 1 and 1.19988 ..
 {down}(4.7229,14.815) .. tension 1 and 4.08789 ..
 {left}(3.27191,3.36686) .. tension 4.88533 and 1.41771 ..
 {up}(1.11371,13.3855) .. tension 1 and 3.45852 ..
 {left}(-0.667703,22.9368) .. tension 5.21376 and 0.930119 ..
 {left}(-17.6422,10.9363) .. tension 1.57284 and 1 ..
 {up} cycle
;
fill
(30.9552,90.959) .. tension 2.49892 and 3.20826 ..
 {left}(17.6086,106.997) .. tension 2.00224 and 4.53346 ..
 {down}(5.38352,99.3708) .. tension 11.5727 and 7.77885 ..
 {right}(27.1419,87.5943) .. tension 1 and 1.44557 ..
 {up} cycle
;
fill
(8.07012,65.0519) .. tension 1.39668 and 2.47949 ..
 {right}(13.4255,59.6363) .. tension 0.752457 and 1.35725 ..
 {up}(14.1816,60.6814) .. tension 2.60653 and 1.40016 ..
 {left}(8.84063,66.1757) .. tension 0.75 and 1.64627 ..
 {down} cycle
;
fill
(3.10091,36.6295) .. tension 4.69626 and 1 ..
(21.1088,47.7141) .. tension 1 and 2.679 ..
 {up}(39.9242,58.3358) .. tension 1.29966 and 6.30318 ..
 {up}(26.7023,74.7771) .. tension 4.80656 and 12.5184 ..
 {right}(34.3953,81.9479) .. tension 3.92175 and 1 ..
(51.3588,69.3828) .. tension 0.805619 and 1 ..
 {right}(90.7015,56.5323) .. tension 1 and 1.49246 ..
(143.056,66.2734) .. tension 1 and 17.2242 ..
 {right}(160.489,75.1525) .. tension 0.75 and 1.04291 ..
 {down}(161.026,73.7474) .. tension 1 and 1 ..
 {down}(160.208,70.158) .. tension 1 and 1 ..
 {right}(167.043,65.8627) .. tension 0.75 and 3.37529 ..
 {right}(183.454,80.0422) .. tension 3.04811 and 0.75 ..
 {down}(183.886,78.1627) .. tension 1.68302 and 1 ..
 {down}(183.269,73.7322) .. tension 1 and 1.22069 ..
 {right}(187.839,69.189) .. tension 1 and 1.07342 ..
 {up}(191.789,76.9409) .. tension 0.916797 and 4.85223 ..
 {right}(193.612,83.0652) .. tension 5.37899 and 0.843608 ..
 {down}(196.333,74.9234) .. tension 1 and 1.38813 ..
 {left}(187.799,59.8863) .. tension 0.75 and 4.23539 ..
 {left}(178.879,64.6187) .. tension 3.5434 and 0.804782 ..
 {left}(165.834,56.5754) .. tension 1 and 1 ..
 {up}(156.402,64.9252) .. tension 1 and 1 ..
 {left}(155.433,65.7005) .. tension 20.4699 and 1 ..
(137.366,56.5671) .. tension 1 and 0.933592 ..
 {left}(92.8872,46.9012) .. tension 1.12255 and 1 ..
(60.2954,53.7006) .. tension 1 and 8.61251 ..
 {left}(41.1624,67.9777) .. tension 2.96287 and 1 ..
 {down}(43.0251,59.111) .. tension 0.75 and 1 ..
(39.0305,48.4517) .. tension 1 and 1.29304 ..
(9.28657,32.8395) .. tension 1 and 3.40841 ..
 {left}(5.18971,28.7103) .. tension 2.08418 and 1 ..
 {up} cycle
;
fill
(-6.74447,128.378) .. tension 1 and 1 ..
(-4.14961,138.488) .. tension 2.17516 and 1.0506 ..
(55.3512,170.705) .. tension 1 and 1 ..
 {right}(63.7236,172.333) .. tension 1 and 1 ..
 {down}(68.2214,167.215) .. tension 1.2634 and 1 ..
(59.7711,150.857) .. tension 1 and 1.26659 ..
(51.5566,140.706) .. tension 1 and 1.45565 ..
 {left}(50.3148,138.77) .. tension 2.17838 and 1 ..
 {up}(50.0087,140.006) .. tension 1.27907 and 1.65607 ..
 {up}(63.8015,162.023) .. tension 1 and 1 ..
 {left}(61.7863,163.801) .. tension 2.9948 and 1 ..
(26.2802,150.239) .. tension 1 and 7.71694 ..
 {down}(-3.68448,132.258) .. tension 4.50797 and 1.36348 ..
 {dir -53.8686}(17.0754,119.54) {dir -126.455} .. tension 0.837224 and 1.00181 ..
 {dir -164.107}(9.07015,113.726) {dir 144.733} .. tension 2.56666 and 2.18021 ..
 {up} cycle
;
fill
(-31.9123,144.189) .. tension 4.36968 and 1 ..
(-21.9877,151.737) .. tension 1 and 8.68214 ..
 {right}(-11.3486,157.243) .. tension 1 and 1 ..
 {down}(-9.91319,156.39) .. tension 2.34875 and 1 ..
(-20.3842,149.329) .. tension 1 and 4.84449 ..
 {left}(-30.6861,143.605) .. tension 0.75 and 0.75 ..
 {up} cycle
;
fill
(-29.4748,139.299) .. tension 2.90628 and 1 ..
(-19.6005,146.51) .. tension 1 and 7.043 ..
 {right}(-7.89499,152.308) .. tension 1.04805 and 1 ..
 {down}(-6.90906,151.052) .. tension 4.20971 and 1 ..
(-17.8299,144.037) .. tension 1 and 2.91872 ..
 {left}(-27.6801,138.76) .. tension 1 and 3.86051 ..
 {up} cycle
;
fill
(-10.6051,87.6316) .. tension 0.793236 and 10.0858 ..
 {right}(-4.08444,110.871) .. tension 1 and 1 ..
 {down}(-3.73549,109.625) .. tension 4.14999 and 1 ..
 {down}(-7.65354,91.7266) .. tension 1 and 1.17852 ..
 {right}(-5.60304,85.7746) .. tension 1 and 4.63507 ..
 {right}(-1.61552,90.4225) .. tension 1.2066 and 1 ..
 {down}(-0.853399,88.8017) .. tension 1.21713 and 1.82099 ..
 {left}(-5.58163,80.6546) .. tension 1.41139 and 1 ..
 {up} cycle
;
fill
(15.5041,128.453) .. tension 2.52352 and 1 ..
(29.1533,140.471) .. tension 1 and 5.04451 ..
 {right}(45.4319,148.608) .. tension 4.89241 and 2.2041 ..
 {dir -117.462}(43.0899,142.071) .. tension 5.52245 and 1 ..
(30.3084,135.584) .. tension 1 and 6.21202 ..
 {up} cycle
;
fill
(61.2189,101.54) .. tension 0.75 and 2.59004 ..
(64.2548,107.912) .. tension 1 and 1.87365 ..
 {right}(64.8155,110.242) .. tension 2.03421 and 1 ..
(65.8887,107.729) .. tension 2.0088 and 1 ..
(68.917,105.336) .. tension 1 and 1.11903 ..
 {down}(70.3332,102.137) .. tension 1 and 1 ..
(69.3227,99.071) .. tension 1 and 1.21221 ..
 {left}(65.6567,97.7467) .. tension 0.97714 and 1 ..
(62.8321,97.854) .. tension 0.808499 and 1.20076 ..
 {up} cycle
;
fill
(68.5808,101.182) .. tension 2.72753 and 1.50063 ..
 {left}(65.1308,105.923) .. tension 3.14963 and 2.39379 ..
(63.6066,100.735) .. tension 3.05792 and 3.35182 ..
 {up} cycle
;
fill
(93.8099,108.879) .. tension 1 and 0.75 ..
 {right}(96.7398,113.361) .. tension 1.39964 and 6.45453 ..
 {right}(106.997,99.9316) .. tension 8.10645 and 1.47237 ..
 {right}(113.851,117.359) .. tension 1 and 1 ..
 {down}(118.347,112.31) .. tension 1.48774 and 2.07472 ..
 {left}(116.979,108.68) .. tension 2.36793 and 1 ..
 {left}(113.503,111.932) .. tension 2.05941 and 5.49305 ..
 {left}(107.895,91.6319) .. tension 2.8585 and 1 ..
(104.754,97.5763) .. tension 1 and 4.73364 ..
 {left}(101.194,104.96) .. tension 1 and 1 ..
 {left}(99.4115,104.201) .. tension 1 and 1 ..
(97.352,105.091) .. tension 1 and 1.86719 ..
 {up} cycle
;
fill
(156.286,97.291) .. tension 4.43433 and 3.93621 ..
 {right}(160.823,103.801) .. tension 3.49008 and 3.89739 ..
 {right}(168.149,99.3069) .. tension 3.34205 and 2.77496 ..
 {right}(171.946,104.656) .. tension 4.5875 and 9.2288 ..
 {down}(179.541,98.8396) .. tension 2.52353 and 5.93918 ..
 {left}(175.858,91.0891) .. tension 3.00606 and 4.88325 ..
 {left}(168.612,96.7095) .. tension 5.09435 and 5.05242 ..
 {left}(164.348,90.7015) .. tension 4.12648 and 3.34049 ..
 {up} cycle
;
fill
(159.388,109.695) .. tension 4.96231 and 2.83802 ..
 {right}(163.382,116.903) .. tension 4.8251 and 5.84756 ..
 {down}(171.287,111.477) .. tension 4.07771 and 4.48799 ..
 {left}(167.409,103.844) .. tension 4.26059 and 6.17298 ..
 {up} cycle
;
fill
(93.4265,69.6493) .. tension 8.2984 and 4.9378 ..
 {up}(140.42,103.521) .. tension 1.45895 and 1.33604 ..
 {up}(132.233,109.913) .. tension 1.06696 and 2.02793 ..
 {right}(143.448,124.157) .. tension 1 and 1 ..
 {down}(147.486,117.338) .. tension 0.75 and 1 ..
 {down}(145.467,109.353) .. tension 1.7043 and 2.73843 ..
 {down}(149.57,106.585) .. tension 1 and 4.10868 ..
 {left}(147.037,101.95) .. tension 1 and 1 ..
 {left}(143.336,103.185) .. tension 24.2624 and 1 ..
(140.432,99.2165) .. tension 1.07305 and 11.4931 ..
 {left}(94.4518,69.3088) .. tension 1 and 10.726 ..
 {up} cycle
;
fill
(136.551,113.727) .. tension 1.63801 and 2.46329 ..
 {right}(141.856,109.555) .. tension 0.75 and 1.83567 ..
 {up}(144.682,112.352) .. tension 1.81544 and 1.76186 ..
 {left}(141.452,118.168) .. tension 2.47493 and 1 ..
 {down} cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture shifted (-10,-50) scaled 7;
endchar;
defchar(minikaf,-1,-1,-1,-1);
%%backgroundimage:images/kkaf.fina.jpg,-449.167,-956.89,1,0,0,1
%%beginparams
z1 = (683.006,10.9988);
z3 = (721.065,-12.7315);

%%beginverbatim
righttatweel := charrt +0.5;
z2 = z1 + (0,lineheight ) rotated -10;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);


%%endverbatim

%%beginpaths

fill
(160.916,217.129) .. tension 1.6784 and 1.33888 ..
 {right}(183.742,226.573) .. tension 4.85985 and 0.75 ..
 {right}(259.007,219.235) .. tension 1.49002 and 2.49931 ..
 {up}(393.783,287.258) .. tension 2.06098 and 0.961189 ..
 {up}(279.936,345.341) .. tension 1 and 2.02954 ..
 {right}(393.437,532.182) .. tension 1 and 2.20322 ..
 {down}(427.951,501.604) .. tension 2.69063 and 4.2656 ..
 {left}(391.256,436.468) .. tension 0.85293 and 1 ..
 {left}(357.959,459.064) .. tension 2.46845 and 1.6639 ..
 {down}(322.999,411.692) .. tension 1.23183 and 1.17831 ..
 {down}(435.608,349.199) .. tension 1.16642 and 0.75 ..
 {left}(266.976,179.751) .. tension 1 and 2.03546 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing:=0;
rightbearing:=0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
savepicture:=1;
enddefchar;
beginchar(three_dots,-1,-1,-1,-1);
%%backgroundimage:images/dots.jpg,-125.732,-297.828,1,0,0,1
%%beginparams
z2 = (25.0415,133.882);
%%beginbody
interim leftbearing := 0;
interim rightbearing := 0;
drawcomp(two_dots,identity);
drawcomp(one_dot,identity shifted z2);


endchar;
beginchar(onedotup,-1,-1,-1,-1);
%%backgroundimage:images/dots.jpg,-125.732,-297.828,1,0,0,1
%%beginparams
z1 = (106,25);
%%beginbody
interim leftbearing := 0;
interim rightbearing := 0;
drawcomp(one_dot,identity);
endchar;
beginchar(onedotdown,-1,-1,-1,-1);
%%backgroundimage:images/dots.jpg,-125.732,-297.828,1,0,0,1
%%beginparams
z1 = (106,25);
%%beginbody
interim leftbearing := 0;
interim rightbearing := 0;
drawcomp(one_dot,identity);
endchar;
beginchar(twodotsup,-1,-1,-1,-1);
%%backgroundimage:images/dots.jpg,-125.732,-297.828,1,0,0,1
%%beginparams
z1 = (106,25);
%%beginbody
interim leftbearing := 0;
interim rightbearing := 0;
drawcomp(two_dots,identity);
endchar;
beginchar(twodotsdown,-1,-1,-1,-1);
%%backgroundimage:images/dots.jpg,-125.732,-297.828,1,0,0,1
%%beginparams
z1 = (106,25);
%%beginbody
interim leftbearing := 0;
interim rightbearing := 0;
drawcomp(two_dots,identity);
endchar;
defchar(fatha,1614,-1,-1,-1);
%%backgroundimage:images/shadda.jpg,-1598.98,-850.892,1,0,0,1
%%beginparams
z1 = (0,0);

%%beginverbatim



_offset1 := 1/9;
_offset2 := 1/40;
_tension :=1;
_height := penwidth / 1.8;
_angle1 := 25;
_angle2 := _angle1;

numeric lefttatweel_;

lefttatweel_ := lefttatweel*nuqta + 2.3*nuqta;

z2 = z1 + (-lefttatweel_,0 );
z3 = z2 + (0,_height ) rotated -_angle2;
z4 = z1 + (0,_height ) rotated -_angle2;

z11 = _offset2[z1,z2];
z12 = _offset1[z1,z4];

z21 = _offset2[z2,z1];
z22 = _offset1[z2,z3];

z31 = _offset1[z3,z2];
z32 =_offset2[z3,z4];

z41 = _offset2[z4,z3];
z42 = _offset1[z4,z1];


%%endverbatim

%%beginpaths

fill
z11 --
z21 {left} .. tension _tension ..
 {up}z22 --
z31 {up} .. tension _tension ..
 {right}z32 --
z41 {right} .. tension _tension ..
 {down}z42 --
z12 {down} .. tension _tension ..
 {left} cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture rotated _angle1;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

%labels(1,2,3,4);

savepicture:=1;
enddefchar;
beginchar(kasra,1616,-1,-1,-1);
%%beginbody

drawcomp(fatha,identity);
leftbearing := 0;
rightbearing := 0;
endchar;
beginchar(fathatan,1611,-1,-1,-1);
%%backgroundimage:images/kastratan.jpg,-388,-876,1,0,0,1
%%beginbody


picture _temppic;
_temppic := image(
fatha_(0,0);
);

draw _temppic;

draw _temppic shifted (0,100);


leftbearing := 0;
rightbearing := 0;
savepicture:=1;
endchar;
beginchar(kasratan,1613,-1,-1,-1);
%%backgroundimage:images/kastratan.jpg,-388,-876,1,0,0,1
%%beginbody

leftbearing := rightbearing := 0;
drawcomp(fathatan,identity);



endchar;
defchar(fathatanidgham,2288,-1,-1,-1);
%%backgroundimage:images/kastratanidgham.jpg,-2861,-1448,1,0,0,1
%%beginbody

lefttatweel := lefttatweel - 0.5;
picture _temppic;
_temppic := image(
fatha_(lefttatweel,righttatweel);
);

draw _temppic;

z.s = ulcorner _temppic;

draw _temppic shifted (-x.s + x.s*0.3, y.s + 20);

show z.s;


enddefchar;
defchar(kasratanidgham,2290,-1,-1,-1);
%%beginbody
fathatanidgham_(lefttatweel,righttatweel);

enddefchar;
beginchar(damma,1615,-1,-1,-1);
%%backgroundimage:images/damma.jpg,-181.157,-471.404,1,0,0,1

%%beginpaths

fill
(12.0563,129.243) {up} .. tension 5.42049 and 1 ..
(152.874,237.751) .. tension 1 and 1.25778 ..
 {curl1}(186.632,276.331) .. tension 2.6829 and 1 ..
 {up}(106.577,344.811) .. tension 1.25258 and 2.0234 ..
 {right}(201.488,451.241) .. tension 1 and 1 ..
 {down}(243.056,372.299) .. tension 1 and 1 ..
(199.17,227.142) .. tension 1.00116 and 3.46588 ..
 {left}(25.5594,125.386) .. tension 1 and 1 ..
 cycle
;
fill
(144.341,355.662) {dir -36.1193} .. tension 1.24283 and 1.22257 ..
 {dir -21.6977}(211.991,321.167) {curl1} .. tension 2.31758 and 1.16977 ..
 {left}(187.543,396.854) .. tension atleast 1.33323 ..
 {curl1} cycle
;

%%endpaths
%%beginbody
leftbearing := rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
savepicture:=1;
endchar;
beginchar(dammatanidgham,2289,477,327,-2);
%%backgroundimage:images/dammatanidgham.jpg,-628,-606,1,0,0,1
%%beginbody

drawcomp(damma,identity scaled 0.9 rotated 5);
drawcomp(damma,identity shifted (134,-10));

leftbearing := 0;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);

endchar;
beginchar(dammatan,1612,-1,-1,-1);
%%backgroundimage:images/dammatanii.jpg,-438,-893,1,0,0,1

%%beginpaths

fill
(-4.62422,105.182) {up} .. tension 5.42049 and 1 ..
(127.281,199.235) .. tension 1 and 1.46163 ..
 {curl1}(174.399,250.953) .. tension 4.07696 and 1 ..
 {up}(119.116,310.571) .. tension 1.25258 and 2.0234 ..
 {right}(195.236,407.271) .. tension 1.34919 and 1 ..
 {down}(224.662,340.091) .. tension 0.75 and 3.81871 ..
(214.673,291.489) .. tension 2.04194 and 1.25376 ..
(249.947,257.162) .. tension 1.2359 and 2.37807 ..
 {curl1}(226.854,212.225) {dir 160.519} .. tension 1.67568 and 1.94716 ..
 {left}(204.882,234.542) .. tension 3.16615 and 0.8377 ..
(161.883,191.054) .. tension 1.49499 and 3.46588 ..
 {left}(5.339,103.702) .. tension 1 and 1 ..
 cycle
;
fill
(150.128,330.585) {dir -49.8073} .. tension 1.10336 and 1.24954 ..
 {dir -4.26042}(197.869,294.78) {curl1} .. tension 3.53849 and 2.79775 ..
 {left}(186.096,360.203) .. tension atleast 0.944705 ..
 {curl1} cycle
;
fill
(84.761,216.28) {left} .. tension 3.01758 and 1 ..
 {up}(56.5685,295.01) .. tension 1.33165 and 1.25463 ..
(161.984,486.106) .. tension 1 and 1.48376 ..
 {right}(245.847,511.278) .. tension 1.57986 and 1 ..
 {down}(299.035,450.438) .. tension 0.817628 and 31.1653 ..
 {left}(247.654,335.177) .. tension 1 and 1 ..
 {up}(239.902,341.961) .. tension 2.09493 and 0.75 ..
 {up}(267.386,421.795) .. tension 1.17904 and 0.863688 ..
 {left}(230.243,466.664) .. tension 1.3915 and 1 ..
(140.703,431.46) .. tension 1 and 2.94198 ..
 {down}(83.647,322.998) .. tension 0.80266 and 2.51675 ..
 {down}(107.729,249.831) .. tension 1.51202 and 1.75711 ..
 cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(dammatan.ii,-1,-1,-1,-1);
%%backgroundimage:images/dammatan.jpg,-2762.05,-783.434,1,0,0,1

%%beginpaths

fill
(-4.62422,105.182) {up} .. tension 5.42049 and 1 ..
(111.849,204.54) .. tension 1 and 3.02233 ..
 {curl1}(183.562,268.796) .. tension 2.6829 and 1 ..
 {up}(106.577,344.811) .. tension 1.25258 and 2.0234 ..
 {right}(201.023,453.567) .. tension 1.34919 and 1 ..
 {down}(244.917,369.508) .. tension 0.75 and 3.81871 ..
(233.963,308.85) .. tension 2.04194 and 1 ..
(277.918,265.36) .. tension 0.978806 and 1.53022 ..
 {curl1}(256.754,221.87) {dir 160.519} .. tension 1.67568 and 1.94716 ..
 {left}(219.35,249.01) .. tension 3.16615 and 0.8377 ..
(143.075,179.962) .. tension 1.49499 and 3.46588 ..
 {left}(5.339,103.702) .. tension 1 and 1 ..
 cycle
;
fill
(144.341,355.662) {dir -49.8073} .. tension 1.10336 and 1.24954 ..
 {dir -4.26042}(215.712,317.446) {curl1} .. tension 2.84324 and 1.16977 ..
 {left}(187.543,396.854) .. tension atleast 1.33323 ..
 {curl1} cycle
;
fill
(78.4917,235.088) {left} .. tension 3.01758 and 1 ..
 {up}(31.0091,323.463) .. tension 0.92696 and 1.25463 ..
(93.6086,491.688) .. tension 1 and 1.48376 ..
 {right}(278.5,613.98) .. tension 1 and 1 ..
 {down}(338.386,516.301) .. tension 0.817628 and 31.1653 ..
 {left}(293.423,411.645) .. tension 1 and 1 ..
 {up}(285.671,418.429) .. tension 2.09493 and 0.75 ..
 {up}(308.969,504.124) .. tension 1.17904 and 0.863688 ..
 {left}(258.151,556.807) .. tension 1.3915 and 1 ..
(143.494,495.37) .. tension 1 and 1 ..
 {down}(65.5067,346.72) .. tension 0.80266 and 2.51675 ..
 {down}(101.942,273.461) .. tension 1.51202 and 1.75711 ..
 cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(shadda,1617,-1,-1,-1);
%%backgroundimage:images/shadda.2.jpg,-136.605,-807.326,1,0,0,1

%%beginpaths

fill
(23.5267,63.6891) {up} .. tension 1 and 3.06146 ..
 {right}(51.1188,147.087) .. tension 1 and 1 ..
(56.9059,129.244) .. tension 1 and 1 ..
 {down}(48.7076,94.5216) .. tension 1 and 0.988392 ..
 {right}(75.2315,74.267) .. tension 1 and 1.19094 ..
(117.931,100.121) .. tension 1 and 1 ..
 {up}(122.813,147.65) .. tension 2.44577 and 1.99521 ..
 {right}(143.524,192.231) .. tension 1.72764 and 0.75 ..
 {down}(150.463,126.35) .. tension 0.85953 and 1 ..
 {right}(185.631,106.866) .. tension 1.5346 and 1.81482 ..
 {up}(220.872,122.975) .. tension 1 and 0.75 ..
 {up}(213.629,157.413) .. tension 1 and 2.50095 ..
 {right}(232.928,217.978) .. tension 2.24266 and 1 ..
 {down}(253.064,137.554) .. tension 0.901623 and 1.6035 ..
 {left}(167.181,50.2347) .. tension 0.75 and 1.25405 ..
 {curl1}(142.544,58.8595) {curl1} .. tension 2.1578 and 0.890023 ..
 {left}(56.0419,17.4415) .. tension 1.33199 and 0.75 ..
 cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(headkhah,1761,-1,-1,-1);
%%backgroundimage:images/sukun.jpg,-1778.91,-655.535,1,0,0,1

%%beginpaths

fill
(13.4178,112.477) {up} .. tension 1.5854 and 2.68637 ..
 {right}(103.887,190.885) .. tension 1.19051 and 8.97536 ..
 {down}(266.425,153.884) .. tension 1.74661 and 2.06803 ..
 {left}(62.1353,28.4875) .. tension 1 and 1 ..
 {up}(47.3462,38.427) .. tension 7.04933 and 2.88268 ..
 {dir 77.3022}(187.031,123.192) {dir 163.909} .. tension 0.98243 and 2.71315 ..
 {left}(82.3678,133.726) .. tension 1 and 3.4983 ..
 {left}(22.0603,106.072) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture rotated 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;
savepicture:=1;
endchar;
beginchar(sukun,1618,-1,-1,-1);
%%beginbody

drawcomp(headkhah,identity);

leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(maddahabove,1619,-1,-1,-1);
%%backgroundimage:images/madda.2.jpg,-358.007,-749.167,1,0,0,1

%%beginpaths

fill
(1.70738,-9.84514) {left} .. tension 1.00862 and 1 ..
 {up}(-1.01362,1.28861) .. tension 2.27837 and 2.37702 ..
 {right}(68.0727,118.452) .. tension 4.2737 and 0.75 ..
 {right}(165.653,95.9442) .. tension 1.11303 and 2.05456 ..
(339.429,154.221) .. tension 1 and 1.60776 ..
 {right}(366.917,174.407) .. tension 1 and 1 ..
 {down}(374.44,166.884) .. tension 1 and 1 ..
(339.528,106.406) .. tension 1.71721 and 1 ..
 {left}(161.94,41.0612) .. tension 0.75 and 2.76344 ..
 {left}(61.7171,58.0703) .. tension 1.44225 and 1.70524 ..
 cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture rotated 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;
endchar;
beginchar(smallalef,1648,-1,-1,-1);
%%backgroundimage:images/alefsmall.jpg,-345.838,-749.068,1,0,0,1

%%beginpaths

fill
(111.883,3.37577) .. tension 7.93795 and 0.922012 ..
(82.05,140.378) .. tension 1 and 4.96397 ..
 {up}(33.3782,243.514) {dir 110.858} .. tension 3.24633 and 3.2478 ..
 {right}(51.3979,286.206) .. tension 5.12138 and 1.5292 ..
 {down}(126.402,99.8487) .. tension 1.21064 and 5.16082 ..
 {left} cycle
;

%%endpaths
%%beginbody
currentpicture := currentpicture rotated 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;
savepicture:=1;
endchar;
beginchar(smallalef.joined,-1,-1,-1,-1);
%%beginbody
leftbearing := 0;
rightbearing := 0;
drawcomp(smallalef,identity);
endchar;
beginchar(smallalef.isol,-1,-1,-1,-1);
%%beginbody
%leftbearing := 50;
rightbearing := 0;
drawcomp(smallalef,identity shifted (0,150));
endchar;
beginchar(smallalef.replacement,-1,-1,-1,-1);
%%beginbody
leftbearing := 0;
rightbearing := 0;
drawcomp(smallalef,identity);
endchar;
beginchar(wasla,-1,-1,-1,-1);
%%backgroundimage:images/wasla.jpg,-584.203,-400.423,1,0,0,1

%%beginpaths

fill
(10.3211,-1.34307) {left} .. tension 0.75 and 1 ..
 {up}(6.96798,3.56052) .. tension 4.64161 and 2.55661 ..
 {right}(70.0394,96.6308) .. tension 2.65461 and 1 ..
 {right}(105.794,76.4754) .. tension 11.8175 and 1 ..
(130.944,108.752) .. tension 1 and 1 ..
 {right}(251.732,197.219) .. tension 0.980342 and 1 ..
 {down}(296.44,144.396) .. tension 1.27113 and 1.05341 ..
 {left}(133.342,32.0966) .. tension 1.67088 and 1 ..
(83.7245,39.1831) .. tension 1 and 1.40058 ..
 {left}(59.7994,48.7076) .. tension 2.36357 and 1.92764 ..
 cycle
;
fill
(143.711,77.1605) {down} .. tension 7.36221 and 1 ..
 {right}(147.487,72.2899) .. tension 2.87405 and 1.47583 ..
 {up}(261.097,123.073) .. tension 1.9568 and 1.35251 ..
 {left}(223.576,149.498) .. tension 3.64437 and 1.18742 ..
 cycle
;

%%endpaths
%%beginbody

%currentpicture := currentpicture rotated _angle1;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(meemiqlab,1762,-1,-1,-1);
%%backgroundimage:images/meemiqlab.jpg,-578.676,-977.056,1,0,0,1

%%beginpaths

fill
(116.389,440.304) .. tension 1.77325 and 3.65957 ..
 {down}(213.156,327.932) .. tension 1 and 1 ..
(191.454,287.423) .. tension 2.62213 and 1.94588 ..
 {down}(43.2504,244.825) .. tension 1.47115 and 2.20312 ..
 {down}(68.6414,103.04) .. tension 1 and 1.68516 ..
(66.5554,23.4689) .. tension 1 and 1.52359 ..
 {left}(55.9414,6.75154) .. tension 10.6076 and 1 ..
(41.9915,102.072) .. tension 1 and 1.06711 ..
 {up}(21.0508,216.883) .. tension 1 and 0.75 ..
(25.5179,260.185) .. tension 0.75 and 1.84317 ..
(90.4068,319.734) .. tension 1 and 3.08406 ..
 {up}(161.073,336.613) .. tension 8.34311 and 1.15892 ..
 {left}(95.9684,386.285) .. tension 2.2481 and 6.51377 ..
 {left}(47.2608,330.343) .. tension 1 and 1 ..
 {up}(43.4096,340.646) .. tension 1 and 1.67214 ..
 {right} cycle
;

%%endpaths
%%beginbody

leftbearing := 0;
rightbearing := 0;

currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
savepicture:=1;
endchar;
beginchar(smalllowmeem,1773,-1,-1,-1);
%%beginbody

drawcomp(meemiqlab,identity);
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(smallwaw,1765,-1,-1,-1);
%%backgroundimage:images/waw.added.jpg,-659,-752,1,0,0,1

%%beginpaths

fill
(-236.93,-58.4475) .. tension 5.44274 and 1 ..
(-147.1,-31.9143) .. tension 1 and 1.72791 ..
 {up}(-20.317,94.4879) .. tension 1 and 1 ..
 {left}(-24.224,96.1629) .. tension 3.51137 and 0.983038 ..
 {left}(-60.907,71.7609) .. tension 0.776556 and 1.08022 ..
 {up}(-113.131,123.178) .. tension 1 and 1.70965 ..
 {right}(-52.739,217.362) .. tension 1.32588 and 1 ..
 {down}(0.178,119.986) .. tension 1.67788 and 1.01698 ..
(-90.285,-48.8839) .. tension 1.25379 and 1 ..
 {left}(-227.687,-65) .. tension 1 and 1 ..
 {up} cycle
;
fill
(-63.906,162.258) .. tension 2.58629 and 1.44442 ..
 {down}(-88.093,138.768) .. tension 1.48833 and 1 ..
 {right}(-56.929,119.466) .. tension 1 and 2.98115 ..
 {up}(-33.672,132.024) .. tension 1.8894 and 1.56084 ..
 {left} cycle
;

%%endpaths
%%beginbody
savepicture:=1;
endchar;
beginchar(smallhighwaw,-1,-1,-1,-68);
%%beginbody
drawcomp(smallwaw,identity);
leftbearing := 0;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);

endchar;
beginchar(smallyeh,1766,-1,-1,-1);
%%backgroundimage:images/smallyeh.jpg,-1037.51,-1348.78,1,0,0,1

%%beginpaths

fill
(-349.447,67.5676) .. tension 1.16162 and 0.906448 ..
(-299.609,162.1) .. tension 1 and 1 ..
(-186.607,233.481) .. tension 1 and 1.39576 ..
 {right}(-126.808,295.209) .. tension 2.01433 and 1 ..
 {down}(-85.816,208.404) .. tension 1 and 4.85191 ..
 {left}(-102.213,166.448) .. tension 1 and 1 ..
 {up}(-107.035,171.752) .. tension 1.32878 and 2.80692 ..
 {left}(-139.346,247.466) .. tension 1.15217 and 1 ..
(-198.034,191.054) .. tension 0.783049 and 1.79279 ..
 {down}(-317.782,98.459) .. tension 1.40902 and 1 ..
 {right}(-250.049,78.9272) .. tension 1 and 1.4289 ..
 {right}(-12.678,94.8991) .. tension 1 and 1 ..
 {down}(-3.833,90.7339) .. tension 1 and 1 ..
(-28.428,60.3519) .. tension 1.99406 and 1 ..
 {left}(-297.553,31.0937) .. tension 0.75 and 1.24447 ..
 {up} cycle
;

%%endpaths
%%beginbody
savepicture:=1;
endchar;
beginchar(smallhighyeh,-1,-1,-1,-1);
%%beginbody
drawcomp(smallyeh,identity);
leftbearing := 0;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
endchar;
beginchar(smallhighroundedzero,1759,-1,-1,-1);
%%backgroundimage:images/smallhighroundedzero.jpg,-145.991,-381.589,1,0,0,1

%%beginpaths

fill
(32.9861,78.125) .. tension 1.32524 and 1.30655 ..
 {right}(130.91,204.13) .. tension 0.976859 and 1 ..
 {down}(197.338,114.005) .. tension 0.960952 and 0.993211 ..
 {left}(86.8055,14.4676) .. tension 1.31623 and 1 ..
 {up} cycle
;
fill
(118.335,153.615) .. tension 1.76196 and 1.92985 ..
 {down}(61.9213,98.9583) .. tension 1.2509 and 1 ..
 {right}(115.741,70.0231) .. tension 1 and 1.37331 ..
 {up}(171.875,97.8009) .. tension 1.40535 and 1.14345 ..
 {left} cycle
;

%%endpaths
%%beginbody


currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;


endchar;
beginchar(rectangularzero,1760,-1,-1,-1);
%%backgroundimage:images/rectangularzero.jpg,-522,-444,1,0,0,1

%%beginpaths

fill
(27.7778,107.06) .. tension 1 and 1.07978 ..
 {right}(109.954,231.481) .. tension 1 and 1 ..
 {down}(179.977,114.149) .. tension 1.08861 and 0.90942 ..
 {left}(97.1522,3.07838) .. tension 1.08558 and 1 ..
 {up} cycle
;
fill
(103.259,197.917) .. tension 1.17949 and 1 ..
 {down}(61.5069,110.946) .. tension 1 and 1.40111 ..
 {right}(104.877,34.2051) .. tension 1.3686 and 1 ..
 {up}(143.56,119.697) .. tension 1 and 1.16167 ..
 {left} cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(roundedfilledhigh,1772,-1,-1,-1);
%%backgroundimage:images/filleddot.jpg,-381,-565,1,0,0,1
%%beginbody

fill fullcircle scaled 180;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

savepicture:=1;
endchar;
beginchar(roundedfilledlow,1628,-1,-1,-1);
%%backgroundimage:images/filleddotlow.jpg,-1319,-1780,1,0,0,1
%%beginbody
leftbearing := 0;
rightbearing := 0;
drawcomp(roundedfilledhigh,identity scaled 1);

endchar;
beginchar(hamzaabove,1620,-1,-1,-1);
%%backgroundimage:images/hamzabelow.jpg,-779.894,-1760.27,1,0,0,1

%%beginpaths

fill
(65.9079,30.5427) .. tension 4.05069 and 0.847414 ..
 {up}(16.0751,82.7868) .. tension 1.11911 and 1.51927 ..
 {right}(129.575,202.039) .. tension 1.32853 and 1.3809 ..
 {down}(168.217,171.415) .. tension 1.54051 and 1.45415 ..
 {left}(146.605,128.564) .. tension 1.70344 and 1.62132 ..
 {left}(106.118,157.177) .. tension 1.55471 and 2.21974 ..
 {down}(51.2078,110.475) .. tension 3.74267 and 0.75 ..
 {right}(133.888,69.9472) .. tension 1 and 5.10949 ..
 {right}(224.115,85.6023) .. tension 1 and 2.31866 ..
 {down}(238.313,81.5811) .. tension 1.69061 and 2.63082 ..
(222.238,45.8141) .. tension 4.18845 and 1 ..
(126.491,18.6229) .. tension 1 and 5.54159 ..
 {left}(35.7671,-24.924) .. tension 1 and 1 ..
 {up}(25.8491,-16.6079) .. tension 0.865136 and 29.1229 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := 0;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
savepicture:=1;
endchar;
beginchar(hamzaabove.joined,-1,-1,-1,-1);
%%beginbody

drawcomp(hamzaabove,identity);

leftbearing := 0;
rightbearing := 0;
endchar;
beginchar(hamzaabove.lamalef,-1,-1,-1,-1);
%%backgroundimage:images/hamzaabove.lamalef.jpg,-1584.88,-676.814,1,0,0,1

%%beginpaths

fill
(66.4549,52.9139) .. tension 4.59938 and 0.888969 ..
 {up}(16.8789,87.6093) .. tension 1.09688 and 2.33501 ..
 {right}(105.157,198.277) .. tension 1.32853 and 2.32599 ..
 {down}(152.608,162.557) .. tension 2.76752 and 3.43091 ..
 {left}(126.849,128.104) .. tension 1.70344 and 1.21592 ..
 {left}(85.9439,152.74) .. tension 1.55471 and 17.2199 ..
 {down}(52.4134,113.288) .. tension 3.74267 and 1.42769 ..
 {right}(121.821,85.5878) .. tension 1 and 5.10949 ..
 {right}(209.24,103.418) .. tension 1 and 0.951435 ..
 {down}(218.37,94.5756) .. tension 1.69061 and 2.63082 ..
(208.775,64.4567) .. tension 4.18845 and 1 ..
(131.403,42.7355) .. tension 1 and 5.54159 ..
 {left}(56.8099,9.78263) .. tension 1 and 1 ..
 {up}(47.1486,11.1217) .. tension 0.75 and 7.96304 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := 0;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
endchar;
beginchar(hamzaabove.ii,-1,-1,-1,-1);
%%backgroundimage:images/hamzamark.jpg,-378.14,-416.744,1,0,0,1

%%beginpaths

fill
(69.5248,46.2159) .. tension 4.05069 and 1.43422 ..
 {up}(16.8789,87.6093) .. tension 1.11911 and 2.33501 ..
 {right}(121.939,194.805) .. tension 1.32853 and 2.32599 ..
 {down}(158.974,157.349) .. tension 1.54051 and 3.43091 ..
 {left}(134.951,122.937) .. tension 1.70344 and 1.62132 ..
 {left}(103.305,147.532) .. tension 1.55471 and 5.48645 ..
 {down}(52.4134,113.288) .. tension 3.74267 and 0.75 ..
 {right}(143.533,75.5735) .. tension 1 and 5.10949 ..
 {right}(200.404,82.3873) .. tension 1 and 1.3896 ..
 {down}(220.229,76.3567) .. tension 1.69061 and 2.63082 ..
(203.752,43.8047) .. tension 4.18845 and 1 ..
(132.519,32.6886) .. tension 1 and 5.54159 ..
 {left}(59.8798,2.80559) .. tension 1 and 1 ..
 {up}(47.1486,11.1217) .. tension 0.75 and 7.96304 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := 0;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
endchar;
beginchar(hamzabelow,1621,-1,-1,-1);
%%beginbody
drawcomp(hamzaabove,identity);
leftbearing := 0;
rightbearing := 0;
endchar;
beginchar(waqf.lam,1753,-1,-1,-1);
%%beginbody

fill (-242.0,1307.0)
 .. controls (-242.0,1328.09472656) and (-213.0,1342.0) .. (-204.0,1342.0)
 .. controls (-153.959960938,1342.0) and (3.876953125,1174.0) .. (8.0,1174.0)
 .. controls (17.0556640625,1174.0) and (24.0,1274.0) .. (24.0,1338.0)
 .. controls (24.0,1354.03125) and (44.0,1379.0) .. (50.0,1379.0)
 .. controls (62.0419921875,1379.0) and (64.0,1352.0) .. (64.0,1333.0)
 .. controls (64.0,1326.29199219) and (86.0,1328.01953125) .. (86.0,1302.0)
 .. controls (86.0,1286.59863281) and (58.0,1272.0) .. (58.0,1266.0)
 .. controls (58.0,1250.0) and (58.0,1240.0) .. (58.0,1224.0)
 .. controls (58.0,1195.0) and (30.0,1146.16503906) .. (30.0,1134.0)
 .. controls (30.0,1125.75390625) and (52.0,1099.0) .. (52.0,1045.0)
 .. controls (52.0,1005.98730469) and (-14.0,1004.0) .. (-60.0,1004.0)
 .. controls (-112.086914062,1004.0) and (-114.0,1008.0) .. (-114.0,1046.0)
 .. controls (-114.0,1060.03613281) and (-95.0,1063.0) .. (-86.0,1063.0)
 .. controls (-39.9892578125,1063.0) and (-21.0,1111.0) .. (-21.0,1120.0)
 .. controls (-21.0,1123.16210938) and (-21.9697265625,1130.03027344) .. (-25.0,1133.0)
 .. controls (-75.0,1182.0) and (-139.16015625,1252.0) .. (-161.0,1252.0)
 .. controls (-176.0,1252.0) and (-183.0,1246.0) .. (-198.0,1246.0)
 .. controls (-227.0,1246.0) and (-242.0,1278.0) .. cycle;

leftbearing := 0;
rightbearing := 0;

%currentpicture := currentpicture scaled 0.6;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
endchar;
beginchar(waqf.qaf,1751,-1,-1,-1);
%%backgroundimage:images/qaf.waqf.2.jpg,-76.9301,-616.605,1,0,0,1

%%beginpaths

fill
(17.331,48.3965) {up} .. tension 1.24719 and 1 ..
(52.1547,146.652) .. tension 0.75 and 2.54464 ..
 {up}(86.6042,195.245) .. tension 2.47951 and 9.13875 ..
(81.6127,215.804) .. tension 1.78333 and 1.26233 ..
 {up}(41.6615,398.73) .. tension 3.12678 and 2.2869 ..
 {right}(56.7282,421.694) .. tension 9.33496 and 1.36811 ..
(110.226,238.514) .. tension 0.75 and 0.952899 ..
 {right}(184.194,204.622) .. tension 1 and 1.7365 ..
 {up}(256.398,239.921) .. tension 2.25361 and 2.99046 ..
 {left}(233.424,284.864) .. tension 4.80591 and 0.75 ..
 {left}(178.902,241.596) .. tension 1.64758 and 1 ..
 {up}(132.151,282.654) .. tension 0.776297 and 2.0083 ..
 {right}(192.399,381.616) .. tension 4.31897 and 0.75 ..
 {down}(277.497,260.149) .. tension 0.827197 and 1.05125 ..
 {left}(172.941,154.79) .. tension 1 and 1.30255 ..
(121.033,161.019) .. tension 1 and 3.37208 ..
 {left}(102.881,172.674) .. tension 16.2437 and 3.15898 ..
 {down}(45.6801,84.7961) .. tension 0.812443 and 1 ..
 {right}(69.659,66.6447) .. tension 2.17311 and 1 ..
(235.21,93.6486) .. tension 1 and 7.0817 ..
 {right}(378.943,122.399) .. tension 1.28659 and 1.22131 ..
 {down}(380.345,118.69) .. tension 1.4337 and 2.8957 ..
(355.327,76.0889) .. tension 8.56989 and 2.6518 ..
 {left}(40.1996,22.1972) .. tension 2.89103 and 1.3358 ..
 cycle
;
fill
(183.993,330.21) .. tension 2.00798 and 1.26465 ..
 {down}(160.818,304.087) .. tension 1.49451 and 1.66896 ..
 {right}(188.509,292.193) .. tension 1.34763 and 1 ..
 {up}(213.665,310.987) .. tension 2.49626 and 1.79883 ..
 {left} cycle
;
fill
(123.977,457.549) .. tension 1.62597 and 3.85973 ..
 {right}(147.793,498.291) .. tension 2.71049 and 3.62643 ..
 {right}(182.852,480.188) .. tension 4.30558 and 1.97373 ..
 {right}(210.193,515.931) .. tension 2.78006 and 4.38078 ..
 {down}(258.322,485.11) .. tension 1.67243 and 2.23152 ..
 {left}(231.035,448.379) .. tension 3.13849 and 2.59674 ..
 {left}(186.71,472.954) .. tension 4.96434 and 2.28162 ..
 {left}(163.689,436.63) .. tension 3.34363 and 10.4049 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := 0;
rightbearing := 0;

%currentpicture := currentpicture scaled 0.6;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);




endchar;
beginchar(waqf.jeem,1754,287,1199,0);
%%backgroundimage:images/jeem_waqf.jpg,-1184,-1161,1,0,0,1

%%beginpaths

fill
(35.2206,254.522) {left} .. tension 1 and 1.30985 ..
 {up}(33.1921,261.52) .. tension 2.17499 and 1 ..
(70.3412,311.038) .. tension 0.94239 and 1 ..
(142.367,323.792) .. tension 1 and 1 ..
 {right}(240.723,332.209) .. tension 1 and 1 ..
 {down}(244.81,327.53) .. tension 1.72651 and 2.29697 ..
(227.182,289.538) .. tension 7.05297 and 0.75 ..
(127.078,257.548) .. tension 1 and 1.10784 ..
 {down}(70.6635,143.336) .. tension 1.30606 and 1.80754 ..
(127.442,75.01) .. tension 2.22796 and 1.63003 ..
 {right}(257.974,71.9437) .. tension 1 and 1 ..
 {right}(317.818,76.3567) .. tension 1 and 1 ..
 {down}(321.502,69.9937) .. tension 2.59706 and 1.41289 ..
 {left}(142.529,21.138) .. tension 0.881513 and 1.18369 ..
 {up}(38.1784,129.271) .. tension 0.872686 and 32.0185 ..
 {up}(96.9796,273.408) .. tension 1 and 1 ..
 {left}(89.2989,280.51) .. tension 0.954033 and 3.89572 ..
 cycle
;
fill
(158.76,169.742) {up} .. tension 1.73752 and 3.00849 ..
 {right}(182.501,204.305) .. tension 2.41423 and 4.57608 ..
 {down}(239.836,170.873) .. tension 1.793 and 3.87913 ..
 {left}(215.287,138.087) .. tension 2.79222 and 2.41511 ..
 cycle
;

%%endpaths
%%beginbody
currentpicture := currentpicture shifted (-xpart llcorner currentpicture,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(waqf.sad,1750,-1,-1,-1);
%%backgroundimage:images/saad_waqf.jpg,-763.968,-1703.61,1,0,0,1

%%beginpaths

fill
(9.9258,38.242) {up} .. tension 1 and 1 ..
(17.1596,82.449) .. tension 0.85553 and 2.93269 ..
 {up}(85.881,186.54) .. tension 1.68752 and 1 ..
(72.5852,234.571) .. tension 1 and 1 ..
 {up}(60.7521,308.71) .. tension 0.958643 and 2.72414 ..
 {right}(85.6797,352.511) .. tension 1 and 1 ..
 {down}(91.172,341.794) .. tension 1 and 1 ..
(100.884,269.327) .. tension 1 and 0.882645 ..
 {right}(171.615,177.357) .. tension 0.75 and 2.90775 ..
 {right}(230.155,218.69) .. tension 1.4542 and 0.761481 ..
 {right}(267.73,198.591) .. tension 5.74358 and 1.14238 ..
 {right}(401.959,284.317) .. tension 0.987815 and 1 ..
 {down}(450.785,237.57) .. tension 1.23767 and 1 ..
 {left}(288.493,147.687) .. tension 0.75 and 2.0234 ..
 {left}(224.194,162.021) .. tension 1.93451 and 1 ..
 {left}(162.237,129.469) .. tension 0.75 and 1.52062 ..
(115.62,141.19) .. tension 1 and 1.17692 ..
 {left}(97.133,160.413) .. tension 21.834 and 2.86571 ..
 {down}(41.2722,84.86) .. tension 1 and 1 ..
 {right}(85.0764,60.0782) .. tension 2.52275 and 1 ..
(360.43,101.002) .. tension 2.20236 and 2.02512 ..
 {right}(495.193,115.135) .. tension 0.75 and 0.75 ..
 {down}(500.082,111.719) .. tension 3.06602 and 2.59487 ..
(477.108,68.5172) .. tension 29.3902 and 1 ..
(224.796,32.6831) .. tension 1 and 1 ..
 {left}(55.7398,11.317) .. tension 1 and 1 ..
 cycle
;
fill
(314.134,196.25) {down} .. tension 1 and 1 ..
 {right}(322.172,191.562) .. tension 1.24795 and 3.70793 ..
 {up}(414.604,222.037) .. tension 2.24573 and 1 ..
 {left}(369.392,236.773) .. tension 2.6823 and 1.92038 ..
 cycle
;

%%endpaths
%%beginbody


currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(waqf.smallhighthreedots,1755,-1,-1,-1);
%%backgroundimage:images/threedots.jpg,-1556.63,-369.787,1,0,0,1
%%beginparams
z1 = (101,-129);
%%beginbody

drawcomp(one_dot,identity rotated -5 scaled 1.2);
picture pic_;
pic_ := currentpicture;
draw pic_ shifted z1;
draw pic_ shifted (-x1+30,y1-10);
leftbearing := 0;
rightbearing := 0;
currentpicture := currentpicture shifted (-xpart llcorner currentpicture,-ypart llcorner currentpicture);
endchar;
beginchar(waqf.meem,1752,-1,-1,-1);
%%backgroundimage:images/meem.waqf.jpg,-236.35,-613.558,1,0,0,1

%%beginpaths

fill
(28.3685,19.6143) .. tension 8.92991 and 1 ..
(156.285,92.0969) .. tension 1 and 3.7514 ..
 {up}(242.001,160.301) .. tension 1 and 1 ..
 {up}(234.375,197.917) .. tension 1 and 1.68035 ..
 {right}(289.799,280.352) .. tension 2.98207 and 0.914822 ..
 {down}(369.928,174.991) .. tension 1 and 1.76663 ..
 {left}(350.724,148.653) .. tension 1.28767 and 1 ..
(286.896,152.099) .. tension 1 and 1 ..
 {left}(275.463,154.419) .. tension 10000 and 1 ..
(255.081,129.215) .. tension 1.40515 and 1 ..
(184.586,66.9738) .. tension 1 and 2.42739 ..
 {left}(57.4908,18.1403) .. tension 1 and 3.25209 ..
 {up} cycle
;
fill
(261.574,210.648) .. tension 1.2968 and 1.72777 ..
 {right}(308.519,183.987) .. tension 1 and 2.02671 ..
 {up}(338.542,190.394) .. tension 1.33262 and 2.8668 ..
 {left}(287.616,242.477) .. tension 1.3737 and 3.91512 ..
 {down} cycle
;

%%endpaths
%%beginbody

leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(smallhighseen,-1,-1,-1,-1);
%%backgroundimage:images/smallhighseen.jpg,-1316.42,-567.369,1,0,0,1

%%beginpaths

fill
(30.9446,172.004) .. tension 1 and 11.0606 ..
 {right}(88.0112,317.483) .. tension 1 and 1 ..
 {down}(93.2356,309.848) .. tension 2.61702 and 1 ..
 {down}(62.6929,218.22) .. tension 1 and 1.06837 ..
 {right}(155.928,141.059) .. tension 1 and 1.90125 ..
 {up}(308.642,222.64) .. tension 1 and 2.3544 ..
 {up}(260.839,347.412) .. tension 1 and 6.15949 ..
 {right}(274.081,393.84) .. tension 1.29006 and 1 ..
(311.985,342.474) .. tension 1.88522 and 1.08892 ..
(390.223,359.279) .. tension 1.55499 and 3.80107 ..
 {right}(419.158,419.56) .. tension 1 and 1 ..
(431.494,412.204) .. tension 1.37684 and 1 ..
 {down}(424.394,362.159) .. tension 4.20285 and 1 ..
 {right}(467.596,347.747) .. tension 1 and 1 ..
 {up}(485.468,363.119) .. tension 1 and 2.11708 ..
 {right}(502.202,437.623) .. tension 1.23024 and 1 ..
 {down}(516.011,393.036) .. tension 1 and 1.79145 ..
 {left}(465.374,292.969) .. tension 1 and 2.1618 ..
 {dir 160.952}(397.212,307.737) {dir -148.296} .. tension 1.26049 and 1.74738 ..
 {dir 148.761}(330.475,297.483) {dir -78.4766} .. tension 0.924468 and 1 ..
 {left}(158.913,84.0018) .. tension 0.87866 and 1 ..
 cycle
;

%%endpaths
%%beginbody
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);

leftbearing := 0;
rightbearing := 0;
savepicture:=1;
endchar;
beginchar(smalllowseen,1763,-1,-1,-1);
%%beginbody
draw smallhighseen.pic;
leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(smallhighnoon,-1,-1,-1,-1);
%%backgroundimage:images/smallhighnoon.jpg,-1227.5,-1039.44,1,0,0,1

%%beginpaths

fill
(38.098,83.4298) .. tension 0.882099 and 2.89397 ..
 {right}(74.7492,196.438) .. tension 1 and 1 ..
 {down}(82.0634,183.578) .. tension 1 and 1 ..
 {down}(60.2816,117.67) .. tension 1 and 1 ..
 {right}(140.336,55.4591) .. tension 1.22788 and 1.80911 ..
 {up}(245.949,117.67) .. tension 1 and 2.17415 ..
 {up}(192.419,231.964) .. tension 1 and 3.9155 ..
 {right}(199.171,268.133) .. tension 3.91529 and 1 ..
 {down}(270.597,135.822) .. tension 0.846066 and 1.11054 ..
 {left}(145.158,5.78704) .. tension 1 and 1.02896 ..
 {up} cycle
;
fill
(71.5342,314.268) .. tension 1 and 1.69742 ..
 {right}(86.8056,361.288) .. tension 3.33203 and 2.39159 ..
 {down}(131.414,331.951) .. tension 1 and 1.95952 ..
 {left}(120.563,289.352) .. tension 3.51287 and 1.95394 ..
 {up} cycle
;

%%endpaths
%%beginbody



currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);

leftbearing := 0;
rightbearing := 0;

endchar;
beginchar(heh.fina.allah,-1,-1,-1,-1);
%%beginbody


path mypath_[];
save i_;
i_ := 0;

for item within allah.pic:
  if filled item :
    mypath_[incr i_] := pathpart item;
  fi;

endfor

%numeric dt_[];

%dt_1 = directiontime right of mypath_[1];

%dt_2 = directiontime right of subpath(dt_1+epsilon,infinity) of mypath_[1];
%dt_3 = directiontime right of subpath(dt_2+2epsilon,infinity) of mypath_[1];

%z1 = point dt_1 of mypath_[1];
%z2 = point dt_2 of subpath(dt_1+epsilon,infinity) of mypath_[1];
%z3 = point dt_3 of  subpath(dt_2+2epsilon,infinity) of mypath_[1];

%show dt_2;


z1 = glyphs[allah].points[1];
z2 = glyphs[allah].points[2];
z3 = glyphs[allah].points[3];
z4 = glyphs[allah].points[4];

save t,u,dm,l;
numeric t[], u[];
pair dm[];


l = 5;

(t1,u1) = mypath_[1] intersectiontimes z1;
(t2,u2) = mypath_[1] intersectiontimes z2;
(t3,u3) = mypath_[1] intersectiontimes z3;
(t4,u4) = mypath_[1] intersectiontimes z4;

path tpath_;

show t1;
show t4;

dm1 := z1 + l* unitvector direction t1 of mypath_[1];
dm2 := z4 - l* unitvector direction t4 of mypath_[1];

%tpath_ := subpath(0,t1) of mypath_[1] .. controls   dm1 .. 1/2[dm1,dm2] .. controls dm2 ..  subpath(t4,length mypath_[1]) of mypath_[1] .. cycle  ;
tpath_ := subpath(t4,length mypath_[1]  + t1) of mypath_[1] .. controls   dm1 .. 1/2[dm1,dm2] .. controls dm2 ..  cycle  ;


fill tpath_;
fill mypath_[2];

charwd := x1 - xpart llcorner currentpicture  + leftbearing;
currentpicture := currentpicture shifted (-xpart llcorner currentpicture + leftbearing,0);

endchar;
beginchar(lam.medi.allah,-1,-1,-1,-1);
%%beginbody


path mypath_[];
save i_;
i_ := 0;

for item within allah.pic:
  if filled item :
    mypath_[incr i_] := pathpart item;
  fi;

endfor

%numeric dt_[];

%dt_1 = directiontime right of mypath_[1];

%dt_2 = directiontime right of subpath(dt_1+epsilon,infinity) of mypath_[1];
%dt_3 = directiontime right of subpath(dt_2+2epsilon,infinity) of mypath_[1];

%z1 = point dt_1 of mypath_[1];
%z2 = point dt_2 of subpath(dt_1+epsilon,infinity) of mypath_[1];
%z3 = point dt_3 of  subpath(dt_2+2epsilon,infinity) of mypath_[1];

%show dt_2;


z1 = glyphs[allah].points[1];
z2 = glyphs[allah].points[2];
z3 = glyphs[allah].points[3];
z4 = glyphs[allah].points[4];

save t,u,dm,l;
numeric t[], u[];
pair dm[];


l = 5;

(t1,u1) = mypath_[1] intersectiontimes z1;
(t2,u2) = mypath_[1] intersectiontimes z2;
(t3,u3) = mypath_[1] intersectiontimes z3;
(t4,u4) = mypath_[1] intersectiontimes z4;

path tpath_;

show t1;
show t4;

dm1 := z1 - l* unitvector direction t1 of mypath_[1];
dm2 := z2 + l* unitvector direction t2 of mypath_[1];
dm3 := z3 - l* unitvector direction t3 of mypath_[1];
dm4 := z4 + l* unitvector direction t4 of mypath_[1];

tpath_ := subpath(t1,t2) of mypath_[1] .. controls   dm2 .. 1/2[dm2,dm3] .. controls dm3 ..  subpath(t3,t4) of mypath_[1] .. controls dm4 ..    1/2[dm4,dm1]  .. controls dm1 .. cycle;


fill tpath_;
%fill mypath_[2];

charwd := x2 - x1;
currentpicture := currentpicture shifted (-x1,0);

endchar;
beginchar(lam.init.allah,-1,-1,-1,-1);
%%beginbody


path mypath_[];
save i_;
i_ := 0;

for item within allah.pic:
  if filled item :
    mypath_[incr i_] := pathpart item;
  fi;

endfor

%numeric dt_[];

%dt_1 = directiontime right of mypath_[1];

%dt_2 = directiontime right of subpath(dt_1+epsilon,infinity) of mypath_[1];
%dt_3 = directiontime right of subpath(dt_2+2epsilon,infinity) of mypath_[1];

%z1 = point dt_1 of mypath_[1];
%z2 = point dt_2 of subpath(dt_1+epsilon,infinity) of mypath_[1];
%z3 = point dt_3 of  subpath(dt_2+2epsilon,infinity) of mypath_[1];

%show dt_2;


z1 = glyphs[allah].points[1];
z2 = glyphs[allah].points[2];
z3 = glyphs[allah].points[3];
z4 = glyphs[allah].points[4];

save t,u,dm,l;
numeric t[], u[];
pair dm[];


l = 5;

(t1,u1) = mypath_[1] intersectiontimes z1;
(t2,u2) = mypath_[1] intersectiontimes z2;
(t3,u3) = mypath_[1] intersectiontimes z3;
(t4,u4) = mypath_[1] intersectiontimes z4;

path tpath_;

%show t1;
%show t4;

dm1 := z1 - l* unitvector direction t1 of mypath_[1];
dm2 := z2-  l* unitvector direction t2 of mypath_[1];
dm3 := z3 + l* unitvector direction t3 of mypath_[1];
dm4 := z4 + l* unitvector direction t4 of mypath_[1];

tpath_ := subpath(t2,t3) of mypath_[1] .. controls   dm3 .. 1/2[dm3,dm2] .. controls dm2 ..  cycle;


fill tpath_;


charwd := xpart lrcorner currentpicture - x2 + rightbearing;
currentpicture := currentpicture shifted (-x2,0);


endchar;
beginchar(hamza.isol,1569,-1,-1,-1);
%%backgroundimage:images/hamza.jpg,-1969.7,-1615.21,1,0,0,1

%%beginpaths

fill
(52.0833,141.204) .. tension 1.29041 and 3.66404 ..
 {right}(214.25,321.873) .. tension 1 and 1.57465 ..
 {down}(274.56,259.16) .. tension 6.3755 and 3.9619 ..
 {left}(234.284,200.387) .. tension 1.55216 and 2.09024 ..
(223.776,214.654) .. tension 1 and 1.34417 ..
 {left}(185.764,251.736) .. tension 1.07159 and 7.34378 ..
 {down}(105.557,193.719) .. tension 1.63096 and 2.40414 ..
 {right}(240.741,135.995) .. tension 0.75 and 1.61228 ..
 {right}(385.893,162.599) .. tension 1 and 1 ..
 {down}(393.183,153.93) .. tension 2.13549 and 2.56704 ..
(361.912,101.944) .. tension 2.84105 and 1 ..
(233.109,70.0178) .. tension 1 and 11.0243 ..
 {left}(104.978,2.76366) .. tension 0.75 and 1 ..
(98.3796,9.25926) .. tension 0.980757 and 4.82517 ..
 {up}(140.046,79.2824) .. tension 12.9605 and 1.50368 ..
 {up} cycle
;

%%endpaths
%%beginbody

currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);

endchar;
defchar(alef.isol,1575,-1,-1,-1);

%%beginpaths

fill
(166.502,0) {left} .. tension 2.4601 and 1 ..
 {up}(163.027,48.9969) .. tension 1.21675 and 2.07948 ..
 {up}(63.9518,710.103) .. tension 1.49022 and 3.49347 ..
 {right}(83.3721,778.75) .. tension 3.90822 and 1 ..
 {down}(93.2634,753.382) .. tension 5.00179 and 0.970942 ..
(147.124,461.614) .. tension 2.97427 and 0.982883 ..
 {down}(192.131,131.381) .. tension 1.00275 and 4.02237 ..
 {left} cycle
;

%%endpaths
%%beginbody
leftbearing := leftbearing + 0;
enddefchar;
defchar(alef.isol.ii,-1,-1,-1,-1);
%%backgroundimage:images/alef.isol.ii.jpg,-1944.68,-934.792,1,0,0,1

%%beginpaths

fill
(172.016,0) {left} .. tension 2.4601 and 1 ..
 {up}(163.027,48.9969) .. tension 3.49419 and 2.07948 ..
 {up}(83.049,513.922) .. tension 8.69373 and 2.52449 ..
 {right}(102.469,582.569) .. tension 3.90822 and 1 ..
 {down}(113.366,547.824) .. tension 5.00179 and 3.99676 ..
 {down}(192.71,116.913) .. tension 0.863364 and 11.8932 ..
 {left} cycle
;

%%endpaths
%%beginbody
leftbearing := leftbearing; % - 50;
enddefchar;
defchar(alef.fina,-1,-1,-1,-1);
%%backgroundimage:images/testheight.jpg,-12360.3,-1476.82,1,0,0,1
%%beginparams
z1 = (227.307,8.175);
z2 = (214.577,102.4);
z3 = (308.106,1.11633);

%%beginverbatim
righttatweel := righttatweel -0.2 ;

righttatweel_const := (x3 - x1,y3 - y1);

%which_in_baseline := 3;

%%endverbatim

%%beginpaths

fill
(54.9873,698.975) .. tension 1.29322 and 10.6712 ..
 {right}(80.5787,772.824) .. tension 3.6286 and 1.61658 ..
(109.946,663.456) .. tension 3.28624 and 2.76737 ..
(150.463,302.662) .. tension 1.51205 and 1 ..
(185.417,140.278) .. tension 1 and 1.35542 ..
z2 join
z1 ..
(136.025,117.691) .. tension 2.00581 and 1 ..
(94.3287,441.953) .. tension 1 and 1.50214 ..
(61.4494,673.351) .. tension 1 and 1.57007 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing := leftbearing ; %- 50;
enddefchar;
defchar(alef.fina.afterkaf,-1,-1,-1,-1);
%%backgroundimage:images/kafinitaleffina.jpg,-1985.62,-1287.62,1,0,0,1
%%beginparams
z1 = (218.241,8.61867);
z2 = (192.916,132.608);
z3 = (315.876,1.44676);

%%beginverbatim
righttatweel := -0.6 ;

righttatweel_const := (x3 - x1,y3 - y1);

%%endverbatim

%%beginpaths

fill
(92.1208,727.428) .. tension 1.29322 and 10.6712 ..
 {right}(111.925,822.978) .. tension 9.26248 and 1.61658 ..
(130.934,676.805) .. tension 3.28624 and 2.76737 ..
(169.271,304.591) .. tension 1.51205 and 1.33163 ..
z2 join
z1 ..
(150.975,150.966) .. tension 8.01869 and 2.88876 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing := leftbearing; % - 50;
enddefchar;
defchar(lam.medi.laf,-1,-1,-1,-1);
%%backgroundimage:images/lamaleffina.jpg,-1004.91,-1261.34,1,0,0,1
%%beginparams
z1 = (732.796,55.5319);
z11 = (598.418,151.953);

%%beginverbatim
righttatweel := charrt +0.6 ;

if right_verticalextratio = 0 : right_verticalextratio := 12; fi

righttatweel_const :=  (100,-90);
right_tatweeltension := 0.75 + righttatweel * 0.215  ;
if right_tatweeltension > 1.7 : right_tatweeltension := 1.7; fi




z2 = z1 + (0,penwidth ) rotated -18;

%%endverbatim

%%beginpaths

fill
(133.547,-71.6685) {up} ..
 {right}(154.32,-53.7903) .. tension 1.01891 and 1.11455 ..
z11 .. tension 1.15521 and 1.26746 ..
 {up}(648.033,299.055) .. tension 1.91848 and 1 ..
 {up}(617.485,607.84) .. tension 1 and 4.95109 ..
 {right}(636.878,694.31) .. tension 10.7827 and 1 ..
(689.622,434.028) .. tension 1 and 1 ..
(711.507,305.413) .. tension 1 and 1.1924 ..
z2 join
z1 .. tension 1.56004 and 1 ..
(710.093,121.846) ..
 {curl1}(675.459,280.666) ..
(638.008,128.953) .. tension 0.75 and 1.52439 ..
(456.473,-50.2061) .. tension 1.09781 and 2.66853 ..
 {left}(160.037,-84.655) ..
 cycle
;

%%endpaths
%%beginbody


leftbearing := -450;


setCursiveAnchor(aleffinalaf,z11 +(-25,-43), ExitAnchor);

enddefchar;
defchar(lam.medi.laf.afterkaf,-1,-1,-1,-1);
%%beginbody

lam.medi.laf_(charlt,charrt-0.4);


enddefchar;
defchar(alef.fina.laf,-1,-1,-1,-1);
%%backgroundimage:images/lamaleffina.jpg,-889.361,-1326.47,1,0,0,1
%%beginparams
z1 = (725.273,67.106);
z11 = (569.777,217.906);

%%beginverbatim

%%endverbatim

%%beginpaths

fill
(264.711,551.118) {left} .. tension 3.27976 and 1 ..
(255.496,533.547) .. tension 1 and 2.26506 ..
 {left}(234.624,528.787) .. tension 3.29762 and 1.83048 ..
(125.655,582.047) .. tension 3.38468 and 4.72892 ..
 {right}(116.698,723.21) .. tension 8.71071 and 1 ..
(305.516,610.461) .. tension 1.04396 and 1.33341 ..
(426.183,506.898) ..
(551.075,316.576) .. tension 1 and 1.41668 ..
 {down}z11 .. tension 0.908872 and 9.46526 ..
 {left}(562.653,194.97) .. tension 10000 and 0.861182 ..
(461.604,384.517) .. tension 1.73238 and 5.70823 ..
 cycle
;

%%endpaths
%%beginbody



setCursiveAnchor(aleffinalaf,z11, EntryAnchor);

enddefchar;
defchar(lamalef.isol,-1,680,-1,-1);
%%backgroundimage:images/lamalef.2.jpg,-2492,-1573,1,0,0,1
%%beginparams
z1 = (505.513,86.3828);
z2 = (524.443,8.01897);

%%beginpaths

fill
(326.463,55.8755) .. tension 13.9962 and 1 ..
(392.523,115.283) .. tension 0.968917 and 0.889178 ..
 {up}(512.027,426.666) .. tension 3.32247 and 3.93128 ..
(493.856,660.865) .. tension 1 and 1 ..
 {up}(470.484,715.613) .. tension 1 and 6.44873 ..
 {right}(512.232,801.901) .. tension 1 and 1 ..
(519.735,790.389) .. tension 1 and 1 ..
(541.028,739.064) .. tension 1 and 7.14594 ..
 {down}(579.659,696.318) .. tension 1.03208 and 4.52356 ..
(545.436,630.047) .. tension 16.7888 and 0.770696 ..
(552.194,420.429) .. tension 0.902425 and 1.46144 ..
(471.306,156.495) .. tension 0.93248 and 21.9394 ..
 {down}(396.272,68.5325) .. tension 24.5038 and 11.3742 ..
z1 .. tension 22.2749 and 0.75 ..
(400.07,261.228) .. tension 1 and 8.15346 ..
 {left}(199.643,475.003) .. tension 1.82294 and 1 ..
 {left}(172.002,472.05) .. tension 1.51642 and 3.05861 ..
(71.4469,508.357) .. tension 3.15275 and 1 ..
 {up}(30.8653,655.449) .. tension 1 and 1 ..
 {right}(37.1908,666.263) .. tension 4.23328 and 0.830346 ..
(263.565,491.044) .. tension 1.34344 and 2.16394 ..
 {down}(536.937,83.013) .. tension 1 and 6.48608 ..
z2 .. tension 27.3804 and 21.246 ..
(283.93,-41.1507) .. tension 6.05504 and 9.72315 ..
(283.096,42.6124) .. tension 5.93256 and 3.42702 ..
 cycle
;

%%endpaths
%%beginbody
%charwd := xpart lrcorner currentpicture -x1 + rightbearing;
%currentpicture := currentpicture shifted (-x1,0);

glyphs[lamalef.isol].points[1] := z1;
glyphs[lamalef.isol].points[2] := z2;
enddefchar;
beginchar(lamalef.isol.ii,-1,-1,-1,-1);
%%backgroundimage:images/lamalef.jpg,-3318.6,-1251.8,1,0,0,1
%%beginparams
z1 = (527.84,91.1272);
z2 = (547.591,12.6486);

%%beginpaths

fill
(352.697,63.1316) .. tension 13.9962 and 1 ..
(468.991,193.705) .. tension 0.968917 and 0.889178 ..
 {up}(563.657,580.44) .. tension 3.32247 and 3.93128 ..
(552.742,720.309) .. tension 1 and 1 ..
 {up}(525.463,787.616) .. tension 1 and 6.44873 ..
 {right}(569.444,875.579) .. tension 1 and 1 ..
(576.389,865.741) .. tension 1 and 1 ..
(598.798,805.765) .. tension 1 and 7.14594 ..
 {down}(652.778,770.833) .. tension 1.03208 and 2.12287 ..
 {left}(603.206,712.376) .. tension 16.7888 and 0.770696 ..
(599.638,510.293) .. tension 0.831037 and 1.46144 ..
(561.449,285.431) .. tension 0.93248 and 21.9394 ..
 {down}(416.924,74.6723) .. tension 24.5038 and 11.3742 ..
z1 .. tension 22.2749 and 0.75 ..
(354.256,350.445) .. tension 1 and 8.15346 ..
 {left}(184.294,527.749) .. tension 1.82294 and 1 ..
 {left}(156.094,513.633) .. tension 1.51642 and 3.05861 ..
(61.4,543.8) .. tension 3.15275 and 1 ..
 {up}(26.4,694.8) .. tension 1 and 1 ..
 {right}(34.4,709.8) .. tension 4.23328 and 0.830346 ..
(293.706,510.022) .. tension 1.34344 and 2.16394 ..
 {down}(562.4,88.8) .. tension 1 and 6.48608 ..
z2 .. tension 27.3804 and 21.246 ..
(267.726,-31.3127) .. tension 6.05504 and 9.72315 ..
(281.649,44.5414) .. tension 5.93256 and 3.42702 ..
 cycle
;

%%endpaths
%%beginbody
%charwd := xpart lrcorner currentpicture -x1 + rightbearing;
%currentpicture := currentpicture shifted (-x1,0);


endchar;
beginchar(lam.init.lamalef,-1,-1,-1,-1);
%%backgroundimage:images/lamalef.jpg,-3318.6,-1251.8,1,0,0,1
%%beginbody


save t,u,dm,l;

path p;
pair dm[];
numeric t[], u[];

l = 2;

lamalef.isol_(charlt,charrt);

p := pathpart currentpicture;

(t1,u1) = p intersectiontimes z1;
(t2,u2) = p intersectiontimes z2;

dm1 := z1 - l* unitvector direction t1 of p;
dm2 := z2-  l* unitvector direction t2 of p;

currentpicture:=nullpicture;

fill subpath(0,t1) of p .. controls   dm1 .. 1/2[dm1,dm2] .. controls dm2 ..  subpath(t2,length p ) of p  .. cycle ;

setCursiveAnchor(lamalef,z1, ExitAnchor);

leftbearing := 0;
%charwd := xpart lrcorner currentpicture - x1 + rightbearing;
%currentpicture := currentpicture shifted (-x1,0);



endchar;
beginchar(alef.fina.lamalef,-1,-1,-1,-1);
%%backgroundimage:images/lamalef.jpg,-3318.6,-1251.8,1,0,0,1
%%beginbody

path p;

lamalef.isol_(charlt,charrt);
p := pathpart currentpicture;

save t,u,dm,l;
numeric t[], u[];
pair dm[];


l = 2;

(t1,u1) = p intersectiontimes z1;
(t2,u2) = p intersectiontimes z2;


dm1 := z1 - l* unitvector direction t1 of p;
dm2 := z2 +  l* unitvector direction t2 of p;

currentpicture:=nullpicture;

fill subpath(t1,t2) of p .. controls   dm2 .. 1/2[dm2,dm1] .. controls dm1 ..   cycle ;


setCursiveAnchor(lamalef,z1, EntryAnchor);

%leftbearing := -0;
rightbearing := -250;
%charwd := x1 -  xpart llcorner currentpicture  +leftbearing;
%currentpicture := currentpicture shifted (-xpart llcorner currentpicture + leftbearing,0);



endchar;
defchar(behshape.isol,-1,-1,-1,-1);
%%backgroundimage:images/teh.fina.jpg,-283.339,-1305.02,1,0,0,1

%%beginpaths

fill
(50.6879,139.167) .. tension 1.77809 and 2.37253 ..
 {right}(123.526,288.735) .. tension 3.08415 and 2.19563 ..
(122.91,265.768) .. tension 2.03209 and 1 ..
(96.0506,183.497) .. tension 1.03789 and 1.21559 ..
 {right}(292.361,49.8611) .. tension 1 and 3.44436 ..
 {up}(744.444,192.3) .. tension 1 and 7.70909 ..
 {up}(665.263,322.583) .. tension 1.41144 and 17.3247 ..
 {right}(684.722,399.861) .. tension 5.16974 and 0.867889 ..
 {down}(771.952,240.756) .. tension 0.756175 and 1.02009 ..
(656.346,54.3943) .. tension 1.38787 and 1.06234 ..
 {left}(303.832,-27.445) .. tension 1 and 1 ..
 {up} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(behshape.init,-1,-1,-1,-1);
%%backgroundimage:images/beh.init.jpg,-1385,-1287,1,0,0,1
%%beginparams
z1 = (192,41);

%%beginverbatim
%charlt := 4; % numeric
lefttatweel := lefttatweel +1;
%interim left_verticalextratio :=10;
if left_tatweeltension = 0 : left_tatweeltension := 1.2; fi
if left_verticalextratio = 0 : left_verticalextratio := 12; fi
lefttatweel_const := (163, y1 -1 );



z2 = z1 + (0,penwidth ) rotated -5;

%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension 1 and 2.69891 ..
 {up}(279.566,163.447) .. tension 1.01595 and 11.9251 ..
(194.346,272.787) .. tension 5.61953 and 5.94319 ..
 {right}(211.539,356.269) .. tension 5.85287 and 1 ..
(277.778,301.408) .. tension 1.13818 and 1.06645 ..
 {down}(309.595,192) ..
(264.219,74.071) ..
 cycle
;

%%endpaths
%%beginbody



enddefchar;
defchar(behshape.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/behshape.init.yeh.jpg,-885.528,-1239.69,1,0,0,1
%%beginparams
z1 = (0,90);

%%beginpaths

fill
(47.9628,389.173) .. tension 1.04084 and 2.91058 ..
 {right}(69.0638,469.166) .. tension 245.934 and 0.897121 ..
 {down}(195.015,247.589) .. tension 1.09638 and 52.2617 ..
 {up}z1 .. tension 1.32883 and 5.07289 ..
 {right}(23.6625,151.317) .. tension 1.22913 and 2.78952 ..
 {up}(167.036,214.357) .. tension 1.84368 and 89.7661 ..
 {up} cycle
;

%%endpaths
%%beginbody
charwd:= xpart (urcorner currentpicture) - x1 + rightbearing; 
setCursiveAnchor(yehcursive1,z1,ExitAnchor);
enddefchar;
defchar(behshape.init.beforeseen,-1,-1,-1,-1);
%%backgroundimage:images/behbeforeseen.jpg,-3329,-1124.89,1,0,0,1
%%beginparams
lefttatweel := 0; % numeric
z1 = (43.5244,24.3482);
z2 = (25.8564,91.8973);

%%beginverbatim

lefttatweel := charlt + 0.3;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi



%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension atleast 1 ..
(39.4853,109.889) {up} .. tension 1.35214 and 8.89964 ..
 {up}(-62.1407,323.426) .. tension 0.75 and 2.6421 ..
 {right}(-44.4,409) .. tension 7.15864 and 1.54359 ..
(71.3081,142.649) {down} .. tension 0.75 and 1.59291 ..
 cycle
;

%%endpaths
%%beginbody


enddefchar;
defchar(behshape.init.beforenoon,-1,-1,-1,-1);
%%backgroundimage:images/behnoon.jpg,-511.096,-1089.58,1,0,0,1
%%beginparams
z1 = (-0.482253,-0.482253);

%%beginverbatim
%charlt := 4; % numeric
lefttatweel := charlt +0;
%interim left_verticalextratio :=10;
if left_tatweeltension = 0 : left_tatweeltension := 1; fi
if left_verticalextratio = 0 : left_verticalextratio := 10; fi



%%endverbatim

%%beginpaths

fill
(78.6219,10.5741) join
(104.415,94.6096) .. tension 1 and 2.57083 ..
 {up}(149.127,121.787) .. tension 1 and 1.78806 ..
 {up}(104.198,194.106) .. tension 1 and 3.33785 ..
 {right}(117.752,273.624) .. tension 3.03159 and 1 ..
 {down}(185.61,133.894) .. tension 1 and 1.14234 ..
(168.326,55.8225) .. tension 1.27178 and 1 ..
 cycle
;

%%endpaths
%%beginbody

setCursiveAnchor(behnoon,(0,0), ExitAnchorRTL);

enddefchar;
defchar(behshape.init.beforehah,-1,-1,-1,-1);
%%backgroundimage:images/yeh_hah.init.jpg,-2401.81,-1310,1,0,0,1
%%beginbody

path p;
save mypath_,i_, t,u,dm,l,dm,l;
path mypath_[], tpath_;
pair dm[];
numeric t[], u[];
i_ := 0;
l = 2;

behshape_hah.init_(charlt,charrt);

 p := pathpart currentpicture;
%fill subpath(0,9) of p .. tension 2 ..  point 20 of p  ..    subpath(21,length p) of p .. cycle;

(t3,u3) = p intersectiontimes z3;
(t4,u4) = p intersectiontimes z4;

dm1 := z3 - l* unitvector direction t3 of p;
dm2 := z4+  l* unitvector direction t4 of p;

currentpicture:=nullpicture;

fill subpath(t3,t4) of p .. controls   dm2 .. 1/2[dm2,dm1] .. controls dm1 ..   cycle ;


rightbearing := 150;
pair leftanchor;
leftbearing := x3 - x4;
setCursiveAnchor(beforehah1,z4, ExitAnchorRTL);

enddefchar;
defchar(behshape.init.beforehahyeh,-1,-1,-1,-1);
%%backgroundimage:images/behhahyeh.jpg,-1415.46,-1005.31,1,0,0,1
%%beginparams
z1 = (28.9352,193.866);

%%beginpaths

fill
z1 .. tension 1.79766 and 1 ..
(81.0185,270.255) .. tension 1.4004 and 1 ..
(253.472,352.431) .. tension 0.802302 and 2.90569 ..
 {right}(442.708,506.944) .. tension 1.32115 and 1 ..
 {down}(480.324,398.727) .. tension 1 and 2.36968 ..
 {left}(455.44,314.236) .. tension 3.09635 and 2.30682 ..
 {left}(418.402,418.981) .. tension 2.70585 and 1 ..
(282.407,306.713) .. tension 1.31313 and 35.3027 ..
 {up} cycle
;

%%endpaths
%%beginbody
%%beginbody
leftbearing :=0;
rightbearing := 130;
leftbearing := -250;

setCursiveAnchor(beforehah1,z1, ExitAnchorRTL);

enddefchar;
beginchar(behshape.init.test.i,-1,-1,-1,-1);
%%beginparams
left_verticalextratio := 9; % numeric
lefttatweel := 2.6; % numeric
testtension := 9; % numeric
z100 = (339,-110);
%%beginbody

left_tatweeltension := testtension / 10; 

lefttatweel := charlt + x100/nuqta;

behshape.init_(lefttatweel,0);
endchar;
defchar(behshape.medi,-1,-1,-1,-1);
%%backgroundimage:images/behshape.medi.2.jpg,-836.499,-1438.96,1,0,0,1
%%beginparams
z1 = (219.986,47.4628);
z3 = (132.906,38.769);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := lefttatweel + 0;
righttatweel := righttatweel - 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
righttatweel_const := (120,-y1);
%which_in_baseline := 2;
z2 = z1 + (penwidth,0) rotated 70;

z4 = z3 + (penwidth,0) rotated 83;

%which_in_baseline := 3;
%%endverbatim

%%beginpaths

fill
(181.269,99.7718) {dir -116.055} ..
z3 join
z4 .. tension 0.932615 and 2.98102 ..
(182.631,170.321) .. tension 1.7264 and 1.23542 ..
 {right}(195.366,180.062) .. tension 1.7264 and 1 ..
z2 join
z1 ..
 {curl1} cycle
;

%%endpaths
%%beginbody
%labels(1,2,3,4);

show leftanchor;
show rightanchor;

enddefchar;
defchar(behshape.medi.beforeseen,-1,-1,-1,-1);
%%backgroundimage:images/behlongsenna.jpg,-2516,-1032,1,0,0,1
%%beginparams
z1 = (219.986,43.1091);
z3 = (112.812,25.0382);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt + 0;
righttatweel := charrt - 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
righttatweel_const := (120,-y1);
%which_in_baseline := 2;
z2 = z1 + (penwidth,0) rotated 70;

z4 = z3 + (penwidth,0) rotated 83;

which_in_baseline := 2;
%%endverbatim

%%beginpaths

fill
(181.269,99.7718) {dir -116.055} ..
z3 join
z4 .. tension 0.932615 and 2.98102 ..
 {up}(161.532,143.194) .. tension 1.7264 and 1 ..
 {up}(144.676,231.749) .. tension 1 and 2.01696 ..
 {right}(159.197,304.979) .. tension 9.83217 and 1 ..
(193.571,208.307) .. tension 1 and 1 ..
z2 join
z1 ..
 {curl1} cycle
;

%%endpaths
%%beginbody
%labels(1,2,3,4);

show leftanchor;
show rightanchor;

enddefchar;
defchar(behshape.medi.afterlam,-1,-1,-1,-1);
%%backgroundimage:images/behshape.medi.2.jpg,-836.499,-1438.96,1,0,0,1
%%beginparams
z1 = (219.986,47.4628);
z3 = (132.906,38.769);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt + 0;
righttatweel := charrt - 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
righttatweel_const := (120,-y1);
%which_in_baseline := 2;
z2 = z1 + (penwidth,0) rotated 70;

z4 = z3 + (penwidth,0) rotated 83;

%%endverbatim

%%beginpaths

fill
(181.269,99.7718) {dir -116.055} ..
z3 join
z4 .. tension 0.932615 and 2.98102 ..
(182.631,170.321) .. tension 1.7264 and 1.23542 ..
 {right}(195.366,180.062) .. tension 1.7264 and 1 ..
z2 join
z1 ..
 {curl1} cycle
;

%%endpaths
%%beginbody
%labels(1,2,3,4);


enddefchar;
defchar(behshape.medi.beforeyeh,-1,-1,-1,-1);
%%beginbody

behmediyehfina_(charlt,charrt);

cutGlyph(z2,z3,true);
cutGlyph(z4,z5,false);

rightanchor := z5;
leftbearing := - x2 + x1;

setCursiveAnchor(behmediyehfina,z2, ExitAnchor);

tr_ := tr_ shifted (0,-164);

enddefchar;
defchar(behshape.medi.beforereh,-1,-1,-1,-1);
%%backgroundimage:images/behreh.jpg,-1516,-1251,1,0,0,1
%%beginparams
z1 = (273.876,-176.505);
z3 = (5.21991,-30.6748);
z4 = (-6.22594,45.0977);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt + 0;
righttatweel := charrt + 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
righttatweel_const := (100,-20);
which_in_baseline := 3;
z2 = z1 + (penwidth,0) rotated 80;

%%endverbatim

%%beginpaths

fill
(132.335,38.752) {dir -129.323} ..
 {left}z3 .. tension 1 and 1 ..
 {up}(-57.8225,41.0417) .. tension 1 and 2.34159 ..
 {right}(-48.0783,58.1372) .. tension 3.10136 and 0.75 ..
 {right}z4 .. tension 0.932615 and 2.98102 ..
(108.475,98.0808) .. tension 1.7264 and 2.82674 ..
 {right}(138.277,176.997) .. tension 2.98366 and 1 ..
(166.555,86.422) .. tension 1 and 1.21844 ..
(237.847,-62.5) .. tension 1 and 1.37056 ..
z2 join
z1 ..
(159.907,-71.592) .. tension 2.28647 and 0.82217 ..
 {dir 103.703} cycle
;

%%endpaths
%%beginbody

%charwd:= xpart rightanchor - xpart leftanchor;    

%pair leftanchor;
leftbearing := 0;

setCursiveAnchor(behshapereh,(4,183), ExitAnchor);


currentpicture := currentpicture shifted (0,- ypart rightanchor); 
rightanchor := rightanchor  shifted (0,- ypart rightanchor);




enddefchar;
defchar(behshape.fina,-1,-1,-1,-1);
%%backgroundimage:images/teh.fina.2.jpg,-144,-1121,1,0,0,1
%%beginparams
z1 = (837.906,47.3136);
z2 = (869.14,101.131);
z3 = (969,-2);

%%beginverbatim

%z2 = z1 + (0,penwidth ) rotated 20;
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
%right_tatweeltension := 1 + righttatweel * 0.315  ;
%if right_tatweeltension > 3 : right_tatweeltension := 3; fi
righttatweel_const := (x3 - x1,y3 - y1);

%%endverbatim

%%beginpaths

fill
(90,115) .. tension 1.38823 and 10.4251 ..
 {right}(163.163,280.535) .. tension 1 and 1 ..
 {down}(167.907,270.674) .. tension 1.25574 and 1 ..
 {down}(137.628,182.046) .. tension 0.984028 and 1.00589 ..
 {right}(386.539,17.4823) .. tension 0.93229 and 2.57559 ..
(570.489,32.0944) .. tension 1.43151 and 1.40908 ..
(846.485,170.609) .. tension 1 and 1.61846 ..
 {right}(863.558,198.302) .. tension 1 and 1 ..
 {down}(869,187) .. tension 1 and 1.40304 ..
 {down}(848,116) .. tension 2.95247 and 1 ..
z2 join
z1 .. tension 1.49205 and 7.09381 ..
 {left}(821.861,68.9069) .. tension 15.5787 and 0.807264 ..
(722.589,3.25595) .. tension 2.15633 and 0.851729 ..
 {left}(376,-65) .. tension 1.14002 and 1 ..
 {up} cycle
;

%%endpaths
%%beginbody

enddefchar;
beginchar(seen.isol,1587,-1,-1,-1);
%%backgroundimage:images/seen.isol.jpg,-25.2994,-1452.74,1,0,0,1

%%beginpaths

fill
(44.4721,-47.0628) .. tension 0.904492 and 8.4763 ..
 {right}(99.8294,182.066) .. tension 2.16212 and 1.92446 ..
 {down}(111,165) .. tension 3.50408 and 1.02765 ..
 {down}(77.9396,26.0373) .. tension 1.07129 and 0.939778 ..
 {right}(276.131,-139.344) .. tension 1 and 2.02046 ..
 {up}(545,11) .. tension 1.67752 and 1 ..
(516.291,148.46) .. tension 0.75 and 10000 ..
 {up}(471,227) .. tension 1 and 27.9974 ..
 {right}(484.804,314) .. tension 4.06396 and 1 ..
(534,238) .. tension 1 and 1 ..
 {right}(606.411,223.991) .. tension 1 and 0.893335 ..
(706.903,272.071) .. tension 1 and 13.0541 ..
 {right}(749,364) .. tension 1.31024 and 1 ..
 {down}(757.009,339.009) .. tension 3.05237 and 2.90237 ..
 {down}(732,278.392) .. tension 2.23839 and 1.70605 ..
(842.159,241.177) .. tension 1 and 1 ..
 {up}(861,255) .. tension 1 and 1 ..
 {up}(847.494,343.11) .. tension 3.66057 and 13.0696 ..
 {right}(868.206,415.019) .. tension 4.96657 and 1 ..
 {down}(895,295) .. tension 1 and 2.30256 ..
 {left}(839,160) .. tension 1 and 1 ..
(750.73,175.821) .. tension 1 and 10000 ..
 {left}(700,205) .. tension 4.24451 and 0.75 ..
(643.562,154.89) .. tension 1.75101 and 0.983575 ..
 {left}(567.397,142.99) {dir -88.19} .. tension 1.08104 and 1.1434 ..
(537.93,-83.9571) .. tension 1.09281 and 1.17457 ..
 {left}(269.147,-219.986) .. tension 0.919648 and 1.14905 ..
 {up} cycle
;

%%endpaths
%%beginbody
savepicture:=1;
endchar;
defchar(seen.init,-1,-1,-1,-1);
%%backgroundimage:images/seen.init.jpg,-2132.16,-1301.28,1,0,0,1
%%beginparams
z1 = (79.4975,10.3767);

%%beginverbatim
%charlt := 4; % numeric

%interim left_verticalextratio :=10;
if left_tatweeltension = 0 : left_tatweeltension := 1; fi
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
%lefttatweel_const := (163, y1 -1 );



z2 = z1 + (0,penwidth ) rotated -15;

%%endverbatim

%%beginpaths

fill
(238.202,24.8446) .. tension 0.75 and 1.73694 ..
 {left}(150.851,31.2291) .. tension 1.67481 and 0.75 ..
z1 join
z2 .. tension 1.65623 and 1 ..
(224.405,149.715) .. tension 0.770994 and 8.65667 ..
 {right}(273.518,223.817) .. tension 1 and 1 ..
 {down}(277.778,210.745) .. tension 1.23162 and 6.14294 ..
 {down}(235.212,96.4261) .. tension 8.99697 and 1.71558 ..
(408.951,146.174) .. tension 1.66726 and 9.44034 ..
 {right}(489.563,267.168) .. tension 1 and 1 ..
 {down}(493.827,254.63) .. tension 1 and 2.57963 ..
 {down}(470.679,188.561) .. tension 5.9891 and 1 ..
 {right}(578.704,140.818) .. tension 1 and 1 ..
 {up}(591.242,150.945) .. tension 0.75 and 7.69413 ..
 {up}(579.641,250.086) .. tension 1.43597 and 9.78838 ..
 {right}(606.674,326.968) .. tension 4.54313 and 1 ..
 {down}(623.553,220.872) .. tension 1 and 3.87789 ..
 {left}(564.718,59.7994) .. tension 0.75 and 8.89694 ..
 {left}(434.894,120.257) .. tension 3.61835 and 1 ..
(365.326,55.6225) .. tension 1.64816 and 0.75 ..
 {left} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(seen.init.beforereh,-1,-1,-1,-1);
%%backgroundimage:images/seeninitreh.jpg,-3427.12,-1143.68,1,0,0,1
%%beginparams
z1 = (-20.8312,12.6749);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
(83.5068,39.4358) .. tension 0.967623 and 2.22813 ..
 {left}z1 .. tension 0.75 and 1 ..
(-54.6553,22.907) .. tension 1.23054 and 4.54894 ..
 {right}(-59.1068,93.2986) .. tension 0.843775 and 6.73346 ..
 {right}(204.528,159.246) .. tension 5.74099 and 1 ..
 {down}(210.199,141.903) .. tension 1 and 1.44469 ..
(208.207,100.403) .. tension 1.302 and 2.73639 ..
 {right}(265.306,83.4817) .. tension 1.13 and 0.875768 ..
(411.467,140.213) .. tension 1 and 2.92953 ..
 {right}(458.999,219.987) .. tension 1 and 1 ..
 {down}(464.555,200.417) .. tension 1 and 12.2986 ..
 {down}(450.235,138.117) .. tension 2.56782 and 2.10028 ..
 {right}(542.285,96.7696) .. tension 2.41486 and 1 ..
 {up}(549.791,107.446) .. tension 1 and 1 ..
(533.325,196.194) .. tension 1 and 3.40805 ..
 {right}(556.932,278.195) .. tension 2.3248 and 1 ..
 {down}(585.327,152.5) .. tension 1.34609 and 2.32635 ..
 {left}(532.247,11.9885) .. tension 2.41485 and 1 ..
 {dir 115.187}(412.913,72.3271) {dir -118.028} .. tension 1.43159 and 0.75 ..
 {left}(274.981,0.874727) .. tension 0.75 and 1.18679 ..
(177.192,57.97) .. tension 1 and 0.75 ..
 {left}(171.192,64.1396) .. tension 10.1716 and 1 ..
 cycle
;

%%endpaths
%%beginbody
leftbearing := 0;
setCursiveAnchor(seenreh,z1 + (0,15), ExitAnchor);
enddefchar;
defchar(seen.init.beforeyeh,-1,-1,-1,-1);
%%beginbody

behmediyehfina_(charlt,charrt);


cutGlyph(z2,z3,true);



setCursiveAnchor(behmediyehfina,z2, ExitAnchor);

tr_ := tr_ shifted (0,-100);

enddefchar;
defchar(seen.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/seenyeh.jpg,-1288.06,-967.732,1,0,0,1
%%beginparams
z10 = (563.545,26.8501);
z20 = (590.028,75.6836);
z30 = (695.023,-26.0417);
z40 = (287.362,-17.2679);

%%beginverbatim
righttatweel := charrt +0;
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
right_tatweeltension := 1 + righttatweel * 0.215  ;
if right_tatweeltension > 3 : right_tatweeltension := 3; fi
righttatweel_const := (x30 - x10,y30 - y10);

z50 = z40 + (0,lineheight);


%%endverbatim

%%beginpaths

fill
z40 endlink
z50 .. tension 1 and 0.911937 ..
(401.754,96.9211) .. tension 1 and 4.83872 ..
 {right}(463.44,166.56) .. tension 0.75 and 1 ..
 {down}(468.465,148.131) .. tension 1.98298 and 5.09898 ..
 {down}(444.881,75.0162) .. tension 4.72672 and 4.30029 ..
 {right}(499.057,55.8982) .. tension 1.48298 and 3.63522 ..
 {right}(578.745,174.392) .. tension 1.78057 and 1.94583 ..
(574.144,105.003) .. tension 1 and 1 ..
z20 join
z10 .. tension 1 and 1.33159 ..
 {left}(548.335,34.4486) .. tension 8.39164 and 1.16765 ..
 {left}(478.215,-29.554) .. tension 0.75 and 5.58464 ..
 {left}(394.111,19.3836) .. tension 6.73744 and 0.75 ..
 {left} cycle
;

%%endpaths
%%beginbody

picture pic_;

pic_ :=  currentpicture;

currentpicture:=nullpicture;

behmediyehfina_(charlt,charrt);

cutGlyph(z2,z3,true);
cutGlyph(z4,z5,false);

z60 = (-x5 + x40,-y5+y40);

currentpicture := currentpicture shifted z60;

addto pic_ also currentpicture;


currentpicture:= pic_;
leftbearing := 0;

z70 = z2 shifted z60;



setCursiveAnchor(behmediyehfina,z70, ExitAnchor);

tr_ := tr_ shifted (0,25);

enddefchar;
defchar(seen.medi.beforereh,-1,-1,-1,-1);
%%backgroundimage:images/seenreh.jpg,-2456.51,-1172.74,1,0,0,1
%%beginparams
z1 = (567.362,32.9727);
z11 = (693,12);
z2 = (587.331,100.768);
z3 = (-20.8312,12.6749);

%%beginverbatim



righttatweel := charrt + 0.0;

if right_verticalextratio = 0 : right_verticalextratio := 10; fi

righttatweel_const := (x11 - x1,y11 - y1);

%z2 = z1 + (penwidth,0) rotated 60;


%%endverbatim

%%beginpaths

fill
(102.395,47.0715) .. tension 0.967623 and 2.22813 ..
 {left}z3 .. tension 0.75 and 1 ..
(-54.6553,22.907) .. tension 1.23054 and 4.54894 ..
 {right}(-59.1068,93.2986) .. tension 0.843775 and 6.73346 ..
 {right}(207.341,170.499) .. tension 5.74099 and 1 ..
 {down}(206.026,125.349) ..
 {right}(306.086,75.8102) .. tension 1.13 and 0.913684 ..
(419.56,149.981) .. tension 1 and 2.92953 ..
 {right}(451.185,219.708) .. tension 1 and 1 ..
 {down}(460.648,205.44) .. tension 1 and 12.2986 ..
 {down}(434.606,125) .. tension 1.87963 and 2.10028 ..
 {right}(521.633,89.5135) .. tension 2.41486 and 2.52602 ..
 {right}(589.026,230.472) .. tension 1 and 1 ..
 {down}(597.048,219.759) .. tension 1.34609 and 6.50716 ..
(572.3,116.066) .. tension 0.978555 and 1 ..
z2 join
z1 .. tension 1 and 3.23234 ..
 {left}(543.225,50.5383) .. tension 22.7753 and 2.80295 ..
 {left}(512.153,7.52317) .. tension 2.41485 and 1 ..
(418.981,36.4583) .. tension 1 and 2.31351 ..
 {left}(401.238,49.9541) .. tension 8.59967 and 0.75 ..
 {left}(305.445,-2.68098) .. tension 0.75 and 1 ..
(190.731,71.486) .. tension 1 and 4.03831 ..
 {left}(187.669,81.0185) .. tension 10.1716 and 1 ..
 cycle
;

%%endpaths
%%beginbody
leftbearing := 0;
setCursiveAnchor(seenreh,z3 + (0,15), ExitAnchor);
enddefchar;
defchar(seen.fina,-1,-1,-1,-1);
%%backgroundimage:images/sheen.medi.jpg,-74.4111,-1141.68,1,0,0,1
%%beginparams
z1 = (885.119,52.522);
z2 = (931.174,98.5424);
z3 = (1015.95,-1.20563);

%%beginverbatim

%z2 = z1 + (0,penwidth ) rotated 20;
if right_verticalextratio = 0 : right_verticalextratio := 12; fi
%right_tatweeltension := 1 + righttatweel * 0.215  ;

righttatweel_const := (x3 - x1,y3 - y1);

%%endverbatim

%%beginpaths

fill
(70.6743,-131.721) .. tension 0.75 and 11.5928 ..
 {right}(141.823,94.5532) .. tension 1 and 1 ..
 {down}(147,85.5887) .. tension 1 and 1 ..
 {down}(114,-47) .. tension 1 and 0.82221 ..
 {right}(288,-207) .. tension 1 and 1.60001 ..
 {up}(528.744,-47.865) .. tension 0.75 and 9.60857 ..
 {up}(466.447,116.589) .. tension 3.47442 and 4.30026 ..
 {right}(488.34,194) .. tension 2.14843 and 1.2941 ..
(544.646,99.0282) .. tension 1.43195 and 1 ..
 {right}(602.665,82.5887) .. tension 1 and 0.911937 ..
(712.598,119.177) .. tension 1 and 4.83872 ..
 {right}(769.483,203.522) .. tension 0.75 and 1 ..
 {down}(776.335,191.244) .. tension 1.98298 and 3.66238 ..
 {down}(748.411,108.364) .. tension 1.72515 and 4.30029 ..
 {right}(813.411,68.8038) .. tension 1.37203 and 2.68077 ..
 {right}(916.107,219.719) .. tension 3.85955 and 1.11653 ..
(902,132) .. tension 1 and 1 ..
z2 join
z1 .. tension 1 and 1.33159 ..
 {left}(869.909,61.4601) .. tension 8.39164 and 1.16765 ..
 {left}(794.766,-13.929) .. tension 0.75 and 10000 ..
 {left}(708.894,45.8727) .. tension 6.73744 and 0.75 ..
 {left}(606.835,3.41596) .. tension 1 and 3.91012 ..
 {curl1}(563.579,7.18686) {down} .. tension 1.28876 and 1 ..
(532.086,-141.461) .. tension 1.21366 and 1 ..
 {left}(272.51,-292.535) .. tension 0.943765 and 1 ..
 {up} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(seen.medi,-1,-1,-1,-1);
%%backgroundimage:images/sheen.medi.jpg,-1461.84,-1113.08,1,0,0,1
%%beginparams
z.mark1 = (-40,263);
z1 = (567.362,32.9727);
z11 = (693,12);
z2 = (587.331,100.768);
z3 = (67.8872,4.56659);

%%beginverbatim


%charlt := 4; % numeric

if left_verticalextratio = 0 : left_verticalextratio := 12; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
%left_tatweeltension := 1 + lefttatweel * 0.315  ;
%if left_tatweeltension < 0.75 : left_tatweeltension := 0.75; fi
%if left_tatweeltension > 3 : left_tatweeltension := 3; fi
righttatweel_const := (x11 - x1,y11 - y1);

%z2 = z1 + (penwidth,0) rotated 60;

z4 = z3 + (penwidth,0) rotated 80;
%%endverbatim

%%beginpaths

fill
(107.378,14.7606) .. tension 0.967623 and 1 ..
z3 join
z4 ..
(217.148,169.023) .. tension 1 and 4.86892 ..
 {right}(232.836,197.215) .. tension 1 and 1 ..
 {down}(239.005,189.236) .. tension 0.75 and 6.33946 ..
 {down}(201.307,81.2976) .. tension 1 and 1 ..
 {right}(204.861,75.2315) .. tension 2.80295 and 1.08543 ..
(419.56,149.981) .. tension 1 and 9.68006 ..
 {right}(457.755,217.593) .. tension 1 and 1 ..
 {down}(465.278,207.176) .. tension 1 and 12.2986 ..
 {down}(434.606,125) .. tension 3.68724 and 10000 ..
 {right}(501.736,95.4861) .. tension 1.36092 and 4.02398 ..
 {right}(591.341,235.102) .. tension 1 and 1 ..
 {down}(597.048,219.759) .. tension 1 and 6.50716 ..
(576.968,137.153) .. tension 1 and 1 ..
(574.615,110.858) .. tension 0.978555 and 1 ..
z2 join
z1 .. tension 1 and 3.23234 ..
 {left}(531.651,51.6957) .. tension 2.18124 and 2.80295 ..
 {left}(498.264,11.5741) .. tension 1.33775 and 1.26325 ..
 {left}(389.468,50.3472) .. tension 2.18145 and 1 ..
(333.223,25.6755) .. tension 1 and 1.22044 ..
 {left}(208.502,7.52315) .. tension 1 and 1 ..
(152.099,8.65153) .. tension 1.02042 and 0.75 ..
 {left} cycle
;

%%endpaths
%%beginbody

setMarkAnchor(smallhighwaw,z.mark1);
enddefchar;
beginchar(lam.isol,1604,-1,-1,-1);
%%backgroundimage:images/lam.isol.jpg,-146.417,-1196.67,1,0,0,1

%%beginpaths

fill
(57.7222,78.9167) .. tension 1 and 22.9655 ..
 {right}(117.917,270.583) .. tension 1 and 1 ..
 {down}(125.778,251.139) .. tension 3.23009 and 1 ..
 {down}(94.4292,139.767) .. tension 0.947674 and 1.03981 ..
 {right}(245.917,2.5278) .. tension 0.791682 and 3.57374 ..
 {up}(466.056,112.944) .. tension 3.93885 and 22.6982 ..
(381.693,667.594) .. tension 0.75 and 1 ..
(374.425,707.829) .. tension 1 and 1 ..
 {up}(358.417,757.537) .. tension 1 and 1 ..
(374.09,791.553) .. tension 1 and 1.70946 ..
 {right}(398.522,832.054) .. tension 4.35321 and 1 ..
(424.66,773.134) .. tension 0.937294 and 4.02183 ..
 {down}(461.864,731.818) .. tension 0.75 and 10000 ..
 {down}(423.286,663.688) .. tension 11.1761 and 0.960907 ..
 {down}(493.139,140.722) .. tension 0.933983 and 1 ..
 {left}(233.62,-81.426) .. tension 1 and 0.892519 ..
 {up} cycle
;

%%endpaths
endchar;
defchar(lam.init,-1,-1,-1,-1);
%%backgroundimage:images/lam.init.jpg,-2743.84,-1151.57,1,0,0,1
%%beginparams
z1 = (95.5417,4.47223);
z2 = (125.011,87.5606);

%%beginverbatim
%charlt := 4; % numeric
lefttatweel := charlt +0.0;
%interim left_verticalextratio :=10;
if left_tatweeltension = 0 : left_tatweeltension := 1; fi
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
%lefttatweel_const := (163, y1 -1 );





%%endverbatim

%%beginpaths

fill
z1 join
z2 ..
(150.602,100.12) .. tension 1 and 2.1102 ..
 {up}(165.366,118.63) .. tension 6.60465 and 15.5474 ..
(86.3148,649.949) .. tension 1 and 1 ..
 {up}(63,694) .. tension 2.23645 and 2.62674 ..
 {right}(109.523,778.736) .. tension 3.26377 and 1.33449 ..
(133,720) .. tension 1.34175 and 1.47482 ..
 {down}(171,689.528) .. tension 0.75 and 4.0387 ..
(137.208,615.477) .. tension 14.0434 and 1.47049 ..
 {down}(198.63,171.315) .. tension 2.10317 and 1 ..
(159.722,38.7731) .. tension 0.979627 and 0.923892 ..
 cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(lam.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/lamyeh.jpg,-418.662,-1152.51,1,0,0,1
%%beginparams
z1 = (7,66);

%%beginpaths

fill
(22.7334,57.6389) .. tension 1 and 1.80573 ..
 {up}z1 .. tension 0.931123 and 17.2324 ..
(21.9297,113.67) .. tension 9.35828 and 2.14161 ..
 {up}(145.139,168.056) .. tension 2.76593 and 1 ..
(95.245,441.663) .. tension 1 and 4.71263 ..
(49.6897,661.545) .. tension 3.29886 and 1 ..
 {up}(22.3669,722.734) .. tension 1.75176 and 5.36694 ..
 {right}(59.3171,797.222) .. tension 3.42493 and 1.83084 ..
(84.3171,745.618) .. tension 1.54652 and 1.39398 ..
 {down}(128.36,684.179) .. tension 1 and 1.50717 ..
 {down}(99.4502,641.339) .. tension 11.0116 and 0.824407 ..
(142.667,409.513) .. tension 1 and 1 ..
 {down}(172.332,184.33) .. tension 1 and 1 ..
 {left} cycle
;

%%endpaths
%%beginbody

leftbearing := 0;
rightbearing:=70;


setCursiveAnchor(yehcursive1,z1,ExitAnchor);

enddefchar;
defchar(lam.init.lam_hah,-1,-1,-1,-1);
%%backgroundimage:images/lam_hah.jpg,-2532.89,-1323.26,1,0,0,1
%%beginparams
lefttatweel := 1.14; % numeric
z1 = (113,295);
z3 = (105.33,283.623);
z4 = (301.09,265.392);
z5 = (643.65,229.071);

%%beginverbatim

if left_verticalextratio = 0 : left_verticalextratio := 12; fi
 
lefttatweel := charlt + 0;
z2 = z1 + (0,lineheight) rotated -15;

%%endverbatim

%%beginpaths

fill
z3 .. tension 0.75 and 1 ..
z1 {dir 23} .. tension 1.95607 and 8.30676 ..
 {up}(296.226,318.129) .. tension 2.44495 and 2.44495 ..
(249.945,572.217) .. tension 1 and 7.35794 ..
(222.285,759.25) .. tension 1.66263 and 0.966451 ..
 {up}(194.557,803.303) .. tension 1.88324 and 7.2459 ..
 {right}(240.29,881.804) .. tension 22.6369 and 52.9346 ..
 {down}(301.251,788.925) .. tension 1.17354 and 4.55519 ..
 {down}(266.637,731.489) .. tension 4.87812 and 3.22481 ..
(321.065,451.85) .. tension 1.33352 and 3.40307 ..
 {down}(325.718,330.69) .. tension 2.33188 and 3.30683 ..
z4 .. tension 11.7745 and 3.40192 ..
 {up} cycle
;

%%endpaths
%%beginbody

rightbearing :=  313 + rightbearing;
leftbearing := -190;

%leftanchor := z1;

setCursiveAnchor(lamhah,z1, ExitAnchorRTL);

enddefchar;
defchar(lam.init.beforehahyeh,-1,-1,-1,-1);
%%backgroundimage:images/lamhahyeh.jpg,-2267.66,-1218.06,1,0,0,1
%%beginparams
lefttatweel := 1.14; % numeric
z1 = (-38.4761,9.82253);
z3 = (-72.67,-6.377);
z4 = (88.7371,-19.9622);
z5 = (643.65,229.071);

%%beginverbatim


%%endverbatim

%%beginpaths

fill
z3 .. tension 3.65307 and 1 ..
z1 {dir 13.818} .. tension 2.89027 and 4.08856 ..
 {up}(93.92,36.81) .. tension 2.44495 and 2.44495 ..
(37.644,326.018) .. tension 1 and 7.35794 ..
(12.0864,460.248) .. tension 1.66263 and 0.966451 ..
 {up}(-12.378,499.414) .. tension 1.88324 and 7.2459 ..
 {right}(29.883,580.23) .. tension 5.75232 and 1 ..
(57.532,532.917) .. tension 1 and 12.3329 ..
 {down}(97.402,489.762) .. tension 1.17354 and 4.55519 ..
 {down}(60.859,432.808) .. tension 4.87812 and 3.22481 ..
(93.0712,302.507) .. tension 1.33352 and 3.40307 ..
 {down}(127.865,98.5768) .. tension 2.33188 and 1.66766 ..
z4 .. tension 11.7745 and 3.40192 ..
 {up} cycle
;

%%endpaths
%%beginbody

rightbearing :=  305 + rightbearing;

leftbearing := -120;

setCursiveAnchor(lamhah,z1 , ExitAnchorRTL);

enddefchar;
defchar(lam.init.beforedal,-1,-1,-1,-1);
%%backgroundimage:images/lamdal.jpg,-1174,-1194,1,0,0,1
%%beginparams
z1 = (54.232,-23.037);
z2 = (38.805,41.3897);
z3 = (0,-54);
z4 = (77.195,60.8428);

%%beginverbatim

lefttatweel := charlt +0.0;


which_in_baseline := 3;
lefttatweel_const := z3;
x3 := 0;



%%endverbatim

%%beginpaths

fill
z1 join
z2 ..
 {up}(46.824,61.3985) .. tension 6.60465 and 1 ..
(2.397,303.413) .. tension 1 and 15.5474 ..
(-56.5324,657.532) .. tension 1 and 3.24152 ..
 {up}(-92,715.472) .. tension 2.23645 and 2.62674 ..
 {right}(-50.107,799.63) .. tension 3.26377 and 1.33449 ..
(-34.576,765.467) .. tension 1.34175 and 1.31692 ..
 {down}(5.583,705.792) .. tension 1.60019 and 4.0387 ..
(-16.635,664.148) .. tension 14.0434 and 1.77001 ..
 {down}z4 .. tension 2.10317 and 1 ..
 cycle
;

%%endpaths
%%beginbody

charwd := x4;

enddefchar;
defchar(lam.medi,-1,-1,-1,-1);
%%backgroundimage:images/lam.medi.jpg,-1364.06,-1307.24,1,0,0,1
%%beginparams
z1 = (284.937,45.8811);
z11 = (383,3);
z12 = (178,-28);
z3 = (134.622,14.9279);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt - 0.0;
righttatweel := charrt - 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
y11 := 0;
y12 := y3;
%x12 := x3;
righttatweel_const := (x11 - x1,y11 - y1);
lefttatweel_const := (x12 - x3,y12 - y3);

x1 := x1 + righttatweel * 30;
y1 := y1 - righttatweel * 10;


z2 = z1 + (penwidth,0) rotated 75;

z4 = z3 + (penwidth,0) rotated 70;
%%endverbatim

%%beginpaths

fill
(192.995,49.7508) .. tension 1 and 1 ..
z3 join
z4 ..
 {up}(216.564,178.28) .. tension 1.79974 and 1 ..
(181.202,394.049) .. tension 3.43687 and 13.4898 ..
 {up}(135.417,670.139) .. tension 1.60026 and 2.03022 ..
 {right}(162.678,742.28) .. tension 17.8258 and 1 ..
(223.611,395.139) .. tension 1 and 1.58165 ..
(265.298,184.78) .. tension 1.21643 and 1.46864 ..
z2 join
z1 ..
 {dir 118.639}(225.743,129.339) {dir -100.503} .. tension 1.22321 and 1 ..
 cycle
;

%%endpaths
enddefchar;
defchar(lam.medi.afterkaf,-1,-1,-1,-1);
%%backgroundimage:images/kafinitlammedi.jpg,-2468.03,-1389,1,0,0,1
%%beginparams
z1 = (240.955,14.4784);
z11 = (302.083,-16.7824);
z12 = (208,-27);
z3 = (136.358,16.0853);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt - 0.0;
righttatweel := charrt - 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi

y12 := y3;
%x12 := x3;
righttatweel_const := (x11 - x1,y11 - y1);

lefttatweel_const := (x12 - x3,y12 - y3);

x1 := x1 + righttatweel * 30;
y1 := y1 - righttatweel * 10;


z2 = z1 + (penwidth,0) rotated 75;

z4 = z3 + (penwidth,0) rotated 75;

which_in_baseline := 3;
%%endverbatim

%%beginpaths

fill
(188.365,41.6489) .. tension 1 and 1 ..
z3 join
z4 ..
 {up}(202.407,142.174) .. tension 8.90903 and 1 ..
(174.836,392.313) .. tension 3.43687 and 6.886 ..
 {up}(137.732,700.232) .. tension 1.60026 and 3.58099 ..
 {right}(168.465,805.937) .. tension 17.8258 and 1 ..
(210.301,467.477) .. tension 1 and 2.56192 ..
(248.843,123.843) .. tension 1 and 1.58165 ..
z2 join
z1 ..
 {dir 106.491}(211.275,70.8899) {dir -100.503} .. tension 1.22321 and 1 ..
 cycle
;

%%endpaths
%%beginbody

show leftanchor,rightanchor;
enddefchar;
defchar(lam.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/ala.jpg,-474.601,-1101.55,1,0,0,1
%%beginparams
z1 = (230.076,3.8302);
z3 = (256.944,22.2222);
z4 = (0,-79);

%%beginverbatim
righttatweel := charrt +0;
z2 = z1 + (0,lineheight ) rotated -12;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);


%%endverbatim

%%beginpaths

fill
z4 .. tension 2.18838 and 1 ..
(17.7115,-31.7595) .. tension 1.78306 and 1.20664 ..
(41.046,69.2138) .. tension 1.50391 and 1.86795 ..
 {up}(-24.4585,593.139) .. tension 2.76613 and 9.46491 ..
 {right}(1.47166,667) .. tension 9.62774 and 1.43051 ..
(20.881,572.771) .. tension 4.40733 and 1 ..
(58.6112,350) .. tension 0.75 and 1 ..
(124.188,176.821) .. tension 0.75 and 1.21705 ..
z2 join
z1 .. tension 0.75 and 0.980347 ..
 {dir 119.288}(75.1629,143.734) {dir -88.5156} .. tension 1.23046 and 1.57175 ..
(27.4604,-72.4882) .. tension 1 and 1 ..
 {left}(11.0282,-83.85) .. tension 1.85756 and 3.07581 ..
 {up} cycle
;

%%endpaths
%%beginbody

charwd:= xpart (rightanchor);

setCursiveAnchor(yehcursive1,z4, ExitAnchor);


enddefchar;
defchar(lam.medi.beforeheh,-1,-1,-1,-1);
%%backgroundimage:images/lamkaf.jpg,-4552.97,-1108,1,0,0,1
%%beginparams
z1 = (156.234,11.1588);
z11 = (222.721,5.25656);
z12 = (0,-54);
z3 = (65.5791,0);
z4 = (40.2608,42.5304);

%%beginverbatim

which_in_baseline := 3;


%charlt := 4; % numeric
lefttatweel := charlt + 0.0;
righttatweel := charrt + 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
y11 := 0;

%x12 := x3;
righttatweel_const := (x11 - x1,y11 - y1);
lefttatweel_const := z12;

z2 = z1 + (penwidth,0) rotated 95;


%%endverbatim

%%beginpaths

fill
z3 join
z4 ..
 {up}(53.2603,76.9454) .. tension 3.07244 and 4.80937 ..
 {up}(-46.7045,702.509) .. tension 1.60026 and 2.59865 ..
 {right}(-25.4004,770.405) .. tension 33.8783 and 25.1081 ..
(87.6363,184.433) .. tension 1.21643 and 0.75 ..
z2 join
z1 ..
(123.252,28.6603) .. tension 1 and 1.6801 ..
 {curl1}(79.4053,63.4954) {dir -95.6409} .. tension 0.836762 and 1 ..
 cycle
;

%%endpaths
%%beginbody


%setCursiveAnchor(lam_heh,z12, ExitAnchor);
leftbearing :=0;

tr_:= tr_ shifted (0,-54);


enddefchar;
beginchar(lam.fina,-1,-1,-1,-1);
%%backgroundimage:images/lam.fina.jpg,-727.528,-997,1,0,0,1
%%beginparams
z1 = (610.294,25.6982);
z11 = (723.958,-1.15741);
z3 = (85.5103,13.3402);

%%beginverbatim


%charlt := 4; % numeric

righttatweel := charrt  + 0;
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi


y1 := y1 + righttatweel*2;
x1 := x1 + righttatweel*30;
z2 = z1 + (penwidth,0) rotated 65;

y11 := 0;
righttatweel_const := (x11 - x1,y11 - y1);
%%endverbatim

%%beginpaths

fill
(48.6111,-90.2778) .. tension 1 and 9.24347 ..
 {right}(116.113,118.902) .. tension 1 and 1 ..
 {down}(119.454,111.848) .. tension 1.11576 and 1 ..
 {down}(92.5797,-26.2732) .. tension 0.943093 and 0.873211 ..
 {right}(289.554,-195.358) .. tension 1.15616 and 2.14162 ..
 {up}(519.261,-56.3593) .. tension 2.18589 and 4.51974 ..
 {up}(424.421,648.148) .. tension 1.1566 and 37.074 ..
 {right}(452.894,753.24) .. tension 19.2653 and 0.817189 ..
(524.306,302.083) .. tension 1 and 1 ..
(565.584,178.697) .. tension 1 and 1 ..
z2 join
z1 ..
(568.886,61.6899) .. tension 1 and 5.78232 ..
 {left}(547.917,90.2778) .. tension 9.35072 and 1 ..
 {down}(548.965,-18.0845) .. tension 1.07609 and 1 ..
 {left}(273.611,-273.611) .. tension 1 and 1 ..
 {up} cycle
;

%%endpaths
endchar;
beginchar(lam.fina.afterkaf,-1,-1,-1,-1);
%%backgroundimage:images/kaflam.jpg,-1331,-1271,1,0,0,1
%%beginparams
z1 = (658.244,10.5709);
z11 = (723.958,-1.15741);
z2 = (627.51,105.34);
z3 = (85.5103,13.3402);

%%beginverbatim


%charlt := 4; % numeric

righttatweel := charrt  - 0.3;
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi


y1 := y1 + righttatweel*2;
x1 := x1 + righttatweel*30;


y11 := 0;
righttatweel_const := (x11 - x1,y11 - y1);
%%endverbatim

%%beginpaths

fill
(100.611,-124.278) .. tension 0.781827 and 9.24347 ..
 {right}(167.039,77.2353) .. tension 1 and 1 ..
 {down}(173.081,61.983) .. tension 3.94246 and 0.774482 ..
 {down}(138.78,-28.3952) .. tension 0.75 and 0.83776 ..
 {right}(302.864,-233.552) .. tension 1.15616 and 1.55797 ..
 {up}(533.15,-76.0352) .. tension 2.18589 and 4.51974 ..
 {up}(389.421,670.148) .. tension 1.1566 and 8.88908 ..
 {right}(413.894,732.24) .. tension 19.2653 and 0.817189 ..
(477.195,439.333) .. tension 1 and 1 ..
(550.584,214.697) .. tension 1 and 1 ..
z2 join
z1 ..
 {dir 131.782}(553.435,82.7955) {dir -81.3859} .. tension 2.71316 and 1 ..
 {down}(563.965,-65.0845) .. tension 1.07609 and 1 ..
 {left}(276.611,-316.611) .. tension 1 and 1 ..
 {up} cycle
;

%%endpaths
endchar;
beginchar(meem.isol,1605,-1,-1,-1);
%%backgroundimage:images/meem.isol.jpg,-28.4208,-1032.28,1,0,0,1

%%beginpaths

fill
(58.3333,-59.1667) .. tension 43.3254 and 0.75 ..
(270.544,47.2608) .. tension 1 and 1.93999 ..
(421.667,175) .. tension 3.60745 and 1.32845 ..
 {right}(530.833,339.167) .. tension 1.58938 and 1.68367 ..
 {down}(643.071,160.22) .. tension 1.49551 and 1.41483 ..
 {left}(611.667,77.5) .. tension 1 and 1 ..
(458.623,111.883) .. tension 1 and 4.52521 ..
 {left}(429.167,134.167) .. tension 10000 and 1 ..
(397.135,82.6389) .. tension 1.33087 and 0.75 ..
(265.721,-17.8434) .. tension 1 and 1.46855 ..
 {left}(75,-70) .. tension 1 and 1 ..
 {up} cycle
;
fill
(458.727,211.975) .. tension 4.59561 and 1 ..
 {right}(571.667,165) .. tension 1 and 1 ..
 {up}(590,167.5) .. tension 6.14356 and 2.20122 ..
 {left}(516.713,259.78) .. tension 0.932393 and 5.78647 ..
 {down} cycle
;

%%endpaths
%%beginbody
leftbearing :=- 50;
endchar;
beginchar(meem.isol.ii,-1,-1,-1,-1);
%%backgroundimage:images/meem.isol.ii.jpg,-295.653,-1198.18,1,0,0,1

%%beginpaths

fill
(207.073,219.847) {up} .. tension 0.850581 and 2.44095 ..
 {right}(298.506,347.329) .. tension 1.17705 and 17.0635 ..
 {down}(445.656,207.924) .. tension 1 and 1.87676 ..
 {dir -125.784}(424.107,136.915) {dir -178.781} .. tension 1.6533 and 1.73332 ..
 {down}(168.148,45.8281) .. tension 2.96404 and 1.20427 ..
 {down}(241.232,-431.947) .. tension 1 and 5.57931 ..
 {left}(225.101,-498.607) .. tension 21.1643 and 1.65815 ..
(183.521,-249.993) .. tension 1.63032 and 3.18621 ..
 {up}(138.05,11.4525) .. tension 0.881308 and 0.75 ..
 {dir 8.2577}(365.29,208.45) {dir 115.703} .. tension 2.18133 and 1 ..
 {left}(282.94,265.134) .. tension 0.966248 and 4.43381 ..
 {left}(214.754,204.083) .. tension 1.03949 and 1 ..
 cycle
;

%%endpaths
%%beginbody

endchar;
defchar(meem.init,-1,-1,-1,-1);
%%backgroundimage:images/meem.init.jpg,-1700.28,-1335.38,1,0,0,1
%%beginparams
z1 = (108.791,42.3241);

%%beginverbatim


%charlt := 8; % numeric
%interim left_verticalextratio :=15;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt + 0;


left_tatweeltension := 1 + lefttatweel * 0.115  ;
%if left_tatweeltension = 0 : left_tatweeltension := 1.1; fi


%lefttatweel_const := (0,y1 - 15);



z2 = z1 + (0,penwidth ) rotated -20;

%%endverbatim

%%beginpaths

fill
z1 join
z2 ..
(154.907,147.444) .. tension 1.83772 and 1 ..
(192.115,223.759) .. tension 1 and 1.82148 ..
 {right}(293.97,336.814) .. tension 1.5804 and 1.83398 ..
 {down}(394.078,155.699) .. tension 1.08167 and 5.01752 ..
 {left}(373.227,71.5881) .. tension 0.805722 and 2.61533 ..
 {dir 129.387}(169.546,132.43) {curl1} ..
 cycle
;
fill
(338.816,149.872) {curl1} .. tension 2.36269 and 1.39381 ..
 {left}(265.901,264.779) .. tension 2.00687 and 1.48793 ..
(204.55,201.003) {down} .. tension 4.11654 and 6.5306 ..
 {up} cycle
;

%%endpaths
%%beginbody



%labels(1,2);
enddefchar;
defchar(meem.init.beforehah,-1,-1,-1,-1);
%%backgroundimage:images/meemhah.jpg,-4003.61,-1170,1,0,0,1
%%beginparams
z1 = (-126.067,-55.091);

%%beginverbatim


%%endverbatim

%%beginpaths

fill
z1 .. tension 1.22088 and 1 ..
(105.717,109.346) .. tension 1.83772 and 1 ..
(180.059,194.342) .. tension 1 and 1.3562 ..
 {right}(297.828,342.601) .. tension 1.5804 and 1.83398 ..
 {down}(390.22,159.557) .. tension 1.08167 and 5.01752 ..
 {left}(365.993,71.1058) .. tension 0.75 and 1.65478 ..
 {dir 122.242}(179.674,131.465) {dir -126.316} .. tension 0.75 and 1 ..
(31.8287,12.0563) .. tension 1 and 14.9944 ..
 {up} cycle
;
fill
(337.851,161.928) .. tension 1.95676 and 1.82251 ..
 {left}(281.333,255.134) .. tension 2.00687 and 2.78042 ..
(214.677,201.003) {down} .. tension 4.99275 and 6.5306 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := -200 ;
rightbearing := 80;
setCursiveAnchor(meemhah,z1, ExitAnchorRTL);
enddefchar;
defchar(meem.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/meeminit_yeh.jpg,-458,-1108,1,0,0,1
%%beginparams
z1 = (-84.713,58.838);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
(26.433,99.4633) .. tension 1 and 7.63558 ..
 {up}z1 .. tension 1.57522 and 1 ..
(-50.6816,125.845) .. tension 4.14884 and 1.03908 ..
(131.277,254.449) .. tension 1.83772 and 1 ..
(173.115,341.759) .. tension 1 and 1.41817 ..
 {right}(274.97,454.814) .. tension 1.5804 and 1.83398 ..
 {down}(382.022,270.227) .. tension 1.08167 and 5.01752 ..
 {left}(350.755,197.69) .. tension 0.805722 and 2.61533 ..
 {dir 129.387}(156.912,244.064) {curl1} ..
 cycle
;
fill
(322.71,281.182) {curl1} .. tension 2.36269 and 1.39381 ..
 {left}(250.952,373.52) .. tension 2.00687 and 1.90366 ..
(185.55,317.846) {down} .. tension 6.39374 and 6.5306 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing := 0;

setCursiveAnchor(yehcursive1,z1,ExitAnchor);
enddefchar;
defchar(meem.medi,-1,-1,-1,-1);
%%backgroundimage:images/meem.medi.jpg,-1089,-1541,1,0,0,1
%%beginparams
z1 = (496.396,10.8116);
z11 = (557,3);
z12 = (135.299,22);
z3 = (103.971,14);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := lefttatweel - 0.0;
righttatweel := righttatweel - 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
if left_tatweeltension = 0:
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
fi;
y11 := 0;
y12 := y3;
%x12 := x3;
righttatweel_const := (x11 - x1,y11 - y1);
lefttatweel_const := (x12 - x3,y12 - y3);

z2 = z1 + (penwidth,0) rotated 90;

z4 = z3 + (penwidth,0) rotated 70;

%which_in_baseline := 3;

%%endverbatim

%%beginpaths

fill
(165.217,56.0008) .. tension 1 and 1 ..
z3 join
z4 ..
(168.647,117.169) .. tension 1 and 1 ..
(201.341,157.938) .. tension 1 and 2.03022 ..
 {right}(302.262,253.392) .. tension 1.08595 and 1.58165 ..
(376.409,186.169) .. tension 1.21643 and 1 ..
z2 join
z1 ..
 {curl1}(446.177,36.8038) .. tension 2.34693 and 10.4171 ..
 {left}(428.177,-5.01878) .. tension 1 and 2.79227 ..
(221.747,55.3964) .. tension 1 and 1.6801 ..
 {curl1}(198.66,91.1446) .. tension 2.77486 and 1 ..
 cycle
;
fill
(230,157) .. tension 1.40371 and 6.54598 ..
 {right}(347.646,87.9381) .. tension 1 and 0.873124 ..
 {up}(364,94.3849) .. tension 10000 and 0.75 ..
(323.158,158.058) .. tension 1.4282 and 10.3996 ..
 {left}(294,205) .. tension 1.56086 and 4.27536 ..
 {down} cycle
;

%%endpaths
%%beginbody
%show penangle;
enddefchar;
defchar(meem.medi.afterhah,-1,-1,-1,-1);
%%backgroundimage:images/hahmeem.jpg,-2089,-1386,1,0,0,1
%%beginparams
z1 = (132.421,35.5726);
z3 = (254.53,242.472);

%%beginverbatim


%charlt := 8; % numeric
%interim left_verticalextratio :=15;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt + 0;


left_tatweeltension := 1 + lefttatweel * 0.115  ;
%if left_tatweeltension = 0 : left_tatweeltension := 1.1; fi


%lefttatweel_const := (0,y1 - 15);



z2 = z1 + (0,penwidth ) rotated -20;

%%endverbatim

%%beginpaths

fill
z1 join
z2 ..
(174.623,130.79) .. tension 1.83772 and 1 ..
(198.867,176.98) .. tension 1 and 2.38216 ..
 {right}z3 .. tension 1.58704 and 1.10448 ..
 {down}(392.149,87.7013) .. tension 1.53803 and 1.02193 ..
(381.944,40.9915) .. tension 1.59086 and 1.13839 ..
 {dir 123.926}(182.993,82.0187) {curl1} ..
 cycle
;
fill
(342.192,109.845) {curl1} .. tension 2.36269 and 1.39381 ..
 {left}(269.277,174.115) .. tension 1.52875 and 1.48793 ..
(216.606,141.204) {down} .. tension 5.20857 and 4.56757 ..
 {up} cycle
;

%%endpaths
%%beginbody


setCursiveAnchor(hahmeem,z3,EntryAnchorRTL);
enddefchar;
defchar(meem.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/meemmediyeh.jpg,-1555.54,-1102.8,1,0,0,1
%%beginparams
z1 = (500.44,7.88133);
z11 = (626,2);
z12 = (204.299,21);
z3 = (4.8011,-69.1123);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt - 0.0;
righttatweel := charrt - 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
if left_tatweeltension = 0:
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
fi;
y11 := 0;
y12 := y3;
%x12 := x3;
righttatweel_const := (x11 - x1,y11 - y1);
lefttatweel_const := (x12 - x3,y12 - y3);

z2 = z1 + (penwidth,0) rotated 90;

z4 = z3 + (penwidth,0) rotated 70;
%%endverbatim

%%beginpaths

fill
(168.582,-11.3043) .. tension 1 and 11.234 ..
 {up}z3 ..
(25.0029,-19.0246) .. tension 2.43688 and 0.75 ..
(159.245,39.8456) .. tension 0.75 and 1.76676 ..
 {right}(359.331,215.577) .. tension 2.82523 and 1.58165 ..
(419.757,151.377) .. tension 1.21643 and 10000 ..
z2 join
z1 ..
 {curl1}(493.478,9.2213) .. tension 1.03631 and 2.93508 ..
 {left}(472.557,-26.7499) .. tension 1 and 2.79227 ..
(326.251,-9.89932) .. tension 1 and 1.6801 ..
 {curl1}(258.622,70.7252) .. tension 2.77486 and 1 ..
 cycle
;
fill
(290.325,112.905) .. tension 3.20495 and 8.06402 ..
 {up}(405.72,64.7093) .. tension 3.85484 and 4.89912 ..
 {left}(351.534,159.51) .. tension 0.75 and 1.6549 ..
 {down} cycle
;

%%endpaths
%%beginbody
leftbearing := 0 ;
setCursiveAnchor(yehcursive1,z3,ExitAnchor);

enddefchar;
defchar(meem.fina,-1,-1,-1,-1);
%%backgroundimage:images/meem.fina.ii.jpg,-223.903,-996.583,1,0,0,1
%%beginparams
z.mark1 = (214,-161);
z1 = (413.082,-7.80877);
z2 = (426.335,72.5811);
z3 = (473.958,-14.4676);

%%beginverbatim

%z2 = z1 + (0,penwidth ) rotated 20;
righttatweel_const := (x3 - x1,y3 - y1);

%%endverbatim

%%beginpaths

fill
(182.31,10.9683) .. tension 0.75 and 1 ..
 {up}(163.388,93.0826) .. tension 1.73037 and 1.61706 ..
 {right}(263.574,201.121) .. tension 2.84357 and 1 ..
(339.506,124.904) .. tension 1 and 1 ..
z2 join
z1 .. tension 1 and 1 ..
 {left}(382.667,-1.36019) .. tension 2.0746 and 2.03251 ..
 {left}(363.384,-32.2552) .. tension 1.46691 and 6.1513 ..
 {left}(213.952,-7.0012) .. tension 2.37283 and 5.43489 ..
(197.279,-34.0152) .. tension 1.17645 and 18.3373 ..
 {left}(-91.6645,-201.547) .. tension 1 and 1 ..
 {up}(-108.796,-187.5) .. tension 15.7865 and 1 ..
(-38.1944,-151.042) .. tension 1.73988 and 6.20895 ..
 {up} cycle
;
fill
(253.756,154.485) .. tension 1.78577 and 1.74187 ..
 {down}(189.465,101.606) .. tension 6.28125 and 7.79118 ..
 {up}(311.352,63.4851) .. tension 8.86867 and 3.01664 ..
 {left} cycle
;

%%endpaths
%%beginbody
%%beginbody
leftbearing :=- 50;

setMarkAnchor(buttonmark,z.mark1);

enddefchar;
defchar(meem.fina.afterheh,-1,-1,-1,-1);
%%backgroundimage:images/heh_meem.jpg,-570,-1294,1,0,0,1
%%beginparams
z1 = (-27,75);

%%beginverbatim
righttatweel := charrt + 0.0;


righttatweel_const := (x3 - x1,y3 - y1);

which_in_baseline := 3;
%%endverbatim

%%beginpaths

fill
(-482.279,-322.966) .. tension 13.6564 and 1 ..
(-303.819,-220.872) .. tension 1 and 4.24422 ..
 {up}(-160.114,-91.6348) .. tension 2.44071 and 1 ..
(-174.147,-55.8163) .. tension 1 and 1.93999 ..
 {up}(-180.184,-4.45416) .. tension 1.43668 and 0.779608 ..
 {right}(-65.238,112.019) .. tension 1.58938 and 1.03381 ..
 {down}(53.497,-39.8974) .. tension 1.49551 and 4.76822 ..
 {left}(38.8692,-114.598) .. tension 2.34705 and 0.75 ..
(-64.1454,-118.641) .. tension 1 and 4.52521 ..
 {left}(-129.77,-105.037) .. tension 4.46135 and 0.75 ..
(-178.681,-191.77) .. tension 1.33087 and 0.75 ..
(-271.975,-260.12) .. tension 1 and 3.4651 ..
 {left}(-465.13,-333.317) .. tension 1 and 1 ..
 {up} cycle
;
fill
(-154.223,5.56373) .. tension 4.59561 and 0.813871 ..
 {right}(-16.6879,-37.071) .. tension 1 and 1 ..
 {up}(4.05633,-25.8904) .. tension 6.14356 and 2.20122 ..
 {left}(-105.4,43.7237) .. tension 0.799344 and 5.78647 ..
 {down} cycle
;

%%endpaths
%%beginbody
setCursiveAnchor(hehmeem,z1, EntryAnchor);
setCursiveAnchor(kafmeem,(-65,112), EntryAnchor);
leftbearing :=- 200;
enddefchar;
defchar(meem.fina.afterkaf,-1,-1,-1,-1);
%%beginbody
meem.fina.afterheh_(0,0);
enddefchar;
defchar(meem.fina.ii,-1,-1,-1,-1);
%%backgroundimage:images/meem.fina.ii.alt1.jpg,-4474.23,-1399.15,1,0,0,1
%%beginparams
z1 = (413.082,-7.80877);
z2 = (417.172,70.1698);
z3 = (528.067,-21.7014);

%%beginverbatim

%z2 = z1 + (0,penwidth ) rotated 20;
righttatweel_const := (x3 - x1,y3 - y1);

%%endverbatim

%%beginpaths

fill
(43.6143,-109.691) .. tension 0.75 and 1 ..
(104.166,65.7874) .. tension 1 and 1.34941 ..
 {right}(256.822,195.816) .. tension 1.32087 and 1 ..
(339.988,115.259) .. tension 1 and 1 ..
z2 join
z1 .. tension 1 and 1 ..
 {left}(389.225,-1.26373) .. tension 2.0746 and 2.03251 ..
 {left}(370.907,-51.9311) .. tension 2.25189 and 1.88503 ..
(209.612,-22.0475) .. tension 1 and 5.43489 ..
 {left}(147.126,7.55492) .. tension 1.47039 and 1.94979 ..
 {down}(79.597,-56.9271) .. tension 1 and 1.35851 ..
 {down}(139.045,-521.473) .. tension 1.84582 and 7.39836 ..
 {left}(118.728,-621.49) .. tension 16.2078 and 1 ..
(98.31,-454.372) .. tension 1.33978 and 1.4148 ..
 {up} cycle
;
fill
(259.929,143.104) .. tension 1.48938 and 2.46238 ..
 {down}(166.992,91.3819) .. tension 6.28125 and 7.79118 ..
 {up}(325.048,43.4233) .. tension 6.1579 and 1 ..
(294.483,86.1594) .. tension 1 and 3.22298 ..
 {left} cycle
;

%%endpaths
%%beginbody

enddefchar;
beginchar(meem.fina.ii.alt1,-1,-1,-1,-1);
%%backgroundimage:images/meem.fina.jpg,-1500.4,-1317.38,1,0,0,1
%%beginparams
z1 = (413.082,-7.80877);
z2 = (426.335,72.5811);
z3 = (528.067,-21.7014);

%%beginverbatim
righttatweel := charrt + 0;
%z2 = z1 + (0,penwidth ) rotated 20;
righttatweel_const := (x3 - x1,y3 - y1);

%%endverbatim

%%beginpaths

fill
(59.0464,-160.328) .. tension 0.75 and 1 ..
(120.563,65.3051) .. tension 1 and 1.61706 ..
 {right}(263.574,201.121) .. tension 2.84357 and 1 ..
(339.506,124.904) .. tension 1 and 1 ..
z2 join
z1 .. tension 1 and 1 ..
 {left}(395.977,-3.675) .. tension 2.0746 and 2.03251 ..
 {left}(370.907,-51.9311) .. tension 2.25189 and 1.88503 ..
(216.846,-22.0475) .. tension 1 and 5.43489 ..
 {left}(162.558,12.8597) .. tension 1.47039 and 1.94979 ..
 {down}(86.3485,-67.5367) .. tension 1 and 1.17541 ..
 {down}(155.442,-599.116) .. tension 1 and 7.39836 ..
 {left}(139.465,-687.559) .. tension 16.2078 and 1 ..
(104.097,-450.996) .. tension 1 and 1.11487 ..
 {up} cycle
;
fill
(250.284,146.962) .. tension 1.78577 and 1.74187 ..
 {down}(187.729,89.4529) .. tension 6.28125 and 7.79118 ..
 {up}(321.19,38.6008) .. tension 6.1579 and 1 ..
(291.107,84.7126) .. tension 1 and 5.51416 ..
 {left} cycle
;

%%endpaths
%%beginbody

endchar;
beginchar(noon.isol,1606,-1,-1,-1);
%%backgroundimage:images/noon.isol.jpg,-355.34,-1736.05,1,0,0,1
%%beginparams
z11 = (183,373);
z2 = (491,229);

%%beginpaths

fill
(272.286,-126.676) .. tension 0.826526 and 0.960362 ..
 {up}(39.3269,61.3558) .. tension 1.35039 and 4.7386 ..
 {right}(101.442,260.84) .. tension 0.75 and 1.12749 ..
 {down}(107.161,252.556) .. tension 5.01017 and 0.75 ..
 {down}(76.5,94.138) .. tension 1.30132 and 0.880385 ..
 {right}(262.573,-39.632) .. tension 1.0148 and 1.64161 ..
 {up}(501.703,98.6751) .. tension 0.893232 and 7.19193 ..
 {up}(412.092,304.74) .. tension 6.32194 and 8.80145 ..
 {right}(434.597,379.441) .. tension 19.7266 and 0.805273 ..
 {down}(529.16,146.88) .. tension 1.13331 and 0.956441 ..
(479.514,-20.304) .. tension 0.985502 and 1.18217 ..
 {left} cycle
;

%%endpaths
%%beginbody
draw one_dot.pic  shifted z11;


setCursiveAnchor(behnoon,z2, EntryAnchorRTL);
endchar;
defchar(noon.fina,-1,-1,-1,-1);
%%backgroundimage:images/noon.fina.jpg,-107.196,-1167.59,1,0,0,1
%%beginparams
z1 = (547.232,84.2321);
z11 = (210,243);
z2 = (566.857,-7.65763);
z3 = (617.05,77.8638);

%%beginverbatim
righttatweel := charrt -0.2;
%z2 = z1 + (0,penwidth ) rotated 20;
righttatweel_const := (x3 - x1,y3 - y1);

%%endverbatim

%%beginpaths

fill
(249.98,-310.843) .. tension 0.789758 and 1 ..
 {up}(48,-116.325) .. tension 0.75 and 7.66316 ..
 {right}(128.887,105.589) .. tension 0.75 and 1.12749 ..
 {down}(134.397,96.1424) .. tension 5.01017 and 0.75 ..
 {down}(86.858,-78.3812) .. tension 1.40455 and 0.768469 ..
 {right}(274.503,-223.985) .. tension 1.32949 and 1.40115 ..
 {up}(512.731,-81.3249) .. tension 0.893232 and 7.19193 ..
 {up}(439.092,124.74) .. tension 6.32194 and 8.80145 ..
 {right}(459.923,198.436) .. tension 19.7266 and 1 ..
(499.835,130.013) .. tension 1 and 1 ..
z1 join
z2 .. tension atleast 0.87609 ..
 {dir 131.222}(542.858,5.98354) {curl1} .. tension 2.6502 and 1.14803 ..
 {down}(543.709,-45.7184) .. tension 1.13331 and 1.13195 ..
(497.561,-195.588) .. tension 1.11747 and 1.18217 ..
 {left} cycle
;

%%endpaths
%%beginbody
draw one_dot.pic  shifted z11;

show rightanchor;

enddefchar;
defchar(noon.fina.afterbeh,-1,-1,-1,-1);
%%backgroundimage:images/behmedinoon.jpg,-16.5117,-931.766,1,0,0,1
%%beginparams
z1 = (213,308);
z2 = (503,149);

%%beginverbatim
righttatweel := charrt -0.5;
%z2 = z1 + (0,penwidth ) rotated 20;


%%endverbatim

%%beginpaths

fill
(294.589,-167.904) .. tension 0.826526 and 0.960362 ..
 {up}(66.7847,-3.93165) .. tension 1.35039 and 6.06284 ..
 {right}(124.467,194.809) .. tension 0.75 and 1.12749 ..
 {down}(130.145,185.824) .. tension 5.01017 and 0.75 ..
 {down}(98.7905,37.711) .. tension 1.30132 and 0.880385 ..
 {right}(279.85,-89.8314) .. tension 1.0148 and 1.25059 ..
 {up}(508.53,55.2903) .. tension 0.830009 and 7.19193 ..
 {up}(455.371,216.14) .. tension 6.32194 and 4.82685 ..
 {right}(465.642,296.608) .. tension 19.7266 and 0.805273 ..
 {down}(542.109,68.4587) .. tension 1.13331 and 0.852813 ..
(498.25,-80.2068) .. tension 0.985502 and 1.18217 ..
 {left} cycle
;

%%endpaths
%%beginbody
draw one_dot.pic  shifted z1;

setCursiveAnchor(behnoon,z2, EntryAnchorRTL);
setCursiveAnchor(behmedinoon,z2, EntryAnchor);

enddefchar;
defchar(qaf.isol,1602,-1,-1,-1);
%%backgroundimage:images/kaf.jpg,-342.667,-1784.22,1,0,0,1
%%beginparams
z11 = (329.861,526.042);

%%beginpaths

fill
(66.7361,109.783) .. tension 0.75 and 5.5838 ..
 {right}(146.223,318.816) .. tension 1 and 1 ..
 {down}(151.132,309.991) .. tension 2.49598 and 1 ..
 {down}(108.421,174.685) .. tension 1 and 1 ..
 {right}(287.539,8.26929) .. tension 1.08626 and 1.8148 ..
 {up}(606.528,164.708) .. tension 1 and 2.26133 ..
 {left}(588,244) .. tension 33.0823 and 0.75 ..
 {left}(505.553,185.553) .. tension 1 and 1 ..
 {up}(421,264) .. tension 1 and 1.97716 ..
 {right}(502.894,444.518) .. tension 1.57954 and 1.08318 ..
 {down}(640.169,203.087) .. tension 0.957641 and 1.02927 ..
(554.109,14.9498) .. tension 1.28094 and 1.01553 ..
 {left}(283.972,-76.6906) .. tension 0.983264 and 1 ..
 {up} cycle
;
fill
(494.792,355.903) .. tension 1.00346 and 2.4594 ..
 {down}(456.694,311.883) .. tension 1.59598 and 0.837748 ..
 {right}(512.635,267.65) .. tension 1 and 2.1614 ..
 {up}(563.754,291.763) .. tension 2.69079 and 1 ..
 {left} cycle
;

%%endpaths
%%beginbody




draw two_dots.pic  shifted z11;
savepicture:=1;
enddefchar;
beginchar(feh.isol,1601,-1,-1,-1);
%%backgroundimage:images/feh.isol.jpg,-126.477,-1060.16,1,0,0,1
%%beginparams
z11 = (477.431,464.699);

%%beginpaths

fill
(41.6667,90.2778) .. tension 1 and 7.26853 ..
 {right}(111.13,291.205) .. tension 1 and 1 ..
 {down}(115.972,284.722) .. tension 3.43789 and 1 ..
 {down}(85.8949,168.744) .. tension 0.992238 and 0.865462 ..
 {right}(334.686,-3.24943) .. tension 0.848078 and 4.55809 ..
 {up}(730.556,133.333) .. tension 1 and 6.76413 ..
 {left}(709.719,204.406) .. tension 2.93102 and 1 ..
(684.722,156.25) .. tension 1.18048 and 1 ..
 {left}(619.444,130.556) .. tension 1 and 1.57995 ..
 {up}(542.361,193.507) .. tension 0.75 and 3.25279 ..
 {right}(618.069,387.969) .. tension 1.80127 and 0.997829 ..
 {down}(759.496,184.395) .. tension 0.75 and 1 ..
(611.111,-20.8333) .. tension 1 and 1 ..
 {left}(301.76,-84.3762) .. tension 0.75 and 1.66484 ..
 {up} cycle
;
fill
(678.472,238.194) .. tension 2.09429 and 1.4754 ..
 {left}(611.111,303.472) .. tension 3.97112 and 1.09701 ..
 {down}(574.535,249.066) .. tension 1.10587 and 1.3085 ..
 {right}(627.336,210.4) .. tension 1 and 1.76085 ..
 {up} cycle
;

%%endpaths
%%beginbody




draw one_dot.pic  shifted z11;
savepicture:=1;
endchar;
defchar(qaf.fina,-1,-1,-1,-1);
%%backgroundimage:images/qaf.fina.jpg,-150.103,-969.206,1,0,0,1
%%beginparams
z1 = (620.765,-0.34572);
z11 = (265.641,332.755);
z3 = (655.377,0);
z4 = (602.635,88.6661);

%%beginverbatim
righttatweel := righttatweel +0;
z2 = z1 + (0,lineheight );
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);
y4 := lineheight;

%%endverbatim

%%beginpaths

fill
(52.7778,-122.917) .. tension 1 and 6.41744 ..
 {right}(117.361,100.694) .. tension 1 and 1 ..
 {down}(123.611,92.3611) .. tension 1 and 1 ..
 {down}(90.9722,-52.0833) .. tension 1 and 1 ..
 {right}(271.528,-209.028) .. tension 0.75 and 5.38906 ..
 {up}(594.444,-50) .. tension 1 and 4.47302 ..
 {left}(583.353,-0.0964148) .. tension 1.51898 and 1 ..
 {left}(483.141,-1.21523) .. tension 0.890434 and 0.75 ..
 {up}(386.284,87.4421) .. tension 1 and 2.12774 ..
 {right}(477.894,243.479) .. tension 1.28201 and 4.69626 ..
 {down}z4 {right} .. tension 1 and 2.71347 ..
 {right}z2 join
z1 {curl1} .. tension 1.43469 and 1 ..
(588.348,-138.406) .. tension 1.20455 and 1 ..
 {left}(283.333,-288.889) .. tension 1 and 1 ..
 {up} cycle
;
fill
(421.981,114.408) .. tension 2.12429 and 1 ..
 {right}(503.243,77.5419) .. tension 1 and 1.61967 ..
 {up}(538.194,86.1111) .. tension 2.70198 and 1.8007 ..
 {left}(465.278,159.015) .. tension 1.84946 and 2.30145 ..
 {down} cycle
;

%%endpaths
%%beginbody
drawcomp(two_dots,identity shifted z11);
enddefchar;
defchar(fehshape.init,-1,-1,-1,-1);
%%backgroundimage:images/feh.init.jpg,-2338.99,-1185.34,1,0,0,1
%%beginparams
z1 = (37.6792,3.86655);
z2 = (50.2739,88.2608);

%%beginverbatim
%charlt := 4; % numeric
lefttatweel := charlt + 0.7;

%%endverbatim

%%beginpaths

fill
(171.228,73.3526) .. tension 1 and 1 ..
z1 join
z2 ..
(98.84,99.6095) .. tension 1 and 1.18279 ..
 {up}(164.458,156.701) .. tension 1 and 14.8838 ..
 {left}(141.493,221.035) .. tension 34.5134 and 0.885978 ..
 {left}(65.3511,148.894) .. tension 0.831576 and 2.08491 ..
 {up}(-21.9183,236.575) .. tension 0.798746 and 2.70287 ..
 {right}(63.0108,410.447) .. tension 1.69552 and 1.1401 ..
 {down}(198.245,187.461) .. tension 1 and 1.13522 ..
 cycle
;
fill
(20.5285,273.035) .. tension 1 and 1 ..
 {right}(59.9753,235.553) .. tension 1 and 1.19098 ..
 {up}(114.975,263.553) .. tension 1.66075 and 1.53984 ..
 {left}(58.4576,319.035) .. tension 1.60243 and 1 ..
 {down} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(fehshape.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/fi.jpg,-417.778,-1448.53,1,0,0,1
%%beginparams
z1 = (0,90);

%%beginpaths

fill
(-7.01408,334.775) .. tension 0.895824 and 2.08468 ..
 {right}(80.0592,529.351) .. tension 1.3742 and 1.29636 ..
 {down}(208.325,297.936) .. tension 1.08046 and 52.2617 ..
 {up}z1 .. tension 1.32883 and 5.07289 ..
 {right}(23.6625,151.317) .. tension 0.899958 and 2.24118 ..
 {up}(179.189,232.296) .. tension 1.40417 and 6.24416 ..
 {left}(158.76,320.039) .. tension 4.91627 and 0.753809 ..
 {left}(73.6962,261.097) .. tension 1 and 1.29391 ..
 {up} cycle
;
fill
(25.1361,389.028) .. tension 0.915561 and 1.38577 ..
 {right}(81.0642,342.478) .. tension 1 and 1.44268 ..
 {up}(137.997,379.316) .. tension 5.6761 and 1.4191 ..
 {left}(76.0412,445.291) .. tension 1.36248 and 1.24129 ..
 {down} cycle
;

%%endpaths
%%beginbody
charwd:= xpart (urcorner currentpicture) - x1 + rightbearing; 

setCursiveAnchor(yehcursive1,z1,ExitAnchor);
enddefchar;
defchar(fehshape.medi,-1,-1,-1,-1);
%%backgroundimage:images/feh.medi.jpg,-923,-1016,1,0,0,1
%%beginparams
z1 = (293.435,27.8949);
z3 = (76.8991,7.37626);

%%beginverbatim

%lefttatweel := lefttatweel - 0.6;

if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
%left_tatweeltension := 1 + charlt * 0.115  ;
%if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
%righttatweel_const := (120,-20);

right_tatweeltension := 1;

z2 = z1 + (penwidth,0) rotated 70;

righttatweel_const := (150,lineheight - y2);

z4 = z3 + (penwidth,0) rotated 70;
%%endverbatim

%%beginpaths

fill
(127.938,19.6905) .. tension 1 and 1 ..
z3 join
z4 ..
(134.797,98.3504) .. tension 1 and 6.49127 ..
 {up}(147.183,107.812) .. tension 1.6364 and 0.75 ..
 {up}(132.077,157.309) .. tension 1.06773 and 1.73226 ..
 {right}(283.218,379.507) .. tension 1.45603 and 1 ..
 {down}(354.594,267.925) .. tension 0.75 and 1.37172 ..
(328.831,156.686) .. tension 1 and 2.80352 ..
 {down}(302.183,124.812) .. tension 1.45172 and 0.75 ..
z2 join
z1 ..
(227.56,54.6592) .. tension 1 and 2.80466 ..
 {left}(216.183,60.8117) .. tension 17.2554 and 1 ..
 cycle
;
fill
(264.112,291.777) .. tension 1 and 4.53552 ..
 {down}(169.006,201.954) .. tension 1.74653 and 3.21958 ..
 {right}(242.239,149.33) .. tension 3.25314 and 1.71963 ..
 {up}(321.183,216.812) .. tension 2.02529 and 1 ..
 {left} cycle
;

%%endpaths
enddefchar;
defchar(fehshape.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/fehshapeyeh.jpg,-470.797,-1157.57,1,0,0,1
%%beginparams
z1 = (349.461,-3.88807);
z3 = (18.2369,-11.814);

%%beginverbatim




if right_verticalextratio = 0 : right_verticalextratio := 10; fi
righttatweel_const := (40,y1);

z2 = z1 + (penwidth,0) rotated 68;


%%endverbatim

%%beginpaths

fill
(114.996,10.5204) .. tension 1 and 9.24615 ..
 {up}z3 ..
(63.8802,49.1693) .. tension 3.13837 and 4.35001 ..
 {up}(181.461,105.339) .. tension 1.6364 and 0.75 ..
 {up}(153.295,169.572) .. tension 1.04 and 2.33375 ..
 {right}(288.36,375.025) .. tension 2.27443 and 1.11578 ..
 {down}(365.43,263.443) .. tension 0.906482 and 1.37172 ..
(334.308,148.52) .. tension 1 and 4.59674 ..
 {down}(299.791,110.777) .. tension 3.17615 and 1 ..
z2 join
z1 ..
(253.573,38.0075) .. tension 1 and 11.8566 ..
 {left}(219.174,64.3894) .. tension 33.0257 and 1 ..
 cycle
;
fill
(267.915,283.611) .. tension 2.45365 and 4.53552 ..
 {down}(186.874,207.626) .. tension 1.74653 and 5.40707 ..
 {right}(246.859,142.611) .. tension 4.8715 and 2.26396 ..
 {up}(328.295,213.067) .. tension 2.02529 and 1.09837 ..
 {left} cycle
;

%%endpaths
%%beginbody


setCursiveAnchor(yehcursive1,z3,ExitAnchor);
enddefchar;
defchar(feh.fina,-1,-1,-1,-1);
%%backgroundimage:images/feh.fina.jpg,-8.88889,-1206.69,1,0,0,1
%%beginparams
z1 = (889.348,1.02336);
z11 = (618,384);
z2 = (855.299,88.817);
z3 = (957.272,0);

%%beginverbatim
righttatweel := righttatweel +0;
%z2 = z1 + (0,penwidth ) rotated 20;
righttatweel_const := (x3 - x1,y3 - y1);

%%endverbatim

%%beginpaths

fill
(56,92) .. tension 1 and 4.4706 ..
 {right}(122.73,267.654) .. tension 1 and 1 ..
 {down}(130.556,262.5) .. tension 1.49403 and 1 ..
 {down}(94.4444,156.582) .. tension 0.856535 and 2.00403 ..
 {right}(335.379,6.06674) .. tension 1 and 7.23208 ..
 {up}(665.278,79.1667) .. tension 1 and 1 ..
 {up}(654.861,111.805) .. tension 0.875867 and 3.45127 ..
 {right}(776.717,329.167) .. tension 1.13647 and 1.70119 ..
 {down}(858.591,206.035) .. tension 1.02457 and 13.8965 ..
 {down}(828.839,103.51) .. tension 1.8494 and 1 ..
z2 join
z1 .. tension 1.03454 and 2.527 ..
 {left}(731.582,31.9091) .. tension 8.82087 and 0.75 ..
 {left}(336.111,-69.4445) .. tension 0.878883 and 1.01539 ..
 {up} cycle
;
fill
(759.722,244.444) .. tension 3.02486 and 2.54224 ..
 {down}(687.5,161.111) .. tension 1.67778 and 1.42114 ..
 {right}(749.305,115.278) .. tension 1.16368 and 1.63001 ..
 {up}(819.444,165.278) .. tension 2.08776 and 1 ..
 {left} cycle
;

%%endpaths
%%beginbody
drawcomp(one_dot,identity shifted z11);
enddefchar;
defchar(hah.isol,1581,-1,-1,-1);
%%backgroundimage:images/hah.isol.jpg,-130.833,-1156.69,1,0,0,1
%%beginparams
z1 = (642.295,370.279);
z2 = (933.189,-404.023);

%%beginpaths

fill
(120.833,-192.361) .. tension 1.27262 and 1.06096 ..
(265.574,223.377) .. tension 1 and 10.5011 ..
 {up}(353.472,300) .. tension 5.32831 and 1.84291 ..
(175.152,282.989) .. tension 1 and 1.66749 ..
 {left}(152.778,265.972) .. tension 1 and 1 ..
 {up}(148.537,278.113) .. tension 2.51385 and 0.802395 ..
 {right}(275.089,397.394) .. tension 8.88517 and 0.75 ..
 {right}(444.444,375) .. tension 0.75 and 4.05516 ..
 {right}(622.917,391.667) .. tension 1 and 1 ..
 {down}z1 .. tension 1 and 1.37031 ..
(596.404,315.712) {curl1} .. tension 1.07061 and 1 ..
(410.744,267.436) .. tension 0.825647 and 1.11217 ..
 {down}(162.476,-138.912) .. tension 1 and 0.804677 ..
 {right}(599.2,-423.636) .. tension 0.75 and 1.13911 ..
 {right}(873.153,-404.812) .. tension 1 and 6.548 ..
 {down}z2 .. tension 0.75 and 0.75 ..
(892.168,-428.669) .. tension 1.35123 and 1.29479 ..
(725.485,-475.575) .. tension 1 and 0.75 ..
 {left}(518.75,-502.083) .. tension 0.850755 and 1 ..
 {up} cycle
;

%%endpaths
%%beginbody
rightbearing := rightbearing - (x2-x1);
enddefchar;
defchar(hah.isol.thuluth,-1,1,-1,-1);
%%backgroundimage:D:/hah.PNG,-45.9743,-1372.77,1,0,0,1

%%beginpaths

fill
(62,274) .. tension 1.81948 and 0.75 ..
(446.38,941.196) .. tension 1.00217 and 250.334 ..
 {up}(808.472,1132) .. tension 19.5195 and 11.7295 ..
 {down}(120.886,1149.95) .. tension 4.93172 and 1 ..
(307.302,1001.85) .. tension 1 and 0.984943 ..
(497.048,772.257) .. tension 1 and 0.75 ..
 {down}(611.154,379.169) .. tension 1 and 14.054 ..
 {left}(556.644,74.7546) .. tension 10.1483 and 1 ..
(541.464,363.037) .. tension 1 and 1 ..
(390.228,768.732) .. tension 1.51488 and 1.03468 ..
(190.681,949.781) .. tension 1.23024 and 1.05703 ..
 {up}(43.8611,1135) .. tension 1 and 1.20852 ..
 {right}(229.376,1311.09) .. tension 1.40093 and 14.6626 ..
 {down}(1081.53,1247.97) .. tension 2.7312 and 0.75 ..
 {dir -112.515}(1040.6,1113.18) {dir -138.689} .. tension 4.60723 and 1.20845 ..
(397.917,793.611) .. tension 0.940882 and 1.12302 ..
 {down}(118.083,229.056) .. tension 0.801522 and 0.973788 ..
 {right}(498.417,-119.694) .. tension 0.825672 and 2.36742 ..
 {up}(1132.98,287.499) .. tension 0.75 and 3.15898 ..
(939.253,487.702) .. tension 1.24386 and 1.44277 ..
(542.447,-116.031) .. tension 1.10719 and 5.57337 ..
 {left}(514.417,-225.012) .. tension 3.04298 and 1 ..
(500.361,-28.5554) .. tension 3.70679 and 0.953115 ..
 {right}(956.291,635.41) .. tension 0.929823 and 0.961634 ..
 {down}(1185.59,365.29) .. tension 1.12146 and 1.12348 ..
 {left}(529.098,-276.009) .. tension 0.75 and 0.83858 ..
 {up} cycle
;
fill
(89.1346,844.661) .. tension 1.99951 and 1.96509 ..
 {right}(141.606,899.329) .. tension 2.43044 and 2.08333 ..
 {down}(193.036,845.293) .. tension 2.5188 and 2.65227 ..
 {left}(140.258,792.096) .. tension 2.25157 and 2.02897 ..
 {up} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(hah.init,-1,-1,-1,-1);
%%backgroundimage:images/jeem.init.jpg,-1527.71,-1022.26,1,0,0,1
%%beginparams
z1 = (79.6351,9.36419);

%%beginverbatim
%charlt := 4; % numeric
lefttatweel := lefttatweel -  0.0;
%interim left_verticalextratio :=10;
if left_tatweeltension = 0 : left_tatweeltension := 1; fi
if left_verticalextratio = 0 : left_verticalextratio := 8; fi
%lefttatweel_const := (163, y1 -1 );



z2 = z1 + (0,penwidth ) rotated -15;

%%endverbatim

%%beginpaths

fill
(255.787,74.6527) .. tension 1 and 1 ..
z1 join
z2 ..
(193.676,118.845) .. tension 1 and 6.34228 ..
 {up}(321.201,181.775) .. tension 83.2245 and 1.63017 ..
 {left}(167.824,230.324) .. tension 0.93697 and 2.31909 ..
(75.7784,212.606) .. tension 1 and 2.50292 ..
 {left}(39.4548,184.233) .. tension 0.882523 and 1 ..
 {up}(35.3118,194.235) .. tension 1.45052 and 1.3939 ..
 {right}(175.975,325.714) .. tension 1.91368 and 0.841609 ..
(323.913,260.015) .. tension 1 and 2.30651 ..
 {right}(549.866,220.183) .. tension 1 and 2.3242 ..
 {right}(632.823,226.015) .. tension 1 and 1 ..
 {down}(637.731,221.065) .. tension 5.83816 and 4.05417 ..
 {left}(593.75,135.995) .. tension 1.26894 and 1 ..
 {left}(447.803,154.616) .. tension 1 and 1 ..
 cycle
;

%%endpaths
enddefchar;
defchar(hah.init.ii,-1,-1,-1,-1);
%%backgroundimage:images/hah.init.ii.jpg,-1743,-1342,1,0,0,1
%%beginparams
z1 = (47.8224,2.54835);

%%beginverbatim
%charlt := 4; % numeric
lefttatweel := charlt + 0;
%interim left_verticalextratio :=10;
if left_tatweeltension = 0 : left_tatweeltension := 1; fi
if left_verticalextratio = 0 : left_verticalextratio := 8; fi
%lefttatweel_const := (163, y1 -1 );



z2 = z1 + (0,penwidth ) rotated -15;

%%endverbatim

%%beginpaths

fill
(243.981,78.8194) .. tension 1 and 1 ..
z1 join
z2 ..
(193.676,118.845) .. tension 1 and 6.34228 ..
 {up}(258.91,150.429) .. tension 14.4266 and 1.63017 ..
 {left}(95.0842,199.781) .. tension 0.93697 and 2.31909 ..
(-27.906,121.38) .. tension 1 and 2.50292 ..
 {left}(-52.977,88.1843) .. tension 0.882523 and 1 ..
 {up}(-57.5219,98.99) .. tension 1.45052 and 1.3939 ..
 {right}(112.478,271.059) .. tension 1.62047 and 0.841609 ..
(333.558,209.378) .. tension 1 and 2.30651 ..
 {right}(429.453,203.043) .. tension 1 and 2.3242 ..
 {right}(490.02,213.835) .. tension 1 and 1 ..
 {down}(495.466,204.186) .. tension 5.83816 and 4.05417 ..
 {left}(457.639,132.523) .. tension 1.26894 and 1 ..
 {left}(402.765,134.585) .. tension 0.75 and 1 ..
 cycle
;

%%endpaths
enddefchar;
defchar(behshape.medi.beforenoon,-1,-1,-1,-1);
%%backgroundimage:images/behmedinoon.jpg,-576,-751.579,1,0,0,1
%%beginparams
z1 = (273.876,-176.505);
z3 = (39.156,8.0977);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt + 0;
righttatweel := charrt + 0.0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
righttatweel_const := (100,-20);
which_in_baseline := 3;
z2 = z1 + (penwidth,0) rotated 80;

z4 = z3 + (penwidth,0) rotated 83;
%%endverbatim

%%beginpaths

fill
(141.32,123.823) {dir -101.725} ..
z3 join
z4 .. tension 0.932615 and 2.98102 ..
(96.4041,126.918) .. tension 1.7264 and 2.82674 ..
 {right}(144.44,227.515) .. tension 2.98366 and 1 ..
(189.555,104.422) .. tension 1 and 1.21844 ..
(240.741,-56.713) .. tension 1 and 1.37056 ..
z2 join
z1 ..
(195.727,-88.8053) .. tension 2.28647 and 1.24656 ..
 {dir 100.584} cycle
;

%%endpaths
%%beginbody

%charwd:= xpart rightanchor - xpart leftanchor;    

%pair leftanchor;
%leftbearing := 0;

currentpicture := currentpicture shifted (0,- ypart rightanchor); 
setCursiveAnchor(behmedinoon,(0,- ypart rightanchor), ExitAnchor);
rightanchor := rightanchor  shifted (0,- ypart rightanchor);





enddefchar;
defchar(hah.init.beforemeem,-1,-1,-1,-1);
%%backgroundimage:images/hahmeem.jpg,-2480.94,-1300.47,1,0,0,1
%%beginparams
z1 = (74.3146,159.432);

%%beginverbatim
%charlt := 4; % numeric
lefttatweel := charlt + 0;
%interim left_verticalextratio :=10;
if left_tatweeltension = 0 : left_tatweeltension := 1; fi
if left_verticalextratio = 0 : left_verticalextratio := 8; fi
%lefttatweel_const := (163, y1 -1 );



z2 = z1 + (0,penwidth ) rotated -15;

%%endverbatim

%%beginpaths

fill
(68.1029,125.765) .. tension 0.75 and 1.85096 ..
 {dir 44.5871}z1 .. tension 5.96621 and 20.5856 ..
 {up}(294.846,196.981) .. tension 8.26438 and 1 ..
 {left}(131.893,242.812) .. tension 0.75 and 2.23585 ..
 {left}(42.2898,211.697) .. tension 1 and 1 ..
 {up}(39.3224,221.093) .. tension 2.40221 and 0.75 ..
 {right}(166.433,330.666) .. tension 9.87965 and 1 ..
(351.563,241.127) .. tension 1 and 1.98627 ..
 {right}(532.772,195.36) .. tension 1 and 1 ..
 {right}(561.065,195.158) .. tension 1 and 1 ..
 {down}(570.923,186.149) .. tension 3.34991 and 5.82314 ..
 {left}(525.1,114.486) .. tension 3.18506 and 1 ..
(496.721,123.457) .. tension 2.41416 and 0.827159 ..
 {left}(339.524,148.061) .. tension 2.20572 and 5.13197 ..
 {left}(98.553,107.585) .. tension 1.33992 and 4.68736 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing := -170;
setCursiveAnchor(hahmeem,z1,ExitAnchorRTL);
enddefchar;
defchar(hah.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/hahyeh.jpg,-1417,-1109,1,0,0,1
%%beginparams
z1 = (0,30.8629);

%%beginpaths

fill
z1 .. tension 1.06971 and 1 ..
(97.6856,134.687) .. tension 1 and 16.001 ..
 {up}(297.525,207.698) .. tension 3.74612 and 1 ..
 {left}(170.071,249.845) .. tension 0.75 and 2.23585 ..
 {left}(43.6294,211.362) .. tension 1 and 1 ..
 {up}(35.9466,222.54) .. tension 2.40221 and 0.823915 ..
 {right}(166.446,338.408) .. tension 2.04653 and 1 ..
 {right}(506.958,218.066) .. tension 1 and 1 ..
 {right}(579.819,223.624) .. tension 1 and 1 ..
 {down}(585.659,211.266) .. tension 3.34991 and 1.8585 ..
 {left}(541.149,146.984) .. tension 1 and 1 ..
 {left}(475.613,160.68) .. tension 2.20572 and 1 ..
(232.619,129.769) .. tension 1 and 27.5278 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftanchor := z1;
setCursiveAnchor(yehcursive1,z1,ExitAnchor);
enddefchar;
defchar(hah.medi,-1,-1,-1,-1);
%%backgroundimage:images/heh.medi.jpg,-872.518,-1173,1,0,0,1
%%beginparams
z1 = (460,38);
z11 = (543,-2);
z3 = (85,13);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt + 0.3;
%which_in_baseline := 2;
righttatweel := righttatweel + 1;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi


%y1 := y1 + righttatweel*2;
%x1 := x1 + righttatweel*30;
z2 = z1 + (penwidth,0) rotated 70;

z4 = z3 + (penwidth,0) rotated 70;
y11 := 0;
righttatweel_const := (x11 - x1,y11 - y1);
%%endverbatim

%%beginpaths

fill
z3 join
z4 ..
(219.796,138.427) .. tension 1 and 29.0915 ..
 {up}(341,210) .. tension 11.0275 and 1.28129 ..
 {left}(154.357,250.511) .. tension 3.16992 and 1.50936 ..
(52.5322,224.081) .. tension 1 and 4.81383 ..
 {left}(39.2248,210.837) .. tension 1 and 1 ..
 {up}(33.6124,217.837) .. tension 1.43151 and 1.03661 ..
 {right}(149.985,344.256) .. tension 1.7407 and 1 ..
(336.735,279.934) .. tension 1 and 1 ..
 {right}(567.969,247.705) .. tension 1 and 1.92009 ..
 {right}(623.408,255.582) .. tension 1 and 1 ..
 {down}(626,250) .. tension 1.19648 and 134.46 ..
 {left}(603,177) .. tension 1 and 1 ..
 {left}(526.264,184.411) .. tension 1 and 1.93482 ..
 {down}(467.713,170.155) .. tension 1 and 1 ..
(473.71,135.962) .. tension 1.34198 and 0.75 ..
z2 join
z1 ..
 {up}(440.938,121.693) .. tension 0.75 and 4.4481 ..
 {left}(445.225,172.38) .. tension 0.855179 and 0.880849 ..
(280.995,99.202) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(hah.medi.afterbeh,-1,-1,-1,-1);
%%backgroundimage:images/yeh_hah.init.jpg,-2402,-1310,1,0,0,1
%%beginbody

path p;
save mypath_,i_, t,u,dm,l,dm,l;
path mypath_[], tpath_;
pair dm[];
numeric t[], u[];
i_ := 0;
l = 2;

behshape_hah.init_(charlt,charrt);

 p := pathpart currentpicture;
%fill subpath(0,9) of p .. tension 2 ..  point 20 of p  ..    subpath(21,length p) of p .. cycle;

(t3,u3) = p intersectiontimes z3;
(t4,u4) = p intersectiontimes z4;

dm1 := z3 + l* unitvector direction t3 of p;
dm2 := z4-  l* unitvector direction t4 of p;

currentpicture:=nullpicture;

fill subpath(0,t3) of p .. controls   dm1 .. 1/2[dm1,dm2] .. controls dm2 ..  subpath(t4,length p) of p .. cycle ;


rightbearing :=x4 - x5;

setCursiveAnchor(beforehah1,z4, EntryAnchorRTL);
enddefchar;
defchar(hah.medi.afterbeh.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/behhahyeh.jpg,-1385.46,-1071.31,1,0,0,1
%%beginparams
z1 = (36.1111,66.9167);
z2 = (65.4997,261.323);
z3 = (148.788,351.237);

%%beginpaths

fill
z1 .. tension 1.73872 and 50.4732 ..
 {up}(350,225.25) .. tension 26.7047 and 1 ..
 {left}(121.107,274.741) .. tension 0.836808 and 5.29231 ..
 {up}z2 .. tension 2.21464 and 1.18822 ..
 {right}z3 .. tension 11.9837 and 0.854769 ..
 {right}(552.976,263.446) .. tension 1 and 41.5213 ..
 {down}(646.528,266.917) .. tension 1.36745 and 7.49392 ..
 {left}(617.471,195.646) .. tension 1 and 1 ..
(116.223,98.4175) .. tension 1 and 24.3399 ..
 {up} cycle
;

%%endpaths
%%beginbody
%%beginbody
leftbearing := 0;
rightbearing := -300;

setCursiveAnchor(yehcursive1,z1,ExitAnchor);
setCursiveAnchor(beforehah1,z2, EntryAnchorRTL);

enddefchar;
defchar(hah.medi.afterlam.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/lamhahyeh.jpg,-2136.28,-1507.73,1,0,0,1
%%beginparams
z1 = (26.8466,33.4149);
z2 = (40.436,208.706);
z3 = (157.016,298.07);

%%beginpaths

fill
z1 .. tension 1.54612 and 6.92546 ..
 {up}(309.739,197.468) .. tension 8.7537 and 1.34014 ..
 {left}(149.475,228.316) .. tension 0.836808 and 6.9559 ..
 {up}z2 .. tension 1.64185 and 0.779603 ..
 {right}z3 .. tension 3.97076 and 0.875195 ..
 {right}(589.641,237.5) .. tension 1 and 18.4082 ..
 {down}(640.299,239.269) .. tension 1.36745 and 7.49392 ..
 {left}(593.945,160.03) .. tension 6.96451 and 0.75 ..
 {left}(495.802,178.745) .. tension 1.63561 and 1 ..
(165.233,88.1043) .. tension 1 and 24.3399 ..
 {up} cycle
;

%%endpaths
%%beginbody
%%beginbody
leftbearing := 0;
rightbearing := -350;

setCursiveAnchor(yehcursive1,z1,ExitAnchor);
setCursiveAnchor(beforehah1,z2, EntryAnchorRTL);
setCursiveAnchor(lamhah,z3, EntryAnchorRTL);

enddefchar;
defchar(hah.medi.lam_hah,-1,-1,-1,-1);
%%backgroundimage:images/lam_hah.jpg,-2275,-1322,1,0,0,1
%%beginparams
lefttatweel := 1.14; % numeric
z1 = (120.521,22.7438);
z3 = (99.2837,288.729);
z4 = (312.09,276.392);
z5 = (643.65,229.071);

%%beginverbatim

if left_verticalextratio = 0 : left_verticalextratio := 12; fi
 
lefttatweel := charlt + 0.5;
z2 = z1 + (lineheight,0) rotated 75;

%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension 1 and 7.16575 ..
 {up}(340.455,185.141) .. tension 10.8298 and 1.79081 ..
(229.926,211.323) .. tension 0.815564 and 3.20171 ..
 {left}(113.343,219.514) .. tension 0.75 and 6.35213 ..
 {left}(59.8623,197.764) .. tension 1.39544 and 1 ..
 {up}(53.7016,206.034) .. tension 1.74599 ..
z3 {dir 23} .. tension 1.57381 and 1.71452 ..
(415,246) .. tension 1 and 6.46414 ..
 {right}(638.508,235.235) .. tension 2.71262 and 0.75 ..
 {down}z5 .. tension 7.62288 ..
 {left}(589.319,146.203) .. tension 2.71262 and 0.75 ..
 {left}(479.209,163.152) .. tension 1.01367 and 0.75 ..
(294.472,95.6429) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

rightbearing := -400;

setCursiveAnchor(lamhah,z3, EntryAnchorRTL);

enddefchar;
defchar(hah.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/hahyeh.jpg,-1518.69,-1076.88,1,0,0,1
%%beginparams
z2 = (500.096,1.44676);

%%beginverbatim
z3 = z2 + (0,lineheight);


%%endverbatim

%%beginpaths

fill
(393.519,75.2315) .. tension 1 and 2.23436 ..
 {right}(414.738,175.54) .. tension 1 and 1 ..
 {down}(419.078,146.605) .. tension 1 and 0.86785 ..
 {right}z3 .. tension 1.81533 and 1 ..
 {down}(514.082,41.956) .. tension 1 and 1.60987 ..
 {left}z2 .. tension 1.11415 and 0.958962 ..
 {up} cycle
;

%%endpaths
%%beginbody

hah.init.beforeyeh_(charlt,charrt);
rightanchor := z2;
setCursiveAnchor(yehcursive1,z1,ExitAnchor);
enddefchar;
defchar(hah.medi.beforemeem,-1,-1,-1,-1);
%%backgroundimage:images/meemhah.jpg,-4016,-1416,1,0,0,1
%%beginparams
lefttatweel := 1.14; % numeric
z1 = (120.521,22.7438);
z3 = (45.124,267.657);
z5 = (582.886,202.547);
z6 = (-13.9478,194.982);

%%beginverbatim

if left_verticalextratio = 0 : left_verticalextratio := 12; fi
 
lefttatweel := charlt + 0;
z2 = z1 + (0,lineheight) rotated -15;

%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension 1 and 18.3423 ..
 {up}(278.164,152.656) .. tension 10.8298 and 1.79081 ..
 {left}(47.3681,195.066) .. tension 0.75 and 6.35213 ..
 {left}(-7.78709,186.712) .. tension 1.39544 and 1 ..
 {up}z6 .. tension 1.74599 ..
 {right}z3 .. tension 1.57381 and 1.11563 ..
(484.444,205.008) .. tension 1 and 6.46414 ..
 {right}(570.993,213.534) .. tension 2.71262 and 0.75 ..
 {down}z5 .. tension 7.62288 ..
 {left}(543.505,132.7) .. tension 2.71262 and 0.75 ..
 {left}(501.393,140.004) .. tension 1.01367 and 0.75 ..
(295.812,93.6335) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody


setCursiveAnchor(meemhah,z6, EntryAnchorRTL);

rightanchor := z6 + (200,0) ;

enddefchar;
defchar(hah.fina,-1,-1,-1,-1);
%%backgroundimage:images/hah.fina.jpg,-79,-1132,1,0,0,1
%%beginparams
z1 = (505.854,53.6437);
z3 = (662.037,-0.578704);

%%beginverbatim
righttatweel := charrt +0;
z2 = z1 + (0,lineheight ) rotated -12;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);
y4 := lineheight;

%%endverbatim

%%beginpaths

fill
(104.977,-260.355) .. tension 1.2314 and 1 ..
(216.344,92.4318) .. tension 0.75 and 30.7822 ..
 {up}(321,209.843) .. tension 4.92356 and 1.91629 ..
(154.807,201.044) .. tension 1 and 5.4508 ..
 {left}(117.086,177.619) .. tension 1 and 1 ..
 {up}(111.62,189.995) .. tension 1.54638 and 0.907437 ..
 {right}(231.697,306.542) .. tension 3.3099 and 0.75 ..
 {right}(447.845,289.084) .. tension 0.75 and 6.22381 ..
 {right}(630.27,314.393) .. tension 1 and 1 ..
 {down}(641.995,308.033) .. tension 2.68716 and 3.89111 ..
(612,238) .. tension 21.5682 and 1 ..
(453.787,208.976) .. tension 1.21989 and 1 ..
(237.108,57.9373) .. tension 0.860448 and 1.82147 ..
 {down}(145.486,-251.106) .. tension 1 and 0.878379 ..
 {right}(554.91,-580.033) .. tension 1 and 6.11459 ..
 {right}(967.078,-535.236) .. tension 1 and 1 ..
 {down}(983.901,-543.299) .. tension 4.71328 and 1 ..
(885.728,-584.073) .. tension 1 and 4.02264 ..
(797.755,-617.931) .. tension 1.27981 and 1 ..
 {left}(537.944,-664.167) .. tension 0.75 and 1 ..
 {up} cycle
;
fill
(523.665,240.777) .. tension 2.13028 and 1 ..
(514.543,178.854) .. tension 1 and 0.75 ..
z2 join
z1 .. tension 0.75 and 1 ..
 {up}(480.046,126.741) .. tension 1 and 16.3629 ..
 {left}(500.018,232.891) --
 cycle
;

%%endpaths
enddefchar;
beginchar(dal.isol,1583,-1,-1,-1);
%%backgroundimage:images/dal.isol.jpg,-1348.15,-1233.42,1,0,0,1

%%beginpaths

fill
(62,49) .. tension 1 and 5.77471 ..
 {right}(77.3085,112.869) .. tension 1.03809 and 1 ..
 {down}(79.9736,108.105) .. tension 1.65623 and 0.828972 ..
 {right}(140.969,82.8695) .. tension 1 and 9.77129 ..
 {up}(330.777,140.677) .. tension 5.38067 and 1.04703 ..
(194.13,312.675) .. tension 1 and 1.95173 ..
 {left}(150.808,322.062) .. tension 1 and 1.88674 ..
(138.038,327.124) .. tension 3.32475 and 5.19555 ..
(146.261,399.856) .. tension 0.980196 and 1.7155 ..
 {right}(163.917,407.641) .. tension 1.47023 and 1.44482 ..
 {down}(352.069,176.485) .. tension 0.849554 and 1.9499 ..
(332.54,64.9252) .. tension 3.15796 and 1 ..
 {left}(134.036,0) .. tension 0.7731 and 1 ..
 {up} cycle
;

%%endpaths
endchar;
defchar(dal.fina,-1,-1,-1,-1);
%%backgroundimage:images/dal.fina.jpg,-818.43,-1091.08,1,0,0,1
%%beginparams
z1 = (439.55,86.7224);
z3 = (619.695,1.44676);

%%beginverbatim
righttatweel := righttatweel + 0.0;
z2 = z1 + (0,lineheight ) rotated -15;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);


%%endverbatim

%%beginpaths

fill
(69.1712,104.617) .. tension 1 and 5.58004 ..
 {right}(81.5257,156.054) .. tension 5.11224 and 1 ..
(109.713,139.452) .. tension 1 and 1 ..
 {right}(168.403,131.366) .. tension 1 and 2.45586 ..
 {up}(375,236.867) .. tension 10.598 and 1 ..
(336.773,351.241) .. tension 1 and 1 ..
 {up}(321.759,455.44) .. tension 1 and 4.56942 ..
 {right}(334.587,506.993) .. tension 10.1375 and 1 ..
(367.316,388.616) .. tension 1.94655 and 2.41325 ..
(426.319,216.045) .. tension 1 and 1 ..
z2 join
z1 .. tension 0.928459 and 1.33548 ..
(398.299,167.949) .. tension 1 and 22.0046 ..
 {left}(385.446,200.716) .. tension 16.8371 and 1 ..
(363.194,143.805) .. tension 1 and 1.35948 ..
 {left}(148.11,44.5602) .. tension 0.885773 and 1 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(dal.fina.afterlam,-1,-1,-1,-1);
%%backgroundimage:images/lamdal.jpg,-731.517,-1193.87,1,0,0,1
%%beginparams
z1 = (335.5,20.4796);
z2 = (423.913,30.3278);
z3 = (442,-56);

%%beginverbatim
righttatweel := charrt + 0.0;


righttatweel_const := (x3 - x1,y3 - y1);

which_in_baseline := 3;
%%endverbatim

%%beginpaths

fill
(-0.482253,56.0707) .. tension 1 and 5.58004 ..
 {right}(12.0812,107.426) .. tension 1.3739 and 1 ..
(35.4455,97.6564) .. tension 1 and 1 ..
 {right}(81.3565,91.6614) .. tension 1 and 2.489 ..
 {up}(271.799,171.199) .. tension 10.598 and 1 ..
(238.876,279.867) .. tension 1 and 1 ..
 {up}(227.037,360.817) .. tension 1 and 4.56942 ..
 {right}(234.84,408.255) .. tension 4.95442 and 1 ..
(267.49,321.582) .. tension 1.94655 and 2.41325 ..
(321.108,159.782) .. tension 1 and 1.25343 ..
z2 join
z1 .. tension 0.928459 and 26.907 ..
 {left}(287.419,124.794) {dir -113.825} .. tension 0.964932 and 1 ..
 {left}(62.3226,4.34028) .. tension 0.885773 and 1.20899 ..
(6.91226,22.1032) .. tension 1 and 1.43928 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(reh.isol,1585,-1,-1,-1);
%%backgroundimage:images/waw.isol.jpg,-2218.93,-1413.6,1,0,0,1
%%beginparams
z1 = (-247.942,-170.396);
z2 = (198,257);

%%beginpaths

fill
z2 .. tension 1 and 4.21397 ..
 {right}(218.028,337.402) .. tension 18.8797 and 1.27718 ..
 {down}(341,163) .. tension 1.17234 and 1.01288 ..
 {left}(24.9956,-198.948) .. tension 1 and 6.74952 ..
 {up}z1 .. tension 1 and 1 ..
 {right}(-230.308,-150.917) .. tension 12.4552 and 0.75 ..
 {right}(-122.443,-159.448) .. tension 0.75 and 2.79232 ..
 {up}(307,120) .. tension 1.89688 and 6.26455 ..
 {up} cycle
;

%%endpaths
%%beginbody
%%backgroundimage:images/reh.isol.jpg,-1895,-1493,1,0,0,1
leftbearing := -180; %4*50 - (x2-x1);
enddefchar;
defchar(reh.fina,-1,-1,-1,-1);
%%backgroundimage:images/waw.fina.jpg,-1537.12,-1081.07,1,0,0,1
%%beginparams
z1 = (343.434,8.69117);
z3 = (426.325,6.36306);
z4 = (234.722,102.778);
z5 = (-241.826,-235.569);

%%beginverbatim
righttatweel := charrt + 0.0;
z2 = z1 + (0,lineheight ) rotated -5;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);

%%backgroundimage:images/reh.fina.jpg,-495,-1174,1,0,0,1
%%endverbatim

%%beginpaths

fill
z5 .. tension 1 and 1 ..
 {right}(-222.072,-218.642) .. tension 2.92944 and 1.23772 ..
 {right}(-112.705,-236.832) .. tension 1.20626 and 1 ..
(215.961,-101.237) .. tension 1 and 3.12081 ..
 {up}(298.611,22.2222) .. tension 10000 and 0.792016 ..
 {up}z4 .. tension 1 and 5.22376 ..
 {right}(259.575,170.213) .. tension 1.77596 and 1 ..
(271.937,150.704) .. tension 1.22707 and 0.869847 ..
z2 join
z1 ..
(325.793,14.7066) {curl1} .. tension 1.53426 and 1 ..
(274.281,-115.205) .. tension 1 and 1 ..
 {left}(-9.47417,-276.354) .. tension 1.46627 and 4.98242 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing := -180; % 230- (x4-x5);
enddefchar;
defchar(reh.fina.afterbehshape,-1,-1,-1,-1);
%%backgroundimage:images/behreh.jpg,-1271,-1233,1,0,0,1
%%beginparams
z1 = (250.947,199.364);

%%beginverbatim

%%endverbatim

%%beginpaths

fill
z1 .. tension 1.84563 and 1.28405 ..
 {down}(272.371,187.375) .. tension 1.35679 and 0.940093 ..
 {left}(-64.3308,-128.007) .. tension 1 and 5.76066 ..
 {up}(-293.948,-78.4492) .. tension 1 and 1 ..
 {right}(-277.433,-59.8212) .. tension 7.69896 and 0.75 ..
 {right}(-146.844,-79.5633) .. tension 0.85716 and 2.28212 ..
 {up}(233.676,190.586) .. tension 1.89688 and 0.880897 ..
 {right} cycle
;

%%endpaths
%%beginbody
leftbearing := -150;
setCursiveAnchor(behshapereh,z1, EntryAnchor);
enddefchar;
defchar(reh.fina.afterseen,-1,-1,-1,-1);
%%backgroundimage:images/seenreh.jpg,-2182.73,-1183.2,1,0,0,1
%%beginparams
z1 = (254.588,25.8892);

%%beginverbatim

%%endverbatim

%%beginpaths

fill
z1 .. tension 1.04246 and 1.28405 ..
 {down}(267.516,9.05502) .. tension 1.13446 and 1 ..
 {left}(-3.02277,-312.271) .. tension 1 and 6.17979 ..
 {up}(-278.112,-206.241) .. tension 1 and 1 ..
 {right}(-266.994,-193.947) .. tension 7.69896 and 0.75 ..
 {right}(-96.7622,-251.821) .. tension 0.758227 and 1.95613 ..
 {up}(239.416,4.86508) .. tension 0.905697 and 1.37064 ..
 {right} cycle
;

%%endpaths
%%beginbody
leftbearing := -200;
setCursiveAnchor(seenreh,z1, EntryAnchor);
enddefchar;
beginchar(waw.isol,1608,-1,-1,-1);
%%backgroundimage:images/waw.isol.jpg,-2219.19,-1407.55,1,0,0,1
%%beginparams
z1 = (-219.929,-171.518);
z2 = (182,151);

%%beginpaths

fill
z1 .. tension 1 and 1 ..
 {right}(-206,-154) .. tension 11.0464 and 0.75 ..
 {right}(-60,-163) .. tension 1.3105 and 0.825582 ..
(231.08,-44.4898) .. tension 1 and 1.78132 ..
 {up}(344.258,128.674) .. tension 1 and 1.80856 ..
 {left}(339,142) .. tension 6.67317 and 0.873373 ..
 {left}(265.802,83) .. tension 1 and 1.8525 ..
 {up}z2 .. tension 0.864958 and 2.57426 ..
 {right}(270.678,355.865) .. tension 2.58977 and 1 ..
 {down}(375.376,195.269) .. tension 1 and 1 ..
(253.665,-108.897) .. tension 1 and 0.75 ..
 {left}(-8.32602,-203) .. tension 1 and 6.22418 ..
 {up} cycle
;
fill
(262.128,264.198) .. tension 1 and 3.84864 ..
 {down}(215.392,210.776) .. tension 1 and 2.43661 ..
 {right}(278.891,166.031) .. tension 4.32986 and 1.62627 ..
 {up}(319,196) .. tension 2.43662 and 2.10073 ..
 {left} cycle
;

%%endpaths
%%beginbody
leftbearing := -180; %200- (x2-x1);

endchar;
defchar(waw.fina,-1,-1,-1,-1);
%%backgroundimage:images/waw.fina.jpg,-1479.12,-1058.07,1,0,0,1
%%beginparams
z1 = (272.207,3.79139);
z3 = (311,3);
z4 = (182.774,103.202);
z5 = (-206,-256.964);

%%beginverbatim
righttatweel := 0;
z2 = z1 + (0,lineheight );
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);


%%endverbatim

%%beginpaths

fill
z5 .. tension 1 and 1 ..
 {right}(-192,-240) .. tension 6.1768 and 1 ..
 {right}(-40.0502,-258.624) .. tension 1 and 1.74828 ..
 {up}(333.894,25.098) .. tension 1 and 2.88629 ..
 {left}(261.125,165.983) .. tension 2.31073 and 1 ..
 {down}(220.626,113.588) .. tension 1.29622 and 1 ..
z2 join
z1 .. tension 1 and 1.30089 ..
(216.667,21.9127) .. tension 1.03662 and 0.75 ..
 {up}z4 .. tension 1.11869 and 3.2826 ..
 {right}(263.562,258.209) .. tension 1.64466 and 1.39362 ..
 {down}(372.3,49.1898) .. tension 1.21074 and 1 ..
 {left}(46.7785,-295.139) .. tension 1 and 6.12368 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing :=-180; %200 - (x4 - x5);



enddefchar;
beginchar(heh.isol,1607,-1,-1,-1);
%%backgroundimage:images/heh.isol.jpg,-437.442,-1748.84,1,0,0,1

%%beginpaths

fill
(51.2938,97.158) .. tension 1.20527 and 0.776444 ..
(84.3988,230.892) .. tension 0.75 and 5.48944 ..
 {up}(108.733,268.204) .. tension 2.06497 and 1.78035 ..
 {up}(103.791,277.736) .. tension 8.33879 and 3.79505 ..
 {right}(135.62,354.897) .. tension 18.7293 and 0.75 ..
 {down}(301.515,195.753) .. tension 0.768395 and 1.07681 ..
 {left}(137.531,26) .. tension 1.13472 and 1 ..
 {up} cycle
;
fill
(126.843,259.218) .. tension 20.1291 and 1.30474 ..
 {down}(91.5417,160.259) .. tension 1 and 1.12065 ..
 {right}(167.931,104.704) .. tension 2.03587 and 1.95416 ..
 {up}(259.944,142.898) .. tension 1.36077 and 2.63017 ..
 {left} cycle
;

%%endpaths
endchar;
defchar(heh.init,-1,-1,-1,-1);
%%backgroundimage:images/heh.init.2.jpg,-5378.74,-1338.79,1,0,0,1
%%beginparams
z1 = (43.6754,4.00912);
z1.5 = (31,-58);
z2 = (50.3734,86.8948);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := lefttatweel  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.115  ;






%z2 = z1 + (0,penwidth ) rotated (-15 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
(128.472,46.2854) .. tension 1 and 1 ..
z1 join
z2 ..
(84.4907,105.324) .. tension 1 and 1 ..
 {up}(90.8565,116.898) .. tension 1 and 1 ..
 {up}(82.0031,145.254) .. tension 1 and 8.238 ..
 {up}(126.341,308.383) .. tension 4.35422 and 1 ..
 {up}(117.456,368.72) .. tension 1 and 1.0381 ..
 {right}(148.539,436.439) .. tension 6.26067 and 1.29565 ..
 {down}(420.429,144.05) .. tension 1.28053 and 1.6186 ..
 {left}(346.011,10.7627) .. tension 1.65052 and 2.92974 ..
 {left}(203.118,71.3957) .. tension 1.05192 and 1 ..
 cycle
;
fill
(166.927,278.472) .. tension 2.25893 and 1.14592 ..
 {down}(108.039,173.778) .. tension 1.45447 and 1 ..
 {right}(173.817,148.456) .. tension 0.868507 and 1.46318 ..
 {up}(244.186,174.514) .. tension 1.47077 and 1.34136 ..
 {left} cycle
;
fill
(279.046,246.557) .. tension 16.8218 and 2.07776 ..
(269.027,187.155) .. tension 1.86439 and 3.54194 ..
 {down}(253.651,133.694) .. tension 6.50377 and 3.64835 ..
 {right}(364.122,88.1) .. tension 1.4814 and 0.967274 ..
 {up}(388.394,119.805) .. tension 2.80955 and 5.69699 ..
 {left} cycle
;

%%endpaths
enddefchar;
defchar(heh.init.beforemeem,-1,-1,-1,-1);
%%backgroundimage:images/heh_meem.jpg,-570,-1294,1,0,0,1
%%beginparams
z1 = (-27,75);

%%beginverbatim







%z2 = z1 + (0,penwidth ) rotated (-15 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
(-23.5717,44.9272) .. tension 3.46585 and 4.17301 ..
 {right}(-65.3051,104.823) .. tension 3.2099 and 3.38377 ..
 {up}(79.8049,156.751) .. tension 1.68737 and 1 ..
(63.9655,180.845) .. tension 1.35971 and 1 ..
 {up}(52.8111,229.145) .. tension 0.75 and 11.609 ..
 {up}(92.4143,373.567) .. tension 4.35422 and 1 ..
 {up}(79.4546,392.935) .. tension 1 and 3.82791 ..
 {right}(95.3086,485.548) .. tension 4.6509 and 1.2273 ..
 {down}(391.357,223.865) .. tension 1 and 4.14544 ..
 {left}(353.875,99.3816) .. tension 3.08869 and 1.26417 ..
 {left}(184.191,131.714) .. tension 5.24304 and 2.12988 ..
(8.70735,49.5649) .. tension 1 and 1 ..
 {left} cycle
;
fill
(137.772,335.219) .. tension 2.79295 and 1.50554 ..
 {down}(78.1776,253.279) .. tension 1.47925 and 1 ..
 {right}(186.897,217.138) .. tension 1.87197 and 1.46318 ..
 {up}(215.692,245.14) .. tension 2.63606 and 1.34136 ..
 {left} cycle
;
fill
(242.712,316.813) .. tension 3.90255 and 2.07776 ..
 {down}(233.62,259.479) .. tension 1.86439 and 2.43688 ..
 {down}(229.359,212.585) .. tension 6.50377 and 3.64835 ..
 {right}(357.346,184.101) .. tension 0.82779 and 0.967274 ..
 {up}(362.862,196.808) .. tension 2.10259 and 5.69699 ..
 {left} cycle
;

%%endpaths
%%beginbody


setCursiveAnchor(hehmeem,z1, ExitAnchor);
leftbearing :=0;
rightbearing  := -0;


enddefchar;
defchar(heh.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/hehyeh.jpg,-1988.26,-1032.26,1,0,0,1

%%beginverbatim
z1 = (0,lineheight);


%%endverbatim

%%beginpaths

fill
(152.163,134.363) .. tension 1 and 16.9387 ..
 {up}z1 .. tension 0.75 and 1 ..
(30.039,129.957) .. tension 1 and 8.97658 ..
 {up}(190.179,218.836) .. tension 1 and 1 ..
 {up}(170.72,297.985) .. tension 1 and 8.238 ..
 {up}(206.361,429.066) .. tension 4.35422 and 1 ..
 {up}(192.731,492.195) .. tension 1 and 3.82791 ..
 {right}(208.418,539.54) .. tension 3.33458 and 1.34321 ..
 {down}(500.391,305.152) .. tension 1 and 1.64888 ..
 {left}(457.328,169.337) .. tension 3.70142 and 0.86313 ..
 {left}(288.091,198.879) .. tension 1.30067 and 1 ..
 cycle
;
fill
(263.88,395.29) .. tension 2.25893 and 1.14592 ..
 {down}(197.221,303.367) .. tension 1.13593 and 1 ..
 {right}(279.662,270.521) .. tension 1.87197 and 1.46318 ..
 {up}(336.343,316.391) .. tension 1.47077 and 1.34136 ..
 {left} cycle
;
fill
(367.397,368.65) .. tension 13.7844 and 2.07776 ..
 {down}(359.573,311.439) .. tension 1.86439 and 15.3757 ..
 {down}(356.113,269.575) .. tension 6.50377 and 3.64835 ..
 {right}(464.228,245.077) .. tension 1.4814 and 0.967274 ..
 {up}(478.071,270.865) .. tension 2.42111 and 5.69699 ..
 {left} cycle
;

%%endpaths
%%beginbody
charwd:= xpart (urcorner currentpicture) - x1; 

setCursiveAnchor(yehcursive1,z1,ExitAnchor);
enddefchar;
defchar(heh.medi,-1,-1,-1,-1);
%%backgroundimage:images/heh.medi.2.jpg,-755.119,-1278.83,1,0,0,1
%%beginparams
z.mark1 = (267,-262);
z1 = (63.3974,2.20693);
z1.5 = (-2.27022,-41.0371);
z3 = (142.888,-38.1838);
z4 = (375.112,-71.397);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.0  ;






z2 = z1 + (0,penwidth ) rotated (-10 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
z1 join
z2 ..
(152.866,100.258) .. tension 1 and 1.0862 ..
 {dir 23.3288}(177.404,109.184) {dir 53.6179} .. tension 7.50675 and 1 ..
(212.521,225.248) .. tension 1 and 1.85792 ..
 {right}(299.556,348.988) .. tension 1.14769 and 1 ..
 {down}(345.226,263.084) .. tension 0.951806 and 1.05939 ..
(278.895,102.77) .. tension 1 and 4.07979 ..
 {down}(230.582,59.707) .. tension 11.6544 and 1 ..
(328.659,31.9123) .. tension 0.780826 and 1 ..
 {down}z4 .. tension 0.75 and 1.68374 ..
 {left}(302.964,-181.282) .. tension 0.754039 and 2.25827 ..
 {dir 94.1982}(168.9,-22.7806) {dir 162.525} .. tension 1.21186 and 2.01438 ..
(103.793,-16.1095) .. tension 1 and 1.11722 ..
 {dir -160.772}(93.189,8.15238) .. tension 1.1619 and 1 ..
 cycle
;
fill
(280.155,263.803) .. tension 2.01159 and 54.4265 ..
 {down}(199.111,124.086) .. tension 1 and 1.6303 ..
 {right}(204.353,118.102) .. tension 2.48174 and 2.94869 ..
 {up}(315.251,215.667) .. tension 1.31239 and 1.22629 ..
 {left} cycle
;
fill
(206.41,-26.4645) {dir -134.176} .. tension 3.27191 and 1.76196 ..
 {right}(289.464,-99.4717) .. tension 1.1486 and 1.9813 ..
(351.085,-54.9308) .. tension 2.8299 and 6.84222 ..
 cycle
;

%%endpaths
%%beginbody

%%beginbody

rightbearing := rightbearing - (x4 - x3);

rightanchor := (x3,0);

setMarkAnchor(buttonmark,z.mark1);


enddefchar;
defchar(heh.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/hehyeh.jpg,-1989.38,-1042.46,1,0,0,1
%%beginparams
z2 = (593.614,-11.06);

%%beginverbatim
z3 = z2 + (0,lineheight);


%%endverbatim

%%beginpaths

fill
(413.755,112.477) .. tension 1 and 2.47929 ..
 {right}(442.304,195.521) .. tension 1.08923 and 1.55143 ..
 {down}(442.669,151.056) .. tension 1.39769 and 1 ..
(503.552,90.8244) .. tension 1 and 1.7473 ..
 {right}z3 .. tension 0.875055 and 1 ..
 {down}(601.33,37.1651) .. tension 1 and 1.60987 ..
 {left}z2 .. tension 0.973138 and 0.825435 ..
 {up} cycle
;

%%endpaths
%%beginbody

heh.init.beforeyeh_(charlt,charrt);
pair leftanchor;
rightanchor := z2;
setCursiveAnchor(yehcursive1,z1,ExitAnchor);
enddefchar;
defchar(heh.fina,-1,-1,-1,-1);
%%backgroundimage:images/tehmarbouta.fina.jpg,-71.088,-1354.31,1,0,0,1
%%beginparams
z1 = (433.28,26.365);
z3 = (550,-0.694444);

%%beginverbatim

z2 = z1 + (0,lineheight ) rotated -15;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);


%%endverbatim

%%beginpaths

fill
(90.2778,91.6667) .. tension 1 and 1 ..
(182.937,264.726) .. tension 1 and 8.57661 ..
 {up}(293.75,343.056) .. tension 0.910087 and 3.11944 ..
 {right}(313.759,473.754) .. tension 0.985899 and 1 ..
 {down}(322.222,460.417) .. tension 1 and 2.20226 ..
(371.993,215.638) .. tension 1 and 1 ..
z2 join
z1 ..
(349.469,124.379) .. tension 1 and 4.23371 ..
 {left}(336.111,166.667) .. tension 6.79971 and 1 ..
(288.57,104.377) .. tension 1.13506 and 0.75 ..
 {left}(174.519,71.8004) .. tension 1 and 2.69002 ..
 {up} cycle
;
fill
(219.444,152.778) .. tension 1 and 1 ..
 {up}(320.833,208.333) .. tension 3.47055 and 12.31 ..
 {left}(300.694,288.194) .. tension 20.1423 and 3.43297 ..
 {down}(143.042,166.919) .. tension 1.68523 and 1 ..
 {right} cycle
;

%%endpaths
enddefchar;
defchar(heh.fina.afterlam,-1,-1,-1,-1);
%%backgroundimage:images/lamteh.jpg,-681.217,-1312.26,1,0,0,1
%%beginparams
z1 = (289.317,-22.7592);
z3 = (370,-54);

%%beginverbatim
righttatweel := charrt +0;
z2 = z1 + (0,lineheight ) rotated -15;

righttatweel_const := (x3 - x1,y3 - y1);

which_in_baseline := 3;
%%endverbatim

%%beginpaths

fill
(19.6183,56.046) .. tension 1 and 1 ..
(175.934,255.629) .. tension 1 and 9.07057 ..
 {up}(206.693,267.408) .. tension 1.36767 and 3.43775 ..
 {right}(214.646,376.404) .. tension 1.89557 and 1.09646 ..
 {down}(223.591,359.692) .. tension 1.54435 and 2.20226 ..
(266.611,140.954) .. tension 1 and 10000 ..
z2 join
z1 ..
(251.803,46.3196) .. tension 1 and 6.39652 ..
 {left}(233.622,114.167) .. tension 6.79971 and 1 ..
(175.954,67.791) .. tension 1.13506 and 0.75 ..
 {left}(79.7463,42.4485) .. tension 1 and 2.69002 ..
 {up} cycle
;
fill
(86.0913,118.604) .. tension 1 and 9.45032 ..
 {up}(230.401,137.507) .. tension 3.47055 and 5.45648 ..
 {left}(210.262,220.744) .. tension 6.0194 and 2.71774 ..
 {down}(70.9353,129.851) .. tension 0.939321 and 1 ..
 {right} cycle
;

%%endpaths
%%beginbody
setCursiveAnchor(hehmeem,z1, EntryAnchor);

enddefchar;
defchar(sad.isol,1589,-1,-1,-1);
%%backgroundimage:images/dad.isol.2.jpg,-513.597,-1104.5,1,0,0,1
%%beginparams
z.mark1 = (859,-13);

%%beginpaths

fill
(69.25,-61.9167) .. tension 1.00649 and 4.64871 ..
 {right}(121.148,136.222) .. tension 1 and 1 ..
 {down}(128.528,118.361) .. tension 1.3897 and 1 ..
 {down}(98.9413,-0.496078) .. tension 1 and 0.881354 ..
 {right}(250.016,-152.481) .. tension 1 and 1.8009 ..
 {up}(511.019,-4.44331) .. tension 0.8134 and 15.5999 ..
 {up}(455.321,161.232) .. tension 17.5816 and 4.07423 ..
 {right}(467.141,242.452) .. tension 4.81093 and 1.23292 ..
 {curl1}(518.991,152.018) .. tension 1.44944 and 1.18506 ..
 {right}(546.912,140.762) .. tension 4.92822 and 1.21959 ..
 {right}(839.245,381.72) .. tension 0.757598 and 1.51375 ..
 {down}(956.556,285.524) .. tension 1.19835 and 1 ..
 {left}(605.71,56.4236) .. tension 1 and 0.75 ..
(534.791,61.3845) {curl1} .. tension 1.47607 and 1 ..
(495.527,-117.214) .. tension 1.24446 and 1 ..
 {left}(254.044,-227.553) .. tension 0.868687 and 1 ..
 {up} cycle
;
fill
(801.342,295.03) {left} .. tension 1.29616 and 36.4946 ..
 {down}(599.355,136.336) .. tension 2.778 and 2.36469 ..
 {right}(618.322,131.636) .. tension 0.847972 and 7.04096 ..
 {up}(908.588,247.069) .. tension 3.20963 and 1.16318 ..
 {left} cycle
;

%%endpaths
%%beginbody

setMarkAnchor(buttonmark,z.mark1);
savepicture:=1;
enddefchar;
defchar(sad.isol.ii,-1,-1,-1,-1);
%%backgroundimage:images/dad.isol.jpg,-18.1204,-1153.76,1,0,0,1

%%beginpaths

fill
(63,-39) .. tension 1 and 9.93924 ..
 {right}(126.009,164) .. tension 1 and 1 ..
 {down}(132,151) .. tension 1.3897 and 1 ..
 {down}(101,30) .. tension 1 and 0.831304 ..
 {right}(276.405,-149.009) .. tension 1 and 0.932284 ..
 {up}(508.34,26.3673) .. tension 0.8134 and 15.5999 ..
 {up}(458,183) .. tension 17.5816 and 4.07423 ..
 {right}(477.858,256.518) .. tension 3.18855 and 1.23292 ..
 {curl1}(521,172.447) .. tension 1.44944 and 1.18506 ..
 {right}(540.884,163.535) .. tension 3.86529 and 1.44769 ..
 {right}(884.363,404.977) .. tension 0.81656 and 2.42732 ..
 {down}(996.558,341.139) .. tension 0.990348 and 1.10497 ..
 {left}(541,73.2729) {curl1} .. tension 1.29547 and 1 ..
(502.279,-107.569) .. tension 1 and 1 ..
 {left}(254.044,-227.553) .. tension 0.868687 and 1 ..
 {up} cycle
;
fill
(835.836,327.515) {left} .. tension 2.20354 and 36.4946 ..
 {down}(597.708,160.709) .. tension 6.27827 and 2.36469 ..
 {right}(633.858,156.605) .. tension 1.06137 and 5.2592 ..
 {up}(956.813,274.196) .. tension 3.20963 and 1.16318 ..
 {left} cycle
;

%%endpaths
enddefchar;
defchar(sad.init,-1,-1,-1,-1);
%%backgroundimage:images/sad.init.jpg,-2302.75,-1465.38,1,0,0,1
%%beginparams
z.mark1 = (500,-99);
z1 = (99.5659,26.3937);
z1.5 = (0,-51.6011);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.115  ;






z2 = z1 + (0,penwidth ) rotated (-15 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
(132.962,55.7545) .. tension 1.24539 and 1.31659 ..
z1 join
z2 ..
(139.863,122.473) .. tension 1 and 2.59952 ..
 {right}(187.943,177.375) .. tension 1 and 1 ..
 {down}(190.645,167.236) .. tension 1 and 1 ..
 {down}(183.25,131.375) .. tension 3.13519 and 1.85081 ..
 {right}(292.25,84.0495) .. tension 3.98412 and 1.23768 ..
 {right}(625.25,302.375) .. tension 1 and 1 ..
 {down}(736.564,234.532) .. tension 1 and 1 ..
(657.25,83.375) .. tension 1.72016 and 0.7597 ..
 {left}(367.827,-4.76368) .. tension 0.75 and 3.66495 ..
 {left}(256.599,3.33275) .. tension 2.28243 and 6.8879 ..
 {left}(142.915,-208.965) .. tension 1 and 1 ..
 {up}(137.642,-202.613) .. tension 2.77619 and 1 ..
(180.844,-69.3239) .. tension 0.75 and 4.93396 ..
 {up}(213.575,15.7099) .. tension 2.08196 and 1 ..
(181.514,46.8857) .. tension 1 and 6.32267 ..
 {left}(156.25,84.375) .. tension 13.1865 and 0.75 ..
 cycle
;
fill
(580.277,230.556) .. tension 1.56433 and 13.2272 ..
 {down}(360.858,83.7668) .. tension 1.29937 and 1 ..
 {right}(399.027,75.6944) .. tension 1 and 5.06535 ..
 {up}(689.305,177.778) .. tension 1.4777 and 1 ..
 {left} cycle
;

%%endpaths
%%beginbody

setMarkAnchor(buttonmark,z.mark1);
enddefchar;
defchar(sad.init.beforereh,-1,-1,-1,-1);
%%backgroundimage:images/dadinitrehfina.jpg,-3764.52,-1089.58,1,0,0,1
%%beginparams
z1 = (-24.9571,36.5595);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
z1 .. tension 1.24539 and 1.31659 ..
 {left}(-72.0693,30.3474) .. tension 1.7731 and 5.39157 ..
(-77.2394,108.723) .. tension 6.02449 and 0.810177 ..
(136.142,176.894) .. tension 1 and 14.3683 ..
 {right}(167.942,195.748) .. tension 1 and 1 ..
 {down}(179.482,181.888) .. tension 1 and 1 ..
 {down}(179.064,135.794) .. tension 3.13519 and 1.85081 ..
 {right}(292.25,84.0495) .. tension 5.11949 and 1.31781 ..
 {right}(607.889,322.147) .. tension 1 and 2.32865 ..
 {down}(741.869,223.44) .. tension 1 and 1 ..
(657.732,72.7654) .. tension 1.72016 and 0.7597 ..
 {left}(368.309,-7.6572) .. tension 0.75 and 3.66495 ..
 {left}(260.457,1.40374) .. tension 8.56641 and 6.8879 ..
 {left}(229.238,-162.669) .. tension 1 and 1 ..
 {up}(220.477,-155.852) .. tension 2.77619 and 1 ..
(223.637,-66.0679) .. tension 0.75 and 4.93396 ..
 {up}(232.646,12.9191) .. tension 5.04136 and 1 ..
(189.421,54.3279) .. tension 1 and 6.32267 ..
 {left}(164.39,101.12) .. tension 103.862 and 0.75 ..
 cycle
;
fill
(560.505,234.896) .. tension 1.56433 and 36.9649 ..
 {down}(360.858,81.3555) .. tension 2.42061 and 1 ..
 {right}(399.027,75.6944) .. tension 1 and 5.06535 ..
 {up}(685.929,178.26) .. tension 2.61501 and 1 ..
 {left} cycle
;
fill
(139,-146) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody
leftbearing := 0;
setCursiveAnchor(seenreh,z1 + (0,15), ExitAnchor);
enddefchar;
defchar(sad.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/sadinityeh.jpg,-384.061,-1200.48,1,0,0,1
%%beginparams
z10 = (352.004,170.607);

%%beginverbatim





%%endverbatim

%%beginpaths

fill
(318.273,254.195) .. tension 45.8591 and 1.4867 ..
 {right}(623.544,501.054) .. tension 1 and 2.91626 ..
 {down}(750.53,427.986) .. tension 1 and 1 ..
(672.422,259.951) .. tension 1.72016 and 0.7597 ..
 {left}z10 .. tension 0.75 and 3.66495 ..
 cycle
;
fill
(586.42,415.584) .. tension 1.56433 and 33.712 ..
 {down}(383.344,256.485) .. tension 3.48916 and 1 ..
 {right}(418.7,254.842) .. tension 1 and 3.52061 ..
 {up}(701.959,369.236) .. tension 9.88094 and 1 ..
 {left} cycle
;

%%endpaths
%%beginbody

picture pic_;

pic_ :=  currentpicture;

currentpicture:=nullpicture;

behmediyehfina_(charlt,charrt);

cutGlyph(z2,z3,true);
cutGlyph(z4,z5,false);

z60 = (-x5 + x10,-y5+y10);

currentpicture := currentpicture shifted z60;

addto pic_ also currentpicture;


currentpicture:= pic_;
leftbearing := - x2 + x1;

z70 = z2 shifted z60;

tr_ := tr_ shifted (0,-150);



setCursiveAnchor(behmediyehfina,z70, ExitAnchor);


enddefchar;
defchar(sad.medi,-1,-1,-1,-1);
%%beginparams
z11 = (397,-69);
z13 = (665,29);

%%beginverbatim


righttatweel := charrt + 0;
righttatweel_const := (100,0);

y11 := 0;
y13 := lineheight;



z12 = z11 + (2*penwidth,penwidth ) ;
z14 = (x13,0);

%%endverbatim

%%beginpaths

fill
z11 --
z12 ..
z13 join
z14 ..
 cycle
;

%%endpaths
%%beginbody


sad.init_(charlt,charrt);



enddefchar;
defchar(sad.medi.beforereh,-1,-1,-1,-1);
%%backgroundimage:images/sadmedirehfina.jpg,-4384,-1390,1,0,0,1
%%beginparams
z11 = (397,-69);
z13 = (665,29);

%%beginverbatim


righttatweel := charrt + 0;
righttatweel_const := (100,0);

y11 := 0;
y13 := lineheight;



z12 = z11 + (2*penwidth,penwidth ) ;
z14 = (x13,0);

%%endverbatim

%%beginpaths

fill
z11 --
z12 ..
z13 join
z14 ..
 cycle
;

%%endpaths
%%beginbody

leftbearing := 0;
sad.init.beforereh_(charlt,charrt);

setCursiveAnchor(seenreh,z1 + (0,15), ExitAnchor);

enddefchar;
defchar(sad.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/sadinityeh.jpg,-405.333,-1019.33,1,0,0,1
%%beginparams
z11 = (364,20);
z13 = (665,29);

%%beginverbatim


righttatweel := charrt + 0;
righttatweel_const := (100,0);

y11 := 0;




z12 = z11 + (2*penwidth,penwidth ) ;
y13 := y12;
z14 = (x13,0);

%%endverbatim

%%beginpaths

fill
z11 --
z12 --
z13 join
z14 ..
 cycle
;

%%endpaths
%%beginbody

picture pic__;

pic__ :=  currentpicture;

currentpicture:=nullpicture;

sad.init.beforeyeh_(charlt,charrt);



z60.1 = (0,-y5-y60-10);

currentpicture := currentpicture shifted z60.1;



addto pic__ also currentpicture;


currentpicture:= pic__;
leftbearing := - x2 + x1;

z70.1 = z2 shifted z60 shifted z60.1;



setCursiveAnchor(behmediyehfina,z70.1, ExitAnchor);



tr_ := tr_ shifted (0,150);


enddefchar;
defchar(sad.fina,-1,-1,-1,-1);
%%backgroundimage:images/dad.fina.jpg,-144.598,-836.902,1,0,0,1
%%beginparams
z11 = (397,-69);
z13 = (1005.39,13.3267);
z15 = (747.893,3.21502);

%%beginverbatim


righttatweel := charrt + 0;
righttatweel_const := (10,0);

y11 := 0;
y13 := lineheight;
y15 := 0;



z12 = z11 + (2*penwidth,penwidth ) ;
z14 = (x13,0);

%%endverbatim

%%beginpaths

fill
(40,-101) .. tension 1.92076 and 6.44527 ..
 {right}(114.804,134.206) .. tension 1 and 1 ..
 {down}(123,121) .. tension 1.40182 and 1 ..
 {down}(77.3403,-43.858) .. tension 1 and 0.88478 ..
 {right}(236.215,-200.785) .. tension 1 and 1 ..
 {up}(473.674,-41) .. tension 1 and 8.93577 ..
 {up}(421.009,139.402) .. tension 2.7038 and 4.26538 ..
 {right}(440.721,219.93) .. tension 3.92461 and 1.51209 ..
(473.883,157.49) .. tension 1 and 2.01963 ..
(494.515,109.726) {dir -36.3441} .. tension 0.944005 and 3.409 ..
(570.658,87.7406) {dir 9.02388} .. tension 3.74529 and 1 ..
(718.747,200.762) .. tension 1 and 1.05095 ..
 {right}(911.458,315.876) .. tension 1 and 1.97398 ..
 {down}(1012.21,248.203) .. tension 1.95831 and 2.05439 ..
 {dir -151.434}(915.98,83.9292) --
z13 join
z14 --
z15 .. tension 1.27516 and 0.940622 ..
 {dir 164.017}(504.436,17.716) {dir -88.8591} .. tension 1.35689 and 1 ..
(457.667,-172.832) .. tension 1.11205 and 1 ..
 {left}(229.121,-280.858) .. tension 1 and 0.892128 ..
 {up} cycle
;
fill
(858.256,248.423) .. tension 1.8408 and 28.1291 ..
 {down}(628.135,81.8446) .. tension 3.45173 and 1 ..
 {right}(684.156,74.5081) .. tension 1 and 3.52868 ..
 {up}(962.577,201.1) .. tension 2.93077 and 0.850085 ..
 {left} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(ain.isol,1593,-1,-1,-1);
%%backgroundimage:images/ain.isol.jpg,-121.111,-1240,1,0,0,1
%%beginparams
z1 = (457.009,395.66);
z2 = (764,-403.363);

%%beginpaths

fill
(50.6667,-190.167) .. tension 0.757824 and 1.77668 ..
 {dir 45.5958}(183.51,185.573) {dir 127.377} .. tension 1.7405 and 1 ..
 {up}(57.7361,316.468) .. tension 1.22521 and 1 ..
 {right}(228,535) .. tension 1 and 6.66716 ..
 {down}(352,484) .. tension 1 and 1 ..
 {left}(348,475) .. tension 2.71859 and 1 ..
 {left}(296,497) .. tension 1.16171 and 2.61311 ..
 {down}(101,385) .. tension 1.44872 and 1.24232 ..
 {right}(258,262) .. tension 2.83586 and 31.075 ..
 {right}(452.679,409.335) .. tension 1 and 1 ..
 {down}z1 .. tension 1 and 1 ..
(436.679,332.34) .. tension 1.27146 and 1 ..
(217.684,152.379) .. tension 0.93385 and 1.11189 ..
 {down}(87.3333,-152.667) .. tension 0.81556 and 0.825458 ..
 {right}(441.07,-412.67) .. tension 1 and 1 ..
 {right}(749.674,-398.372) .. tension 1 and 1 ..
 {down}z2 .. tension 6.23405 and 1 ..
(627.934,-441.396) .. tension 1 and 0.75 ..
 {left}(405,-484) .. tension 0.75 and 0.881882 ..
 {up} cycle
;

%%endpaths
%%beginbody
rightbearing := rightbearing - (x2-x1);
enddefchar;
defchar(ain.init,-1,-1,-1,-1);
%%backgroundimage:images/gain.init.jpg,-1420.05,-1171.27,1,0,0,1
%%beginparams
z1 = (85.0903,8.0859);

%%beginverbatim

lefttatweel := lefttatweel +0.5;
if left_tatweeltension = 0 : left_tatweeltension := 1; fi
if left_verticalextratio = 0 : left_verticalextratio := 8; fi
%lefttatweel_const := (163, y1 -1 );



z2 = z1 + (0,penwidth ) rotated -15;

%%endverbatim

%%beginpaths

fill
(176.944,88.3334) .. tension 1.24055 and 1 ..
 {up}(58.8893,253.612) .. tension 1.27988 and 1 ..
 {right}(257.885,467.384) .. tension 1.12744 and 9.98644 ..
 {down}(425.003,398.464) .. tension 2.04106 and 1.50103 ..
 {left}(408.194,384.167) .. tension 3.04845 and 1 ..
 {left}(330.127,413.951) .. tension 1 and 2.17216 ..
 {down}(106.805,305.695) .. tension 1.02458 and 1.10548 ..
 {right}(320,143.889) --
 cycle
;
fill
(348.136,171.91) .. tension 1 and 59.0798 ..
 {right}(531.811,275.604) .. tension 1 and 1 ..
 {down}(538.75,269.583) .. tension 1.45652 and 1 ..
(515.597,206.602) .. tension 2.46094 and 1 ..
(281.552,73.0325) .. tension 1 and 1 ..
z1 join
z2 ..
 cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(ain.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/aininityeh.jpg,-452,-1232,1,0,0,1
%%beginparams
z1 = (-30.7068,-39.0913);

%%beginverbatim

%%endverbatim

%%beginpaths

fill
(194.305,105.116) .. tension 1.24055 and 1 ..
 {up}(68.7273,251.297) .. tension 1.27988 and 1 ..
 {right}(245.025,468.188) .. tension 1.12744 and 9.98644 ..
 {down}(421.884,402.386) .. tension 1.27169 and 1.72749 ..
 {left}(408.194,384.167) .. tension 3.04845 and 1 ..
 {left}(330.706,412.794) .. tension 1.27133 and 4.07701 ..
 {down}(117.045,320.822) .. tension 1.02458 and 1.10548 ..
 {right}(312.139,164.722) --
 cycle
;
fill
(415.25,235.005) .. tension 1 and 59.0798 ..
 {right}(503.278,285.249) .. tension 1 and 1 ..
 {down}(510.217,279.228) .. tension 1.45652 and 1.21257 ..
(480.232,203.387) .. tension 2.65581 and 1 ..
(119.358,36.5709) .. tension 1 and 26.1024 ..
 {up}z1 .. tension 1 and 1 ..
(-0.401878,25.3183) .. tension 1.44218 and 1.40552 ..
 cycle
;

%%endpaths
%%beginbody
leftbearing := 0;

setCursiveAnchor(yehcursive1,z1,ExitAnchor);

enddefchar;
defchar(ain.init.finjani,-1,-1,-1,-1);
%%backgroundimage:images/ala.jpg,-730.213,-1097.55,1,0,0,1
%%beginparams
z1 = (74.0866,6.1551);
z1.5 = (0,-82.4653);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt  +x1.5/nuqta;


%left_tatweeltension := 1 + lefttatweel * 0.0  ;

if (lefttatweel < 0 ) :
y1 := y1 + 5 lefttatweel;
fi




z2 = z1 + (0,penwidth ) rotated (-15 -3lefttatweel) ;

%%endverbatim

%%beginpaths

fill
(145.589,25.9118) .. tension 1 and 1 ..
z1 join
z2 .. tension 1 and 1 ..
(164.651,103.076) .. tension 0.75 and 15.3274 ..
 {right}(346.371,233.677) .. tension 0.75 and 1 ..
 {down}(353.009,226.177) .. tension 1.13855 and 1.87381 ..
(282.688,113.811) .. tension 1.07997 and 1 ..
 cycle
;
fill
(13.0208,68.4799) .. tension 2.05219 and 1.07906 ..
 {up}(-48.2253,175.54) .. tension 2.35829 and 1.11796 ..
 {right}(129.726,387.731) .. tension 1 and 3.75866 ..
 {down}(271.508,319.252) .. tension 1.78319 and 2.5167 ..
 {left}(255.903,300.306) .. tension 1.92488 and 1 ..
 {left}(202.064,328.415) .. tension 0.875499 and 2.95486 ..
 {down}(0.482253,229.552) .. tension 1.29609 and 1 ..
 {right}(143.229,102.72) .. tension 1 and 1 ..
(199.653,97.8974) --
 cycle
;

%%endpaths
enddefchar;
defchar(ain.medi,-1,-1,-1,-1);
%%backgroundimage:images/ain.medi.jpg,-863.202,-1089.41,1,0,0,1
%%beginparams
z1 = (307.519,5.78968);
z11 = (339.024,3.37577);
z3 = (52.7776,5.77647);
z4 = (61.2171,88.9651);

%%beginverbatim

%interim which_in_baseline := 2;

%charlt := 4; % numeric
lefttatweel := charlt +0.5;
righttatweel := charrt + 0.7;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi
y11 := 0;
y12 := y3;
%x12 := x3;
righttatweel_const := (x11 - x1,y11 - y1);

if which_in_baseline = 1 :
y1 := y1 - (lefttatweel - righttatweel - 0.3 )*10;
fi;

z2 = z1 + (penwidth,0) rotated (95  - (lefttatweel - righttatweel - 0.3 )*3);

%z4 = z3 + (penwidth,0) rotated 80;
%%endverbatim

%%beginpaths

fill
(168.036,56.9267) .. tension 1 and 1.04384 ..
z3 join
z4 .. tension 0.75 and 5.12775 ..
 {up}(158.293,138.339) .. tension 30.2005 and 1.12253 ..
 {left}(91.7618,185.922) .. tension 0.75 and 1.98381 ..
 {left}(46.7784,171.842) .. tension 1 and 1 ..
 {up}(39.8727,178.878) .. tension 2.50422 and 0.75 ..
 {right}(177.987,285.614) .. tension 0.870046 and 1.07323 ..
 {down}(329.225,202.026) .. tension 0.992653 and 10000 ..
 {down}(271.355,117.632) .. tension 2.01129 and 1 ..
z2 join
z1 .. tension 1 and 1.37611 ..
(214.448,59.2786) .. tension 1 and 5.31108 ..
 {left}(198.146,76.31) .. tension 10000 and 0.984845 ..
 cycle
;

%%endpaths
enddefchar;
defchar(ain.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/ainmediyeh.jpg,-518.899,-1444.23,1,0,0,1
%%beginparams
z1 = (316.199,3.95715);
z11 = (339.024,3.37577);
z3 = (-89.4178,-48.5602);

%%beginverbatim



righttatweel := charrt + 0.7;
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
y11 := 0;
y12 := y3;

righttatweel_const := (x11 - x1,y11 - y1);

if which_in_baseline = 1 :
y1 := y1 - (lefttatweel - righttatweel - 0.3 )*10;
fi;

z2 = z1 + (penwidth,0) rotated (75  - (lefttatweel - righttatweel - 0.3 )*3);


%%endverbatim

%%beginpaths

fill
(-0.752864,-8.7133) .. tension 1 and 9.64257 ..
 {up}z3 .. tension 1.14137 and 1 ..
(-72.338,-5.35837) .. tension 1.28182 and 8.41731 ..
 {up}(158.293,138.339) .. tension 30.2005 and 1.12253 ..
 {left}(69.3627,177.327) .. tension 0.75 and 1.98381 ..
 {left}(32.0611,167.338) .. tension 1 and 1 ..
 {up}(23.76,174.839) .. tension 2.50422 and 0.75 ..
 {right}(170.753,299.117) .. tension 0.870046 and 1.22519 ..
 {down}(329.225,211.189) .. tension 0.992653 and 6.50352 ..
 {down}(276.66,125.348) .. tension 2.63838 and 1 ..
z2 join
z1 .. tension 1 and 1.37611 ..
(216.859,67.4769) .. tension 1 and 5.31108 ..
 {left}(189.465,87.8841) .. tension 14.6587 and 0.984845 ..
 cycle
;

%%endpaths
%%beginbody
leftbearing := 0;

setCursiveAnchor(yehcursive1,z3,ExitAnchor);
enddefchar;
defchar(ain.fina,-1,-1,-1,-1);
%%backgroundimage:images/ain.fina.jpg,-109.971,-879.929,1,0,0,1
%%beginparams
z1 = (387.279,3.63301);
z3 = (428.723,9.64506);

%%beginverbatim
righttatweel := charrt +0;
z2 = z1 + (0,lineheight ) rotated -12;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);
y4 := lineheight;

%%endverbatim

%%beginpaths

fill
(450,-592) .. tension 0.75 and 1 ..
 {up}(80,-279.482) .. tension 1 and 1 ..
(159.144,18.3256) .. tension 1 and 8.27382 ..
 {up}(231,120) .. tension 2.67669 and 1.88798 ..
 {left}(165,161) .. tension 1 and 1 ..
 {left}(122.929,141.071) .. tension 1 and 1 ..
 {up}(119.305,149) .. tension 1.41534 and 1 ..
 {right}(250.482,262.589) .. tension 1 and 1.38908 ..
 {down}(385.894,192.603) .. tension 0.75 and 3.32415 ..
 {down}(346.894,106.284) .. tension 1.62276 and 1 ..
(359.222,95.7434) .. tension 1 and 1 ..
z2 join
z1 .. tension 1.23661 and 1 ..
(297.619,57.6212) .. tension 1 and 5.79552 ..
 {left}(268.316,93.2544) .. tension 5.67309 and 0.893446 ..
 {down}(125.617,-227.402) .. tension 1 and 0.785103 ..
 {right}(477.029,-513.785) .. tension 1 and 4.60106 ..
 {right}(828.009,-486.832) .. tension 1 and 1 ..
 {down}(852.813,-494.421) .. tension 3.83868 and 1 ..
(749.904,-526.861) .. tension 7.37606 and 1 ..
(665.108,-558.61) .. tension 1 and 0.75 ..
 {left} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(tah.isol,1591,-1,-1,-1);
%%backgroundimage:images/tah.isol.jpg,12,-1019.83,1,0,0,1
%%beginparams
z.mark1 = (708,511);

%%beginpaths

fill
(64.2361,98.9583) .. tension 1 and 1 ..
 {right}(78.8323,106.835) .. tension 9.94627 and 0.75 ..
(245.547,53.8516) .. tension 1 and 1.21526 ..
 {right}(392.651,42.0686) .. tension 17.5873 and 1.90969 ..
 {right}(742.123,298.933) .. tension 0.761717 and 2.54062 ..
 {down}(869.948,222.539) .. tension 1.19075 and 1 ..
 {left}(424.769,-34.7222) .. tension 0.913258 and 3.82773 ..
 {up} cycle
;
fill
(702.104,215.495) .. tension 1.98843 and 45.6676 ..
 {down}(461.247,54.0248) .. tension 1 and 1 ..
 {right}(474.537,46.2963) .. tension 0.946512 and 12.4629 ..
 {up}(819.444,157.407) .. tension 1.55556 and 1 ..
 {left} cycle
;
fill
(486.918,114.829) .. tension 1 and 1 ..
 {up}(497.993,239.117) .. tension 1.74819 and 2.3693 ..
 {up}(436.953,682.472) .. tension 2.4198 and 1 ..
 {up}(406.25,750.579) .. tension 2.4704 and 3.9506 ..
 {right}(452.271,832.693) .. tension 1.01926 and 1 ..
 {down}(458.122,819.163) .. tension 1.76904 and 1 ..
(472.07,776.253) .. tension 0.75 and 2.6241 ..
 {down}(508.455,729.809) .. tension 1.4526 and 2.55197 ..
 {down}(480.469,660.252) .. tension 3.81741 and 1.64557 ..
 {down}(533.871,309.403) .. tension 0.75 and 1.3644 ..
(520.994,127.937) --
 cycle
;

%%endpaths
%%beginbody

setMarkAnchor(topmark,z.mark1);
enddefchar;
defchar(tah.init,-1,-1,-1,-1);
%%backgroundimage:images/tah.init.jpg,-1283.89,-1307.16,1,0,0,1
%%beginparams
z1 = (120.399,4.40296);
z1.5 = (-2.31481,-133.681);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.0  ;






z2 = z1 + (0,penwidth ) rotated (-15 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
(335.648,50.926) .. tension 1 and 1 ..
z1 join
z2 .. tension 0.75 and 4.68442 ..
 {up}(476.852,209.491) .. tension 3.42809 and 1 ..
 {left}(376.157,261.574) .. tension 2.16079 and 1 ..
(105.324,70.0231) --
(32.4074,61.9213) .. tension 0.75 and 2.95672 ..
 {right}(410.686,338.333) .. tension 0.75 and 1.9428 ..
 {down}(510.995,277.778) .. tension 0.75 and 1.09677 ..
 cycle
;
fill
(151.173,139.424) .. tension 1.15287 and 0.956207 ..
 {up}(155.979,275.672) .. tension 2.89756 and 1.54613 ..
(87.7701,623.553) .. tension 1.36479 and 6.42089 ..
(70.6173,727.563) .. tension 1.76189 and 1.89014 ..
 {up}(47.6584,773.727) .. tension 2.4704 and 3.9506 ..
 {right}(89.4238,846.582) .. tension 1.01926 and 1 ..
 {down}(95.2748,833.052) .. tension 1.76904 and 1 ..
(117.903,790.142) .. tension 0.75 and 1.49954 ..
 {down}(154.288,747.749) .. tension 0.75 and 10000 ..
 {down}(117.622,677.034) .. tension 8.031 and 1.52358 ..
 {down}(188.385,310.56) .. tension 0.75 and 0.930166 ..
(169.142,147.034) --
 cycle
;

%%endpaths
enddefchar;
defchar(tah.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/tahinit_yeh.jpg,-2057.19,-1207.56,1,0,0,1
%%beginparams
z1 = (40.783,80.7486);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
z1 .. tension 1.381 and 1 ..
(337.851,241.05) .. tension 1 and 6.53584 ..
 {up}(616.111,330.556) .. tension 3.42809 and 1 ..
 {left}(498.923,385.436) .. tension 2.28154 and 7.38854 ..
 {down}(299.174,222.82) ..
 {up}(234.649,210.571) .. tension 11.1562 and 2.20876 ..
 {right}(540.686,467.5) .. tension 0.75 and 1.9428 ..
 {down}(660.285,386.208) .. tension 1.72427 and 1.41156 ..
(603.09,257.929) .. tension 1.82326 and 1 ..
(347.014,174.498) .. tension 1 and 1 ..
(142.056,124.826) .. tension 1 and 17.6999 ..
 {up} cycle
;
fill
(337.79,299.053) .. tension 1.15287 and 0.956207 ..
 {up}(342.37,438.516) .. tension 2.89756 and 1.54613 ..
(301.087,643.73) .. tension 1.36479 and 6.42089 ..
(275.318,775.47) .. tension 1.76189 and 1.89014 ..
 {up}(251.507,824.624) .. tension 2.4704 and 3.9506 ..
 {right}(295.684,905.115) .. tension 1.01926 and 0.866448 ..
 {down}(303.593,891.81) .. tension 1.76904 and 1 ..
(306.883,869.974) .. tension 0.917007 and 1.37458 ..
 {down}(347.286,815.123) .. tension 1.48383 and 1.76249 ..
 {down}(321.921,763.521) .. tension 8.031 and 1.52358 ..
 {down}(376.384,472.6) .. tension 0.858378 and 0.930166 ..
(352.945,317.739) --
 cycle
;

%%endpaths
%%beginbody


leftbearing := 0;

setCursiveAnchor(yehcursive1,z1,ExitAnchor);
enddefchar;
defchar(tah.medi,-1,-1,-1,-1);
%%beginparams
z11 = (148,15);
z13 = (452,41);

%%beginverbatim



righttatweel_const := (100,0);

%y11 := 0;
y13 := lineheight;
left_verticalextratio := 100;



z12 = z11 + (2*penwidth,penwidth ) ;
%y12 := lineheight;
z14 = (x13,0);

tah.init_(charlt,charrt);
%%endverbatim

%%beginpaths

fill
z11 --
z12 ..
z13 join
z14 ..
 cycle
;

%%endpaths
%%beginbody




enddefchar;
defchar(tah.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/tahmedi_yeh.jpg,-475.47,-1279.96,1,0,0,1
%%beginparams
z1 = (8.73904,-61.7067);
z11 = (269.528,8.24846);
z13 = (452,41);

%%beginverbatim

righttatweel_const := (150,0);

%y11 := 0;
y13 := lineheight;




z12 = z11 + (2*penwidth,penwidth ) ;
%y12 := lineheight;
z14 = (x13,0);

%%endverbatim

%%beginpaths

fill
z11 --
z12 ..
z13 join
z14 ..
 cycle
;
fill
z1 .. tension 1.381 and 1 ..
(272.049,64.3548) .. tension 1 and 3.60332 ..
 {up}(535.842,161.577) .. tension 3.42809 and 1 ..
 {left}(424.923,203.436) .. tension 2.28154 and 7.38854 ..
 {down}(229.514,58.1811) ..
 {up}(149.075,39.1806) .. tension 11.1562 and 2.74589 ..
 {right}(466.686,285.5) .. tension 0.75 and 1.9428 ..
 {down}(586.285,204.208) .. tension 1.72427 and 1.41156 ..
(517.034,75.4467) .. tension 1.82326 and 1 ..
(107.19,-26.2389) .. tension 0.75 and 13.5269 ..
 {up} cycle
;
fill
(274.4,132.485) .. tension 1.15287 and 0.956207 ..
 {up}(278.98,224.205) .. tension 2.89756 and 1.54613 ..
(229.016,492.594) .. tension 1.36479 and 6.42089 ..
(199.871,629.157) .. tension 1.76189 and 1.89014 ..
 {up}(178.954,673.488) .. tension 2.4704 and 3.9506 ..
 {right}(217.558,758.802) .. tension 1.70133 and 0.866448 ..
(238.523,713.065) .. tension 0.917007 and 1.37458 ..
 {down}(273.768,655.307) .. tension 1.48383 and 1.76249 ..
 {down}(250.815,604.669) .. tension 8.031 and 1.52358 ..
 {down}(309.136,308.926) .. tension 0.858378 and 0.930166 ..
(293.413,150.689) --
 cycle
;

%%endpaths
%%beginbody


leftbearing := 0;

setCursiveAnchor(yehcursive1,z1,ExitAnchor);


enddefchar;
defchar(tah.fina,-1,-1,-1,-1);
%%beginparams
z11 = (432,7);
z13 = (853,26);

%%beginverbatim



righttatweel := charrt + 0;
righttatweel_const := (50,0);

%y11 := 0;
y13 := lineheight;



z12 = z11 + (2*penwidth,penwidth ) ;
%y12 := lineheight;
z14 = (x13,0);

%%endverbatim

%%beginpaths

controlledPath (3,0)(__test)(
z11 --
z12 ..
z13 join
z14 ..
 cycle
);

%%endpaths
%%beginbody

tah.isol_(0,0);
currentpicture := currentpicture  shifted (0,- ypart (llcorner currentpicture)); 

fill __test;




enddefchar;
beginchar(alefmaksura.isol,1609,-1,-1,-1);
%%backgroundimage:images/yeh.isol.jpg,-152.194,-1072,1,0,0,1

%%beginpaths

fill
(63.071,77.8372) .. tension 0.933222 and 23.7714 ..
 {right}(131.674,288.558) .. tension 1 and 1 ..
 {down}(139,266) .. tension 2.31842 and 1.16062 ..
 {down}(98.1398,120.93) .. tension 1.11444 and 0.863457 ..
 {right}(299.823,-64.1065) .. tension 0.88904 and 7.72769 ..
 {up}(629,88) .. tension 2.93371 and 1 ..
 {left}(593,101.248) .. tension 1 and 1 ..
 {left}(494,99) .. tension 1 and 1 ..
 {up}(357,184) .. tension 1 and 2.19609 ..
 {right}(583.305,511.894) .. tension 1.08196 and 9.18809 ..
 {down}(667,448) .. tension 1.24106 and 6.97322 ..
 {left}(647.929,370.837) .. tension 1.63568 and 1 ..
 {left}(575,432) .. tension 1.36275 and 4.90662 ..
 {down}(392,262) .. tension 1 and 0.75 ..
 {right}(515.518,176.269) .. tension 1 and 1 ..
 {right}(613,180.035) .. tension 1.1154 and 1.24598 ..
 {down}(679.823,119.497) .. tension 1.98422 and 0.870135 ..
 {left}(271.035,-144.34) .. tension 1.01668 and 0.835859 ..
 {up} cycle
;

%%endpaths
%%beginbody
savepicture:=1;
endchar;
defchar(yehshape.isol,1610,-1,-1,-1);
%%beginbody
draw alefmaksura.isol.pic;
savepicture:=1;
enddefchar;
beginchar(yehshape.fina,-1,-1,-1,-1);
%%backgroundimage:images/yehfina.jpg,-821.243,-1239,1,0,0,1
%%beginparams
z1 = (455.532,-80.6952);
z2 = (729.142,81.4761);

%%beginverbatim
y2 := lineheight;
z3 = z2 - (0,lineheight);

%%endverbatim

%%beginpaths

fill
(102.681,-163.432) .. tension 1.15253 and 4.91385 ..
 {right}(194.826,84.0413) .. tension 1 and 1 ..
 {down}(199.688,75.7073) .. tension 1.37074 and 1 ..
 {down}(145.849,-91.9814) .. tension 1 and 0.756543 ..
 {right}(377.462,-282.057) .. tension 0.9246 and 6.40317 ..
 {up}(672.615,-138.262) .. tension 27.3999 and 1 ..
(569.855,-112.956) .. tension 0.891928 and 3.01261 ..
 {up}z1 .. tension 1.28934 and 2.9362 ..
 {right}z2 endlink
z3 .. tension 1 and 4.78479 ..
 {down}(548.878,-38.0463) .. tension 22.0528 and 0.75 ..
(590.717,-46.5339) .. tension 1 and 0.83062 ..
 {down}(718.129,-119.046) .. tension 1.60425 and 0.907298 ..
 {left}(342.424,-365.49) .. tension 0.832488 and 1 ..
 {up} cycle
;

%%endpaths
%%beginbody

savepicture:=1;
endchar;
defchar(yehshape.fina.ii,-1,-1,-1,-1);
%%backgroundimage:images/ala.jpg,-47.8111,-1250.15,1,0,0,1
%%beginparams
z1 = (400,62);
z2 = (641.667,49.3056);

%%beginpaths

fill
(24,0) .. tension 1.15253 and 9.08662 ..
 {right}(94.4444,218.056) .. tension 1 and 1 ..
 {down}(99.3056,209.722) .. tension 1 and 1 ..
 {down}(60.4167,61.8056) .. tension 1 and 0.915494 ..
 {right}(274.669,-132.128) .. tension 1.09661 and 6.40317 ..
 {up}(591.483,6.7644) .. tension 2.37563 and 1 ..
(502.749,29.7389) .. tension 0.891928 and 4.56354 ..
 {up}z1 .. tension 10000 and 2.9362 ..
 {right}(438.29,115.973) .. tension 6.05533 and 0.75 ..
(504.437,98.8619) .. tension 1.39519 and 0.75 ..
 {right}(593.789,91.2423) .. tension 1 and 1 ..
 {down}z2 .. tension 1.42107 and 1.14232 ..
 {left}(252.652,-213.632) .. tension 0.907684 and 1 ..
 {up} cycle
;

%%endpaths
%%beginbody
rightbearing := x1 - x2;
setCursiveAnchor(yehcursive1,z1,EntryAnchor);
enddefchar;
defchar(yehshape.fina.afterbeh,-1,-1,-1,-1);
%%beginbody

behmediyehfina_(charlt,charrt);

cutGlyph(z2,z3,false);

rightbearing := x2 - x6;

setCursiveAnchor(behmediyehfina,z2, EntryAnchor);
setCursiveAnchor(behmediyehfinartl,z2, EntryAnchorRTL);



enddefchar;
defchar(kaf.isol,1603,-1,-1,-1);
%%backgroundimage:images/kaf.fina.jpg,-68.7566,-977.691,1,0,0,1

%%beginpaths

fill
(41.6667,45.1389) .. tension 1.17418 and 3.62471 ..
 {right}(70.0682,111.7) .. tension 1.47205 and 1 ..
(79.8174,98.5158) .. tension 1.53532 and 1 ..
 {right}(133.102,83.3333) .. tension 1.3288 and 11.9941 ..
 {up}(660.88,141.204) .. tension 5.09844 and 1 ..
(619.282,362.527) .. tension 0.75 and 5.58424 ..
 {up}(552.145,706.897) .. tension 1 and 1.85313 ..
 {up}(526.62,755.208) .. tension 1.36139 and 2.85021 ..
 {right}(576.442,834.811) .. tension 8.99394 and 1 ..
(604.491,779.475) .. tension 1 and 4.95188 ..
 {down}(640.005,733.961) .. tension 1.13875 and 9.07035 ..
 {down}(596.902,669.384) .. tension 4.10535 and 1.14023 ..
(682.808,204.882) .. tension 0.75 and 2.33008 ..
 {dir -120.931}(657.924,64.1008) {dir -88.0701} .. tension 19.7294 and 1 ..
 {left}(113.087,0) .. tension 1 and 1 ..
 {up} cycle
;
fill
(178.916,272.473) .. tension 1.6784 and 1.33888 ..
 {right}(197.723,286.941) .. tension 4.85985 and 0.75 ..
 {right}(284.53,268.133) .. tension 1.49002 and 2.49931 ..
 {up}(399.788,323.11) .. tension 2.06098 and 1.23565 ..
 {up}(304.302,366.512) .. tension 1 and 2.9092 ..
 {right}(404.128,568.094) .. tension 1 and 2.20322 ..
 {down}(438.368,530.478) .. tension 2.69063 and 4.2656 ..
 {left}(406.057,465.374) .. tension 1 and 1 ..
 {left}(377.604,491.898) .. tension 1.89894 and 1.13985 ..
 {down}(342.4,437.886) .. tension 1.23183 and 1 ..
 {down}(437.404,384.838) .. tension 1.06986 and 0.75 ..
 {left}(285.976,237.751) .. tension 1 and 2.03546 ..
 {up} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(kaf.init,-1,-1,-1,-1);
%%backgroundimage:images/kaf.init.jpg,-1599,-1157,1,0,0,1
%%beginparams
z1 = (91.93,14.8357);
z1.5 = (86,-24);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := lefttatweel  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.0  ;


if (lefttatweel < 0 ) :
y1 := y1 + 5 lefttatweel;
fi



z2 = z1 + (0,penwidth ) rotated (-15 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
(157.389,42.463) .. tension 1 and 1 ..
z1 join
z2 ..
(200.213,136.037) .. tension 1 and 1 ..
 {up}(218.472,159.579) .. tension 1.67402 and 5.60646 ..
 {left}(47,414) .. tension 12.2228 and 0.75 ..
 {left}(30.6281,391.605) .. tension 2.52601 and 1 ..
 {up}(-87,495) .. tension 1 and 1.21903 ..
(-53.489,674.283) .. tension 1.59171 and 0.910654 ..
(205.298,887.392) .. tension 1.8592 and 24.1761 ..
 {right}(538.497,1076.81) .. tension 31.2255 and 13.9908 ..
 {dir -117.499}(514.397,994.255) .. tension 19.2158 and 1 ..
(238.841,836.201) .. tension 1 and 2.54535 ..
 {down}(-60.6852,586.69) .. tension 9.18768 and 1 ..
(24.8843,530.671) .. tension 6.25338 and 0.774053 ..
 {down}(247.041,172) .. tension 0.857029 and 1 ..
 cycle
;

%%endpaths
%%beginbody

rightbearing := -200; % 50 - (xpart (urcorner currentpicture) - 172);

enddefchar;
defchar(kaf.init.beforelam,-1,-1,-1,-1);
%%backgroundimage:images/kafinitlammedi.jpg,-2695,-1404,1,0,0,1
%%beginparams
z1 = (-20.8192,-0.262036);
z1.5 = (86,-24);
z4 = (-68.521,386.816);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.0  ;


if (lefttatweel < 0 ) :
y1 := y1 + 5 lefttatweel;
fi



z2 = z1 + (0,penwidth ) rotated (-10 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension 1 and 2.74067 ..
 {up}(103.864,155.811) .. tension 2.23139 and 1 ..
(-53.0478,289.69) .. tension 1 and 1.32211 ..
 {up}(-190.035,381.716) .. tension 0.974983 and 1 ..
(-20.1653,613.503) .. tension 1.59171 and 0.910654 ..
(166.316,762.006) .. tension 1.8592 and 24.1761 ..
 {right}(339.53,882.108) .. tension 6.62921 and 13.9908 ..
 {dir -117.499}(316.453,797.142) .. tension 9.38911 and 1 ..
(199.39,713.293) .. tension 1 and 4.66693 ..
 {down}(-148.974,409.803) .. tension 16.4737 and 0.75 ..
z4 .. tension 2.17049 and 0.891964 ..
 {down}(138.824,175.749) .. tension 0.75053 and 1.07285 ..
 cycle
;

%%endpaths
enddefchar;
defchar(kaf.init.beforelamalef,-1,-1,-1,-1);
%%backgroundimage:images/kaflaf.jpg,-3472.39,-1440.22,1,0,0,1
%%beginparams
z1 = (-38.6963,0.654244);
z1.5 = (86,-24);
z4 = (-168.733,361.397);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.0  ;


if (lefttatweel < 0 ) :
y1 := y1 + 5 lefttatweel;
fi



z2 = z1 + (0,penwidth ) rotated (-15 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension 1 and 2.09088 ..
 {up}(51.185,148.433) .. tension 2.75215 and 1 ..
(-192.631,286.102) .. tension 1 and 0.91803 ..
 {up}(-245.086,333.201) .. tension 1.99864 and 1 ..
(-168.082,497.415) .. tension 2.55094 and 22.7178 ..
 {right}(338.642,860.748) .. tension 1.18385 and 4.7041 ..
 {dir -140.453}(324.403,787.459) .. tension 19.2158 and 1 ..
(-15.454,543.415) .. tension 1 and 3.17741 ..
 {down}(-197.284,379.119) .. tension 3.912 and 1.50507 ..
z4 .. tension 6.25338 and 0.873858 ..
 {down}(80.7638,167.653) .. tension 0.75 and 1.04102 ..
 cycle
;

%%endpaths
enddefchar;
defchar(kaf.init.beforelamfina,-1,-1,-1,-1);
%%backgroundimage:images/kaflam.jpg,-2030,-1271.93,1,0,0,1
%%beginparams
z1 = (-46.2581,-3.87893);
z1.5 = (66,-20);
z4 = (-25.7578,299.715);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := charlt  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.0  ;


if (lefttatweel < 0 ) :
y1 := y1 + 5 lefttatweel;
fi



z2 = z1 + (0,penwidth ) rotated (-15 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
z1 join
z2 .. tension 1 and 2.74067 ..
 {up}(43.972,144.084) .. tension 2.14002 and 1 ..
(-143.847,249.203) .. tension 1 and 0.91803 ..
 {up}(-283.315,316.781) .. tension 1.99864 and 1 ..
(-160.79,507.022) .. tension 1.59171 and 0.910654 ..
(165.834,767.311) .. tension 1.8592 and 24.1761 ..
 {right}(325.905,882.539) .. tension 1.89963 and 3.08676 ..
 {dir -120.218}(312.366,805.005) .. tension 19.2158 and 1.10351 ..
(-182.005,412.782) .. tension 1 and 2.84143 ..
 {down}(-232.736,340.909) .. tension 18.0043 and 1 ..
z4 .. tension 6.25338 and 0.75 ..
 {down}(75.986,164.43) .. tension 0.75053 and 1.07285 ..
 cycle
;

%%endpaths
enddefchar;
defchar(kaf.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/kafinit_yeh.jpg,-1189.91,-1356.48,1,0,0,1
%%beginparams
z1 = (63.3545,113.979);

%%beginverbatim

%%endverbatim

%%beginpaths

fill
(189.647,159.106) .. tension 1 and 11.8038 ..
 {up}z1 .. tension 1 and 2.01759 ..
(72.0774,170.667) .. tension 5.38046 and 1.5951 ..
 {up}(237.119,274.417) .. tension 1.67402 and 5.60646 ..
(184.258,403.144) .. tension 1.35748 and 14.3601 ..
 {left}(95.1314,519.499) .. tension 1.09635 and 1 ..
 {up}(1.0912,592.477) .. tension 1 and 1.21903 ..
(34.6022,771.76) .. tension 1.59171 and 0.910654 ..
(296.676,977.53) .. tension 1.8592 and 24.1761 ..
 {right}(665.176,1176.72) .. tension 2.83286 and 13.9908 ..
 {dir -117.499}(643.576,1099.26) .. tension 19.2158 and 1 ..
(323.483,921.15) .. tension 0.913449 and 2.54535 ..
 {down}(44.1884,690.533) .. tension 9.18768 and 1 ..
(108.925,618.31) .. tension 6.25338 and 0.774053 ..
 {down}(265.109,276.421) .. tension 0.857029 and 1 ..
 cycle
;

%%endpaths
%%beginbody
leftbearing := 0;

setCursiveAnchor(yehcursive1,z1,ExitAnchor);

rightbearing := -300;

enddefchar;
defchar(kaf.medi,-1,-1,-1,-1);
%%backgroundimage:images/kaf.medi.2.jpg,-8202.72,-1558.83,1,0,0,1
%%beginparams
z1 = (459.084,10.094);
z11 = (565.972,-1.73611);
z3 = (93.0659,10.5902);

%%beginverbatim


%charlt := 4; % numeric
lefttatweel := charlt + 1;
righttatweel := charrt +0;
if left_verticalextratio = 0 : left_verticalextratio := 10; fi
if right_verticalextratio = 0 : right_verticalextratio := 10; fi
left_tatweeltension := 1 + charlt * 0.115  ;
if left_tatweeltension > 1.7 : left_tatweeltension := 1.7; fi


y1 := y1 + righttatweel*2;
x1 := x1 + righttatweel*30;
z2 = z1 + (penwidth,0) rotated 65;

z4 = z3 + (penwidth,0) rotated 70;
y11 := 0;
righttatweel_const := (x11 - x1,y11 - y1);
%%endverbatim

%%beginpaths

fill
z3 join
z4 .. tension 1 and 1.89315 ..
 {up}(187.046,147.972) .. tension 3.09503 and 1 ..
(21.2995,386.606) .. tension 0.75 and 2.83376 ..
 {up}(-101.722,505.269) .. tension 1 and 1.12687 ..
(-69.9352,660.63) .. tension 1.72702 and 26.3603 ..
 {right}(509.63,1037.33) .. tension 1 and 1 ..
(515.736,1024.02) .. tension 1 and 3.23637 ..
(490.157,959.184) .. tension 63.8091 and 1 ..
(203.991,800.588) .. tension 0.75 and 3.14117 ..
 {down}(-79.3426,567.343) .. tension 5.64565 and 0.75 ..
(198.958,365.046) .. tension 0.842115 and 1.21701 ..
z2 join
z1 .. tension 1.25837 and 1 ..
(300.745,179.708) .. tension 1.63995 and 5.41674 ..
 {left}(168.915,307.547) .. tension 3.98975 and 1 ..
(196.92,239.921) .. tension 1 and 1 ..
 {down}(208.173,173.611) .. tension 1 and 1 ..
(174.999,58.7474) .. tension 1 and 1 ..
 cycle
;

%%endpaths
enddefchar;
defchar(kaf.init.ii,-1,-1,-1,-1);
%%backgroundimage:images/kaf.init.ii_2.jpg,-2244.17,-980,1,0,0,1
%%beginparams
z1 = (237.212,0.724611);
z1.5 = (132.12,289.272);

%%beginverbatim



if left_verticalextratio = 0 : left_verticalextratio := 10; fi
lefttatweel := lefttatweel  +x1.5/nuqta;


left_tatweeltension := 1 + lefttatweel * 0.0  ;

z2 = z1 + (0,penwidth ) rotated (-15 -lefttatweel) ;

%%endverbatim

%%beginpaths

fill
(935.48,77.388) .. tension 9.24751 and 1 ..
z1 join
z2 ..
(815.486,139.903) .. tension 1 and 10.2147 ..
 {up}(954.463,160.899) .. tension 3.13619 and 1 ..
 {left}(814.435,231.005) .. tension 14.691 and 0.940466 ..
 {left}(88.2669,166.508) .. tension 2.52601 and 2.18255 ..
 {up}(-101.167,212.417) .. tension 1.47306 and 1.21903 ..
(-15.261,378.22) .. tension 1.59171 and 0.910654 ..
(165.333,513.441) .. tension 1.8592 and 1.43529 ..
 {right}(248.816,549.639) .. tension 1.99444 and 6.32225 ..
 {left}(231.749,478.001) .. tension 5.20633 and 1 ..
(56.3776,381.521) .. tension 1 and 7.65937 ..
 {down}(-51.685,275.375) .. tension 4.42215 and 1 ..
 {right}(198.146,235.738) .. tension 1.27328 and 1.70921 ..
 {right}(887.43,308.666) .. tension 1.40423 and 1.28104 ..
 {down}(996.873,213.611) .. tension 1.22035 and 2.68873 ..
 cycle
;

%%endpaths
%%beginbody

%rightbearing := 50 - (xpart (urcorner currentpicture) - 172);

enddefchar;
defchar(kaf.medi.beforemeem,-1,-1,-1,-1);
%%backgroundimage:images/kaf_meem.jpg,-1615.56,-1172.72,1,0,0,1
%%beginparams
z1 = (483.501,14.7329);
z11 = (565.972,-1.73611);
z3 = (36.1214,46.0068);
z4 = (-23.6806,125.926);

%%beginverbatim



righttatweel := charrt +0;
if right_verticalextratio = 0 : right_verticalextratio := 10; fi



y1 := y1 + righttatweel*2;
x1 := x1 + righttatweel*30;
z2 = z1 + (penwidth,0) rotated 70;

y11 := 0;
righttatweel_const := (x11 - x1,y11 - y1);
%%endverbatim

%%beginpaths

fill
z3 .. tension 3.13854 and 1.98199 ..
 {right}z4 .. tension 1.45365 and 1 ..
 {right}(33.5648,120.37) .. tension 1.4488 and 1.89315 ..
 {up}(127.864,193.593) .. tension 3.09503 and 0.877474 ..
(-35.4167,386.806) .. tension 1 and 1.44438 ..
(-101.617,433.828) .. tension 0.75 and 2.83376 ..
 {up}(-115.611,485.825) .. tension 1 and 0.827749 ..
(-26.899,662.482) .. tension 1.72702 and 9.61706 ..
 {right}(469.958,994.49) .. tension 1 and 1 ..
 {down}(476.064,979.32) .. tension 2.39549 and 3.23637 ..
(451.591,915.891) .. tension 28.0512 and 1 ..
(86.745,679.853) .. tension 0.75 and 2.17334 ..
 {down}(-86.9537,516.287) .. tension 5.64565 and 0.75 ..
(177.18,354.435) .. tension 0.842115 and 1.47001 ..
z2 join
z1 .. tension 1.88763 and 1 ..
(293.801,179.708) .. tension 1.63995 and 5.41674 ..
 {left}(136.238,297.941) .. tension 3.98975 and 1 ..
(144.837,233.671) .. tension 1 and 1 ..
 {down}(150.65,153.028) .. tension 1 and 1 ..
(115.971,72.6363) .. tension 1 and 1 ..
 {left} cycle
;

%%endpaths
%%beginbody


setCursiveAnchor(kafmeem,z4, ExitAnchor);
leftbearing :=0;

enddefchar;
defchar(kaf.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/kafmedi_yeh.jpg,-2516,-1219,1,0,0,1
%%beginparams
z1 = (440.299,41.5247);
z11 = (565.972,-1.73611);
z3 = (30.3344,-9.54877);
z4 = (46.9213,26.389);
z5 = (-115.611,485.825);

%%beginverbatim



righttatweel := charrt +0;
if right_verticalextratio = 0 : right_verticalextratio := 10; fi



y1 := y1 + righttatweel*2;
x1 := x1 + righttatweel*30;
z2 = z1 + (penwidth,0) rotated 78;

y11 := 0;
righttatweel_const := (x11 - x1,y11 - y1);
%%endverbatim

%%beginpaths

fill
z3 .. tension 3.36928 and 1.98199 ..
z4 .. tension 2.74334 and 1 ..
(133.102,49.1895) .. tension 1.4488 and 1.17168 ..
 {up}(206.568,117.204) .. tension 2.01189 and 0.877474 ..
(-37.1528,381.019) .. tension 1 and 1.44438 ..
(-101.617,433.828) .. tension 0.75 and 2.83376 ..
 {up}z5 .. tension 1 and 0.827749 ..
(-26.899,662.482) .. tension 1.72702 and 9.61706 ..
 {right}(674.014,1042.49) .. tension 3.23448 and 30.2707 ..
(655.647,963.891) {dir -159.788} .. tension 3.25395 and 1 ..
(233.25,749.227) .. tension 0.75 and 3.14117 ..
 {down}(-86.9537,516.287) .. tension 5.64565 and 0.75 ..
(182.967,348.648) .. tension 0.842115 and 1.47001 ..
z2 join
z1 .. tension 1.88763 and 1 ..
(329.239,140.067) .. tension 1.63995 and 4.19024 ..
 {left}(223.014,226.09) .. tension 9.8865 and 0.862852 ..
 {down}(234.007,150.493) .. tension 1 and 1 ..
(190.624,18.8169) .. tension 1.14744 and 8.81823 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing := x5 - x3;

setCursiveAnchor(yehcursive1,z3,ExitAnchor);

enddefchar;
defchar(kaf.medi.beforelam,-1,-1,-1,-1);
%%backgroundimage:images/kafmedialeffina.jpg,-2351.58,-1332.64,1,0,0,1
%%beginparams
z10 = (275.998,47.282);
z11 = (411,19);
z6 = (261.998,144.282);

%%beginverbatim



kaf.init.beforelam_(charlt,0);

righttatweel_const := (x11 - x10,y11 - y10);

which_in_baseline := 1;

picture pic_;pic_ :=  currentpicture;currentpicture:=nullpicture;

pair dir_;

dir_ := diratpoint z4 of (pathpart pic_);
z4 = z5 + (penwidth,0) rotated 105;
%z6 =  z10 + (penwidth,0) rotated 95;
%%endverbatim

%%beginpaths

fill
z5 ..
z4 {dir_} .. tension 1 and 1 ..
z6 join
z10 .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

addto currentpicture also pic_;
enddefchar;
defchar(kaf.medi.beforelamalef,-1,-1,-1,-1);
%%backgroundimage:images/kafmedialeffina.jpg,-2189.58,-1410.64,1,0,0,1
%%beginparams
z10 = (112.783,136.418);
z11 = (355,-2);
z6 = (180.783,166.418);

%%beginverbatim



kaf.init.beforelamalef_(charlt,0);

righttatweel_const := (x11 - x10,y11 - y10);

which_in_baseline := 1;

picture pic_;pic_ :=  currentpicture;currentpicture:=nullpicture;

pair dir_;

dir_ := diratpoint z4 of (pathpart pic_);
z4 = z5 + (penwidth,0) rotated 105;
%z6 =  z10 + (penwidth,0) rotated 100;
%%endverbatim

%%beginpaths

fill
z5 ..
z4 {dir_} .. tension 1 and 1 ..
z6 join
z10 .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

addto currentpicture also pic_;
enddefchar;
defchar(kaf.medi.beforelamfina,-1,-1,-1,-1);
%%backgroundimage:images/kafmedialeffina.jpg,-2189.58,-1410.64,1,0,0,1
%%beginparams
z10 = (163.27,85.6913);
z11 = (369,-4);

%%beginverbatim



kaf.init.beforelamfina_(charlt,0);

righttatweel_const := (x11 - x10,y11 - y10);

which_in_baseline := 1;

picture pic_;pic_ :=  currentpicture;currentpicture:=nullpicture;

pair dir_;

dir_ := diratpoint z4 of (pathpart pic_);
z4 = z5 + (penwidth,0) rotated 110;
z6 =  z10 + (penwidth,0) rotated 80;
%%endverbatim

%%beginpaths

fill
z5 ..
z4 {dir_} .. tension 1 and 1 ..
z6 join
z10 .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

addto currentpicture also pic_;
enddefchar;
defchar(kaf.fina,-1,-1,-1,-1);
%%backgroundimage:images/kkaf.fina.jpg,-262.167,-1138.89,1,0,0,1
%%beginparams
z1 = (683.006,10.9988);
z3 = (721.065,-12.7315);

%%beginverbatim
righttatweel := charrt +0.5;
z2 = z1 + (0,lineheight ) rotated -10;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);


%%endverbatim

%%beginpaths

fill
(29,99) .. tension 0.951748 and 8.89073 ..
 {right}(98.6792,294.005) .. tension 1 and 1 ..
 {down}(105.005,283.34) .. tension 1 and 1.62263 ..
 {down}(68,186) .. tension 1 and 0.914803 ..
 {right}(278.361,40.4722) .. tension 0.810168 and 3.22513 ..
 {up}(608.991,215.41) .. tension 5.82038 and 2.64594 ..
 {up}(524.694,709.861) .. tension 2.30568 and 4.5919 ..
 {right}(553.944,757.157) .. tension 21.8622 and 1.87371 ..
(612.736,444.157) .. tension 1 and 4.06984 ..
(672,120) .. tension 1 and 1 ..
z2 join
z1 .. tension 1.23661 and 1 ..
(644.624,52.1986) .. tension 1 and 5.64446 ..
 {left}(618.861,164) .. tension 1.6623 and 1 ..
(614.873,155.728) .. tension 1.31527 and 1 ..
 {left}(251.435,-35.2639) .. tension 0.75 and 1.81337 ..
 {up} cycle
;
fill
(160.916,217.129) .. tension 1.6784 and 1.33888 ..
 {right}(183.742,226.573) .. tension 4.85985 and 0.75 ..
 {right}(259.007,219.235) .. tension 1.49002 and 2.49931 ..
 {up}(393.783,287.258) .. tension 2.06098 and 0.961189 ..
 {up}(279.936,345.341) .. tension 1 and 2.02954 ..
 {right}(393.437,532.182) .. tension 1 and 2.20322 ..
 {down}(427.951,501.604) .. tension 2.69063 and 4.2656 ..
 {left}(391.256,436.468) .. tension 0.85293 and 1 ..
 {left}(357.959,459.064) .. tension 2.46845 and 1.6639 ..
 {down}(322.999,411.692) .. tension 1.23183 and 1.17831 ..
 {down}(435.608,349.199) .. tension 1.16642 and 0.75 ..
 {left}(266.976,179.751) .. tension 1 and 2.03546 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(kaf.fina.expa.old,-1,1,-1,-1);
%%backgroundimage:images/kaf_fina_exp.jpg,-847.113,-1504.75,1,0,0,1
%%beginparams
d1 := -30; % numeric
t1 := 1.6; % numeric
t1.2 := 1.4; % numeric
z0.1 = (-11.5741,238.715);
z1 = (804.615,-1.21828);
z100 = (615.972,-272.222);
z3 = (789,-27);
z33 = (-302.621,59.598);
z35 = (-35.37,-42.4);

%%beginverbatim
righttatweel := charrt +0.5;
z2 = z1 + (0,lineheight ) rotated -15;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);


lefttatweel := charlt + 0;

lefttatweel := lefttatweel + x100/nuqta;


z10 = dir d1;
z10p = dir (180+d1);
z12 = dir 40;
z12p = dir (40+180);

z33p = z33 rotatepen 115;

z101 = (lefttatweel*nuqta,197.612);
z102 = (lefttatweel*nuqta,0);

z34p = z34 rotatepen 73;

z34 = z35 + z101;

%%endverbatim

%%beginpaths

fill
(563.274,729.553) .. tension 2.30568 and 3.10736 ..
 {right}(594.212,800.669) .. tension 39.6725 and 1.87371 ..
(647.699,442.148) .. tension 1 and 4.06984 ..
(693.3,188.319) .. tension 0.8828 and 1.46248 ..
z2 join
z1 .. tension 1.23661 and 1 ..
(693.653,61.4418) .. tension 1.80671 and 1 ..
(609.568,423.418) .. tension 1 and 5.64446 ..
 {up} cycle
;
fill
(-422.613,166.554) .. tension 0.75 and 1 ..
 {right}(-414.248,176.084) .. tension 3.69555 and 1 ..
 {dir.d1}z33p .. tension 1.5 and 1.4 ..
 {z12}z34p .. tension 1 ..
 {dir 50.116}((38.8887,110.417)+ z101) {down} .. tension 1.02659 and 1.13298 ..
 {z12p}z34 .. tension 1.4 and 1.4 ..
 {z10p}z33 .. tension 1 and 6.56755 ..
 {up} cycle
;

%%endpaths
%%beginbody


drawcomp(minikaf,identity shifted z0.1);

enddefchar;
defchar(kaf.fina.afterlam,-1,-1,-1,-1);
%%backgroundimage:images/kkaf.fina.jpg,-212.848,-1143.21,1,0,0,1
%%beginparams
z1 = (673.673,9.35991);
z3 = (720.139,-44.4444);

%%beginverbatim
righttatweel := charrt +0.5;
z2 = z1 + (0,lineheight ) rotated -10;

righttatweel_const := (x3 - x1,y3 - y1);
which_in_baseline := 3;

%%endverbatim

%%beginpaths

fill
(28,105) .. tension 0.951748 and 8.89073 ..
 {right}(97.6792,300.005) .. tension 1 and 1 ..
 {down}(104.005,289.34) .. tension 1 and 1.62263 ..
 {down}(67,192) .. tension 1 and 0.914803 ..
 {right}(277.361,46.4722) .. tension 0.810168 and 3.22513 ..
 {up}(607.991,221.41) .. tension 5.82038 and 2.64594 ..
 {up}(523.694,715.861) .. tension 2.30568 and 4.5919 ..
 {right}(552.944,763.157) .. tension 21.8622 and 1.87371 ..
(611.736,450.157) .. tension 1 and 4.06984 ..
(671,126) .. tension 1 and 10000 ..
z2 join
z1 .. tension 1.23661 and 0.764483 ..
 {dir 93.6552}(621.076,171.608) {dir -115.778} .. tension 0.75 and 1 ..
 {left}(250.435,-29.2639) .. tension 0.75 and 1.81337 ..
 {up} cycle
;
fill
(159.916,223.129) .. tension 1.6784 and 1.33888 ..
 {right}(182.742,232.573) .. tension 4.85985 and 0.75 ..
 {right}(258.701,227.318) .. tension 1.49002 and 2.49931 ..
 {up}(392.783,293.258) .. tension 2.06098 and 0.961189 ..
 {up}(278.936,351.341) .. tension 1 and 2.02954 ..
 {right}(392.437,538.182) .. tension 1 and 2.20322 ..
 {down}(426.951,507.604) .. tension 2.69063 and 4.2656 ..
 {left}(390.256,442.468) .. tension 0.85293 and 1 ..
 {left}(356.959,465.064) .. tension 2.46845 and 1.6639 ..
 {down}(321.999,417.692) .. tension 1.23183 and 1.17831 ..
 {down}(434.608,355.199) .. tension 1.16642 and 0.75 ..
 {left}(265.976,185.751) .. tension 1 and 2.03546 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(kaf.medi.ii,-1,-1,-1,-1);
%%backgroundimage:images/kaf.ii.jpg,-724,-1009,1,0,0,1
%%beginparams
z.medi1 = (930.81,87.4816);

%%beginverbatim

kaf.init.ii_(lefttatweel,righttatweel);
righttatweel_const := (100,lineheight - y.medi1 + ypart leftanchor);

picture pic_;

pic_ := currentpicture;

clearit;
%%endverbatim

%%beginpaths

fill
(916.24,22.5043) .. tension 1 and 5.99967 ..
 {right}(906.32,139.799) .. tension 1 and 1 ..
 {down}(917.49,128.874) .. tension 1 and 0.75 ..
z.medi1 join
(937.87,4.2867) .. tension 1.0135 and 1 ..
 cycle
;

%%endpaths
%%beginbody

draw pic_;

%show leftanchor;
%show rightanchor;


enddefchar;
beginchar(zeroindic,1632,-1,-1,-1);
%%backgroundimage:images/10.jpg,-1016.12,-1231.58,1,0,0,1

%%beginpaths

fill
(34.7222,112.269) .. tension 1.54346 and 2.12931 ..
 {right}(57.2917,159.144) .. tension 1 and 3.51889 ..
(120.949,126.736) .. tension 2.77152 and 2.22962 ..
 {left}(98.3796,79.8611) .. tension 1.05601 and 4.53746 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture + 80) ;

endchar;
beginchar(oneindic,1633,-1,-1,-1);
%%backgroundimage:images/14.jpg,-957.352,-1047.22,1,0,0,1

%%beginpaths

fill
(10.4167,331.019) .. tension 1.39326 and 3.56267 ..
 {right}(31.8287,381.366) .. tension 3.47588 and 1 ..
(102.431,237.847) .. tension 1 and 1 ..
 {down}(115.162,141.204) .. tension 1 and 1.88622 ..
 {left}(98.9583,84.4907) .. tension 0.997901 and 1.29754 ..
 {up}(92.0193,89.6797) .. tension 1 and 1.16067 ..
(51.1456,261.772) .. tension 1 and 9.19243 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);

endchar;
beginchar(twoindic,1634,-1,-1,-1);
%%backgroundimage:images/12,-900,-1290,1,0,0,1

%%beginpaths

fill
(5.13054,205.838) .. tension 1 and 1.41582 ..
(34.9381,280.192) .. tension 1.60936 and 7.83212 ..
(123,281) .. tension 3.10443 and 2.06047 ..
(113.472,229.421) .. tension 2.9275 and 5.35714 ..
 {down}(35,228) .. tension 5.45053 and 0.971006 ..
 {down}(98,69) .. tension 1 and 3.11629 ..
 {left}(86,0) .. tension 10.0533 and 0.755454 ..
(48.3149,110.021) .. tension 1 and 1.18746 ..
 {up} cycle
;

%%endpaths
%%beginbody


leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
endchar;
beginchar(threeindic,1635,-1,-1,-1);
%%backgroundimage:images/13.jpg,-1222,-1669,1,0,0,1

%%beginpaths

fill
(52.0833,250) .. tension 1.5355 and 5.0645 ..
 {right}(68.0556,297.222) .. tension 1.83967 and 1 ..
(109.722,244.444) .. tension 1 and 1 ..
(148.64,246.489) .. tension 1 and 2.59833 ..
 {right}(175.694,298.611) .. tension 1.56435 and 1 ..
(183.256,260.015) .. tension 1.24227 and 1.75822 ..
(224.19,251.608) .. tension 1.22582 and 1.7517 ..
 {right}(251.935,296.052) .. tension 4.43896 and 1.84503 ..
 {left}(194.627,194.811) .. tension 1 and 1 ..
 {left}(160.05,205.228) .. tension 1.46964 and 1.8738 ..
 {down}(135.854,183.153) .. tension 1 and 1 ..
 {down}(155.556,90.2778) .. tension 1 and 3.25603 ..
 {left}(143.056,3.47222) .. tension 7.9929 and 1 ..
(98.6111,156.944) .. tension 1.42322 and 1.89974 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);

endchar;
beginchar(fourindic,1636,-1,-1,-1);
%%backgroundimage:images/14,-1145,-1044,1,0,0,1

%%beginpaths

fill
(2.31481,156.829) .. tension 1 and 13.3242 ..
 {right}(89.6991,293.981) .. tension 1 and 1 ..
 {down}(98.3796,287.616) .. tension 1.58017 and 1.57133 ..
(83.912,243.056) .. tension 1.70276 and 1.98762 ..
 {down}(41.6667,208.333) .. tension 1.85734 and 1.15593 ..
 {down}(110.532,165.509) .. tension 1 and 5.37804 ..
 {down}(54.9769,72.338) .. tension 1.60697 and 1 ..
 {right}(123.843,55.5556) .. tension 0.75 and 2.78999 ..
 {right}(173.032,70.0231) .. tension 1 and 1 ..
 {down}(182.292,67.1296) .. tension 2.15303 and 1 ..
 {left}(62.5,1.15741) .. tension 1 and 1 ..
 {up}(25.5594,26.3953) .. tension 0.773543 and 3.07729 ..
 {up}(65.3935,130.208) .. tension 9.08576 and 1.53603 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture + 0) ;

endchar;
beginchar(fiveindic,1637,-1,-1,-1);
%%backgroundimage:images/15.jpg,-1036.94,-1169.24,1,0,0,1

%%beginpaths

fill
(29.7518,-126.427) .. tension 1 and 1.89337 ..
 {right}(117.104,66.9592) .. tension 1.85718 and 1 ..
 {down}(205.556,-61.8056) .. tension 1 and 1 ..
 {left}(115.278,-179.167) .. tension 1 and 1.30033 ..
 {up} cycle
;
fill
(55.5556,-73.6111) .. tension 1 and 1 ..
 {right}(115.972,-124.306) .. tension 1 and 2.91894 ..
 {up}(179.167,-96.5278) .. tension 1 and 3.42169 ..
 {left}(103.472,5.55556) .. tension 1.31392 and 2.24769 ..
 {down} cycle
;

%%endpaths
%%beginbody

leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture + 20) ;

endchar;
beginchar(sixindic,1638,-1,-1,-1);
%%backgroundimage:images/16.jpg,-994,-1024,1,0,0,1

%%beginpaths

fill
(21.2191,222.801) .. tension 1 and 4.26327 ..
 {right}(31.3465,266.204) .. tension 2.332 and 0.764135 ..
 {right}(81.5008,253.665) .. tension 1 and 1 ..
 {right}(154.857,266.204) .. tension 1.58667 and 1 ..
 {down}(159.746,261.22) .. tension 1 and 2.50743 ..
(172.807,127.596) .. tension 1 and 3.75983 ..
 {down}(198.688,51.6011) .. tension 1 and 2.84517 ..
 {left}(181.327,2.89352) .. tension 3.09119 and 1.52788 ..
(139.652,117.214) .. tension 1 and 5.93465 ..
(127.797,208.333) .. tension 32.6586 and 1 ..
(57.8704,198.206) .. tension 1 and 1 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture + 10) ;

endchar;
beginchar(sevenindic,1639,-1,-1,-1);
%%backgroundimage:images/47.jpg,-758.439,-948.17,1,0,0,1

%%beginpaths

fill
(36.8056,249.306) .. tension 1 and 4.05726 ..
 {right}(46.1259,301.681) .. tension 5.01019 and 1.44149 ..
(113.329,208.574) .. tension 1 and 7.40182 ..
 {right}(154.861,120.139) .. tension 8.78231 and 4.29673 ..
 {right}(245.358,307.42) .. tension 2.11447 and 0.997834 ..
 {down}(261.002,259.475) .. tension 10000 and 0.75 ..
(204.716,144.483) .. tension 1 and 37.7752 ..
 {left}(156.507,0.401878) .. tension 6.86261 and 1 ..
(101.572,142.432) .. tension 1 and 3.85191 ..
 {up} cycle
;

%%endpaths
%%beginbody

leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture + 0) ;
savepicture:=1;
endchar;
beginchar(eightindic,1640,-1,-1,-1);
%%backgroundimage:images/8.jpg,-299,-802,1,0,0,1
%%beginbody

draw sevenindic.pic reflectedabout ((0,0),(1,0)) ;
leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture + 0) ;
savepicture:=1;
endchar;
beginchar(nineindic,1641,-1,-1,-1);
%%backgroundimage:images/9.jpg,-561.526,-1265.55,1,0,0,1

%%beginpaths

fill
(16.7824,175.347) .. tension 1 and 1.84159 ..
 {right}(92.0138,307.87) .. tension 1.75664 and 1.13319 ..
(155.093,144.676) .. tension 0.762016 and 4.92235 ..
 {down}(187.317,45.3853) .. tension 1.38635 and 2.78375 ..
 {left}(168.981,0) .. tension 2.29535 and 2.52864 ..
 {dir 102.333}(114.005,144.097) {dir -154.593} .. tension 0.75 and 1 ..
 {left}(69.4444,138.31) .. tension 1 and 2.41462 ..
 {up} cycle
;
fill
(45.1389,222.801) .. tension 1.85915 and 0.808111 ..
 {right}(90.8564,190.972) .. tension 1 and 1.52871 ..
 {up}(111.69,201.389) .. tension 2.0893 and 3.79061 ..
 {left}(76.9676,248.843) .. tension 1 and 2.62157 ..
 {down} cycle
;

%%endpaths
%%beginbody

leftbearing := leftbearingnumber;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture + 0) ;

endchar;
beginchar(tatweel,1600,250,104,0);
%%beginbody
% glyphname:tatweel
%%begincomponents
%%endcomponents
fill (0.0,0.0)
 .. controls (-20.0,0.0) and (-20.0,0.0) .. (-20.0,52.0)
 .. controls (-20.0,104.0) and (-20.0,104.0) .. (0.0,104.0)
 .. controls (78.0,104.0) and (172.0,104.0) .. (250.0,104.0)
 .. controls (270.0,104.0) and (270.0,104.0) .. (270.0,52.0)
 .. controls (270.0,0.0) and (270.0,0.0) .. (250.0,0.0)
 .. controls (172.0,0.0) and (78.0,0.0) .. cycle;
endchar;
beginchar(tah.isol.ii,983121,1114,941,-105);

%%beginpaths

fill
(59,59) .. controls (59,68) and (67,73)  ..
(76,73) .. controls (101.962,73) and (150.872,-9)  ..
(402,-9) .. controls (437.128,-9) and (577,98)  ..
(577,98) .. controls (577.084,103.673) and (577.141,109.34)  ..
(577.141,115) .. controls (577.141,317.806) and (531.62,512.566)  ..
(518,712) .. controls (518,712) and (482,743)  ..
(482,770) .. controls (482,842) and (529.978,941)  ..
(552,941) .. controls (564,941) and (573,932)  ..
(573,920) .. controls (573,896) and (566,882)  ..
(566,858) .. controls (566,834) and (583.977,809)  ..
(605,809) .. controls (614,809) and (620,791)  ..
(620,791) .. controls (620.235,745.799) and (584,719.822)  ..
(584,675) .. controls (606.111,535.254) and (632.043,384.738)  ..
(632.043,241) .. controls (632.043,205.843) and (631.517,168.09)  ..
(628,134) .. controls (628,135.171) and (739.541,249)  ..
(924,249) .. controls (979,249) and (1051,210.087)  ..
(1051,187) .. controls (1051,-39.002) and (707,-105)  ..
(404,-105) .. controls (202.878,-105) and (59,6)  ..
 cycle
;
fill
(834,157) .. controls (705,157) and (542,-1)  ..
(542,-1) .. controls (542,-8.15039) and (558.94,-11.8584)  ..
(586,-11.8584) .. controls (693.952,-11.8584) and (958.417,66.1914)  ..
(960,117) .. controls (960,132.231) and (886,157)  ..
 cycle
;

%%endpaths
endchar;
beginchar(noon.fina.test,983147,261,321,0);
%%backgroundimage:D:/projects/Fonts/design/BackgroundImages/noon.fina.tawil.jpg,-246.944,-1367.45,1.18,0,0,1.18
%%beginparams
elongation := 1624; % numeric
ltens1 := 13.428; % ltens (0,10)
ltens2 := 1.42158; % ltens (0,1)
ltens3 := 0.75; % ltens (0,2)
ltens5 := 0.75; % ltens (0,4)
ltens6 := 1.69389; % ltens (0,5)
ltens7 := 2.16414; % ltens (0,6)
ltens77 := 0.953283; % ltens (0,7)
ltens8 := 2.12005; % ltens (0,8)
points1 := (-128.736,213.696);
points2a := (-9.552,-8.208);
points3a := (157.022,-200.285);
points4a := (1392.73,-303.131);
points5a := (62.642,67.2976);
points6a := (-35.018,217.567);
points77a := (1629.4,-54.0648);
points7a := (-77.048,156.655);
points8a := (31.68,108);
points9a := (61.92,-138.24);
rtens1 := 0.840057; % rtens (0,1)
rtens2 := 4.00493; % rtens (0,1)
rtens4 := 4.14795; % rtens (0,3)
rtens5 := 2.79855; % rtens (0,4)
rtens6 := 1.53629; % rtens (0,5)
rtens7 := 1.06482; % rtens (0,6)
rtens77 := 2.16042; % rtens (0,7)
rtens9 := 0.75; % rtens (0,9)
var3a := 0.2; % numeric
var3b := -313.65; % numeric
%%beginbody
% glyphname:behshape.medi
%%begincomponents
%%endcomponents
%points3 := (60 + elongation ,160 + elongation * (0.5/12));
points77:= points1 + (elongation,-54.0648);
points2:= points1 + points2a;
points3:= points1 + (var3a*elongation,var3b);
points3:= points1 + (326,var3b);
points4:= points1 +  (0.854*elongation,-303.131);
points5:= points77 + points5a;
points6:= points77 + points6a;
points7:= points77 + points7a;
points8:= points4 + (110,0) rotated 75;
points9:= points3 + (110,0) rotated 100;
rtens8 := rtens3; % rtens (0,8)
ltens9 := ltens4; % ltens (0,9)
path pp[];
pp1 :=  points3 {dir -19} ..   points4 {dir 17 };
pp2 := points9 {dir -19 } ..   points8 {dir 17};
pp3 := reverse pp2;
fill  points1 {left}
 .. tension rtens1 and ltens2 ..{down }  points2 
 .. tension rtens2 and ltens3 ..   pp1
 .. tension rtens4 and ltens5 .. {up} points5
 .. tension rtens5 and ltens6 .. {left} points6 
 .. tension rtens6  and ltens7 ..  {down} points7 
 .. tension rtens6  and ltens77 ..  {down} points77 
 ..  tension rtens77  and ltens8 .. pp3 
.. tension rtens9  and ltens1 .. {left} cycle; 

%labelspoints.top(1,2,3,4,5,6,7,77,8,9);
endchar;
defchar(noonsinna,-1,0,-1,-1);
%%backgroundimage:images/noon.taweel.jpg,-1102.83,-1042.14,1,0,0,1
%%beginparams
z1 = (296.92,130.759);

%%beginpaths

fill
(219.977,272.33) .. tension 2.81433 and 3.82732 ..
 {right}(242.103,357.692) .. tension 6.53539 and 1 ..
(314.766,254.117) .. tension 1.09902 and 0.75 ..
 {down}((29.7766,29.1227)+ z1) .. tension 4.77746 and 6.30243 ..
 {up}z1 .. tension 1.12909 and 5.40337 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(behsinna,-1,0,-1,-1);
%%backgroundimage:images/beh.isol.expa.jpg,-1314.69,-1042.05,1,0,0,1
%%beginparams
z1 = (298.581,107.599);
z2behsinna = (342.076,137.056);

%%beginpaths

fill
(208.977,233.33) .. tension 2.81433 and 2.61563 ..
 {right}(242.891,310.138) .. tension 6.53539 and 1 ..
(309.177,247.635) .. tension 1.87851 and 0.75 ..
 {down}z2behsinna .. tension 4.77746 and 0.787797 ..
 {dir -156.111}(294.416,63.7506) {curl1} ..
 {up}z1 .. tension 1.30421 and 5.40337 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(expaform.ii,-1,-1,-1,-1);
%%backgroundimage:images/beh.isol.expa.jpg,-336.256,-941.48,1,0,0,1
%%beginparams
d1 := 169.44; % numeric
z33 = (-230.163,45.5019);
z34 = (863.733,83.4418);
z35 = (974.307,215.364);

%%beginverbatim

lefttatweel := lefttatweel -2;

z33p = z33 rotatepen 105;
z34p = z34 rotatepen 72;

z101 = (-lefttatweel*nuqta,0 );
z102 = (-(lefttatweel+2)*nuqta / 1.5, -(lefttatweel +2) *7 );

z10 = dir d1;
z10p = -z10;






%%endverbatim

%%beginpaths

fill
((-385.986,177.089)+ z101) .. tension 1.10251 and 0.877686 ..
 {right}((-379.955,187.306)+ z101) .. tension 3.69555 and 0.958307 ..
((-326.392,163.92)+ z101) ..
z33p + z101 .. tension atleast 0.977039 ..
((382.566,70.8723)+ z102) {right} .. tension atleast 1.28097 ..
z34p ..
 {curl1}z35 {down} ..
z34 .. tension atleast 1.13363 ..
((334.149,-7.3231)+ z102) {left} .. tension atleast 1.32141 ..
z33 + z101 .. tension 1.84753 and 3.02182 ..
 {up} cycle
;

%%endpaths
%%beginbody



enddefchar;
defchar(expaform.i,-1,0,-1,-1);
%%backgroundimage:images/noon.taweel.jpg,-548.542,-1065.9,1,0,0,1
%%beginparams
ll := 21; % numeric
rr := 45; % numeric
ten := 1.22; % numeric
z1 = (1329.01,156.631);
z3 = (312.809,-102.498);

%%beginverbatim

lefttatweel := lefttatweel   - 4;

x1 := x1 + lefttatweel *  nuqta;


z3p = z3 rotatepen 95;

z10 = dir -ll;
z10p =-z10;
z12 = dir (rr   - lefttatweel );
z12p = -z12;

if  known param1 and known param2 :
z1dis = z1 + (param1,param2);
else:
z1dis = (29.7766,29.1227)+ z1;
fi;


	


%%endverbatim

%%beginpaths

fill
z1dis {down} .. tension atleast 1 ..
 {z12p}((-57.1759,-141.879)+ z1) .. tension ten ..
z3 {z10p} .. tension 0.75 and 2.05837 ..
(96.2191,33.8006) .. tension 2.25106 and 4.7386 ..
 {up}(44.0254,104.838) .. tension 0.75 and 1.21338 ..
 {right}(50.9714,115.33) .. tension 13.4901 and 1 ..
(144.444,55.5556) .. tension 1 and 1.69565 ..
 {z10}z3p .. tension ten ..
 {z12}((-40.5093,-54.7497)+ z1) .. tension 0.779442 and 2.27884 ..
 {up}z1 .. tension 2.08632 and 2.37393 ..
 cycle
;

%%endpaths
%%beginbody



enddefchar;
defchar(behshape.fina.expa,-1,-1,-1,-1);
%%backgroundimage:images/behshape.fina.expa.ii.jpg,-2739,-1324,1,0,0,1
%%beginparams
z1 = (-57.1139,358.285);

%%beginverbatim




righttatweel_const := (10,0);


%%endverbatim

%%beginpaths

fill
(-25.5694,313.282) .. tension 1 and 1.05645 ..
 {dir 112.833}z1 {dir 36.4847} .. tension 0.886502 and 1 ..
(9.25926,420.139) .. tension 1 and 2.11294 ..
 {right}(36.4861,460.259) .. tension 1 and 1.46819 ..
 {down}(43.8565,445.736) .. tension 1 and 2.31366 ..
 {down}(8.48611,363.545) .. tension 2.82467 and 1 ..
(63.932,331.967) join
(126.834,237.5) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody



path sinna;
sinna := pathpart currentpicture;

currentpicture:= nullpicture;

lefttatweel := lefttatweel - 2;

expaform.ii_(lefttatweel,0);

pair shift;

shift := (z35-z1);

currentpicture := currentpicture shifted  - shift;

path p;

p := pathpart currentpicture;

currentpicture:= nullpicture;

%sinna := sinna  shifted shift;
fill  subpath(0,4) of p .. tension 1.2 .. subpath(1,11) of sinna {curl1}  .. subpath(7,length p) of p ..  cycle;

%fill subpath(0,4) of p .. z35 ..  subpath(6,length p ) of p .. cycle;

%rightanchor := rightanchor +  shift;

tr_  := tr_ shifted (0,-240);



enddefchar;
defchar(noonshape.expa,-1,-1,-1,-1);
%%backgroundimage:images/noon.taweel.jpg,-15.828,-1062.14,1,0,0,1
%%beginbody


expaform.i_(lefttatweel,0);


z1000 = z1;x1 := whatever;y1 := whatever;

picture sinna;


sinna := image(noonsinna_(lefttatweel,0));

draw sinna shifted (z1000 - z1);

x1:= x1000;
y1:= y1000;


enddefchar;
defchar(feh.isol.expa,-1,-1,-1,-1);
%%backgroundimage:images/beh.isol.expa.jpg,-103.142,-1056.74,1,0,0,1
%%beginbody

lefttatweel := lefttatweel - 2;

picture pic;
path p;
pic := feh.isol.pic;

i := 1;

for item within pic:
p := pathpart item;
if i = 1: 
fill p;
z10 = point 5 of p;
z11 = point 11 of p;
cutGlyph(z10,z11,true);
else:
fill p;
fi;
i := i + 1;

endfor



pic := currentpicture;

currentpicture:= nullpicture;

points10 := z10;
points11 := z11;

begingroup;
clearxy;
expaform.ii_(lefttatweel,0);
p := pathpart currentpicture;
currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;
points1 := z35 ;
fill subpath(0,4) of p .. tension 1.4 ..  points1 - (10,15) ..  points1 {up}  .. (points1 + points11 -points10 ) {down} ..  subpath(7,length p ) of p .. cycle;

endgroup;


draw pic shifted (points1 - z10);





enddefchar;
defchar(feh.fina.expa,-1,-1,-1,-1);
%%beginbody

begingroup;
clearxy;
feh.fina_(lefttatweel,righttatweel);
endgroup;

picture pic;
path p;
pic := currentpicture;

i := 1;

for item within pic:
p := pathpart item;
if i = 1: 
fill p;
z10 = point 5 of p;
z11 = point 15 of p;
cutGlyph(z10,z11,true);
else:
fill p;
fi;
i := i + 1;

endfor



pic := currentpicture;

currentpicture:= nullpicture;

(param1,param2) = z11 - z10;


begingroup;
clearxy;
expaform.ii_(lefttatweel,0);
p := pathpart currentpicture;
currentpicture:= nullpicture;
fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z35 - (170,70);
endgroup;

pair shift;

shift := (points1 - z10);

currentpicture := currentpicture shifted -shift;

draw pic; % shifted shift;

enddefchar;
defchar(behshape.isol.expa,-1,-1,-1,-1);
%%backgroundimage:images/beh.isol.expa.jpg,-423.44,-1130.56,1,0,0,1
%%beginbody

lefttatweel := lefttatweel - 2;

expaform.ii_(lefttatweel,0);




picture sinna;


sinna := image(behsinna_(lefttatweel,0));

draw sinna shifted (z35 - z2behsinna);




enddefchar;
defchar(kaf.fina.expa,-1,-1,-1,-1);
%%backgroundimage:images/kaf_fina_expa_3.jpg,-588.333,-1270.33,1,0,0,1
%%beginparams
d1 := -25; % numeric
t1 := 1.6; % numeric
t1.2 := 1.4; % numeric
z0.1 = (-83,244);
z1 = (804.615,-1.21828);
z3 = (789,-27);
z33 = (-341.521,56.1133);
z34 = (550.903,174.065);

%%beginverbatim
righttatweel := righttatweel +0.5;
z2 = z1 + (0,lineheight ) rotated -15;
y3 := 0;
righttatweel_const := (x3 - x1,y3 - y1);



z33p = z33 rotatepen 110;

z101 = (-lefttatweel*nuqta,0 );

z34p = z34 rotatepen 73;

z36p = z36 rotatepen 75;

if lefttatweel < 0 :
  z.rtt.1 = (-3*lefttatweel,lefttatweel * 12);
  z10 = dir (d1 - 2 * lefttatweel); 
  z12 = dir (40 );
else :
  z.rtt.1 = (0,0);
  z10 = dir (d1 + 0.2 * lefttatweel); 
  z12 = dir (40 -  0.5 * lefttatweel );
fi;

z10p = -z10;
z12p = -z12;

show lefttatweel;

%%endverbatim

%%beginpaths

fill
(569.274,730.553) .. tension 2.30568 and 3.10736 ..
 {right}(594.212,800.669) .. tension 39.6725 and 1.87371 ..
(647.699,442.148) .. tension 1 and 4.06984 ..
(693.3,188.319) .. tension 0.8828 and 1.46248 ..
z2 join
z1 .. tension 1.23661 and 1 ..
(693.653,61.4418) .. tension 1.80671 and 1 ..
(609.568,423.418) .. tension 1 and 5.64446 ..
 {up} cycle
;
fill
((-493.15,166.338)+ z101) .. tension 1.10251 and 1 ..
 {right}((-474.128,173.395)+ z101) .. tension 3.69555 and 1 ..
 {z10}z33p + z101 .. tension 1.68741 and 1.05244 ..
 {z12}z34p + z.rtt.1 .. tension 1 ..
 {dir 50.116}((645.555,329.25)+ z.rtt.1) {down} .. tension 0.807558 and 1.13298 ..
 {z12p}z34 + z.rtt.1 .. tension 1.19435 and 1.32807 ..
 {z10p}z33 + z101 .. tension 2.32688 and 2.53062 ..
 {up} cycle
;

%%endpaths
%%beginbody

drawcomp(minikaf,identity shifted (z0.1 + z101/2));

enddefchar;
defchar(kaf.fina.afterlam.expa,-1,-1,-1,-1);
%%backgroundimage:images/kaf.fina.afterlam.expa.jpg,-1213,-1921,1,0,0,1
%%beginparams
d1 := -25; % numeric
t1 := 1.6; % numeric
t1.2 := 1.4; % numeric
z0.1 = (8,196);
z1 = (780.637,-111.421);
z3 = (777.84,-131.68);
z33 = (-227.681,10.4333);
z34 = (630.743,81.2183);

%%beginverbatim
righttatweel := righttatweel +0.5;
z2 = z1 + (0,lineheight ) rotated -15;

righttatweel_const := (x3 - x1,y3 - y1);



z33p = z33 rotatepen 110;

z101 = (-lefttatweel*nuqta,0 );

z34p = z34 rotatepen 73;

z36p = z36 rotatepen 75;

if lefttatweel < 0 :
  z.rtt.1 = (-3*lefttatweel,lefttatweel * 12);
  z10 = dir (d1 - 2 * lefttatweel); 
  z12 = dir (40 );
else :
  z.rtt.1 = (0,0);
  z10 = dir (d1 + 0.2 * lefttatweel); 
  z12 = dir (40 -  0.5 * lefttatweel );
fi;

z10p = -z10;
z12p = -z12;



which_in_baseline := 3;

%%endverbatim

%%beginpaths

fill
(612.781,628.151) .. tension 2.30568 and 3.10736 ..
 {right}(644.663,696.183) .. tension 39.6725 and 1.87371 ..
(699.539,374.468) .. tension 1 and 4.06984 ..
(734.723,142.861) .. tension 0.8828 and 1.01077 ..
z2 join
z1 .. tension 1.01309 and 0.857527 ..
(716.326,-18.7382) .. tension 1.80671 and 1 ..
(653.769,355.044) .. tension 1 and 5.64446 ..
 {up} cycle
;
fill
((-376.477,128.991)+ z101) .. tension 1.10251 and 1 ..
 {right}((-365.621,147.382)+ z101) .. tension 3.69555 and 10000 ..
 {z10}z33p + z101 .. tension 1 and 2 ..
 {z12}z34p + z.rtt.1 .. tension 1.706 ..
 {dir 50.116}((704.062,208.237)+ z.rtt.1) {down} .. tension 0.807558 and 1.13298 ..
 {z12p}z34 + z.rtt.1 .. tension 2 and 1 ..
 {z10p}z33 + z101 .. tension 2.32688 and 2.53062 ..
 {up} cycle
;

%%endpaths
%%beginbody

drawcomp(minikaf,identity shifted (z0.1 + z101/2));

enddefchar;
defchar(noon.isol.expa,-1,-1,-1,-1);
%%backgroundimage:images/noon.taweel.jpg,-12.828,-1062.14,1,0,0,1
%%beginparams
z11 = (696.824,159.666);
%%beginbody



noonshape.expa_(lefttatweel,0);

x11 := x11 + lefttatweel *  nuqta / 2;

draw one_dot.pic  shifted z11;

setCursiveAnchor(behnoon,(15,26.0528)+ z1, EntryAnchorRTL);
setCursiveAnchor(behmedinoon,(15,26.0528)+ z1, EntryAnchor);

enddefchar;
defchar(alefmaksura.isol.expa,-1,-1,-1,-1);
%%beginbody

draw  alefmaksura.isol.pic;

path p;
p := pathpart currentpicture;

z10 = point 5 of p;
z11 = point 16 of p;
cutGlyph(z10,z11,true);

picture pic;

pic := currentpicture;

currentpicture:= nullpicture;


(param1,param2) = z11 - z10;


begingroup;
clearxy;
expaform.i_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

draw pic shifted (points1 - z10);


enddefchar;
defchar(noon.fina.expa,-1,-1,-1,-1);
%%backgroundimage:images/noon.fina.jpg,-81.196,-1162.59,1,0,0,1
%%beginparams
z2b = (598.433,28.4592);
z3b = (701.277,82.7086);

%%beginverbatim

z1b = z2b rotatepen 95;

righttatweel := righttatweel  + 0.0;

righttatweel_const := (x3b - x1b,y3b - y1b);

%%endverbatim

%%beginpaths

fill
z1b join
z2b --
 cycle
;

%%endpaths
%%beginbody

picture pic;

pic := currentpicture;
clearit;



noon.isol.expa_(lefttatweel,0);

pair disp;

disp := (-15,86.0528)+ z1 - ((z1b + z2b) / 2);

 rightanchor := z_4 shifted disp;

draw pic shifted disp;

tr_ := tr_ shifted (0,-ypart disp);


enddefchar;
defchar(yehshape.isol.expa,-1,-1,-1,-1);
%%beginbody
alefmaksura.isol.expa_(lefttatweel,0);
enddefchar;
defchar(yehshape.fina.afterbeh.expa,-1,-1,-1,-1);
%%beginbody

yehshape.fina.afterbeh_(lefttatweel,0);

path p;
p := pathpart currentpicture;

z10 = point 5 of p;
z11 = point 10 of p;

cutGlyph(z10,z11,true);



picture pic;

pic := currentpicture;

currentpicture:= nullpicture;


(param1,param2) = z11 - z10;


begingroup;
clearxy;
expaform.i_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p .. subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

draw pic shifted (points1 - z10);

rightbearing := x2 - x6;


setCursiveAnchor(behmediyehfina,z2 shifted (points1 - z10), EntryAnchor);
setCursiveAnchor(behmediyehfinartl,z2 shifted (points1 - z10), EntryAnchorRTL);



enddefchar;
defchar(yehshape.fina.ii.expa,-1,-1,-1,-1);
%%beginbody

yehshape.fina.ii_(lefttatweel,0);

path p;
p := pathpart currentpicture;

z10 = point 5 of p;
z11 = point 11 of p;

cutGlyph(z10,z11,true);

picture pic;

pic := currentpicture;

currentpicture:= nullpicture;


(param1,param2) = z11 - z10;


begingroup;
clearxy;
expaform.i_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p .. subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

draw pic shifted (points1 - z10);

rightbearing := x1 - x2;
setCursiveAnchor(yehcursive1,z1 shifted (points1 - z10),EntryAnchor);
enddefchar;
defchar(yehshape.fina.expa,-1,-1,-1,-1);
%%beginbody

draw  yehshape.fina.pic;

path p;
p := pathpart currentpicture;

z10 = point 5 of p;
z11 = point 13 of p;
cutGlyph(z10,z11,true);

picture pic;

pic := currentpicture;

currentpicture:= nullpicture;


(param1,param2) = z11 - z10;


begingroup;
clearxy;
expaform.i_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

draw pic shifted (points1 - z10);

currentpicture := currentpicture shifted   (-points1 + z10);



enddefchar;
defchar(sad.isol.expa,-1,-1,-1,-1);
%%beginbody



picture pic;
path p;
pic := sad.isol.pic;

i := 1;

for item within pic:
p := pathpart item;
if i = 1: 
fill p;
z10 = point 8 of p;
z11 = point 13 of p;
cutGlyph(z10,z11,true);
else:
fill p;
fi;
i := i + 1;

endfor



pic := currentpicture;

currentpicture:= nullpicture;


%(param1,param2) = z11 - z10;


begingroup;
clearxy;
noonshape.expa_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

draw pic shifted (points1 - z11 + (10,40));

currentpicture := currentpicture shifted (0,-100);

enddefchar;
defchar(sad.fina.expa,-1,-1,-1,-1);
%%beginbody

begingroup;
clearxy;
sad.fina_(lefttatweel,righttatweel);
endgroup;


picture pic;
path p;
pic :=currentpicture;

i := 1;

for item within pic:
p := pathpart item;
if i = 1: 
fill p;
z10 = point 9 of p;
z11 = point 21 of p;
cutGlyph(z10,z11,true);
else:
fill p;
fi;
i := i + 1;

endfor



pic := currentpicture;

currentpicture:= nullpicture;


%(param1,param2) = z11 - z10;


begingroup;
clearxy;
noonshape.expa_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

draw pic shifted (points1 - z11 + (10,40));


currentpicture := currentpicture shifted (0,-100);

rightanchor := rightanchor +  (points1 - z11 + (10,40)) +  (0,-100);

tr_ := tr_ shifted (0,-80);

enddefchar;
defchar(seen.isol.expa,-1,-1,-1,-1);
%%beginbody

picture pic;
path p;
pic := seen.isol.pic;

i := 1;

for item within pic:
p := pathpart item;
if i = 1: 
fill p;
z10 = point 9 of p;
z11 = point 24 of p;
cutGlyph(z10,z11,true);
else:
fill p;
fi;
i := i + 1;

endfor



pic := currentpicture;

currentpicture:= nullpicture;


%(param1,param2) = z11 - z10;


begingroup;
clearxy;
noonshape.expa_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

draw pic shifted (points1 - z11 + (30,45));

currentpicture := currentpicture shifted (0,-100);

enddefchar;
defchar(seen.fina.expa,-1,-1,-1,-1);
%%beginbody

begingroup;
clearxy;
seen.fina_(lefttatweel,righttatweel);
endgroup;


picture pic;
path p;
pic :=currentpicture;

i := 1;

for item within pic:
p := pathpart item;
if i = 1: 
fill p;
z10 = point 8 of p;
z11 = point 26 of p;
cutGlyph(z10,z11,true);
else:
fill p;
fi;
i := i + 1;

endfor



pic := currentpicture;

currentpicture:= nullpicture;


%(param1,param2) = z11 - z10;


begingroup;
clearxy;
noonshape.expa_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

currentpicture := currentpicture shifted - (points1 - z11 + (30,45)) ;

draw pic; % shifted (points1 - z11 + (30,45));




%rightanchor := rightanchor +  (points1 - z11 + (30,45)) +  (0,-100);

enddefchar;
defchar(qaf.isol.expa,-1,-1,-1,-1);
%%beginbody

picture pic;
path p;
pic := qaf.isol.pic;

i := 1;

for item within pic:
p := pathpart item;
if i = 1: 
fill p;
z10 = point 5 of p;
z11 = point 10 of p;
cutGlyph(z10,z11,true);
else:
fill p;
fi;
i := i + 1;

endfor



pic := currentpicture;

currentpicture:= nullpicture;

(param1,param2) = z11 - z10;


begingroup;
clearxy;
expaform.i_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

draw pic shifted (points1 - z10);


enddefchar;
defchar(qaf.fina.expa,-1,-1,-1,-1);
%%beginbody

begingroup;
clearxy;
qaf.fina_(lefttatweel,righttatweel);
endgroup;

picture pic;
path p;
pic := currentpicture;

i := 1;

for item within pic:
p := pathpart item;
if i = 1: 
fill p;
z10 = point 5 of p;
z11 = point 15 of p;
cutGlyph(z10,z11,true);
else:
fill p;
fi;
i := i + 1;

endfor



pic := currentpicture;

currentpicture:= nullpicture;

(param1,param2) = z11 - z10;


begingroup;
clearxy;
expaform.i_(lefttatweel,0);
%p := pathpart currentpicture;
%currentpicture:= nullpicture;
%fill point 0 of p {dir -90} ... subpath(1,length p ) of p .. cycle;

points1 := z1;
endgroup;

pair shift;

shift := (points1 - z10);

draw pic shifted shift;

rightanchor := rightanchor  + shift;

which_in_baseline := 2;


enddefchar;
