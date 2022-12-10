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
%%backgroundimage:images/yeh_hah.init.jpg,-2429.84,-1311.69,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
z1 = (251.667,49.1564);
z3 = (93.5904,268.97);
z4 = (241.056,269.778);
z5 = (707.795,178.635);
z6 = (48.3432,203.355);

%%beginverbatim


 
lefttatweel := lefttatweel + 0;


%%endverbatim

%%beginpaths

fill
bldmvi(z1,(-105.6,-48)) ..
bldmvi((197.701,31.9554),(-272.267,-96.3)) ..
bldmvi((157.16,27.5801),(-609.867,-133.1)) ..
bldmvi((126.902,27.0029),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((172.881,116.696),(-589.8,-136.7)) ..
bldmvi((218.136,120.146),(-297.9,-99.1667)) ..
bldmvi((277.587,130.289),(-145.2,-52.8)) .. tension 1 and 1 ..
(342.037,151.47) .. tension 7.16575 and 7.16575 ..
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
(339.635,234.802) .. tension atleast 1 ..
(578.344,187.71) .. tension atleast 1 ..
 {right}(705.637,186.045) .. tension 1 ..
 {down}z5 .. tension 3.46819 ..
 {left}(654.804,108.009) .. tension 2.71262 and 0.75 ..
(532.972,121.608) .. tension 1 ..
(380,103) .. tension 1 ..
(290.554,68.2335) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(behmediyehfina,-1,1100,-1,-1);
%%backgroundimage:images/behmediyehfina.jpg,-2931.69,-1241.9,1,0,0,1
%%beginparams
z1 = (328.914,133.709);
z2 = (495,58);
z3 = (487,127);
z4 = (678,246);
z6 = (617,79);

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
(816.454,220.332) .. tension 1 and 1 ..
 {left}z5 .. tension 1 and 1 ..
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
leftVerticalRatio := 6;
lefttatweel := 1.14;
z1 = (240.526,53.1637);
z3 = (99.2837,288.729);
z4 = (312.09,276.392);
z5 = (643.65,229.071);

%%beginverbatim


 
lefttatweel := charlt + 0;


%%endverbatim

%%beginpaths

fill
bldmvi(z1,(-32.5,-12.6389)) ..
bldmvi((169.089,23.0307),(-256.667,-85.6944)) ..
bldmvi((132.006,18.758),(-610.139,-130.833)) ..
bldmvi((123.044,18.3225),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((149.654,106.573),(-602.639,-132.5)) ..
bldmvi((181.237,109.858),(104.712,42.7263)) ..
bldmvi((274.704,147.703),(53.3333,26.9444)) .. tension 1 and 2.0868 ..
 {up}(339.876,184.562) .. tension 10.8298 and 1.79081 ..
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
(354.271,126.025) .. tension 1 and 1 ..
 cycle
;

%%endpaths
enddefchar;
beginchar(notdef,0,-1,-1,-1);
%%backgroundimage:images/nodef.png,-752.949,-1178.7,8,0,0,8

%%beginpaths

fill
(-42.2071,588.054) .. tension 1 and 1 ..
 {right}(165.312,781.92) .. tension 1 and 1 ..
 {down}(350.438,636.595) .. tension 1 and 1 ..
 {left}(279.936,518.4) .. tension 1 and 1 ..
 {up}(186.624,586.829) .. tension 0.93744 and 1.09623 ..
 {up}(256.666,697.075) .. tension 1 and 1 ..
 {left}(184.55,740.275) .. tension 1 and 1 ..
 {down}(78.7968,659.405) .. tension 1 and 1 ..
 {down}(201.898,363.101) .. tension 1 and 3.43742 ..
 {left}(168.269,223.776) .. tension 1.56096 and 1 ..
 {up}(151.046,269.088) .. tension 1 and 1 ..
 {up} cycle
;
fill
(73.6776,83.6076) .. tension 1 and 1 ..
(234.921,86.5935) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody
% glyphname:space
%%begincomponents
%%endcomponents
endchar;
beginchar(null,1,-1,-1,-1);
%%beginbody
charwd := 0;
endchar;
beginchar(linefeed,10,-1,-1,-1);
%%beginbody
charwd := 0;
endchar;
beginchar(zwj,8205,-1,-1,-1);
%%beginbody
%847
charwd := 0;
endchar;
beginchar(cgj,847,-1,-1,-1);
%%beginbody
%847
charwd := 0;
endchar;
defchar(space,32,-1,0,0);
%%beginbody


charwd := 100 + lefttatweel  *nuqta ;
enddefchar;
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
%%backgroundimage:images/shadda.jpg,-1594.98,-851.892,1,0,0,1
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
_height := _height + (lefttatweel_ - 2.3*nuqta)* 0.015;

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
defchar(kasra,1616,-1,-1,-1);
%%beginbody

fatha_(lefttatweel,0);
leftbearing := 0;
rightbearing := 0;
enddefchar;
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
beginchar(smallhighyeh,1767,-1,-1,-1);
%%beginbody
drawcomp(smallyeh,identity);
leftbearing := 0;
rightbearing := 0;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
endchar;
beginchar(smallhighwaw,2291,-1,-1,-68);
%%beginbody
drawcomp(smallwaw,identity);
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
beginchar(hamzaabove.small,-1,-1,-1,-1);
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

%%beginpaths

fill
(-242,1307) .. controls (-242,1328.09) and (-213,1342) ..
(-204,1342) .. controls (-153.96,1342) and (3.87695,1174) ..
(8,1174) .. controls (17.0557,1174) and (24,1274) ..
(24,1338) .. controls (24,1354.03) and (44,1379) ..
(50,1379) .. controls (62.042,1379) and (64,1352) ..
(64,1333) .. controls (64,1326.29) and (86,1328.02) ..
(86,1302) .. controls (86,1286.6) and (58,1272) ..
(58,1266) .. controls (58,1250) and (58,1240) ..
(58,1224) .. controls (58,1195) and (30,1146.17) ..
(30,1134) .. controls (30,1125.75) and (52,1099) ..
(52,1045) .. controls (52,1005.99) and (-14,1004) ..
(-60,1004) .. controls (-112.087,1004) and (-114,1008) ..
(-114,1046) .. controls (-114,1060.04) and (-95,1063) ..
(-86,1063) .. controls (-39.9893,1063) and (-21,1111) ..
(-21,1120) .. controls (-21,1123.16) and (-21.9697,1130.03) ..
(-25,1133) .. controls (-75,1182) and (-139.16,1252) ..
(-161,1252) .. controls (-176,1252) and (-183,1246) ..
(-198,1246) .. controls (-227,1246) and (-242,1278) ..
 cycle
;

%%endpaths
%%beginbody


leftbearing := 0;
rightbearing := 0;

%currentpicture := currentpicture scaled 0.6;
currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
endchar;
beginchar(waqf.lam.ii,-1,-1,-1,-1);

%%beginpaths

fill
(-244,302) .. controls (-244,323.09) and (-215,337) ..
(-206,337) .. controls (-155.96,337) ..
(6,169) .. controls (15.0557,169) and (22,269) ..
(22,333) .. controls (22,349.03) and (42,374) ..
(48,374) .. controls (60.042,374) and (62,347) ..
(62,328) .. controls (62,321.29) and (84,323.02) ..
(84,297) .. controls (84,281.6) and (56,267) ..
(56,261) .. controls (56,245) and (56,235) ..
(56,219) .. controls (56,190) and (28,141.17) ..
(28,129) .. controls (28,120.75) and (49.0355,94) ..
(49.0355,40) .. controls (49.0355,0.99) and (-16,-1) ..
(-62,-1) .. controls (-114.087,-1) and (-116,3) ..
(-116,41) .. controls (-116,55.04) and (-97,58) ..
(-88,58) .. controls (-41.9893,58) and (-23,106) ..
(-23,115) .. controls (-23,118.16) and (-23.9697,125.03) ..
(-27,128) .. controls (-77,177) and (-141.16,247) ..
(-163,247) .. controls (-178,247) and (-185,241) ..
(-200,241) .. controls (-229,241) and (-244,273) ..
 cycle
;

%%endpaths
%%beginbody


leftbearing := 0;
rightbearing := 0;

%currentpicture := currentpicture scaled 0.6;
%currentpicture := currentpicture shifted (0,-ypart llcorner currentpicture);
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
beginchar(smallhighseen,1756,-1,-1,-1);
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
beginchar(smallhighnoon,1768,-1,-1,-1);
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
beginchar(kashida.i.min,-1,-1,-1,-1);
%%backgroundimage:images/alef.fina.jpg,-680.656,-1647.46,1,0,0,1
%%beginparams
z1 = (251.992,6.57489);

%%beginverbatim

z2 = z1 + joinvector;

%%endverbatim

%%beginpaths

fill
z1 {left} .. tension 0.75 and 2.3892 ..
(193.402,29.8151) .. tension 1 and 0.906208 ..
 {dir 98.5408}(163.065,90.923) {dir -290.129} .. tension 1 and 1 ..
 {dir 41.4305}(202.518,149.527) {dir -83.0498} .. tension 1 and 1 ..
(225.061,107.029) .. tension 1 and 1 ..
 {right}z2 .. tension 1 and 1 ..
(300.841,106.524) .. tension 1 and 0.966788 ..
 {dir 68.5939}(323.165,149.595) --
(351.554,88.6369) {dir -104.47} .. tension 1.19332 and 1 ..
(328.169,39.9385) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody
savepicture := 1;

endchar;
beginchar(kashida.i.max,-1,-1,-1,-1);
%%backgroundimage:images/kashida.jpg,-2427.44,-1201.64,1,0,0,1
%%beginparams
z1 = (742.959,-33.5821);

%%beginverbatim


z2 = z1 + joinvector;


%%endverbatim

%%beginpaths

fill
z1 {left} .. tension 1 and 1 ..
(460.162,-7.71651) .. tension atleast 1 ..
 {dir 137.542}(269.092,91.655) {dir -304.613} .. tension atleast 1 ..
 {dir 32.9165}(320.993,147.699) {dir -32.7924} .. tension atleast 1 ..
(539.487,70.532) .. tension 1 and 1 ..
 {right}z2 .. tension 1 and 1 ..
(1081.62,85.8371) .. tension 1 and 1 ..
 {dir 24.4192}(1319.96,153.584) --
(1366.74,91.5374) {dir -155.325} .. tension 0.898642 and 1 ..
(1028.55,-7.37747) .. tension 0.75 and 1 ..
 cycle
;

%%endpaths
%%beginbody
savepicture := 1;

endchar;
defchar(kashida.i,-1,-1,-1,-1);
%%beginbody


draw interpolatemm(
kashida.i.min.pic,
kashida.i.max.pic,
2,
lefttatweel
);

defaultkashida("i");


enddefchar;
beginchar(kashida.ii.min,-1,-1,-1,-1);
%%backgroundimage:images/alef.fina.jpg,-680.656,-1647.46,1,0,0,1
%%beginparams
z1 = (251.992,6.57489);

%%beginverbatim

z2 = z1 + joinvector;

%%endverbatim

%%beginpaths

fill
z1 {left} .. tension 0.75 and 2.3892 ..
(193.402,29.8151) .. tension 1 and 0.906208 ..
 {dir 98.5408}(163.065,90.923) {dir -271.965} .. tension atleast 1 ..
 {dir 20.5336}(202.518,149.527) {dir -83.0498} .. tension atleast 1 ..
(225.061,107.029) .. tension 1 and 1 ..
 {right}z2 .. tension 1 and 1 ..
(300.841,106.524) .. tension 1 and 0.966788 ..
 {dir 68.5939}(323.165,149.595) --
(351.554,88.6369) {dir -104.47} .. tension 1.19332 and 1 ..
(328.169,39.9385) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody
savepicture := 1;

endchar;
beginchar(kashida.ii.max,-1,-1,-1,-1);
%%backgroundimage:images/test.jpg,-1006.72,-1133.19,1,0,0,1
%%beginparams
z1 = (656.774,-8.53694);

%%beginverbatim


z2 = z1 + joinvector;


%%endverbatim

%%beginpaths

fill
z1 {left} .. tension 1 and 1 ..
(331.274,34.2176) .. tension 1 and 1 ..
 {dir 113.371}(141.749,164.201) {dir -296.198} .. tension atleast 1 ..
 {dir 25.5839}(189.794,219.337) {dir -50.8295} .. tension atleast 1 ..
(399.645,109.595) .. tension 1 and 1 ..
 {right}z2 .. tension 1 and 1 ..
(1078.2,109.753) .. tension 1 and 1 ..
 {dir 24.4192}(1500.48,229.722) --
(1525.35,167.509) {dir -151.47} .. tension 1 and 1 ..
(1095.83,27.4686) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody
savepicture := 1;

endchar;
defchar(kashida.ii,-1,-1,-1,-1);
%%beginbody


draw interpolatemm(
kashida.ii.min.pic,
kashida.ii.max.pic,
2,
lefttatweel
);

defaultkashida("ii");

enddefchar;
beginchar(kashida.iii.min,-1,1,-1,-1);
%%backgroundimage:images/alef.fina.jpg,-707.844,-1596.91,1,0,0,1
%%beginparams
z1 = (104.192,84.7825);
z3 = (271.116,82.3993);

%%beginverbatim

z2 = z1 rotatepen 70;
z4 = z3 rotatepen 70;


%%endverbatim

%%beginpaths

fill
(234.652,13.1375) .. tension atleast 1 ..
(144.463,14.7293) .. tension atleast 1 ..
 {dir 114.162}z1 --
z2 {dir -73.5446} .. tension atleast 1 ..
(148.808,99.5283) .. tension 1 and 1 ..
(231.702,97.0691) .. tension 1 and 1 ..
 {dir 67.95}z4 --
z3 .. tension 1.06453 and 1.04733 ..
 cycle
;

%%endpaths
%%beginbody
savepicture := 1;
endchar;
beginchar(kashida.iii.default,-1,1,-1,-1);
%%backgroundimage:images/alef.fina.jpg,-713.829,-1657.25,1,0,0,1
%%beginparams
z1 = (141.637,85.2025);
z3 = (416.497,84.5622);

%%beginverbatim

z2 = z1 rotatepen 70;
z4 = z3 rotatepen 70;


%%endverbatim

%%beginpaths

fill
(315.916,4.7088) .. tension atleast 1 ..
(239.795,4.58611) .. tension atleast 1 ..
 {dir 114.162}z1 {dir -277.388} .. tension 1 and 1 ..
 {dir 73.4795}z2 {dir -65.6948} .. tension atleast 1 ..
(250.007,84.2186) .. tension atleast 1 ..
(307.287,79.1153) .. tension atleast 1 ..
z4 --
z3 .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody
savepicture := 1;
endchar;
beginchar(kashida.iii.max,-1,1,-1,-1);
%%backgroundimage:images/expasinna.iii.jpg,-1052.88,-1115.6,1,0,0,1
%%beginparams
z1 = (101.572,82.6869);
z3 = (1557.19,81.672);

%%beginverbatim

z2 = z1 rotatepen 70;
z4 = z3 rotatepen 70;


%%endverbatim

%%beginpaths

fill
(1030.39,-90.1333) .. tension 1.20931 ..
(368.276,-99.5853) .. tension 0.895213 ..
z1 --
z2 {dir -63.395} .. tension 1.06525 ..
(428.435,-24.276) .. tension 1.44311 ..
(1107.37,3.005) .. tension 0.94724 and 5.23359 ..
z4 --
z3 .. tension 1.07849 and 0.75 ..
 cycle
;

%%endpaths
%%beginbody
savepicture := 1;
endchar;
defchar(kashida.iii,-1,-1,-1,-1);
%%beginbody

%draw interpolate(
%kashida_iii_default.pic,
%interpolatepic(kashida_iii_default.pic,kashida_iii_max.pic,1),
%kashida_iii_max.pic,
%lefttatweel
%);

draw interpolate(
kashida.iii.min.pic,
kashida.iii.default.pic,
kashida.iii.max.pic,
lefttatweel
);

enddefchar;
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
%%backgroundimage:images/alef.isol.ii.jpg,-1361.77,-1340.68,1,0,0,1
%%beginparams
z33 = (-36.2269,6.34259);

%%beginverbatim
lefttatweel := lefttatweel - 1;

minLeft := -3;
maxLeft := 0;

y33 := - minLeft *nuqta;

%%endverbatim

%%beginpaths

fill
(166.502,0) {left} .. tension 2.4601 and 1 ..
 {up}bldmvi((159.089,49.4792),(4.16667,0.833333)) .. tension 1.21675 and 1.21675 ..
bldmvi((138.797,235.243),(20.9336,-50.625)) .. tension 2.07948 and 2.07948 ..
 {up}bldmvi((118.786,400.668),(-32.2126,366.651)) .. tension 1.49022 and 7.11926 ..
 {right}bldmvi((133.426,481.656),z33) .. tension bldmvi(1.42517,0.1) and 4.01557 ..
bldmvi((144.911,459.08),(-28.6985,342.232)) ...
 {down}bldmvi((194.886,92.1522),(6.25385,33.0749)) .. tension 1.00275 and 4.02237 ..
 {left} cycle
;

%%endpaths
%%beginbody
leftbearing := leftbearing + 0;
%setCursiveAnchor(basmala,(300,-200), EntryAnchor);
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
%%backgroundimage:images/testheight.jpg,-12404.5,-1469.32,1,0,0,1
%%beginparams
rightVerticalRatio := 3;
z1 = (182.182,24.6574);
z2 = (206.233,107.918);

%%beginverbatim

righttatweel := righttatweel - 0.5;

%%endverbatim

%%beginpaths

fill
(54.9873,698.975) .. tension 1.29322 and 10.6712 ..
 {right}(80.5787,772.824) .. tension 3.6286 and 1.61658 ..
(109.946,663.456) .. tension 3.28624 and 2.76737 ..
(150.463,302.662) .. tension 1.51205 and 1 ..
(185.417,140.278) .. tension 1 and 1.35542 ..
brdmvi(z2,(11.6667,-25)) ..
brdmvi((212.572,100.893),(209.791,-172.477)) ..
brdmvi((219.245,96.5623),(380.39,-223.81)) link
brdmvi((208.768,6.52063),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((202.005,9.33356),(345.967,-205.334)) ..
brdmvi((193.442,14.085),(99.6065,-102.338)) ..
brdmvi(z1,(-2.5,-5.83333)) .. tension atleast 1 ..
brdmvi((137.963,117.497),(0,0)) .. tension 2.00581 and 1 ..
(94.3287,441.953) .. tension 1 and 1.50214 ..
(61.4494,673.351) .. tension 1 and 1.57007 ..
 {up} cycle
;

%%endpaths
%%beginbody
%leftbearing := leftbearing ; %- 50;

enddefchar;
defchar(alef.fina.afterkaf,-1,-1,-1,-1);
%%backgroundimage:images/kafinitaleffina.jpg,-1943.62,-1288.62,1,0,0,1
%%beginparams
z1 = (242.46,-1.24072);
z2 = (196.212,136.078);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
(92.1208,727.428) .. tension 1.29322 and 10.6712 ..
 {right}(111.925,822.978) .. tension 9.26248 and 1.61658 ..
(130.934,676.805) .. tension 3.28624 and 2.76737 ..
(169.271,304.591) .. tension 1.51205 and 1.33163 ..
z2 link
z1 .. tension 1 and 1 ..
(151.645,142.259) .. tension 8.01869 and 2.88876 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing := leftbearing; % - 50;
enddefchar;
defchar(lam.medi.laf,-1,-1,-1,-1);
%%backgroundimage:images/lamaleffina.jpg,-878.868,-1242.4,1,0,0,1
%%beginparams
rightVerticalRatio := 3;
z1 = (730.069,67.1145);
z11 = (598.418,151.953);

%%beginverbatim





righttatweel := righttatweel - 0.5;



%%endverbatim

%%beginpaths

fill
(133.547,-71.6685) {up} .. tension atleast 1 ..
 {right}(154.32,-53.7903) .. tension 1.01891 and 1.11455 ..
z11 .. tension 1.15521 and 1.26746 ..
 {up}(648.033,299.055) .. tension 1.91848 and 1 ..
 {up}(617.485,607.84) .. tension 1 and 4.95109 ..
 {right}(636.878,694.31) .. tension 10.7827 and 1 ..
(689.622,434.028) .. tension 1 and 1 ..
(711.507,305.413) .. tension 1 and 1.1924 ..
brdmvi((739.875,179.626),(45.8333,-78.3333)) ..
brdmvi((758.835,139.307),(225.833,-185.833)) ..
brdmvi((777.014,119.966),(480.833,-256.667)) link
brdmvi((779.283,25.1303),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((759.63,29.9184),(401.667,-228.333)) ..
brdmvi((744.379,44.7885),(136.667,-129.167)) ..
brdmvi(z1,(23.3333,-30.8333)) .. tension 1.56004 and 1 ..
(710.093,121.846) .. tension 1 and 1 ..
 {curl1}(675.459,280.666) .. tension atleast 1 ..
(638.008,128.953) .. tension 0.75 and 1.52439 ..
(456.473,-50.2061) .. tension 1.09781 and 2.66853 ..
 {left}(160.037,-84.655) .. tension atleast 1 ..
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
(426.183,506.898) .. tension 1 and 1 ..
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
%%backgroundimage:images/beh.init.jpg,-1372.65,-1299.44,1,0,0,1
%%beginparams
leftVerticalRatio := 6;

%%beginverbatim

lefttatweel := lefttatweel +0;






%%endverbatim

%%beginpaths

fillc
bldmvi((271.969,64.0626),(-6.74478,0.971265)) ..
bldmvi((258.422,54.7611),(-45.9871,-18.2017)) ..
bldmvi((243.167,48.2116),(-562.096,-129.055)) ..
bldmvi((206.405,41.9699),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((257.245,135.456),(-477.724,-120.382)) ..
bldmvi((265.936,139.164),(-49.3975,-22.6056)) ..
bldmvi((274.887,146.407),(-18.4194,-11.8051)) .. tension 0.856689 and 1.58967 ..
 {up}(277.326,156.702) .. tension 1.11761 and 11.9251 ..
(193.944,262.83) .. tension 5.61953 and 5.94319 ..
 {right}(212.745,344.705) .. tension 5.85287 and 1 ..
(277.778,282.208) .. tension 1.13818 and 1.06645 ..
 {down}(305.174,188.875) .. tension 1 and 1 ..
(302.573,132.864) .. tension atleast 1 ..
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
%%backgroundimage:images/behbeforeseen.jpg,-3328.89,-1124.78,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
z1 = (49.975,31.5895);
z2 = (37.2341,99.5501);

%%beginverbatim


lefttatweel := lefttatweel - 0.5;


%%endverbatim

%%beginpaths

fill
bldmvi(z1,(-25.4167,-8.33333)) ..
bldmvi((27.641,6.07983),(-109.34,-33.5121)) ..
bldmvi((-0.621949,0.0830878),(-364.956,-94.1352)) ..
bldmvi((-21.0544,-1.33146),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((22.7921,88.9447),(-396.304,-101.753)) ..
bldmvi((32.9521,93.935),(-31.1467,-13.4037)) ..
bldmvi(z2,(-8.43556,-6.23648)) .. tension atleast 1 ..
(39.4853,109.889) {up} .. tension 1.35214 and 8.89964 ..
 {up}(-62.1407,323.426) .. tension 0.75 and 2.6421 ..
 {right}(-44.4,409) .. tension 7.15864 and 1.54359 ..
(65.6818,130.191) {down} .. tension 0.75 and 1.59291 ..
 cycle
;

%%endpaths
%%beginbody


enddefchar;
defchar(behshape.init.basmala,-1,-1,-1,-1);
%%backgroundimage:images/basmala_naml.jpeg,-6449.72,-1327.48,1,0,0,1
%%beginparams
leftanchor = (-50.4815,77.237);

%%beginverbatim




%%endverbatim

%%beginpaths

fill
(109.503,-2.09856) .. tension 1.72829 and 3.96771 ..
 {dir 96.8275}(-51.1065,58.929) .. tension 0.946159 and 2.03261 ..
 {dir 50.5996}(-37.9645,105.965) .. tension 10.5096 and 1 ..
 {right}(104.856,77.8973) .. tension atleast 1 ..
(140.287,119.853) {up} .. tension 1.17371 and 13.5704 ..
 {left}(65.9082,335.858) .. tension 1 and 1 ..
 {left}(52.6952,330.482) .. tension 1.07646 and 8.89964 ..
 {up}(16.1787,387.781) .. tension 0.75 and 2.6421 ..
 {right}(42.8485,476.482) .. tension 4.34762 and 1.45182 ..
(148.052,298.997) .. tension 0.75 and 0.955976 ..
(169.663,153.563) {down} .. tension 0.949257 and 4.78865 ..
 {left} cycle
;

%%endpaths
enddefchar;
defchar(behshape.init.beforenoon,-1,-1,-1,-1);
%%backgroundimage:images/behnoon.jpg,-511.096,-1089.58,1,0,0,1
%%beginparams
leftVerticalRatio := 6;

%%beginverbatim

pair _temp;
_temp := joinvector;
joinvector := (0,lineheight);


%%endverbatim

%%beginpaths

fill
bldmvi((150.896,38.1862),(-19.1226,-8.50629)) ..
bldmvi((141.67,31.6744),(-70.3286,-23.9451)) ..
bldmvi((135.159,28.4573),(-310.203,-83.5443)) ..
bldmvi((127.29,26.9844),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((135.654,115.682),(-313.641,-88.8953)) ..
bldmvi((142.747,117.349),(-75.9362,-34.7865)) ..
bldmvi((146.668,120.267),(-17.2807,-16.0751)) .. tension 1 and 0.940951 ..
 {up}(149.592,125.741) .. tension 1 and 1.78806 ..
 {up}(104.198,194.106) .. tension 1 and 3.33785 ..
 {right}(117.752,273.624) .. tension 3.03159 and 1 ..
 {down}(185.61,133.894) .. tension 1 and 1.14234 ..
(172.279,64.1949) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody



joinvector := _temp;


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
defchar(behshape.medi,-1,-1,-1,-1);
%%backgroundimage:images/behshape.medi.2.jpg,-829.021,-1442.98,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 6;

%%beginverbatim

%%endverbatim

%%beginpaths

fill
(173.734,99.2136) {dir bldmvi(-111.323,-17.653)} .. tension 0.75 and 1.75223 ..
bldmvi(bldm((159.752,62.3046),(-170.24,3.90714)),(23.7627,-6.04611)) ..
bldmvi((150.018,42.4625),(-183.628,-57.2797)) ..
bldmvi((130.707,26.935),(-364.883,-88.8129)) ..
bldmvi((126.485,25.7575),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((151.387,121.18),(-303.627,-92.8061)) ..
bldmvi((158.345,132.416),(-106.394,-38.114)) ..
bldmvi(bldm((173.192,156.426),(-128.472,-61.304)),(3.19005,-0.147507)) .. tension 0.932615 and 2.98102 ..
(182.172,171.125) .. tension 1.7264 and 1.23542 ..
 {right}(195.366,180.062) .. tension 1.7264 and 1.7264 ..
brdmvi((198.468,178.586),(13.7526,-6.97981)) .. tension 1.7264 and 1.7264 ..
brdmvi((201.005,166.248),(125.653,-50.7149)) ..
brdmvi((215.243,126.818),(478.96,-123.083)) ..
brdmvi((227.762,116.111),(655.456,-134.806)) link
brdmvi((216.055,26.8342),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((211.926,28.483),(650.903,-133.913)) ..
brdmvi((200.246,38.7092),(457.112,-118.072)) ..
brdmvi((174.716,95.1821),(8.01535,-1.17793)) .. tension 0.963684 and 1.31869 ..
 {dir brdmvi(103.741,40.5562)} cycle
;

%%endpaths
%%beginbody


enddefchar;
defchar(behshape.medi.rtii,-1,-1,-1,-1);
%%backgroundimage:images/behshape.medi.2.jpg,-829.021,-1442.98,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 3;
z1 = (180.094,85.0937);
z3 = (159.752,62.3046);

%%beginverbatim



z2 = z1 + (penwidth,0) rotated 70;



%which_in_baseline := 3;
%%endverbatim

%%beginpaths

fill
(173.734,99.2136) {dir bldmvi(-111.323,-17.653)} .. tension 0.75 and 1.75223 ..
bldmvi(bldm(z3,(-170.24,3.90714)),(23.7627,-6.04611)) ..
bldmvi((150.018,42.4625),(-183.628,-57.2797)) ..
bldmvi((130.707,26.935),(-364.883,-88.8129)) ..
bldmvi((126.485,25.7575),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((151.387,121.18),(-303.627,-92.8061)) ..
bldmvi((158.345,132.416),(-106.394,-38.114)) ..
bldmvi(bldm((173.192,156.426),(-128.472,-61.304)),(3.19005,-0.147507)) .. tension 0.932615 and 2.98102 ..
(182.172,171.125) .. tension 1.7264 and 1.23542 ..
 {right}(195.366,180.062) .. tension 1.7264 and 1.7264 ..
brdmvi((204.687,174.883),(0,0)) ..
brdmvi((220.049,157.214),(0,0)) ..
brdmvi((230.436,141.496),(63.0948,-61.8891)) link
brdmvi((224.986,42.4628),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((213.467,45.9061),(106.096,-83.5905)) ..
brdmvi((193.874,61.4621),(36.169,-34.1596)) ..
brdmvi(z1,(2.55717,-0.807528)) .. tension 0.963684 and 0.857896 ..
 {dir brdmvi(114.046,3.7422)} cycle
;

%%endpaths
%%beginbody
%labels(1,2,3,4);

%show leftanchor;
%show rightanchor;

enddefchar;
defchar(behshape.medi.expa,-1,-1,-1,-1);
%%backgroundimage:images/expasinna.iii.jpg,-2344.43,-1172.69,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 6;
z.mark1 = (105.509,331);

%%beginverbatim

minRight := -0.5;

%lefttatweel := lefttatweel
righttatweel := righttatweel - 0.5;

%%endverbatim

%%beginpaths

fillc
(218.597,109.496) .. tension 11.313 and 1 ..
bldmvi((215.358,105.912),(-15.5633,-6.4892)) ..
bldmvi((191.726,85.3595),(-436.439,-115.258)) ..
bldmvi((167.92,75.0369),(-755.91,-137.531)) ..
bldmvi((160.265,74.1015),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((185.798,163.089),(-747.801,-136.478)) ..
bldmvi((206.64,168.948),(-450.779,-113.812)) ..
bldmvi((218.995,177.515),(-166.86,-55.9414)) .. tension 0.932615 and 1 ..
bldmvi((229.012,190.892),(-40.5093,-12.5386)) .. tension 1 and 1.18468 ..
bldmvi((243.044,211.898),(23.2285,36.6512)) .. tension 1 and 1.3263 ..
 {right}bldmvi((261.36,228.798),(21.7014,36.169)) .. tension 1.64548 and 1.72888 ..
bldmvi((264.611,200.776),(5.70666,-6.99267)) .. tension 0.75 and 3.4906 ..
 {down}brdmvi((250.813,157.966),(0,0)) .. tension 3.32602 and 1 ..
brdmvi(brdmvi((259.193,154.357),(0,0)),(12.1721,-8.10101)) ..
brdmvi((279.015,152.069),(436.567,-111.933)) ..
brdmvi((292.388,151.646),(651.667,-132.5)) link
brdmvi((284.006,64.0887),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((277.895,64.5846),(621.667,-125)) ..
brdmvi((260.078,69.7506),(404.167,-99.1667)) ..
brdmvi((230.418,92.0092),(56.4954,-17.9332)) .. tension 1 and 1 ..
brdmvi((219.883,107.294),(3.62806,-0.837245)) .. tension 1.12681 and 18.8403 ..
 {left} cycle
;

%%endpaths
%%beginbody
%labels(1,2,3,4);


currentpicture := calcDeltasPic(currentpicture);


x.mark1 := x.mark1 - lefttatweel * nuqta * 0.1;

setAnchor(onedotup,z.mark1);
setAnchor(topmark,z.mark1 - (800,0));

enddefchar;
defchar(behshape.medi.beforeseen,-1,-1,-1,-1);
%%backgroundimage:images/behlongsenna.jpg,-2516.69,-1016.72,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 3;

%%beginverbatim

righttatweel := righttatweel - 0.3;
lefttatweel := lefttatweel - 0.3;



%%endverbatim

%%beginpaths

fill
(180.575,84.494) {dir bldmvi(-107.232,-9.83108)} .. tension 0.77581 and 1 ..
bldmvi((165.479,49.3616),(-56.0355,-38.929)) ..
bldmvi((148.47,26.4731),(-154,-72)) ..
bldmvi((127.132,10.7231),(-421,-118)) ..
bldmvi((117.196,9.02417),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((142.43,99.5441),(-563,-137)) ..
bldmvi((150.741,104.396),(-208,-92)) ..
bldmvi((156.933,112.009),(-37.213,-25)) .. tension 0.932615 and 2.98102 ..
 {up}(160.838,127.916) .. tension 1.7264 and 1 ..
 {up}(143.982,216.471) .. tension 1 and 2.01696 ..
 {right}(158.503,289.701) .. tension 9.83217 and 1 ..
(192.877,193.029) .. tension 1 and 1 ..
brdmvi((219.292,127.831),(130.556,-110.417)) ..
brdmvi((232.114,114.299),(327.083,-193.056)) ..
brdmvi((245.354,108.057),(568.056,-250.694)) link
brdmvi((234.79,18.7937),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((227.559,19.9554),(460.417,-220.833)) ..
brdmvi((220.749,24.1028),(269.444,-159.722)) ..
brdmvi((211.03,34.781),(79.8611,-61.8056)) .. tension 1 and 1 ..
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
%%backgroundimage:images/behreh.jpg,-1595.2,-1175.3,1,0,0,1
%%beginparams
rightVerticalRatio := 3;
z.mark1 = (11.1111,186.111);

%%beginverbatim

righttatweel :=righttatweel - 0.3;


%%endverbatim

%%beginpaths

fill
(132.995,231.341) {dir -129.323} .. tension atleast 1 ..
 {left}(10.2199,164.325) .. tension 1 and 1 ..
 {up}(-52.8225,236.042) .. tension 1 and 2.34159 ..
 {right}(-43.0783,253.137) .. tension 3.10136 and 0.75 ..
 {right}(-1.22594,240.098) .. tension 0.932615 and 2.98102 ..
(107.447,326.839) .. tension 1.7264 and 1.59524 ..
 {right}(137.49,370.55) .. tension 2.98366 and 1 ..
(166.25,280.94) .. tension 1 and 1.21844 ..
(194.131,187.266) .. tension 1 and 1.37056 ..
brdmvi((213.383,145.135),(0,0)) ..
brdmvi((227.391,122.515),(102.238,-111.883)) ..
brdmvi((241.768,104.633),(404.128,-230.517)) link
brdmvi((239.026,13.0127),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((215.029,26.7136),(375.675,-222.319)) ..
brdmvi((194.745,52.1757),(88.2523,-103.202)) ..
brdmvi((177.779,84.2068),(0,0)) .. tension atleast 1 ..
(147.157,168.876) .. tension 2.28647 and 0.75 ..
 {dir 99.9711} cycle
;

%%endpaths
%%beginbody


leftbearing := 0;

setCursiveAnchor(behshapereh,z.mark1, ExitAnchor);


%currentpicture := currentpicture shifted (0,- ypart rightanchor); 
%rightanchor := rightanchor  shifted (0,- ypart rightanchor);




enddefchar;
defchar(behshape.medi.beforenoon,-1,-1,-1,-1);
%%backgroundimage:images/behmedinoon.jpg,-545,-900.579,1,0,0,1
%%beginparams
leftVerticalRatio := 16;
rightVerticalRatio := 3;

%%beginverbatim


which_in_baseline := 5;
lefttatweel := lefttatweel - 0.3;
righttatweel := righttatweel - 0.3;

%%endverbatim

%%beginpaths

fill
(172.32,272.823) {dir bldmvi(-102.443,-10.6139)} .. tension atleast 0.884538 ..
bldmvi((156.276,218.66),(-113.889,-53.9352)) ..
bldmvi((133.032,184.946),(-265.046,-48.6111)) ..
bldmvi((109.977,167.367),(-381.944,-45.1389)) ..
bldmvi((87.2593,159.465),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((118.392,254.191),(-381.945,-43.4027)) ..
bldmvi((125.61,264.864),(-116.319,-30.0926)) ..
bldmvi((134.284,282.824),(-29.5139,-13.3102)) .. tension 0.932615 and 2.98102 ..
bldmvi((146.132,312.703),(-5.55556,4.39815)) .. tension 1.7264 and 3.65903 ..
 {right}(175.44,376.515) .. tension 2.98366 and 1 ..
(220.555,253.422) .. tension 1 and 1.21844 ..
brdmvi((234.039,182.141),(26.5239,-54.9769)) ..
brdmvi((241.401,150.157),(61.7284,-103.684)) ..
brdmvi((250.602,123.931),(172.164,-186.15)) link
brdmvi((244.921,25.334),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((227.625,37.327),(170.718,-157.697)) ..
brdmvi((213.959,60.4823),(66.0687,-84.3943)) ..
brdmvi((204.757,89.4993),(1.44676,29.4174)) .. tension 2.28647 and 1.24656 ..
 {dir 100.584} cycle
;

%%endpaths
%%beginbody



setCursiveAnchor(behmedinoon,leftanchor + (10,0), ExitAnchor);






enddefchar;
defchar(behshape.fina,-1,-1,-1,-1);
%%backgroundimage:images/teh.fina.2.jpg,-144,-1121,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim

righttatweel := righttatweel - 0.5;
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
(860.693,149.846) .. tension 2.95247 and 2.95247 ..
 {down}(849.613,118.303) .. tension 3.86712 and 1 ..
brdmvi((850.207,112.524),(147.741,-60.6839)) ..
brdmvi((853.13,106.035),(443,-116)) ..
brdmvi((860.271,99.2407),(730,-139.8)) link
brdmvi((860.941,8.12825),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((844.935,16.6373),(681.6,-141.6)) ..
brdmvi((833.407,32.2013),(409.2,-121.4)) ..
brdmvi((825.06,54.8224),(119.907,-60.5396)) .. tension 1.49205 and 1.14026 ..
 {dir brdmvi(100.635,31.0991)}(821.861,68.9069) {dir -130.554} .. tension 6.85701 and 0.807264 ..
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
%%backgroundimage:images/seen.init.jpg,-2131.93,-1301.28,1,0,0,1
%%beginparams
leftVerticalRatio := 6;

%%beginverbatim



lefttatweel := lefttatweel - 0.3;


%%endverbatim

%%beginpaths

fill
(238.202,24.8446) .. tension 0.75 and 1.73694 ..
 {left}(146.432,31.2291) .. tension 1.67481 and 0.75 ..
bldmvi((113.841,22.7946),(-51.3519,-36.5973)) ..
bldmvi((97.7547,16.244),(-321.806,-112.806)) ..
bldmvi((85.8552,10.8563),(-601.222,-132.394)) ..
bldmvi((76.5137,9.18959),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((108.159,96.7207),(-610.135,-135.782)) ..
bldmvi((130.764,100.417),(-382.574,-120.598)) ..
bldmvi((155.502,107.962),(2.574,-8.02313)) .. tension 1.65623 and 1.65623 ..
bldmvi((173.257,115.965),(19.3056,6.38888)) .. tension 1 and 1 ..
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
leftbearing := xpart (ulcorner currentpicture)  - x1;
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
%%backgroundimage:images/seenyeh.jpg,-1289.06,-967.732,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z40 = (287,-17);

%%beginverbatim
righttatweel := righttatweel - 0.5;


z50 = z40 + (0,lineheight);


%%endverbatim

%%beginpaths

fill
(287.362,-17.2679) endlink
z50 .. tension 1 and 0.911937 ..
(401.754,96.9211) .. tension 1 and 4.83872 ..
 {right}(463.44,166.56) .. tension 0.75 and 1 ..
 {down}(468.465,148.131) .. tension 1.98298 and 5.09898 ..
 {down}(444.881,75.0162) .. tension 4.72672 and 4.30029 ..
 {right}(499.057,55.8982) .. tension 1.48298 and 3.63522 ..
 {right}(578.745,174.392) .. tension 1.78057 and 1.94583 ..
(580.845,117.391) .. tension 1 and 1 ..
brdmvi((581.701,112.445),(22.5509,-23.338)) ..
brdmvi((582.602,110.892),(182.875,-75.745)) ..
brdmvi((584.005,110.24),(567.778,-132.097)) link
brdmvi((568.545,21.8501),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((565.681,22.1439),(534.583,-127.889)) ..
brdmvi((556.686,26.4563),(156.88,-59.537)) ..
brdmvi((550.527,32.514),(18.1018,-6.05093)) .. tension 1 and 1.33159 ..
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

setCursiveAnchor(behmediyehfina,z70 , ExitAnchor);

tr_ := tr_ shifted (0,25);

enddefchar;
defchar(seen.medi.beforereh,-1,-1,-1,-1);
%%backgroundimage:images/seenreh.jpg,-2456.76,-1164.83,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z3 = (-19.9979,-3.15843);

%%beginverbatim



righttatweel := righttatweel - 0.5;



%%endverbatim

%%beginpaths

fill
(103.228,31.2382) .. tension 0.967623 and 2.22813 ..
 {left}z3 .. tension 0.75 and 1 ..
(-53.822,7.07367) .. tension 1.23054 and 4.54894 ..
 {right}(-58.2735,77.4653) .. tension 0.843775 and 6.73346 ..
 {right}(208.174,154.666) .. tension 5.74099 and 1 ..
 {down}(206.859,109.516) .. tension 1 and 1 ..
 {right}(306.919,59.9769) .. tension 1.13 and 0.913684 ..
(420.393,134.148) .. tension 1 and 2.92953 ..
 {right}(452.018,203.875) .. tension 1 and 1 ..
 {down}(461.481,189.607) .. tension 1 and 12.2986 ..
 {down}(435.439,109.167) .. tension 1.87963 and 2.10028 ..
 {right}(522.466,73.6802) .. tension 2.41486 and 2.52602 ..
 {right}(588.309,212.701) .. tension 1 and 1 ..
 {down}(598.656,192.298) .. tension 1.34609 and 6.50716 ..
(595.194,174.428) .. tension 0.978555 and 0.978555 ..
brdmvi((588.572,149.348),(0.393046,-26.6883)) .. tension 1 and 1 ..
brdmvi((583.384,128.832),(55.1898,-55.4537)) ..
brdmvi((581.501,113.325),(268.532,-103.283)) ..
brdmvi((582.896,105.767),(556.927,-136.787)) link
brdmvi((577.775,9.42887),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((570.893,9.97597),(556.606,-126)) ..
brdmvi((558.192,16.0148),(341.445,-104.866)) ..
brdmvi((545.5,31.2243),(18.2442,-12.0727)) ..
 {dir brdmvi(104.346,23.8991)}(544.058,34.705) {dir -106.458} .. tension 16.2156 and 2.80295 ..
 {left}(512.986,-8.31016) .. tension 2.41485 and 1 ..
(419.814,20.625) .. tension 1 and 2.31351 ..
 {left}(402.071,34.1208) .. tension 8.59967 and 0.75 ..
 {left}(306.278,-18.5143) .. tension 0.75 and 1 ..
(191.564,55.6527) .. tension 1 and 4.03831 ..
 {left}(188.502,65.1852) .. tension 10.1716 and 1 ..
 cycle
;

%%endpaths
%%beginbody
leftbearing := 0;
setCursiveAnchor(seenreh,z3 + (0,15), ExitAnchor);
enddefchar;
defchar(seen.medi,-1,-1,-1,-1);
%%backgroundimage:images/sheen.medi.jpg,-1458.56,-1112.46,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 6;
z.mark1 = (-40,263);

%%beginverbatim


righttatweel := righttatweel - 0.5;
lefttatweel := lefttatweel - 0.5;

%%endverbatim

%%beginpaths

fill
(107.378,14.7606) .. tension 0.967623 and 0.967623 ..
(95.9058,14.9113) .. tension 1 and 1 ..
bldmvi((86.2012,13.9133),(-17.621,-6.64)) ..
bldmvi((75.2342,12.3222),(-221.48,-72.383)) ..
bldmvi((66.655,10.8991),(-543.702,-118.881)) ..
bldmvi((47.284,9.08768),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((92.9629,97.8111),(-597.994,-126.833)) ..
bldmvi((109.315,98.3097),(-268.404,-81.8264)) ..
bldmvi((135.859,100.648),(-13.034,-10.5093)) .. tension 1 and 1 ..
(161.687,108.9) .. tension 1.22798 and 1.22798 ..
(190.747,130.3) .. tension 1 and 4.86892 ..
(218.279,168.861) .. tension 1 and 4.86892 ..
 {right}(232.836,197.215) .. tension 1 and 1 ..
 {down}(239.005,189.236) .. tension 0.75 and 6.33946 ..
 {down}(201.307,81.2976) .. tension 1 and 1 ..
 {right}(204.861,75.2315) .. tension 2.80295 and 1.08543 ..
(419.56,149.981) .. tension 1 and 9.68006 ..
 {right}(457.755,217.593) .. tension 1 and 1 ..
 {down}(465.278,207.176) .. tension 1 and 12.2986 ..
 {down}(434.606,125) .. tension 3.68724 and 10000 ..
 {right}(501.736,95.4861) .. tension 1.36092 and 4.02398 ..
 {right}(594.167,235.237) .. tension 1 and 1 ..
 {down}(600.278,222.451) .. tension 1 and 6.50716 ..
(598.586,205.551) .. tension 1 and 1 ..
(590.685,172.484) .. tension 1 and 1 ..
brdmvi((584.06,148.249),(10.7024,2.04333)) ..
brdmvi((581.446,136.937),(52.3753,-12.1576)) ..
brdmvi((578.277,118.318),(688.717,-134.832)) link
brdmvi((562.823,28.5729),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((560.876,31.0482),(683.181,-132.938)) ..
brdmvi((557.402,48.0386),(268.831,-71.8393)) ..
brdmvi((556.951,74.2992),(15.49,-0.443669)) .. tension 0.75 and 3.23234 ..
 {left}(555.362,83.444) .. tension 17.076 and 2.18124 ..
(536.978,51.5284) .. tension 2.80295 and 2.80295 ..
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
defchar(seen.fina,-1,-1,-1,-1);
%%backgroundimage:images/sheen.medi.jpg,-54.7045,-1140.2,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim

righttatweel := righttatweel - 0.5;

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
(900.692,137.982) .. tension 1 and 1 ..
(902.373,134.782) .. tension 1 and 1 ..
brdmvi((904.995,131.513),(30,-25.6667)) ..
brdmvi((908.555,129.055),(194.333,-87.333)) ..
brdmvi((911.833,127.484),(568.667,-136.333)) link
brdmvi((899.08,39.264),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((894.702,40.183),(548.667,-133.5)) ..
brdmvi((886.587,43.9058),(171,-71.333)) ..
brdmvi((876.508,53.8908),(42.8333,-24.5)) .. tension 1 and 1 ..
(871.037,60.723) .. tension 1.33159 and 1.33159 ..
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
leftVerticalRatio := 6;

%%beginverbatim

lefttatweel := lefttatweel +0.0;

%%endverbatim

%%beginpaths

fill
bldmvi((145.207,24.13),(-16.2037,-9.83796)) ..
bldmvi((133.327,15.5155),(-200.81,-79.8611)) ..
bldmvi((125.207,11.5654),(-541.088,-128.472)) ..
bldmvi((113.311,8.544),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((143.064,98.1183),(-543.981,-126.157)) ..
bldmvi((153.029,100.915),(-205.44,-81.0185)) ..
bldmvi((160.499,106.787),(-15.0463,-9.25926)) .. tension atleast 1 ..
(163.499,114.927) .. tension 1 and 2.1102 ..
 {up}(164.901,123.746) .. tension 6.60465 and 15.5474 ..
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
%%backgroundimage:images/lam_hah.jpg,-2276.89,-1323.26,1,0,0,1
%%beginparams
lefttatweel := 1.14;
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

show codechar;

enddefchar;
defchar(lam.init.beforehahyeh,-1,-1,-1,-1);
%%backgroundimage:images/lamhahyeh.jpg,-2267.66,-1218.06,1,0,0,1
%%beginparams
lefttatweel := 1.14;
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
leftVerticalRatio := 6;
z4 = (74.9065,45.5827);

%%beginverbatim

lefttatweel := lefttatweel +0.0;





%%endverbatim

%%beginpaths

fill
bldmvi((56.1934,-15.6878),(0,0)) ..
bldmvi((120.566,-29.7115),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((27.0296,38.7474),(0,0)) ..
bldmvi((42.8588,46.6063),(0,0)) .. tension atleast 1 ..
 {up}(46.824,61.3985) .. tension 6.60465 and 1 ..
(2.397,303.413) .. tension 1 and 15.5474 ..
(-56.5324,657.532) .. tension 1 and 3.24152 ..
 {up}(-92,715.472) .. tension 2.23645 and 2.62674 ..
 {right}(-50.107,799.63) .. tension 3.26377 and 1.33449 ..
(-34.576,765.467) .. tension 1.34175 and 1.31692 ..
 {down}(5.583,705.792) .. tension 1.60019 and 4.0387 ..
(-16.635,664.148) .. tension 14.0434 and 1.77001 ..
 {down}z4 .. tension 1.12357 and 1 ..
 cycle
;

%%endpaths
%%beginbody

%charwd := x4;

enddefchar;
defchar(lam.medi,-1,-1,-1,-1);
%%backgroundimage:images/lam.medi.jpg,-1321.93,-1308.24,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 3;

%%beginverbatim

lefttatweel := lefttatweel;
righttatweel := righttatweel - 0.5;

%%endverbatim

%%beginpaths

fill
(211.6,87.1943) .. tension 1 and 1 ..
bldmvi((200.144,61.6601),(-108.796,-83.912)) ..
bldmvi((193.802,46.7263),(-245.949,-114.583)) ..
bldmvi((186.658,35.6376),(-564.815,-138.889)) ..
bldmvi((170.089,29.231),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((197.034,120.305),(-550.347,-134.259)) ..
bldmvi((203.853,127.051),(-208.912,-96.6435)) ..
bldmvi((208.601,135.674),(-34.1435,-26.6204)) .. tension atleast 1 ..
(212.668,151.134) .. tension atleast 1 and atleast 1 ..
 {up}(216.564,188.28) .. tension 1.79974 and 1 ..
(181.202,394.049) .. tension 3.43687 and 13.4898 ..
 {up}(135.417,670.139) .. tension 1.60026 and 2.03022 ..
(162.678,742.28) .. tension 17.8258 and 1 ..
(222.597,394.134) .. tension 1 and 1.58165 ..
brdmvi((257.949,206.199),(1.11633,5.58163)) .. tension 1.21643 and 1.46864 ..
brdmvi((270.672,159.629),(9.25926,-6.94444)) ..
brdmvi((280.815,135.992),(244.792,-174.19)) ..
brdmvi((291.42,121.466),(603.588,-256.366)) link
brdmvi((284.176,28.716),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((274.524,32.6785),(596.065,-256.366)) ..
brdmvi((260.2,45.6552),(314.815,-195.602)) ..
brdmvi((230.173,111.115),(0,0)) .. tension 1 and 0.773578 ..
 {dir 100.205}(225.743,129.339) {dir -100.503} .. tension 1.22321 and 1 ..
 cycle
;

%%endpaths
enddefchar;
defchar(lam.medi.afterkaf,-1,-1,-1,-1);
%%backgroundimage:images/kafinitlammedi.jpg,-2468.03,-1389,1,0,0,1
%%beginbody


lam.medi_(lefttatweel,-0.2);
enddefchar;
defchar(lam.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/ala.jpg,-476.601,-1109.55,1,0,0,1
%%beginparams
rightVerticalRatio := 3;
z4 = (-2,-71);

%%beginverbatim
righttatweel := righttatweel - 0.5;





%%endverbatim

%%beginpaths

fill
z4 .. tension 2.18838 and 1 ..
(15.7115,-23.7595) .. tension 1.78306 and 1.20664 ..
(39.046,77.2138) .. tension 1.50391 and 1.86795 ..
 {up}(-26.4585,601.139) .. tension 2.76613 and 9.46491 ..
 {right}(-0.52834,675) .. tension 9.62774 and 1.43051 ..
(15.9875,578.842) .. tension 4.40733 and 4.3025 ..
(47.9692,405.84) .. tension 0.75 and 0.75 ..
(62.448,334.832) .. tension 1 and 1 ..
brdmvi((144.291,153.416),(0,0)) ..
brdmvi((166.595,130.788),(319.2,-224.4)) ..
brdmvi((187.099,114.702),(604.8,-274.8)) link
brdmvi((190.209,19.2214),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((173.685,23.9952),(604.8,-274.8)) ..
brdmvi((148.828,42.7974),(291.6,-212.4)) ..
brdmvi((104.832,98.7965),(0,0)) .. tension 0.75 and 0.75 ..
(81.1145,136.608) .. tension 0.980347 and 0.860644 ..
 {dir 113.128}(73.1629,151.734) {dir -88.5156} .. tension 1.23046 and 3.30153 ..
(25.4604,-64.4882) .. tension 1 and 1 ..
 {left}(9.0282,-75.85) .. tension 1.85756 and 3.07581 ..
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
leftVerticalRatio := 6;
rightVerticalRatio := 3;

%%beginverbatim

righttatweel := righttatweel - 0.5;


%%endverbatim

%%beginpaths

fill
bldmvi((65.5791,0),(0,0)) ..
bldmvi((120.861,-31.9258),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((40.2608,42.5304),(0,0)) .. tension atleast 1 ..
 {up}(53.2603,76.9454) .. tension 3.07244 and 4.80937 ..
 {up}(-46.7045,702.509) .. tension 1.60026 and 2.59865 ..
 {right}(-25.4004,770.405) .. tension 33.8783 and 25.1081 ..
(100.175,186.362) .. tension 1.21643 and 0.75 ..
brdmvi((111.559,155.943),(107,-128)) ..
brdmvi((136.038,122.764),(440,-240)) ..
brdmvi((155.38,110.632),(689,-275)) link
brdmvi((148.024,19.8023),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((141.241,20.7713),(666,-272)) ..
brdmvi((126.783,25.9264),(360,-218)) ..
brdmvi((98.9291,42.7566),(1,-12)) .. tension 1 and 1 ..
(83.0327,58.4951) {curl1} .. tension 1.6801 and 1.6801 ..
 {curl1}(79.4053,63.4954) {dir -95.6409} .. tension 0.836762 and 1 ..
 cycle
;

%%endpaths
%%beginbody





tr_:= tr_ shifted (0,-54);


enddefchar;
defchar(lam.fina,-1,-1,-1,-1);
%%backgroundimage:images/lam.fina.jpg,-718.948,-965.443,1,0,0,1
%%beginparams
rightVerticalRatio := 3;

%%beginverbatim

righttatweel := righttatweel -0.5;

%%endverbatim

%%beginpaths

fill
(59.9444,-129.611) .. tension 1 and 9.24347 ..
 {right}(124.446,87.569) .. tension 1 and 1 ..
 {down}(127.787,80.5147) .. tension 1.11576 and 1 ..
 {down}(100.913,-57.6065) .. tension 0.943093 and 0.873211 ..
 {right}(297.887,-226.691) .. tension 1.15616 and 2.14162 ..
 {up}(528.594,-90.6926) .. tension 2.18589 and 4.51974 ..
 {up}(432.754,616.815) .. tension 1.1566 and 37.074 ..
 {right}(461.227,721.907) .. tension 19.2653 and 0.817189 ..
(525,329.083) .. tension 1 and 1.58225 ..
(540.36,249.597) .. tension 1 and 1 ..
brdmvi((560.785,173.684),(120.493,-130.29)) ..
brdmvi((579.212,132.715),(285.795,-205.706)) ..
brdmvi((594.772,110.77),(526.653,-257.249)) link
brdmvi((582.458,20.5653),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((577.292,23.0395),(518.119,-253.844)) ..
brdmvi((569.457,31.9515),(230.855,-164.217)) ..
brdmvi((563.709,41.6472),(61.8093,-56.9138)) .. tension 1 and 1 ..
brdmvi((558.619,52.7873),(15.8822,-14.0333)) .. tension 0.87314 and 10000 ..
 {left}(556.25,58.9445) .. tension 10000 and 1 ..
 {down}(557.298,-49.4178) .. tension 1.07609 and 1 ..
 {left}(281.944,-304.944) .. tension 1 and 1 ..
 {up} cycle
;

%%endpaths
enddefchar;
beginchar(lam.fina.afterkaf,-1,-1,-1,-1);
%%backgroundimage:images/kaflam.jpg,-1331,-1271,1,0,0,1
%%beginparams
rightVerticalRatio := 3;

%%beginverbatim



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
(583,147) .. tension 1 and 1 ..
brdmvi((664.949,94.7513),(0,0)) link
brdmvi((574.603,42.1122),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((578.27,55.5812),(0,0)) .. tension 1 and 1 ..
 {dir 131.782}(553.435,82.7955) {dir -81.3859} .. tension 2.71316 and 1 ..
 {down}(563.965,-65.0845) .. tension 1.07609 and 1 ..
 {left}(276.611,-316.611) .. tension 1 and 1 ..
 {up} cycle
;

%%endpaths
endchar;
beginchar(meem.isol,1605,-1,-1,-1);
%%backgroundimage:images/meem.isol.jpg,-18.4208,-1031.28,1,0,0,1

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
%%backgroundimage:images/meem.init.jpg,-215.142,-887.787,1,0,0,1
%%beginparams
leftVerticalRatio := 6;

%%beginverbatim

lefttatweel := lefttatweel - 0.3;

%%endverbatim

%%beginpaths

fill
bldmvi(bldm((145.763,46.058),(-172.34,-52.4232)),(-75.9984,-49.4945)) ..
bldmvi((132.75,34.8292),(-299.277,-101.041)) ..
bldmvi((125.811,29.6856),(-459.034,-118.455)) ..
bldmvi((114.611,24.6543),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((136.361,114.659),(-501.488,-127.139)) ..
bldmvi((149.122,124.77),(-199.269,-86.0359)) ..
bldmvi(bldm((160.736,137.892),(-438.176,-171.586)),(58.0249,2.60245)) .. tension atleast 1 ..
(173.767,161.309) .. tension 1 ..
(195.325,239.138) .. tension 1.83772 and 1 ..
(228.634,293.543) .. tension 1 and 1.53351 ..
 {right}(291.246,338.97) .. tension 1.41266 and 1.83398 ..
 {down}(388.606,144.111) .. tension 1.20926 and 3.07538 ..
 {left}(368.534,78.116) .. tension 0.790113 and 1.39505 ..
(263.403,82.83) .. tension 1 and 1.60112 ..
 {dir 121.679}(192.553,121.504) {dir -113.635} .. tension 0.943483 and 2.12515 ..
bldm((174.587,82.5358),(-69.1229,-53.8516)) .. tension 1 ..
 cycle
;
fill
(344.045,162.962) .. tension 6.08809 and 1.12262 ..
 {left}(272.528,248.499) .. tension 1.0287 and 5.56708 ..
(212.391,196.463) {down} .. tension 9.88547 and 5.57666 ..
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
 {dir 129.387}(156.912,244.064) {curl1} .. tension 1 and 1 ..
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
%%backgroundimage:images/meem.medi.jpg,-1054.07,-1541.54,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 6;

%%beginverbatim



righttatweel := righttatweel - 0.5;
lefttatweel := lefttatweel - 0.5;

%%endverbatim

%%beginpaths

fill
(165.217,56.0008) .. tension 1 and 1 ..
bldmvi((140.407,34.1559),(-70.5154,-40.7577)) ..
bldmvi((124.01,22.0236),(-207.917,-88.3588)) ..
bldmvi((112.073,13.38),(-458.992,-130.427)) ..
bldmvi((98.971,9),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((127.685,97.8915),(-448.359,-127.285)) ..
bldmvi((142.917,101.883),(-216.195,-88.2786)) ..
bldmvi((153.545,107.799),(-105.741,-61.0123)) .. tension 1 and 1 ..
(171.942,122.402) .. tension 1 and 1 ..
(201.341,157.938) .. tension 1 and 2.03022 ..
 {right}(302.262,253.392) .. tension 1.08595 and 1.58165 ..
brdmvi((376.744,187.843),(20.8333,-9.83796)) .. tension 1.21643 and 1 ..
brdmvi((410.533,154.625),(99.9398,-57.551)) ..
brdmvi((442.851,131.34),(328.7,-121.417)) ..
brdmvi((472.885,116.697),(672.787,-142.158)) link
brdmvi((474.807,24.57),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((470.152,25.7252),(651.445,-136)) ..
brdmvi((456.43,31.6039),(283.598,-101.018)) ..
brdmvi((448.606,38.1327),(19.9462,-10.7389)) .. tension 2.34693 and 10000 ..
(447.049,39.5448) {left} .. tension 10.4171 and 10.4171 ..
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
defchar(meem.medi.ii,-1,-1,-1,-1);
%%backgroundimage:images/meem.medi.jpg,-1050.07,-1542.82,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 3;

%%beginverbatim



righttatweel := righttatweel - 0.5;
lefttatweel := lefttatweel - 0.5;

%%endverbatim

%%beginpaths

fill
(165.217,56.0008) .. tension 1 and 1 ..
bldmvi((140.407,34.1559),(-70.5154,-40.7577)) ..
bldmvi((124.01,22.0236),(-207.917,-88.3588)) ..
bldmvi((112.073,13.38),(-458.992,-130.427)) ..
bldmvi((98.971,9),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((127.685,97.8915),(-448.359,-127.285)) ..
bldmvi((142.917,101.883),(-216.195,-88.2786)) ..
bldmvi((153.545,107.799),(-105.741,-61.0123)) .. tension 1 and 1 ..
(171.942,122.402) .. tension 1 and 1 ..
(201.341,157.938) .. tension 1 and 2.03022 ..
 {right}(302.262,253.392) .. tension 1.08595 and 1.58165 ..
brdmvi((376.744,187.843),(20.8333,-9.83796)) .. tension 1.21643 and 1 ..
brdmvi((410.533,154.625),(125.408,-88.329)) ..
brdmvi((444.197,129.545),(362.505,-203.034)) ..
brdmvi((480.133,114.464),(596.667,-259.198)) link
brdmvi((474.807,24.57),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((469.503,25.5305),(581.045,-260.68)) ..
brdmvi((456.809,30.7926),(313.593,-182.402)) ..
brdmvi((448.606,37.7),(31.0018,-22.3222)) .. tension 2.34693 and 10000 ..
(447.049,39.5448) {left} .. tension 10.4171 and 10.4171 ..
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
leftVerticalRatio := 6;
z3 = (254.53,242.472);

%%beginverbatim


lefttatweel := lefttatweel - 0.3;


%%endverbatim

%%beginpaths

fill
bldmvi((167.145,59.4754),(-103.861,-69.139)) ..
bldmvi((153.849,45.0498),(-297.341,-113.153)) ..
bldmvi((142.008,35.6309),(-553.53,-133.247)) ..
bldmvi((127.421,30.5726),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((148.318,119.108),(-498.328,-128.199)) ..
bldmvi((159.997,120.452),(-314.429,-108.073)) ..
bldmvi((169.365,123.945),(-76.7586,-47.8234)) .. tension atleast 1 ..
(174.623,130.79) .. tension 1.83772 and 1 ..
(198.867,176.98) .. tension 1 and 2.38216 ..
 {right}z3 .. tension 1.58704 and 1.10448 ..
 {down}(392.149,87.7013) .. tension 1.53803 and 1.02193 ..
(381.944,40.9915) .. tension 1.59086 and 1.13839 ..
 {dir 123.926}(182.993,82.0187) {curl1} .. tension 1 and 1 ..
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
%%backgroundimage:images/meemmediyeh.jpg,-1555.45,-1102.8,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z3 = (4.8011,-69.1123);

%%beginverbatim

righttatweel := righttatweel - 0.5;






%%endverbatim

%%beginpaths

fill
(168.582,-11.3043) .. tension 1 and 11.234 ..
 {up}z3 .. tension atleast 1 ..
(25.0029,-19.0246) .. tension 2.43688 and 0.75 ..
(159.245,39.8456) .. tension 0.75 and 1.76676 ..
 {right}(359.331,215.577) .. tension 2.82523 and 1.58165 ..
(421.152,152.075) .. tension 1.21643 and 24.6924 ..
brdmvi((458.863,118.621),(51.6993,-20.6107)) ..
brdmvi((483.236,102.28),(276.422,-88.0837)) ..
brdmvi((508.859,92.044),(647.171,-131.467)) link
brdmvi((502.65,3.27918),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((499.019,3.45358),(614.875,-127.061)) ..
brdmvi((493.929,5.66155),(216.162,-67.7083)) ..
brdmvi((489.781,8.99394),(13.9773,-6.667)) .. tension 0.765293 and 1.49796 ..
 {dir brdmvi(119.615,8.7795)}(485.685,13.0849) {dir -94.3461} .. tension 2.63339 and 2.93508 ..
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
%%backgroundimage:images/meem.fina.ii.jpg,-232.891,-1016.61,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z.mark1 = (221.75,-144.75);

%%beginverbatim

righttatweel := righttatweel - 0.3;

%%endverbatim

%%beginpaths

fill
(176.06,42.2183) .. tension 0.75 and 1 ..
 {up}(157.138,124.333) .. tension 1.73037 and 1.61706 ..
 {right}(257.324,232.371) .. tension 2.84357 and 1 ..
brdmvi((311.657,170.705),(4.34028,1.92901)) .. tension 1 and 1 ..
brdmvi((352.352,133.319),(143.98,-64.549)) ..
brdmvi((372.383,121.442),(326.011,-102.183)) ..
brdmvi((386.58,116.544),(584.594,-132.064)) link
brdmvi((381.827,26.3318),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((378.201,27.2413),(409.19,-113.726)) ..
brdmvi((375.045,29.3671),(202.743,-74.814)) ..
brdmvi((373.408,30.8573),(77.3582,-36.9931)) .. tension 1 and 0.75 ..
 {dir brdmvi(140.402,8.67031)}(371.998,32.2155) {dir -97.3514} .. tension 1.3164 and 2.03251 ..
 {left}(357.134,-1.0052) .. tension 1.46691 and 6.1513 ..
 {left}(207.702,24.2488) .. tension 2.37283 and 5.43489 ..
(191.029,-2.7652) .. tension 1.17645 and 18.3373 ..
 {left}(-97.9145,-170.297) .. tension 1 and 1 ..
 {up}(-115.046,-156.25) .. tension 15.7865 and 1 ..
(-44.4444,-119.792) .. tension 1.73988 and 6.20895 ..
 {up} cycle
;
fill
(247.506,185.735) .. tension 1.78577 and 1.74187 ..
 {down}(183.215,132.856) .. tension 6.28125 and 7.79118 ..
 {up}(305.102,94.7351) .. tension 8.86867 and 3.01664 ..
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
%%backgroundimage:images/meem.fina.ii.alt1.jpg,-4531.23,-1379.15,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim

righttatweel := righttatweel - 0.3;
%%endverbatim

%%beginpaths

fill
(43.6143,-109.691) .. tension 0.75 and 1 ..
(104.166,65.7874) .. tension 1 and 1.34941 ..
 {right}(256.822,195.816) .. tension 1.32087 and 1 ..
brdmvi((339.988,115.259),(21.2995,-0.803755)) .. tension 1 and 1 ..
brdmvi((366.394,95.4185),(91.497,-31.8354)) ..
brdmvi((387.907,86.0642),(316.2,-84.66)) ..
brdmvi((406.935,82.2441),(667.36,-129.64)) link
brdmvi((399.331,-5.53835),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((396.095,-5.44024),(567.016,-116.623)) ..
brdmvi((393.294,-3.66539),(148.324,-42.1078)) ..
brdmvi((390.22,-1.62011),(19.9103,-4.4132)) .. tension 1 and 1 ..
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
%%backgroundimage:images/noon.fina.jpg,-103.029,-1182.59,1,0,0,1
%%beginparams
rightVerticalRatio := 3;
z11 = (214.167,258);

%%beginverbatim
righttatweel := righttatweel - 0.5;



%%endverbatim

%%beginpaths

fill
(254.147,-295.843) .. tension 0.789758 and 1 ..
 {up}(52.1667,-101.325) .. tension 0.75 and 7.66316 ..
 {right}(133.054,120.589) .. tension 0.75 and 1.12749 ..
 {down}(138.564,111.142) .. tension 5.01017 and 0.75 ..
 {down}(91.0247,-63.3812) .. tension 1.40455 and 0.768469 ..
 {right}(278.67,-208.985) .. tension 1.32949 and 1.40115 ..
 {up}(516.898,-66.3249) .. tension 0.893232 and 7.19193 ..
 {up}(443.259,139.74) .. tension 6.32194 and 8.80145 ..
 {right}(464.09,213.436) .. tension 19.7266 and 1 ..
brdmvi((504.002,145.013),(9.6,-3.6)) .. tension 1 and 1 ..
brdmvi((516.302,131.4),(24.4464,-17.6268)) .. tension 1 and 1 ..
brdmvi(brdmvi((527.59,122.09),(744.801,-273.815)),(-544.749,147.394)) ..
brdmvi((542.053,115.064),(436,-216.8)) ..
brdmvi((564.15,110.224),(650,-261)) link
brdmvi((553.607,21.8611),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((552.56,22.2926),(644.185,-260.879)) ..
brdmvi((550.545,23.2212),(358.136,-189.882)) ..
brdmvi((549.435,23.9048),(12.3682,-9.03485)) .. tension atleast 0.87609 ..
(548.741,24.4207) .. tension atleast 0.87609 and atleast 0.75 ..
(547.955,24.9372) {curl1} .. tension 2.6502 and 1.14803 ..
 {down}(547.876,-30.7184) .. tension 1.13331 and 1.13195 ..
(501.728,-180.588) .. tension 1.11747 and 1.18217 ..
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
%%backgroundimage:images/feh.isol.jpg,-132.477,-1058.16,1,0,0,1
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
z1 = (474.422,244.174);
z11 = (262.747,335.166);
z2 = (464.756,160.359);

%%beginverbatim


%%endverbatim

%%beginpaths

fill
(49.402,-121.47) .. tension 1 and 6.41744 ..
 {right}(117.361,100.694) .. tension 1 and 1 ..
 {down}(123.611,92.3611) .. tension 7.13715 and 1 ..
 {down}(90.9722,-52.0833) .. tension 1 and 1 ..
 {right}(271.528,-209.028) .. tension 0.75 and 5.38906 ..
 {up}(591.631,-49.1962) .. tension 1 and 4.47302 ..
(580.942,7.65179) .. tension 1.51898 and 1 ..
(526.416,107.995) .. tension 0.890434 and 1.4933 ..
 {left}z2 .. tension 5.44047 and 3.40173 ..
 {right}z1 .. tension 1.28201 and 4.69626 ..
(615.139,24.9726) {curl1} .. tension 1.43469 and 1 ..
(584.731,-138.406) .. tension 1.20455 and 1 ..
 {left}(283.333,-288.889) .. tension 1 and 1 ..
 {up} cycle
;
fill
(399.564,29.1972) .. tension 1 and 1 ..
(383.028,106.096) .. tension 1 and 1 ..
(407.542,185.667) .. tension 1 and 1.34802 ..
z1 {right} .. tension 3.00215 and 4.73364 ..
z2 {left} .. tension 1 and 1 ..
 {down}(418.621,113.289) .. tension 1.08819 and 1 ..
(436.931,93.6403) link
(530.789,-9.43275) ..
 cycle
;

%%endpaths
%%beginbody
drawcomp(two_dots,identity shifted z11);
enddefchar;
defchar(qaf.fina.expa,-1,-1,-1,-1);
%%backgroundimage:images/qaf.expa.max.jpg,-815.969,-546.433,1,0,0,1
%%beginparams
z1 = (469.117,249.479);
z11 = (290.347,337.566);
z2 = (464.756,155.536);

%%beginverbatim
%%backgroundimage:images/qaf.expa.max.jpg,-815.969,-546.433,1,0,0,1
%%backgroundimage:images/noon.expa.min.jpg,-247.387,-852.505,1,0,0,1

minLeft := 0;
maxLeft := 4.7;

%%endverbatim

%%beginpaths

fill
bldmvi((-203.387,-74.8082),(-maxLeft * nuqta,0)) .. tension 1.23926 and 0.88363 ..
bldmvi((-186.361,-77.7397),(-558,4)) .. tension 5.02552 and 1 ..
bldmvi((-68.4414,-157.232),(-547.851,7.0913)) .. tension 1 and 1 ..
 {right}bldmvi((178.537,-202.984),(-227.4,-58.2)) ..
bldmvi((357.944,-177.463),(-107.433,-47.6)) ..
bldmvi((543.242,-65.6045),(0,0)) .. tension 1 and 1.15551 ..
 {up}(580.733,-2.842) .. tension 1 and 4.47302 ..
(566.474,45.2675) .. tension 1.51898 and 1 ..
(526.416,107.995) .. tension 0.890434 and 1.4933 ..
 {left}z2 .. tension 5.44047 and 3.40173 ..
 {right}z1 .. tension 1.28201 and 4.69626 ..
(573.806,163.392) {curl1} .. tension 1.43469 and 0.776126 ..
 {down}(612.858,35.5801) .. tension 0.863991 and 1.281 ..
bldmvi((572.023,-101.648),(-37.68,-41.04)) .. tension 1 and 1 ..
bldmvi((360.445,-252.334),(-122.12,-56.4)) .. tension 0.906922 and 0.906922 ..
 {left}bldmvi((149.652,-283.932),(-228,-62)) .. tension 1.85292 and 0.873173 ..
bldmvi((-118.866,-190.066),(-566.611,17.2763)) .. tension 0.873173 and 0.873173 ..
bldmvi((-190.802,-104.012),(-560.824,5.00463)) .. tension 4.05368 and 1.11348 ..
 {up} cycle
;
fill
(511.557,-10.2239) .. tension 1.4017 and 1.04079 ..
(424.496,4.74439) .. tension 1 and 1 ..
(387.851,101.756) .. tension 1 and 1 ..
(411.882,182.291) .. tension 1 and 1.34802 ..
z1 {right} .. tension 3.00215 and 4.73364 ..
z2 {left} .. tension 1 and 1 ..
 {down}(422.961,104.608) .. tension 1.08819 and 1 ..
(433.555,94.1226) link
(530.789,-9.43275) ..
 cycle
;

%%endpaths
%%beginbody
drawcomp(two_dots,identity shifted z11);
enddefchar;
defchar(fehshape.init,-1,-1,-1,-1);
%%backgroundimage:images/feh.init.2.jpg,-636.591,-1097.61,1,0,0,1
%%beginparams
leftVerticalRatio := 6;

%%beginverbatim

%lefttatweel :=lefttatweel;

%%endverbatim

%%beginpaths

fill
(178.59,88.8973) .. tension 1 and 0.831709 ..
bldmvi((154.328,44.6047),(-23.3334,-17.3611)) ..
bldmvi((126.039,15.6855),(-291.847,-107.171)) ..
bldmvi((98.5934,2.58463),(-578.078,-133.229)) ..
bldmvi((64.2185,-0.265372),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((103.976,90.2672),(-545.529,-133.033)) ..
bldmvi((128.994,98.5639),(-249.686,-103.389)) ..
bldmvi((149.367,115.005),(-18.0555,-18.1945)) .. tension atleast 0.946192 ..
(159.636,132.58) .. tension 1 and 1.27059 ..
 {up}(162.365,149.026) .. tension 0.75 and 1.94537 ..
 {dir 135.849}(136.842,224.989) {dir -95.641} .. tension 2.22632 and 0.75 ..
 {left}(61.7085,152.502) .. tension 0.786927 and 2.08491 ..
 {up}(-17.0958,237.057) .. tension 0.798746 and 2.70287 ..
 {right}(58.592,411.61) .. tension 1.94666 and 0.926451 ..
 {down}(195.965,176.191) .. tension 1 and 1.13522 ..
 cycle
;
fill
(16.8074,272.337) .. tension 1 and 2.02774 ..
 {right}(68.8129,237.414) .. tension 1 and 2.05693 ..
 {up}(114.975,263.553) .. tension 2.16369 and 1.13985 ..
 {left}(55.8994,318.337) .. tension 1.60243 and 1.41932 ..
 {down} cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(fehshape.init.beforehah,-1,-1,-1,-1);
%%backgroundimage:images/fehhah.jpg,-3618.58,-1218.42,1,0,0,1
%%beginparams
z1 = (-19.505,74.0501);

%%beginverbatim
%charlt := 4; 
lefttatweel := charlt + 0.7;

%%endverbatim

%%beginpaths

fill
(119.145,45.5748) .. tension 1 and 1 ..
(39.9306,28.3565) .. tension 1 and 1 ..
z1 .. tension 2.21929 and 1.41503 ..
 {up}(153.458,146.701) .. tension 3.75901 and 3.3084 ..
 {left}(139.757,208.882) .. tension 4.28107 and 0.75 ..
 {left}(46.3511,155.894) .. tension 0.831576 and 2.08491 ..
 {up}(-31.9183,222.575) .. tension 0.90666 and 2.70287 ..
 {right}(71.0108,412.447) .. tension 1.69552 and 0.924395 ..
 {down}(193.037,185.147) .. tension 1.18307 and 1.36164 ..
 cycle
;
fill
(18.2137,276.507) .. tension 1 and 1.46882 ..
 {right}(74.4429,237.289) .. tension 1.38247 and 1.19098 ..
 {up}(113.818,265.289) .. tension 2.96303 and 2.54524 ..
 {left}(53.2493,318.456) .. tension 1.60243 and 1 ..
 {down} cycle
;

%%endpaths
%%beginbody

rightbearing :=  380 + rightbearing;
leftbearing := -190;

%leftanchor := z1;

setCursiveAnchor(fehhah,z1, ExitAnchorRTL);

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
%%backgroundimage:images/feh.medi.jpg,-925.142,-1017.67,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 6;
z1 = (283.218,379.507);
z2 = (267.3,290.214);

%%beginverbatim

lefttatweel := lefttatweel - 0.5;
righttatweel := righttatweel - 0.5;

%%endverbatim

%%beginpaths

fill
bldmvi((254.622,76.94),(-51.6011,-46.2963)) .. tension 1 and 1 ..
bldmvi((180.619,34.089),(-105.108,-58.804)) ..
bldmvi((150.648,20.0085),(-218.49,-88.6324)) ..
bldmvi((123.251,10.8727),(-511.818,-132.059)) ..
bldmvi((113.557,10.0404),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((138.884,98.7804),(-499.695,-132.117)) ..
bldmvi((174.49,106.246),(-212.778,-91.9445)) ..
bldmvi((239.192,137.132),(-79.6618,-54.0406)) .. tension atleast 1 ..
(301.79,180.089) .. tension 1 and 6.49127 ..
 {up}(327.476,210.262) .. tension 1.2515 and 1.5005 ..
 {left}z2 .. tension 4.37336 and 4.30722 ..
 {right}z1 .. tension 1.06386 and 1 ..
(337.515,343.109) .. tension 1.0253 and 0.75 ..
 {down}(359.631,241.378) .. tension 1.00284 and 1 ..
 cycle
;
fill
(171.255,87.27) .. tension 1 and 1 ..
 {up}(136.202,169.564) .. tension 1 and 1 ..
(199.139,318.75) .. tension 1 and 1.27163 ..
 {right}z1 .. tension 5.38976 and 5.98541 ..
 {left}z2 .. tension 1 and 12.4958 ..
 {down}(171.171,199.873) .. tension 1.89317 and 1 ..
(185.395,178.532) .. tension 1 and 1 ..
brdmvi((232.261,142.904),(93,-53)) ..
brdmvi((281.654,112.118),(376,-113)) ..
brdmvi((322.489,97.9748),(618,-130)) link
brdmvi((313.137,8.95196),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((302.563,10.7632),(637.288,-130.456)) ..
brdmvi((258.366,27.0002),(365,-107)) ..
brdmvi((220.091,49.5314),(9,-6)) .. tension 1 and 1 ..
 cycle
;

%%endpaths
enddefchar;
defchar(fehshape.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/fehshapeyeh.jpg,-470.797,-1157.09,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z1 = (286.082,381.931);
z2 = (272.838,289.967);
z3 = (18.2369,-4.5802);

%%beginverbatim



righttatweel := righttatweel -0.5;



%%endverbatim

%%beginpaths

fill
(207.106,63.086) .. tension 1 and 9.24615 ..
 {up}z3 .. tension atleast 0.80651 ..
(52.7884,48.2048) .. tension 3.13837 and 4.35001 ..
(228.044,133.886) .. tension 1.04 and 5.57772 ..
 {up}(331.281,219.739) .. tension 2.27443 and 1.11578 ..
 {left}z2 .. tension 4.71418 and 7.77195 ..
 {right}z1 .. tension 1.00915 and 4.59674 ..
 {down}(362.966,243.879) .. tension 0.811284 and 3.17615 ..
(332.738,156.25) .. tension 1 and 1 ..
 cycle
;
fill
brdmvi((175.356,122.347),(10,-10)) .. tension 1 and 1 ..
 {up}(154.059,192.901) .. tension 1 and 1 ..
(194.568,298.997) .. tension 1 and 1.58196 ..
 {right}z1 .. tension 2.54056 and 5.6745 ..
 {left}z2 .. tension 1.65835 and 3.60061 ..
 {down}(185.553,213.37) .. tension 1.93649 and 1 ..
(235.519,151.83) .. tension 1 and 1 ..
brdmvi((276.224,122.649),(89,-49)) ..
brdmvi((306.627,106.881),(238,-90)) ..
brdmvi((325.784,100.099),(596,-133)) link
brdmvi((315.117,10.8179),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((292.381,15.0748),(597,-132)) ..
brdmvi((258.737,30.7924),(262,-94)) ..
brdmvi((223.121,61.3363),(88,-48)) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody


setCursiveAnchor(yehcursive1,z3,ExitAnchor);
enddefchar;
defchar(feh.fina,-1,-1,-1,-1);
%%backgroundimage:images/feh.fina.jpg,-2.88889,-1208.69,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z1 = (786.152,334);
z11 = (608.4,393.6);
z2 = (769.234,244.605);

%%beginverbatim
righttatweel := righttatweel -0.5;


%%endverbatim

%%beginpaths

fill
(56,92) .. tension 1 and 4.4706 ..
 {right}(122.73,267.654) .. tension 1 and 1 ..
 {down}(130.556,262.5) .. tension 1.49403 and 1 ..
 {down}(94.4444,156.582) .. tension 0.856535 and 2.00403 ..
 {right}(335.379,6.06674) .. tension 1 and 7.23208 ..
(601.206,53.289) .. tension 1 and 3.09654 ..
 {up}(820.836,170.881) .. tension 2.05893 and 2.35809 ..
 {left}z2 .. tension 5.95701 and 6.62733 ..
 {right}z1 .. tension 1.95949 and 1 ..
 {down}(861.092,200.565) .. tension 1.18594 and 1.42885 ..
(778.415,55.675) .. tension 2.527 and 2.527 ..
(682.144,6.73921) .. tension 8.82087 and 0.75 ..
 {left}(336.111,-69.4445) .. tension 0.878883 and 1.01539 ..
 {up} cycle
;
fill
(653.335,115.747) .. tension 1 and 2.1057 ..
 {right}z1 .. tension 3.20057 and 6.45091 ..
 {left}z2 .. tension 1.7891 and 1.76858 ..
 {down}(687.667,159.433) .. tension 1.06071 and 1 ..
brdmvi((700.684,140.827),(5.55556,-6.25)) .. tension 1 and 1 ..
brdmvi((736.334,116.211),(142.018,-63.2916)) ..
brdmvi((773.088,106.186),(307.986,-99.264)) ..
brdmvi((804.493,100.098),(561.022,-125.111)) link
brdmvi((805.833,10.8333),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((785.278,13.7056),(545.245,-124.768)) ..
brdmvi((748.247,25.0173),(304.363,-98.3843)) ..
brdmvi((698.756,49.591),(6.57593,-4.09447)) .. tension 1 and 1 ..
 {up} cycle
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
defchar(hah.init,-1,-1,-1,-1);
%%backgroundimage:images/jeem.init.jpg,-1526.31,-1022.05,1,0,0,1
%%beginparams
leftVerticalRatio := 6;

%%beginverbatim

lefttatweel := lefttatweel + 0.5;



%%endverbatim

%%beginpaths

fill
bldmvi((227.04,56.6803),(34.5497,22.2648)) ..
bldmvi((207.628,47.0214),(-404.197,-119.146)) ..
bldmvi((181.74,35.8277),(-673.059,-142.912)) ..
bldmvi((128.708,28.0972),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((192.611,117.818),(-662.777,-131.899)) ..
bldmvi((221.857,125.549),(-399.268,-111.222)) ..
bldmvi((248.843,134.385),(-179.167,-60.8333)) .. tension 1.04484 and 1 ..
(295.718,157.211) .. tension 1 and 2.26749 ..
 {up}(323.469,177.382) .. tension 17.0846 and 1.63017 ..
 {left}(167.824,230.324) .. tension 0.93697 and 2.31909 ..
(75.7784,212.606) .. tension 1 and 2.50292 ..
 {left}(39.4548,184.233) .. tension 0.882523 and 1 ..
 {up}(35.3118,194.235) .. tension 1.45052 and 1.3939 ..
 {right}(175.975,325.714) .. tension 1.91368 and 0.841609 ..
(323.913,260.015) .. tension 1 and 2.30651 ..
 {right}(549.866,220.183) .. tension 1 and 2.3242 ..
 {right}(632.823,226.015) .. tension 1 and 1 ..
 {down}(637.731,221.065) .. tension 5.83816 and 4.05417 ..
 {left}(593.75,135.995) .. tension 1.26894 and 1.21765 ..
 {left}(434.965,150.151) .. tension 1.48463 and 2.3458 ..
 cycle
;

%%endpaths
enddefchar;
defchar(hah.init.ii,-1,-1,-1,-1);
%%backgroundimage:images/hah.init.ii.jpg,-1743,-1342,1,0,0,1
%%beginparams
z1 = (0.0793679,8.81764);

%%beginverbatim

%%endverbatim

%%beginpaths

fill
(243.981,78.8194) .. tension 1 and 1 ..
z1 link
(29.9791,97.2436) .. tension 1 and 1 ..
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
defchar(hah.init.beforemeem,-1,-1,-1,-1);
%%backgroundimage:images/hahmeem.jpg,-2480.94,-1300.47,1,0,0,1
%%beginparams
z1 = (74.3146,159.432);

%%beginverbatim
%charlt := 4; 
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
%%backgroundimage:images/heh.medi.jpg,-872.406,-1173.11,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 6;

%%beginverbatim


%righttatweel := righttatweel - 0.5;
lefttatweel := lefttatweel + 0.3;





%%endverbatim

%%beginpaths

fill
bldmvi((225.363,77.2803),(-12,-5)) ..
bldmvi((197.437,59.9032),(-348,-112)) ..
bldmvi((152.02,37.6869),(-576,-132)) ..
bldmvi((110.865,25.763),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((160.042,118.516),(-656,-132)) ..
bldmvi((187.992,127.964),(-317,-95)) ..
bldmvi((234.494,149.036),(-119,-49)) .. tension atleast 1 ..
(287.865,179.592) .. tension 1 and 29.0915 ..
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
 {down}(467.551,170.188) .. tension 1 and 1 ..
brdmvi((469.033,148.926),(9,-4)) .. tension 1.34198 and 0.75 ..
brdmvi((483.128,123.814),(92.6947,-50.9167)) ..
brdmvi((513.746,108.401),(406.235,-124.025)) ..
brdmvi((550.062,101.095),(744.124,-137.592)) link
brdmvi((546.343,11.7547),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((526.466,15.5726),(722.222,-141.667)) ..
brdmvi((485.229,33.0933),(394.157,-134.634)) ..
brdmvi((450.775,83.8264),(84.0095,-66.3994)) ..
brdmvi((445.824,166.249),(0,0)) .. tension 4.4481 and 10000 ..
 {left}(446.679,171.895) .. tension 0.855179 and 0.880849 ..
(306.81,127.808) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(hah.medi.ii,-1,-1,-1,-1);
%%backgroundimage:images/fehhah.jpg,-9177.48,-1420.37,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim



righttatweel := righttatweel - 0.5;
%%endverbatim

%%beginpaths

fill
(-40.7641,-8.52663) link
(63.5052,90.9764) .. tension atleast 1 ..
(139.345,112.185) .. tension 1 and 13.1755 ..
 {up}(241.018,163.905) .. tension 22.9827 and 1.48323 ..
 {left}(26.6452,204.013) .. tension 1.16913 and 1.50936 ..
(-56.9832,165.143) .. tension 1 and 4.81383 ..
 {left}(-106.707,107.312) .. tension 1 and 1 ..
 {up}(-115.111,115.707) .. tension 1.43151 and 1.48305 ..
 {right}(49.5801,280.815) .. tension 0.75 and 1 ..
(223.414,251.106) .. tension 1 and 1.49835 ..
 {right}(443.861,216.794) .. tension 1 and 1.92009 ..
 {right}(487.115,227.027) .. tension 1 and 1 ..
 {down}(497.183,213.274) .. tension 1.19648 and 2.3702 ..
(465.469,146.269) .. tension 1.91897 and 1.46177 ..
 {down}(438.609,114.602) .. tension 1 and 1 ..
(439.389,104.388) .. tension 1 and 1 ..
brdmvi((442.954,99.1376),(2.89352,-6.75154)) .. tension 1 and 1 ..
brdmvi((449.617,94.6351),(111.897,-77.3981)) ..
brdmvi((459.36,91.8488),(466.427,-129.036)) ..
brdmvi((468.317,90.6302),(696.824,-137.17)) link
brdmvi((458.113,2.08398),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((444.755,3.22574),(685.764,-138.31)) ..
brdmvi((419.321,11.7314),(467.593,-131.936)) ..
brdmvi((399.822,34.308),(79.0318,-68.0517)) .. tension 1 and 1 ..
brdmvi((390.611,58.9694),(0.669796,0.669796)) .. tension 0.75 and 2.3554 ..
(389.065,78.0735) {up} .. tension 4.4481 and 4.4481 ..
 {up}(395.411,129.365) .. tension 2.90264 and 1 ..
 {left}(358.721,136.464) .. tension 1 and 0.880849 ..
(212.601,81.6402) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

enddefchar;
defchar(hah.medi.afterbeh,-1,-1,-1,-1);
%%backgroundimage:images/yeh_hah.init.jpg,-2433,-1310,1,0,0,1
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


(t3,u3) = p intersectiontimes z3;
(t4,u4) = p intersectiontimes z4;

dm1 := z3 + l* unitvector direction t3 of p;
dm2 := z4-  l* unitvector direction t4 of p;

currentpicture:=nullpicture;

fill subpath(0,t3) of p .. controls   dm1 .. 1/2[dm1,dm2] .. controls dm2 ..  subpath(t4,length p) of p .. cycle ;


rightbearing :=round (x4 - x5);

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
leftVerticalRatio := 6;
z1 = (244.542,66.2808);
z3 = (99.2837,288.729);
z5 = (643.65,229.071);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
bldmvi(z1,(-125,-55)) ..
bldmvi((194.529,40.3874),(-409.167,-114.167)) ..
bldmvi((133.395,20.0893),(-695.833,-134.167)) ..
bldmvi((115.521,17.7438),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((149.278,106.806),(-695.833,-134.167)) ..
bldmvi((210.387,122.325),(-408.333,-115)) ..
bldmvi((315.389,167.555),(-162.5,-77.5)) .. tension 1 and 3.07933 ..
(336.654,180.817) .. tension 7.16575 and 7.16575 ..
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
(329.435,113.326) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

rightbearing := -345;

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
defchar(hah.medi.aftermeem,-1,-1,-1,-1);
%%backgroundimage:images/meemhah.jpg,-4016,-1416,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
z1 = (192.425,55.0229);
z3 = (45.124,267.657);
z5 = (582.886,202.547);
z6 = (-13.9478,194.982);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
bldmvi(z1,(-111.111,-46.5278)) ..
bldmvi((119.926,38.5811),(-246.528,-86.8056)) ..
bldmvi((73.5285,32.8176),(-527.083,-129.861)) ..
bldmvi((42.7813,32.2114),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((95.0249,120.67),(-498.611,-128.472)) ..
bldmvi((176.66,127.477),(-247.917,-88.1944)) ..
bldmvi((245.191,141.346),(-111.111,-41.6667)) .. tension 1 and 1 ..
(272.468,149.838) .. tension 1.61182 and 18.3423 ..
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
defchar(hah.medi.afterfeh,-1,-1,-1,-1);
%%backgroundimage:images/fehhah.jpg,-3474.67,-1433.67,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
z1 = (217.936,51.679);
z3 = (99.2837,288.729);
z4 = (312.09,276.392);
z5 = (663.141,208.508);

%%beginverbatim


%%endverbatim

%%beginpaths

fill
bldmvi(z1,(-131.944,-42.3611)) ..
bldmvi((181.775,40.6463),(-383.333,-100)) ..
bldmvi((133.41,29.002),(-613.194,-126.389)) ..
bldmvi((115.039,27.3889),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((147.254,115.834),(-609.028,-129.861)) ..
bldmvi((196.428,122.046),(-354.167,-95.1389)) ..
bldmvi((273.603,144.011),(-170.833,-52.7778)) .. tension 1 and 1 ..
(325.242,165.104) .. tension 7.16575 and 7.16575 ..
 {up}(334.205,169.863) .. tension 10.8298 and 1.79081 ..
(225.759,205.767) .. tension 0.815564 and 3.20171 ..
 {left}(113.343,219.514) .. tension 0.75 and 6.35213 ..
 {left}(59.8623,197.764) .. tension 1.39544 and 1 ..
 {up}(53.7016,206.034) .. tension 1.74599 ..
z3 {dir 23} .. tension 1.57381 and 1.71452 ..
(428.262,234.346) .. tension 1 and 6.46414 ..
 {right}(656.827,223.759) .. tension 0.75 and 1.74592 ..
 {down}z5 .. tension 7.62288 ..
 {left}(629.287,143.76) .. tension 2.71262 and 0.75 ..
 {left}(558.376,148.569) .. tension 1.01367 and 0.75 ..
(323.347,95.9676) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

rightbearing := -400;

setCursiveAnchor(fehhah,z3, EntryAnchorRTL);

enddefchar;
defchar(hah.fina,-1,-1,-1,-1);
%%backgroundimage:images/hah.fina.jpg,-24,-1135,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim

righttatweel := righttatweel - 0.5;

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
brdmvi((532.86,130.496),(0,0)) ..
brdmvi((564.505,109.229),(144,-74.4)) ..
brdmvi((592.856,101.052),(566.4,-133.2)) link
brdmvi((600.304,11.305),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((555.235,20.2862),(556.8,-136.8)) ..
brdmvi((508.719,54.1828),(130.8,-82.8)) ..
 {up}brdmvi((480.046,127.706),(0,0)) .. tension 1 and 1 ..
(483.877,164.931) .. tension 16.3629 and 16.3629 ..
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
%%backgroundimage:images/dal.fina.jpg,-803.239,-1066.32,1,0,0,1
%%beginparams
rightVerticalRatio := 3;

%%beginverbatim
righttatweel := righttatweel- 0.5;





%%endverbatim

%%beginpaths

fill
(63.8664,81.4689) .. tension 1 and 5.58004 ..
 {right}(76.2209,132.906) .. tension 5.11224 and 1 ..
(104.408,116.304) .. tension 1 and 1 ..
 {right}(163.098,108.218) .. tension 1 and 2.45586 ..
 {up}(369.695,213.719) .. tension 10.598 and 1 ..
(331.468,328.093) .. tension 1 and 1 ..
 {up}(316.454,432.292) .. tension 1 and 4.56942 ..
 {right}(329.282,483.845) .. tension 10.1375 and 1 ..
(362.011,365.468) .. tension 1.94655 and 2.41325 ..
(405.502,233.406) .. tension 1 and 1 ..
brdmvi((437.087,154.031),(83.2338,-90.6242)) ..
brdmvi((448.404,129.85),(279,-207)) ..
brdmvi((459.096,114.675),(584,-265.833)) link
brdmvi((451.2,21.2102),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((443.198,25.3389),(532,-255)) ..
brdmvi((425.43,52.8665),(255,-198)) ..
brdmvi((408.351,95.1319),(16.8789,-18.5186)) .. tension 1 and 1 ..
(387.483,156.844) .. tension 1.66689 and 22.0046 ..
 {left}(380.141,177.568) .. tension 16.8371 and 1 ..
(357.889,120.657) .. tension 1 and 1.35948 ..
 {left}(142.805,21.4121) .. tension 0.885773 and 1 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(dal.fina.afterlam,-1,-1,-1,-1);
%%backgroundimage:images/lamdal.jpg,-731.517,-1193.87,1,0,0,1

%%beginverbatim

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
(403.176,43.3486) link
(448.5,-55.5204) .. tension 0.928459 and 26.907 ..
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
%%backgroundimage:images/reh.fina.jpg,-496.214,-1172.07,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim
righttatweel := righttatweel - 0.5;



%%backgroundimage:images/reh.fina.jpg,-495,-1174,1,0,0,1
%%endverbatim

%%beginpaths

fill
(-241.826,-235.569) .. tension 1 and 1 ..
 {right}(-222.072,-218.642) .. tension 2.92944 and 1.23772 ..
 {right}(-112.705,-236.832) .. tension 1.20626 and 1 ..
(215.961,-101.237) .. tension 1 and 3.12081 ..
 {up}(298.611,22.2222) .. tension 10000 and 0.792016 ..
 {up}(234.722,102.778) .. tension 1 and 5.22376 ..
 {right}(257.475,168.275) .. tension 1.77596 and 1 ..
(269.353,150.219) .. tension 1.22707 and 1.22707 ..
(291.413,124.656) .. tension 0.869847 and 0.869847 ..
brdmvi((321.628,103.986),(39.1991,-26.8843)) ..
brdmvi((334.795,98.324),(198.676,-81.1574)) ..
brdmvi((346.346,94.7908),(682.431,-138.171)) link
brdmvi((335.777,6.13983),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((335.098,6.09639),(660.44,-133.866)) ..
brdmvi((329.895,8.15366),(155.718,-61.9213)) ..
brdmvi((326.236,10.178),(16.3889,-8.86574)) ..
 {dir 149.825}(323.985,11.5106) {dir -103.249} .. tension 1.88922 and 1 ..
(274.281,-115.205) .. tension 1 and 1 ..
 {left}(-9.47417,-276.354) .. tension 1.46627 and 4.98242 ..
 {up} cycle
;

%%endpaths
%%beginbody
leftbearing := -180; % 230- (x4-x5);
enddefchar;
defchar(reh.fina.ii,-1,-1,-1,-1);
%%backgroundimage:images/reh.fina.jpg,-496.136,-1172.07,1,0,0,1
%%beginparams
rightVerticalRatio := 3;

%%beginverbatim
righttatweel := righttatweel - 0.5;



%%backgroundimage:images/reh.fina.jpg,-495,-1174,1,0,0,1
%%endverbatim

%%beginpaths

fill
(261.401,48.9676) .. tension 9.47073 and 0.792016 ..
 {up}(234.722,102.778) .. tension 1 and 5.22376 ..
 {right}(257.475,168.275) .. tension 1.77596 and 1 ..
(265.988,154.66) .. tension 1.22707 and 1.22707 ..
brdmvi((275.209,139.783),(43.1944,-52.2222)) .. tension 0.869847 and 0.869847 ..
brdmvi((290.043,126.516),(150.546,-133.228)) ..
brdmvi((309.217,115.148),(531.638,-259.778)) ..
brdmvi((329.27,110.021),(715.903,-275.764)) link
brdmvi((331.758,21.0093),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((310.632,22.3567),(705.903,-272.708)) ..
brdmvi((288.714,28.4807),(184.832,-141.909)) ..
brdmvi((273.812,37.6565),(33.8889,-38.588)) ..
 cycle
;
fill
(-241.826,-235.569) .. tension 1 and 1 ..
 {right}(-222.072,-218.642) .. tension 2.92944 and 1.23772 ..
 {right}(-112.705,-236.832) .. tension 1.20626 and 1 ..
(215.961,-101.237) .. tension 1 and 5.24099 ..
 {up}(300.007,29.4318) .. tension 1.7439 and 2.05309 ..
(326.638,22.6047) {dir -93.0271} .. tension 6.80679 and 1 ..
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
rightbearing := -xpart (urcorner currentpicture)  + x1;;
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
%%backgroundimage:images/waw.fina.jpg,-1495.4,-1061.09,1,0,0,1

%%beginverbatim



%%endverbatim

%%beginpaths

fill
(-205.665,-254.285) .. tension 1.31625 and 1.42228 ..
 {right}(-189.321,-236.986) .. tension 6.1768 and 1 ..
 {right}(-40.0502,-258.624) .. tension 1 and 1.74828 ..
 {up}(333.894,25.098) .. tension 1 and 2.88629 ..
 {left}(261.125,165.983) .. tension 2.31073 and 1 ..
 {down}(220.626,113.588) .. tension 1.29622 and 1 ..
(267.384,90.2883) link
(310.499,0.470318) .. tension 1 and 1.30089 ..
(216.667,21.9127) .. tension 1.03662 and 0.75 ..
 {up}(182.774,103.202) .. tension 1.11869 and 3.2826 ..
 {right}(263.562,258.209) .. tension 1.64466 and 1.39362 ..
 {down}(372.3,49.1898) .. tension 1.21074 and 1 ..
 {left}(46.7785,-295.139) .. tension 0.998102 and 5.7417 ..
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
leftVerticalRatio := 6;

%%beginverbatim

lefttatweel := lefttatweel - 0.5;

%%endverbatim

%%beginpaths

fill
(128.472,46.2854) .. tension 1 and 1 ..
bldmvi((91.7782,25.2051),(-39,-22)) ..
bldmvi((73.0177,11.9069),(-380,-113)) ..
bldmvi((49.5802,1.128),(-578,-128)) ..
bldmvi((38.6754,-0.99088),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((59.4107,87.5453),(-564,-129)) ..
bldmvi((66.3811,89.1765),(-356,-110)) ..
bldmvi((80.5749,97.3927),(-22,-13)) .. tension 1 and 1 ..
(87.2363,105.809) .. tension 1 and 1 ..
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
%%backgroundimage:images/heh.medi.2.jpg,-754.607,-1279.3,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
z.mark1 = (267,-262);
z3 = (142.888,-38.1838);
z4 = (375.112,-71.397);

%%beginverbatim

lefttatweel := lefttatweel -0.7;


%%endverbatim

%%beginpaths

fill
bldmvi((88.6618,5.06124),(-10.8964,-17.6433)) ..
bldmvi((79.3258,2.75407),(-152.305,-58.372)) ..
bldmvi((72.3014,1.43699),(-406.607,-110.857)) ..
bldmvi((69.4664,1.02274),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((101.191,89.5431),(-404.903,-110.408)) ..
bldmvi((118.502,90.2713),(-215.592,-73.8125)) ..
bldmvi((133.227,91.4585),(-68.1713,-29.0008)) .. tension atleast 1 ..
(154.639,95.0287) .. tension atleast 1 and atleast 1 ..
(169.922,101.445) .. tension 1 and 1.0862 ..
(177.853,108.511) .. tension 7.50675 and 1 ..
(212.521,225.248) .. tension 1 and 1.85792 ..
 {right}(299.556,348.988) .. tension 1.14769 and 1 ..
 {down}(345.226,263.084) .. tension 0.951806 and 1.05939 ..
(278.895,102.77) .. tension 1 and 4.07979 ..
 {down}(230.582,59.707) .. tension 11.6544 and 1 ..
(328.659,31.9123) .. tension 0.780826 and 1 ..
 {down}z4 .. tension 0.75 and 1.68374 ..
 {left}(302.964,-181.282) .. tension 0.754039 and 2.25827 ..
 {dir 94.1982}(168.9,-22.7806) {dir 162.525} .. tension 1.21186 and 2.01438 ..
(103.793,-16.1095) .. tension 1 and 3.29305 ..
 {dir -174.713}bldmvi((95.0684,6.5355),(7.00607,-13.3289)) .. tension 1.46451 and 1.62342 ..
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

%rightbearing := rightbearing - (x4 - x3);

rightanchor := (x3,0);

setMarkAnchor(buttonmark,z.mark1);


enddefchar;
defchar(heh.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/hehyeh.jpg,-1991.38,-1034.46,1,0,0,1
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
%%backgroundimage:images/tehmarbouta.fina.jpg,-26.088,-1329.31,1,0,0,1
%%beginparams
rightVerticalRatio := 3;

%%beginverbatim


righttatweel := righttatweel - 0.5;

%%endverbatim

%%beginpaths

fill
(93.2778,66.6667) .. tension 1 and 1 ..
(185.937,239.726) .. tension 1 and 8.57661 ..
 {up}(296.75,318.056) .. tension 0.910087 and 3.11944 ..
 {right}(316.759,448.754) .. tension 0.985899 and 1 ..
 {down}(325.222,435.417) .. tension 1 and 2.20226 ..
(374.993,190.638) .. tension 1 and 1 ..
(391.366,144.866) .. tension 1 and 1 ..
brdmvi((401.105,125.122),(119.444,-121.528)) ..
brdmvi((409.773,115.994),(481.25,-250.694)) ..
brdmvi((416.445,111.581),(736.806,-275)) link
brdmvi((407.927,21.7537),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((393.501,26.166),(710.417,-277.083)) ..
brdmvi((372.777,47.5362),(458.333,-258.333)) ..
brdmvi((357.382,80.697),(127.778,-142.361)) .. tension 1 and 1 ..
(342.635,132.075) .. tension 4.23371 and 4.23371 ..
 {left}(339.111,141.667) .. tension 6.79971 and 1 ..
(291.57,79.377) .. tension 1.13506 and 0.75 ..
 {left}(177.519,46.8004) .. tension 1 and 2.69002 ..
 {up} cycle
;
fill
(222.444,127.778) .. tension 1 and 1 ..
 {up}(323.833,183.333) .. tension 3.47055 and 12.31 ..
 {left}(303.694,263.194) .. tension 20.1423 and 3.43297 ..
 {down}(146.042,141.919) .. tension 1.68523 and 1 ..
 {right} cycle
;

%%endpaths
enddefchar;
defchar(heh.fina.afterlam,-1,-1,-1,-1);
%%backgroundimage:images/lamteh.jpg,-681.217,-1312.26,1,0,0,1

%%beginverbatim

%%endverbatim

%%beginpaths

fill
(19.6183,56.046) .. tension 1 and 1 ..
(175.934,255.629) .. tension 1 and 9.07057 ..
 {up}(206.693,267.408) .. tension 1.36767 and 3.43775 ..
 {right}(214.646,376.404) .. tension 1.89557 and 1.09646 ..
 {down}(223.591,359.692) .. tension 1.54435 and 2.20226 ..
(266.611,140.954) .. tension 1 and 2.28072 ..
(297.419,65.3981) link
(370.84,-55.4953) .. tension 1 and 1 ..
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


enddefchar;
defchar(sad.isol,1589,-1,-1,-1);
%%backgroundimage:images/dad.isol.2.jpg,-543.173,-1103.32,1,0,0,1
%%beginparams
z.mark1 = (859,-13);
z1 = (595.395,51.7908);

%%beginverbatim


z2 = z1 + joinvector;

%%endverbatim

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
 {left}z1 .. tension 1.22582 and 2.07478 ..
 {dir 156.648}(536.745,59.4309) {dir -74.5457} .. tension 1.5165 and 1 ..
(495.527,-117.214) .. tension 1.24446 and 1 ..
 {left}(254.044,-227.553) .. tension 0.868687 and 1 ..
 {up} cycle
;
fill
(801.342,295.03) {left} .. tension 1.29616 and 36.4946 ..
 {down}(599.076,143.824) .. tension 1.16714 and 1.59259 ..
 {right}z2 .. tension 0.847972 and 7.04096 ..
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
leftVerticalRatio := 6;
z.mark1 = (500,-99);
z1 = (362.11,-10.2158);

%%beginverbatim



lefttatweel := lefttatweel - 0.5;

z2 = z1 + joinvector;




%%endverbatim

%%beginpaths

fill
(152.58,71.4834) .. tension 1.24539 and 1.31659 ..
bldmvi((141.263,49.0927),(-41.7847,-23.0031)) ..
bldmvi((119.479,16.8596),(-253.027,-95.9629)) ..
bldmvi((100.903,1.63964),(-451.416,-119.651)) ..
bldmvi((93.2263,-1.15605),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((117.001,89.2032),(-431.527,-118.387)) ..
bldmvi((129.747,99.7447),(-242.023,-89.8421)) ..
bldmvi((149.145,126.48),(-88.6857,-46.1397)) .. tension atleast 1 ..
bldmvi((168.145,156.991),(18.5419,25.8308)) .. tension 1 and 2.59952 ..
 {right}bldmvi((187.943,177.375),(17.7775,21.0148)) .. tension 1 and 1 ..
 {down}bldmvi((190.645,167.236),(20.9776,22.587)) .. tension 1 and 1 ..
 {down}bldmvi((183.25,131.375),(7.67475,-3.72109)) .. tension 3.13519 and 1.85081 ..
 {right}(292.25,84.0495) .. tension 3.98412 and 1.23768 ..
 {right}(625.25,302.375) .. tension 1 and 1 ..
 {down}(736.564,234.532) .. tension 1 and 1 ..
(657.25,83.375) .. tension 1.72016 and 0.7597 ..
 {left}z1 .. tension 0.75 and 3.66495 ..
 {left}(256.599,3.33275) .. tension 2.58735 and 6.8879 ..
 {left}(142.915,-208.965) .. tension 1 and 1 ..
 {up}(137.642,-202.613) .. tension 2.77619 and 1 ..
(193.402,-34.4387) .. tension 1 and 13.08 ..
 {up}(210.304,15.7099) .. tension 7.71008 and 1 ..
(209.099,18.1294) .. tension 2.32236 and 1 ..
bldmvi((158.86,79.1541),(6.1398,-3.0699)) .. tension 1.1188 and 3.67109 ..
 {left}bldmvi((157.53,81.6643),(4.88393,-0.232568)) .. tension 13.1865 and 0.75 ..
 cycle
;
fill
(580.277,230.556) .. tension 1.56433 and 13.2272 ..
 {down}(359.518,81.7574) .. tension 3.49688 and 1 ..
 {right}z2 .. tension 1 and 5.06535 ..
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
z2 = (369.101,-9.55072);

%%beginverbatim

z3 = z2 + joinvector;

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
 {left}z2 .. tension 0.75 and 3.66495 ..
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
 {right}z3 .. tension 1 and 5.06535 ..
 {up}(685.929,178.26) .. tension 2.61501 and 1 ..
 {left} cycle
;

%%endpaths
%%beginbody
leftbearing := 0;
setCursiveAnchor(seenreh,z1 + (0,15), ExitAnchor);
enddefchar;
defchar(sad.init.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/sadinityeh.jpg,-405.061,-1201.48,1,0,0,1
%%beginparams
z11 = (383.895,170.607);

%%beginverbatim


z12 = z11 + joinvector;

y10 := y11;
x10 := x11 - 40;

%%endverbatim

%%beginpaths

fill
(318.273,254.195) .. tension 45.8591 and 1.4867 ..
 {right}(623.544,501.054) .. tension 1 and 2.91626 ..
 {down}(750.53,427.986) .. tension 1 and 1 ..
(672.422,259.951) .. tension 1.72016 and 0.889356 ..
z11 {left} .. tension 0.7597 and 0.7597 ..
 {left}z10 .. tension 0.75 and 3.66495 ..
 cycle
;
fill
(586.42,415.584) .. tension 1.56433 and 33.712 ..
 {down}(381.949,261.136) .. tension 5.15141 and 1.47632 ..
 {right}z12 .. tension 1 and 3.52061 ..
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

%%beginverbatim


%%endverbatim
%%beginbody


sad.init_(lefttatweel,0);

rightanchor := z1;

enddefchar;
defchar(sad.medi.beforereh,-1,-1,-1,-1);

%%beginverbatim


%%endverbatim
%%beginbody


sad.init.beforereh_(lefttatweel,0);

rightanchor := z2;

tr_ := tr_ shifted (0,10);

enddefchar;
defchar(sad.medi.beforeyeh,-1,-1,-1,-1);

%%beginverbatim


%%endverbatim
%%beginbody


sad.init.beforeyeh_(lefttatweel,0);

rightanchor := z11;

tr_ := tr_ shifted (0,-20);

enddefchar;
defchar(sad.fina,-1,-1,-1,-1);

%%beginverbatim


%%endverbatim
%%beginbody


sad.isol_(lefttatweel,righttatweel);

rightanchor := z1;

tr_ := tr_ shifted (0,-50);

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
%%backgroundimage:images/ain.init.jpg,-2863.19,-938.194,1,0,0,1
%%beginparams
leftVerticalRatio := 6;

%%beginverbatim

lefttatweel := lefttatweel - 0.3;

%%endverbatim

%%beginpaths

fill
(169.683,101.909) .. tension 1.24055 and 1 ..
 {up}(76.0093,240.508) .. tension 1.38527 and 1.11379 ..
 {right}(255.572,451.466) .. tension 0.762503 and 4.12173 ..
 {down}(404.642,376.116) .. tension 1.11617 and 1.50103 ..
 {left}(397.795,369.755) .. tension 4.45537 and 0.75 ..
 {left}(311.28,394.194) .. tension 0.875889 and 12.0146 ..
 {down}(125.726,304.335) .. tension 0.944053 and 1 ..
(222.033,178.904) .. tension 0.811407 and 1.07465 ..
 {right}(380.11,167.735) --
 cycle
;
fill
(424.785,214.603) .. tension 1 and 59.0798 ..
 {right}(505.162,264.497) .. tension 1 and 1 ..
 {down}(509.422,262.262) .. tension 5.85262 and 3.75754 ..
 {dir -130.966}(492.261,188.916) {dir -138.299} .. tension 1.5104 and 1 ..
(367.49,118.563) .. tension 1 and 0.75 ..
bldmvi((265.552,74.8948),(0,0)) ..
bldmvi((202.72,51.1341),(-398.4,-98.4)) ..
bldmvi((158.261,38.0717),(-696,-133.2)) ..
bldmvi((112.91,33.0026),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((161.038,122.246),(-670.8,-130.8)) ..
bldmvi((206.239,131.802),(-396,-99.6)) ..
bldmvi((278.328,154.245),(-105.6,-39.6)) .. tension atleast 1 ..
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

%%beginverbatim

%%endverbatim

%%beginpaths

fill
(145.589,25.9118) .. tension 1 and 1 ..
(-0.260751,-0.207962) link
(81.1195,88.7404) .. tension 1 and 1 ..
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
%%backgroundimage:images/ain.medi.jpg,-863.072,-1089.28,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 6;
z1 = (86.46,184.236);
z2 = (327.587,199.307);

%%beginverbatim
lefttatweel := lefttatweel - 0.5;
righttatweel := righttatweel - 0.2;
%%endverbatim

%%beginpaths

fill
bldmvi((225.631,81.0048),(8.2269,12.3241)) .. tension 1 and 1.04384 ..
bldmvi((162.291,42.2591),(-60.08,-16.5333)) ..
bldmvi((108.185,17.0175),(-274.167,-84.3333)) ..
bldmvi((58.0041,8.06353),(-571.389,-129.584)) ..
bldmvi((47.0024,8.33493),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((72.0983,96.7741),(-542.028,-129.222)) ..
bldmvi((101.069,104.019),(-246.759,-85.7547)) ..
bldmvi((136.032,117.276),(-53.0783,-25.7315)) .. tension 0.75 and 0.75 ..
bldmvi((174.065,141.768),(7.63257,6.11314)) .. tension 5.12775 and 5.12775 ..
(198.567,159.95) .. tension 1.46501 and 1.46501 ..
(235.49,197.632) .. tension 1.12253 and 2.94419 ..
 {right}(264.626,233.801) .. tension 1.57103 and 1.20848 ..
 {down}z2 .. tension 1.12253 and 1.12253 ..
(289.073,132.997) .. tension 1.12253 and 1.12253 ..
 cycle
;
fill
brdmvi((143.727,154.946),(3.38433,3.08851)) .. tension 1 and 1 ..
(114.591,179.213) .. tension 1 and 1 ..
 {left}z1 .. tension 6.14884 and 2.56634 ..
 {right}(155.449,240.833) .. tension 10000 and 3.53521 ..
brdmvi((174.986,224.602),(1.16284,0.697704)) .. tension 1 and 1 ..
brdmvi((238.442,148.54),(23.8242,2.45867)) .. tension 1 and 1 ..
brdmvi((257.901,124.913),(105,-29)) ..
brdmvi((268.9,106.897),(408.676,-112.532)) ..
brdmvi((279.203,96.7765),(659.787,-136.472)) link
brdmvi((264.937,7.76837),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((257.242,10.1049),(653.051,-135.051)) ..
brdmvi((247.203,17.1298),(400.204,-104.898)) ..
brdmvi((215.34,54.2027),(98,-22)) .. tension 1 and 1 ..
brdmvi((170.529,115.518),(16.9556,3.94613)) .. tension 1 and 1 ..
 cycle
;
fill
z1 .. tension 0.75 and 1.48739 ..
 {left}(48.9292,171.593) .. tension 1 and 1 ..
 {up}(39.5163,177.1) .. tension 1.52712 and 1.01546 ..
(100.842,264.281) .. tension 1.28267 and 0.75 ..
 {right}(199.598,286.092) .. tension 1 and 1 ..
(297.053,254.947) .. tension 1 and 1 ..
 {down}z2 .. tension 1.60809 and 1.60897 ..
 {left}(217.012,142.421) .. tension 1.59872 and 0.75 ..
 {left} cycle
;

%%endpaths
enddefchar;
defchar(ain.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/ainmediyeh.jpg,-518.899,-1444.23,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z3 = (-89.4178,-48.5602);

%%beginverbatim
righttatweel := righttatweel - 0.5;
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
 {down}(276.66,125.348) .. tension 2.63838 and 2.63838 ..
(277.615,122.641) .. tension 1 and 1 ..
brdmvi((279.555,118.064),(11.5741,-9.16281)) ..
brdmvi((286.522,106.899),(267.677,-99.4676)) ..
brdmvi((298.794,98.9167),(607.833,-134.167)) link
brdmvi((287.043,10.1525),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((276.39,11.7941),(591,-133.667)) ..
brdmvi((251.661,25.8643),(249.833,-92.6667)) ..
brdmvi((221.504,55.8267),(26,-10)) .. tension 1 and 1 ..
brdmvi((191.869,86.2653),(0,0)) .. tension 5.31108 and 5.31108 ..
 {left}(189.465,87.8841) .. tension 14.6587 and 0.984845 ..
 cycle
;

%%endpaths
%%beginbody
leftbearing := 0;

setCursiveAnchor(yehcursive1,z3,ExitAnchor);
enddefchar;
defchar(ain.fina,-1,-1,-1,-1);
%%backgroundimage:images/ain.fina.jpg,-82.971,-874.929,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim

righttatweel := righttatweel - 0.5;
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
 {down}(346.894,106.284) .. tension 1.62276 and 1.10373 ..
(347.943,105.181) .. tension 1 and 1 ..
brdmvi((355.408,101.274),(20.4216,-12.2434)) ..
brdmvi((361.715,99.0245),(303.466,-100.724)) ..
brdmvi((369.478,97.3585),(619,-134)) link
brdmvi((356.913,9.08183),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((346.729,9.54282),(605,-129)) ..
brdmvi((324.991,18.0455),(315,-97)) ..
brdmvi((290.031,58.5142),(13.7278,1.97166)) .. tension 1.23661 and 1 ..
(270.409,91.111) .. tension 1 and 5.79552 ..
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
%%backgroundimage:images/tah.isol.jpg,24.7912,-1020.41,1,0,0,1
%%beginparams
z.mark1 = (708,511);
z1 = (455.895,-35.664);

%%beginverbatim


z2 = z1 + joinvector;

%%endverbatim

%%beginpaths

fill
(64.2361,98.9583) .. tension 1 and 1 ..
 {right}(78.8323,106.835) .. tension 9.94627 and 0.75 ..
(245.547,53.8516) .. tension 1 and 1.21526 ..
 {right}(392.651,42.0686) .. tension 17.5873 and 1.90969 ..
 {right}(742.123,298.933) .. tension 0.761717 and 2.54062 ..
 {down}(869.948,222.539) .. tension 1.19075 and 1 ..
 {left}z1 .. tension 0.912587 and 3.82773 ..
 {up} cycle
;
fill
(702.104,215.495) .. tension 1.98843 and 45.6676 ..
 {down}(457.624,57.3841) .. tension 1.5517 and 1 ..
 {right}z2 .. tension 0.946512 and 12.4629 ..
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
leftVerticalRatio := 6;

%%beginverbatim

minLeft := 0;
maxLeft := 6;

lefttatweel := lefttatweel + 0.5;

%%endverbatim

%%beginpaths

fill
(335.648,50.926) .. tension 1 and 1 ..
bldmvi((143.507,7.08214),(17,-26)) ..
bldmvi((75.8651,-1.95228),(-223.8,-94.8)) ..
bldmvi((34.0113,-5.96323),(-553.2,-134.2)) ..
bldmvi((8.56655,-7.6299),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((50.2718,81.3949),(-523.2,-132)) ..
bldmvi((131.709,84.6895),(-252.6,-94.6)) ..
bldmvi((182.247,91.5898),(-29,-37)) .. tension 0.75 and 4.68442 ..
 {up}(476.852,209.491) .. tension 3.42809 and 1 ..
 {left}(376.157,261.574) .. tension 2.16079 and 1 ..
bldmvi((110.324,79.0231),(-63,-65)) --
bldmvi((34.4074,59.9213),(-86,-66)) .. tension 0.75 and bldmvi(1.8993,-0.2) ..
 {bldmvi(right,(0,0))}(410.686,338.333) .. tension 0.75 and 1.9428 ..
 {down}(514.995,277.778) .. tension 0.850852 and 1.0226 ..
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
 {down}(299.174,222.82) .. tension atleast 1 ..
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
rightVerticalRatio := 6;

%%beginverbatim
righttatweel := righttatweel - 0.5;
%%endverbatim

%%beginpaths

fill
(95.0979,11.2677) --
(273.295,61.619) .. tension atleast 1 ..
brdmvi((284.988,79.4869),(549.6,-124.8)) link
brdmvi((296.736,-9.24302),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((264.052,-9.7054),(522,-120)) ..
brdmvi((156.63,-3.08008),(165.6,-43.2)) .. tension atleast 1 and atleast 1 ..
 cycle
;

%%endpaths
%%beginbody

tah.init_(lefttatweel,righttatweel);


enddefchar;
defchar(tah.medi.beforeyeh,-1,-1,-1,-1);
%%backgroundimage:images/tahmedi_yeh.jpg,-529.389,-1278.62,1,0,0,1
%%beginparams
z1 = (8.73904,-61.7067);
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
(321.035,2.2203) --
(112.818,5.69327) --
(194.477,42.2406) .. tension 1 and 1 ..
(232.196,45.4779) link
(422.619,0.0681974) .. tension atleast 1 ..
 cycle
;
fill
z1 .. tension 1.381 and 1 ..
(272.049,64.3548) .. tension 1 and 3.60332 ..
 {up}(535.842,161.577) .. tension 3.42809 and 1 ..
 {left}(424.923,203.436) .. tension 2.28154 and 7.38854 ..
 {down}(229.514,58.1811) .. tension atleast 1 ..
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

%%beginverbatim


%%endverbatim
%%beginbody


tah.isol_(lefttatweel,righttatweel);

rightanchor := z1;

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
%%backgroundimage:images/kaf.init.jpg,-1598.6,-1157,1,0,0,1
%%beginparams
leftVerticalRatio := 6;

%%beginverbatim

lefttatweel := lefttatweel + 0.5;

%%endverbatim

%%beginpaths

fill
(215.996,81.3019) .. tension 1 and 1 ..
(187.411,53.4132) .. tension 1 and 1 ..
bldmvi((139.096,26.5809),(-16,-4)) ..
bldmvi((116.586,16.812),(-390,-99)) ..
bldmvi((97.4461,11.1719),(-624,-127)) ..
bldmvi((86.93,9.8357),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((115.411,99.9675),(-607,-130)) ..
bldmvi((134.655,105.745),(-387,-104)) ..
bldmvi((153.052,112.722),(-173,-59)) .. tension 1 and 1 ..
(176.296,122.292) .. tension 1 and 1 ..
(205.193,139.402) .. tension 1 and 1 ..
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
%%backgroundimage:images/kafinitlammedi.jpg,-2781.63,-1394.84,1,0,0,1
%%beginparams
z4 = (-68.521,386.816);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
(-84.4766,-12.9935) link
(5.22247,91.5814) .. tension 1 and 2.74067 ..
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
%%backgroundimage:images/kaflaf.jpg,-3555.8,-1434.67,1,0,0,1
%%beginparams
z4 = (-165.839,363.808);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
(-97.5312,-9.95532) link
(-18.9239,87.1512) .. tension 1 and 2.09088 ..
 {up}(51.185,148.433) .. tension 2.75215 and 1 ..
(-192.631,286.102) .. tension 1 and 0.91803 ..
 {up}(-245.086,333.201) .. tension 1.99864 and 1 ..
(-168.082,497.415) .. tension 2.55094 and 22.7178 ..
 {right}(338.642,860.748) .. tension 1.18385 and 4.7041 ..
 {dir -140.453}(324.403,787.459) .. tension 19.2158 and 1 ..
(-15.454,543.415) .. tension 1 and 3.17741 ..
 {down}(-197.284,379.119) .. tension 1.97377 and 1.50507 ..
z4 .. tension 6.25338 and 0.873858 ..
 {down}(80.7638,167.653) .. tension 0.75 and 1.04102 ..
 cycle
;

%%endpaths
enddefchar;
defchar(kaf.init.beforelamfina,-1,-1,-1,-1);
%%backgroundimage:images/kaflam.jpg,-2097,-1263.93,1,0,0,1
%%beginparams
z4 = (-25.7578,299.715);

%%beginverbatim


%%endverbatim

%%beginpaths

fill
(-87.2581,-10.8789) link
(-0.2581,95.8789) .. tension 1 and 2.74067 ..
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
%%backgroundimage:images/kaf.medi.ii.jpg,-1561.78,-1156,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
rightVerticalRatio := 6;

%%beginverbatim
lefttatweel := lefttatweel + 0.5;
righttatweel := righttatweel + 0.5;

%%endverbatim

%%beginpaths

fill
bldmvi((127.41,35.4612),(-11.1667,-28.3333)) ..
bldmvi((104.319,33.5713),(-207.488,-95.4363)) ..
bldmvi((82.4193,33.1698),(-487.313,-127.858)) ..
bldmvi((66.6809,32.6118),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((95.7029,121.114),(-469.917,-127.858)) ..
bldmvi((137.948,123.134),(-218.416,-98.383)) ..
bldmvi((165.606,128.249),(-82.8333,-55.8333)) .. tension 1 and 1.09115 ..
(174.57,133.511) .. tension 1.93215 and 0.875988 ..
 {up}(176.841,140.508) .. tension 1.52376 and 1 ..
(76.4256,320.054) .. tension 2.31552 and 6.39673 ..
 {up}(-93.6201,532.468) .. tension 1 and 1.12687 ..
(-49.1019,691.88) .. tension 1.72702 and 26.3603 ..
 {right}(509.63,1037.33) .. tension 1 and 1 ..
(515.736,1024.02) .. tension 1 and 3.23637 ..
(490.157,959.184) .. tension 63.8091 and 1 ..
(203.991,800.588) .. tension 0.75 and 3.14117 ..
 {down}(-65.4537,584.125) .. tension 15.896 and 0.75 ..
(100.08,443.027) .. tension 0.842115 and 0.842115 ..
(239.204,308.366) .. tension 1.21701 and 1.21701 ..
brdmvi((270.922,275.43),(172.2,-131.4)) ..
brdmvi((337.779,203.237),(373.2,-152.4)) ..
brdmvi((409.552,152.144),(600.4,-136.6)) link
brdmvi((409.947,61.1702),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((395.019,63.7379),(565.6,-128.4)) ..
brdmvi((368.391,81.2484),(285.6,-104)) ..
brdmvi((319.747,131.788),(60.6,-40)) .. tension 1.63995 and 1.63995 ..
(283.861,172.347) .. tension 1.63995 and 1.63995 ..
(229.023,232.815) .. tension 19.0656 and 19.0656 ..
 {left}(168.331,300.212) .. tension 26.5342 and 1 ..
(197.74,222.737) .. tension 1 and 1 ..
 {down}(206.164,151.91) .. tension 1 and 1 ..
(172.874,52.8383) .. tension 1 and 1 ..
 cycle
;

%%endpaths
enddefchar;
defchar(kaf.medi.beforemeem,-1,-1,-1,-1);
%%backgroundimage:images/kaf_meem.jpg,-1769.73,-1186.39,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z3 = (36.1214,46.0068);
z4 = (-23.6806,125.926);

%%beginverbatim


righttatweel := righttatweel;

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
brdmvi((229.985,304.884),(0,0)) ..
brdmvi((323.343,227.569),(0,0)) ..
brdmvi((382.331,170.167),(163.2,-87.6)) ..
brdmvi((441.514,111.389),(530.4,-122.4)) link
brdmvi((408.49,23.5663),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((432.739,23.196),(514.8,-116.4)) ..
brdmvi((408.512,50.325),(54,-18)) ..
brdmvi((278.479,177.146),(0,0)) ..
brdmvi((213.523,232.486),(0,0)) .. tension 1.63995 and 5.41674 ..
 {left}(130.405,290.441) .. tension 3.98975 and 1 ..
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
%%backgroundimage:images/kafmedi_yeh.jpg,-2640.3,-1230.52,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z1 = (415.704,69.9776);
z3 = (30.3344,-9.54877);
z4 = (46.9213,26.389);
z5 = (-115.611,485.825);

%%beginverbatim

righttatweel := righttatweel + 1;

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
 {down}(-86.9537,516.287) .. tension 5.64565 and 5.64565 ..
(3.88349,464.14) .. tension 1.43618 and 0.75 ..
brdmvi((234.894,319.09),(63.6,-51.6)) ..
brdmvi((422.762,157.758),(276.8,-103)) ..
brdmvi((481.083,122.391),(507.3,-118.833)) link
brdmvi((445.299,36.5247),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((470.902,34.2433),(476.4,-114.767)) ..
brdmvi((479.019,32.6242),(172.8,-51.6)) ..
brdmvi(z1,(33.5,1.66666)) .. tension 1.88763 and 1 ..
brdmvi((323.239,148.467),(10,-0.833333)) .. tension 1.63995 and 4.19024 ..
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
%%backgroundimage:images/kafmedialeffina.jpg,-2427.63,-1332.21,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim



kaf.init.beforelam_(lefttatweel,0);



picture pic_;
pic_ :=  currentpicture;
currentpicture:=nullpicture;

pair dir_;

dir_ := diratpoint z4 of (pathpart pic_);


%%endverbatim

%%beginpaths

fill
(-55.1316,332.244) .. tension 1 and 1 ..
z4 {dir_} .. tension 1 and 1 ..
(105.748,293.588) .. tension 1 and 1 ..
brdmvi((220.398,197.07),(0,0)) ..
brdmvi((289.41,144.873),(625,-140)) link
brdmvi((296.392,49.5736),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((260.516,62.6868),(594,-143)) ..
brdmvi((203.661,123.864),(0,0)) .. tension 1 and 1 ..
(97.1342,224.848) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

addto currentpicture also pic_;
enddefchar;
defchar(kaf.medi.beforelamalef,-1,-1,-1,-1);
%%backgroundimage:images/kafmedialeffina.jpg,-2189.58,-1410.64,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim

kaf.init.beforelamalef_(lefttatweel,0);
picture pic_;
pic_ :=  currentpicture;
currentpicture:=nullpicture;

pair dir_;

dir_ := diratpoint z4 of (pathpart pic_);
%%endverbatim

%%beginpaths

fill
(-159.813,307.794) .. tension 1 and 1 ..
z4 {dir_} .. tension 1 and 1 ..
(89.0781,239.399) .. tension 1 and 1 ..
brdmvi((195.72,152.521),(0,0)) ..
brdmvi((265.938,103.137),(625,-140)) link
brdmvi((272.92,7.83749),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((246.689,14.5206),(594,-143)) ..
brdmvi((180.591,70.0716),(0,0)) .. tension 1 and 1 ..
(72.8286,160.612) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

addto currentpicture also pic_;
enddefchar;
defchar(kaf.medi.beforelamfina,-1,-1,-1,-1);
%%backgroundimage:images/kafmedialeffina.jpg,-2189.58,-1410.64,1,0,0,1
%%beginparams
rightVerticalRatio := 6;

%%beginverbatim



kaf.init.beforelamfina_(charlt,0);
picture pic_;
pic_ :=  currentpicture;
currentpicture:=nullpicture;

pair dir_;

dir_ := diratpoint z4 of (pathpart pic_);
%%endverbatim

%%beginpaths

fill
(-81.688,301.042) .. tension 1 and 2.96912 ..
z4 {dir_} .. tension 1.23975 and 1 ..
(-7.07557,292.467) .. tension 1 and 1 ..
brdmvi((195.72,152.521),(0,0)) ..
brdmvi((265.938,103.137),(625,-140)) link
brdmvi((272.92,7.83749),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((246.689,14.5206),(594,-143)) ..
brdmvi((180.591,70.0716),(0,0)) .. tension 1 and 1 ..
(-57.862,244.524) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

addto currentpicture also pic_;
enddefchar;
defchar(kaf.fina,-1,-1,-1,-1);
%%backgroundimage:images/kkaf.fina.jpg,-285.556,-1171.22,1,0,0,1
%%beginparams
rightVerticalRatio := 3;
z1 = (659.491,56.122);

%%beginverbatim



%%endverbatim

%%beginpaths

fill
(27.6111,132.333) .. tension 0.951748 and 8.89073 ..
 {right}(97.2903,327.338) .. tension 1 and 1 ..
 {down}(103.616,316.673) .. tension 1 and 1.62263 ..
 {down}(66.6111,219.333) .. tension 1 and 0.914803 ..
 {right}(276.972,73.8055) .. tension 0.810168 and 3.22513 ..
 {up}(607.602,248.743) .. tension 5.82038 and 2.64594 ..
 {up}(523.305,743.194) .. tension 2.30568 and 4.5919 ..
 {right}(552.555,790.49) .. tension 21.8622 and 1.87371 ..
(611.347,477.49) .. tension 1 and 2.6439 ..
brdmvi((653.659,226.973),(22.2244,-8.60751)) .. tension 4.06984 and 1 ..
brdmvi((664.915,178.706),(63.1603,-43.5192)) .. tension 1 and 1 ..
brdmvi((673.35,149.931),(130.132,-96.3055)) ..
brdmvi((684.986,132.613),(310.394,-187.892)) ..
brdmvi((696.751,128),(592,-252)) link
brdmvi((686.617,39.3321),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((680.751,40.0298),(539,-240)) ..
brdmvi((669.233,46.057),(258,-158)) ..
brdmvi(z1,(102.667,-65.5)) .. tension 1.23661 and 1 ..
brdmvi((641.84,85.2528),(45.5,-11.8333)) .. tension 1 and 28.3572 ..
 {left}(617.472,197.333) .. tension 1.6623 and 1 ..
(613.484,189.061) .. tension 1.31527 and 1 ..
 {left}(250.046,-1.93057) .. tension 0.75 and 1.81337 ..
 {up} cycle
;
fill
(159.527,250.462) .. tension 1.6784 and 1.33888 ..
 {right}(182.353,259.906) .. tension 4.85985 and 0.75 ..
 {right}(257.618,252.568) .. tension 1.49002 and 2.49931 ..
 {up}(392.394,320.591) .. tension 2.06098 and 0.961189 ..
 {up}(278.547,378.674) .. tension 1 and 2.02954 ..
 {right}(392.048,565.515) .. tension 1 and 2.20322 ..
 {down}(426.562,534.937) .. tension 2.69063 and 4.2656 ..
 {left}(389.867,469.801) .. tension 0.85293 and 1 ..
 {left}(356.57,492.397) .. tension 2.46845 and 1.6639 ..
 {down}(321.61,445.025) .. tension 1.23183 and 1.17831 ..
 {down}(434.219,382.532) .. tension 1.16642 and 0.75 ..
 {left}(265.587,213.084) .. tension 1 and 2.03546 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(kaf.fina.afterlam,-1,-1,-1,-1);
%%backgroundimage:images/kkaf.fina.jpg,-286.459,-1148.77,1,0,0,1

%%beginverbatim

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
(610.347,445.99) .. tension 1 and 1 ..
(650.917,202.083) .. tension 1 and 1 ..
(741.78,55.0191) .. tension 4.06984 and 0.990895 ..
(769.933,44.6307) link
(785.673,-46.6401) .. tension 1.23661 and 0.764483 ..
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
defchar(kaf.init.ii,-1,-1,-1,-1);
%%backgroundimage:images/kaf.init.ii_2.jpg,-1659.17,-1035.83,1,0,0,1
%%beginparams
leftVerticalRatio := 12;

%%beginverbatim

%lefttatweel := lefttatweel - 1;

%%endverbatim

%%beginpaths

fill
brdmvi((942.98,61.5546),(824.567,89.718)) .. tension 9.24751 and 9.24751 ..
brdmvi((774.972,39),(818.333,86)) .. tension 1 and 1 ..
bldmvi((623.391,20.1042),(-100,-10)) ..
bldmvi((341.128,-0.193599),(-421.616,-44.544)) ..
bldmvi((203.894,-6.84483),(-823.624,-66.456)) ..
bldmvi((-54.3698,-12.7206),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((198.139,77.301),(-762.168,-66.64)) ..
bldmvi((395.307,82.0733),(-466.608,-46.632)) ..
bldmvi((578.12,92.1662),(-166,-19)) .. tension atleast 1 ..
brdmvi((689.667,100.027),(975.588,102.588)) .. tension atleast 1 and atleast 1 ..
brdmvi((957.038,141.333),(807.639,77.7778)) .. tension 1 and 1.62025 ..
 {up}brdmvi((968.63,148.399),(812.483,80.179)) .. tension 3.13619 and 1 ..
 {left}brdmvi((814.435,211.005),(888.169,83.0811)) .. tension 7.6699 and 0.940466 ..
 {left}(189.067,165.068) .. tension 2.52601 and 2.18255 ..
 {up}(-101.167,212.417) .. tension 1.47306 and 1.21903 ..
(-15.261,378.22) .. tension 1.59171 and 0.910654 ..
(106.016,468.591) .. tension 1.8592 and 10.0824 ..
 {right}(320.672,571.823) .. tension 1.99444 and 6.32225 ..
 {left}(291.548,490.54) .. tension 5.20633 and 1 ..
(90.6176,399.364) .. tension 1 and 2.98033 ..
 {down}(-51.685,275.375) .. tension 4.42215 and 1 ..
 {right}(306.146,237.178) .. tension 1.27328 and 4.14509 ..
 {right}brdmvi((860.763,288.666),(782.018,85.9749)) .. tension 1.40423 and 1.28104 ..
 {down}brdmvi((996.83,205.651),(nuqta * 7,96.25)) .. tension 1.22035 and 2.68873 ..
 cycle
;

%%endpaths
%%beginbody



enddefchar;
defchar(kaf.medi.ii,-1,-1,-1,-1);
%%backgroundimage:images/kaf.ii.jpg,-876.083,-986.778,1,0,0,1
%%beginparams
z.medi1 = (1055.19,66.7238);

%%beginverbatim

kaf.init.ii_(lefttatweel,righttatweel);
righttatweel_const := (100,lineheight - y.medi1 + ypart leftanchor);

picture pic_;

pic_ := currentpicture;

clearit;
%%endverbatim

%%beginpaths

fill
(1028.5,50.8747) .. tension 1 and 5.99967 ..
 {right}(1040.61,102.814) .. tension 1 and 1 ..
 {down}(1047.31,88.2608) .. tension 1 and 0.75 ..
z.medi1 link
(1137.17,-37.6581) .. tension 1.0135 and 1 ..
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
defchar(noonsinna,-1,0,-1,-1);
%%backgroundimage:images/noon.taweel.jpg,-1102.83,-1042.14,1,0,0,1
%%beginparams
z1 = (296.92,130.759);

%%beginpaths

fill
(219.977,272.33) .. tension 2.81433 and 3.82732 ..
 {right}(242.103,357.692) .. tension 6.53539 and 1 ..
(314.766,254.117) .. tension 1.09902 and 0.75 ..
 {down}(29.7766,29.1227) + z1 .. tension 4.77746 and 6.30243 ..
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
 {dir -156.111}(294.416,63.7506) {curl1} .. tension atleast 1 ..
 {up}z1 .. tension 1.30421 and 5.40337 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(expaform.i,-1,0,-1,-1);
%%backgroundimage:images/noon.taweel.jpg,-548.542,-1065.9,1,0,0,1
%%beginparams
ll := 21;
rr := 45;
ten := 1.22;
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
 {z12p}(-57.1759,-141.879) + z1 .. tension ten ..
z3 {z10p} .. tension 0.75 and 2.05837 ..
(96.2191,33.8006) .. tension 2.25106 and 4.7386 ..
 {up}(44.0254,104.838) .. tension 0.75 and 1.21338 ..
 {right}(50.9714,115.33) .. tension 13.4901 and 1 ..
(144.444,55.5556) .. tension 1 and 1.69565 ..
 {z10}z3p .. tension ten ..
 {z12}(-40.5093,-54.7497) + z1 .. tension 0.779442 and 2.27884 ..
 {up}z1 .. tension 2.08632 and 2.37393 ..
 cycle
;

%%endpaths
%%beginbody

setAnchor(dbal,z1 + (0,-300));

enddefchar;
defchar(expaform.ii,-1,-1,-1,-1);
%%backgroundimage:images/beh.isol.expa.jpg,-336.256,-941.48,1,0,0,1
%%beginparams
d1 := 169.44;
z33 = (-230.163,45.5019);
z34 = (863.733,83.4418);
z35 = (974.307,215.364);
z36 = (334.149,-7.3231);

%%beginverbatim

lefttatweel := lefttatweel -2;

z33p = z33 rotatepen 105;
z34p = z34 rotatepen 72;

z101 = (-lefttatweel*nuqta,0 );
z55 = (-lefttatweel*nuqta,0 );
z102 = (-(lefttatweel+2)*nuqta / 1.5, -(lefttatweel +2) *7 );

z10 = dir d1;
z10p = -z10;



z37 = z36 + z102;


%%endverbatim

%%beginpaths

fill
(-385.986,177.089) + z101 .. tension 1.10251 and 0.877686 ..
 {right}(-379.955,187.306) + z101 .. tension 3.69555 and 0.958307 ..
(-326.392,163.92) + z101 .. tension atleast 1 ..
z33p + z101 .. tension atleast 0.977039 ..
(382.566,70.8723) + z102 {right} .. tension atleast 1.28097 ..
z34p .. tension atleast 1 ..
 {curl1}z35 {down} .. tension atleast 1 ..
z34 .. tension atleast 1.13363 ..
z37 {left} .. tension atleast 1.32141 ..
z33 + z101 .. tension 1.84753 and 3.02182 ..
 {up} cycle
;

%%endpaths
%%beginbody

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
%%backgroundimage:images/feh.expa.min.jpg,-207.583,-1090.05,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z1 = (786.152,334);
z11 = (608.4,393.6);
z2 = (769.234,244.605);
z3 = (-156.205,127.403);
z33 = (-570.759,-15.0463);

%%beginverbatim
righttatweel := righttatweel -0.5;
minLeft := 0;


%lefttatweel :=  -x33 / nuqta;

maxLeft := 4.8011596638655467; % lefttatweel;

%%backgroundimage:images/feh.expa.min.jpg,-207.583,-1090.05,1,0,0,1
%%backgroundimage:images/feh.expa.max.jpg,-783.583,-990.69,1,0,0,1

lefttatweel := lefttatweel - 1;

%%endverbatim

%%beginpaths

fill
bldmvi(z3,z33) .. tension 1.14751 and 1.08746 ..
 {right}bldmvi((-154.112,139.614),(-572.338,-13.8889)) .. tension 2.58819 and 2.78036 ..
bldmvi((-103.254,116.027),(-565.394,-11.5741)) .. tension 1 and 1 ..
bldmvi((-24.3134,80.6926),(-568.866,-4.62963)) .. tension 1.49403 and 1.49403 ..
bldmvi((68.709,50.821),(-531.515,-12.9574)) .. tension 1 and 1 ..
 {right}bldmvi((281.328,28.0343),(-322.8,-32)) .. tension 1 and 1 ..
bldmvi((581.384,59.0278),(-49.2,-8.4)) .. tension 7.23208 and 7.23208 ..
bldmvi((693.641,88.2245),(0,0)) .. tension 1 and 2.8955 ..
 {up}(820.836,170.881) .. tension 2.05893 and 2.35809 ..
 {left}z2 .. tension 5.95701 and 6.62733 ..
 {right}z1 .. tension 1.95949 and 1 ..
 {down}(861.092,200.565) .. tension 1.18594 and 1.42885 ..
bldmvi((838.021,126.698),(0,0)) ..
bldmvi((710.593,23.4975),(-29.8,-6.2)) .. tension 1 and 1 ..
 {left}bldmvi((268.841,-56.6019),(-364.2,-28.8935)) .. tension 0.878883 and 0.878883 ..
bldmvi((86.6092,-36.8635),(-566.349,3.78703)) .. tension 0.878883 and 0.878883 ..
bldmvi((-54.9746,26.286),(-561.343,-3.47222)) .. tension 1.03219 and 1.03219 ..
bldmvi((-134.403,95.2235),(-567.13,-10.9954)) .. tension 0.943626 and 1.13578 ..
 cycle
;
fill
(653.335,115.747) .. tension 1 and 2.1057 ..
 {right}z1 .. tension 3.20057 and 6.45091 ..
 {left}z2 .. tension 1.7891 and 1.76858 ..
 {down}(687.667,159.433) .. tension 1.06071 and 1 ..
brdmvi((700.684,140.827),(5.55556,-6.25)) .. tension 1 and 1 ..
brdmvi((736.334,116.211),(142.018,-63.2916)) ..
brdmvi((773.088,106.186),(307.986,-99.264)) ..
brdmvi((804.493,100.098),(561.022,-125.111)) link
brdmvi((805.833,10.8333),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((785.278,13.7056),(545.245,-124.768)) ..
brdmvi((748.247,25.0173),(304.363,-98.3843)) ..
brdmvi((698.756,49.591),(6.57593,-4.09447)) .. tension 1 and 1 ..
 {up} cycle
;

%%endpaths
%%beginbody
drawcomp(one_dot,identity shifted z11);
show lefttatweel;
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
%%backgroundimage:images/kaf_fina_expa_3.jpg,-589.027,-1271.72,1,0,0,1
%%beginparams
d1 := -25;
rightVerticalRatio := 3;
t1 := 1.6;
t1.2 := 1.4;
z0.1 = (-83,244);
z33 = (-341.521,56.1133);
z34 = (550.903,174.065);

%%beginverbatim




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
(647.699,442.148) .. tension 1 and 1 ..
brdmvi((674.093,283.396),(3.37577,-0.482253)) .. tension 1 and 1 ..
brdmvi((688.464,210.932),(21.9136,-29.9383)) .. tension 1 and 1 ..
brdmvi((708.481,162.207),(38.2137,-48.399)) ..
brdmvi((723.881,139.167),(225,-175)) ..
brdmvi((740.45,127.159),(579.167,-254.861)) link
brdmvi((732.852,37.9941),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((722.925,40.2813),(561.111,-260.417)) ..
brdmvi((701.287,56.5415),(157.465,-144.715)) ..
brdmvi((686.726,75.1706),(37.1142,-56.3657)) .. tension 1.23661 and 1 ..
brdmvi((658.218,137.433),(12.3071,-11.7863)) .. tension 1.80671 and 1.80671 ..
(636.57,260.337) .. tension 1 and 1 ..
(609.568,423.418) .. tension 1 and 5.64446 ..
 {up} cycle
;
fill
(-493.15,166.338) + z101 .. tension 1.10251 and 1 ..
 {right}(-474.128,173.395) + z101 .. tension 3.69555 and 1 ..
 {z10}z33p + z101 .. tension 1.68741 and 1.05244 ..
 {z12}z34p + z.rtt.1 .. tension 1 ..
 {dir 50.116}(645.555,329.25) + z.rtt.1 {down} .. tension 0.807558 and 1.13298 ..
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
d1 := -25;
t1 := 1.6;
t1.2 := 1.4;
z.mark1 = (95.6353,661.814);
z0.1 = (8,196);
z33 = (-227.681,10.4333);
z34 = (630.743,81.2183);

%%beginverbatim


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





%%endverbatim

%%beginpaths

fill
(612.781,628.151) .. tension 2.30568 and 3.10736 ..
 {right}(644.663,696.183) .. tension 39.6725 and 1.87371 ..
(699.539,374.468) .. tension 1 and 4.06984 ..
(736.459,139.389) .. tension 0.8828 and 0.8828 ..
(753.736,64.3657) .. tension 1.01077 and 1.01077 ..
(784.843,-9.94907) .. tension 1.01077 and 1.01077 ..
(828.669,-46.7221) link
(840.822,-141.514) .. tension 1.22174 and 0.857527 ..
(722.692,-32.6271) .. tension 1.80671 and 1 ..
(653.769,355.044) .. tension 1 and 5.64446 ..
 {up} cycle
;
fill
(-376.477,128.991) + z101 .. tension 1.10251 and 1 ..
 {right}(-365.621,147.382) + z101 .. tension 3.69555 and 10000 ..
 {z10}z33p + z101 .. tension 1 and 2 ..
 {z12}z34p + z.rtt.1 .. tension 1.706 ..
 {dir 50.116}(704.062,208.237) + z.rtt.1 {down} .. tension 0.807558 and 1.13298 ..
 {z12p}z34 + z.rtt.1 .. tension 2 and 1 ..
 {z10p}z33 + z101 .. tension 2.32688 and 2.53062 ..
 {up} cycle
;

%%endpaths
%%beginbody

drawcomp(minikaf,identity shifted (z0.1 + z101/2));


setMarkAnchor(topmarks,z.mark1  + z101/2);
%showAnchors := 1;

enddefchar;
defchar(noon.isol.expa,-1,-1,-1,-1);
%%backgroundimage:images/noon.taweel.jpg,-12.828,-1062.14,1,0,0,1
%%beginparams
z11 = (398.824,200.666);
%%beginbody



noonshape.expa_(lefttatweel,0);

x11 := x11 + lefttatweel *  nuqta / 2;

draw one_dot.pic  shifted z11;

setCursiveAnchor(behnoon,(15,26.0528)+ z1, EntryAnchorRTL);
setCursiveAnchor(behmedinoon,(15,26.0528)+ z1, EntryAnchor);



enddefchar;
defchar(noon.fina.expa.afterbeh,-1,-1,-1,-1);
%%backgroundimage:images/noon.taweel.jpg,-12.828,-1062.14,1,0,0,1
%%beginparams
z11 = (398.824,200.666);
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
rightVerticalRatio := 6;
z2b = (598.433,28.4592);
z3b = (701.277,82.7086);

%%beginverbatim

z1b = z2b rotatepen 95;

righttatweel := righttatweel  + 0.0;

righttatweel_const := (x3b - x1b,y3b - y1b);

%%endverbatim

%%beginpaths

fill
brdmvi(z1b,(0,0)) ..
brdmvi((617.898,104.255),(222.5,-74)) ..
brdmvi((607.017,106.252),(516,-122.5)) link
brdmvi((555.522,20.236),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((590.701,18.7475),(499,-118.333)) ..
brdmvi((618.003,15.6602),(179.5,-54.3333)) ..
brdmvi(z2b,(0,0)) --
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

picture pic_;
pic_ := currentpicture;
clearit;

draw pic shifted disp;
draw pic_;

tr_ := tr_ shifted (0,-ypart disp);

setAnchor(basmala,z1);



%labels(1);

enddefchar;
defchar(noon.fina.basmala,-1,-1,-1,-1);
%%beginbody

noon.fina.expa_(lefttatweel,righttatweel);


picture q;
q := currentpicture;
a := 0;
currentpicture := nullpicture;



pair p_;

for item within q:
  if filled item :
	p := pathpart item;
  if a = 3 : 
    p_ = point 0 of p;
    fill p shifted ((-180,280) + z1 - p_); 
  else :
    if filled item : fill p; fi;    
  fi
  a := a + 1;
    
  fi;
 
endfor


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

setAnchor(dbal,(600,-450));

%showAnchors:=1;

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
z11 = point 31 of p;
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

draw pic shifted  (points1 - z11 + (30,45)) ;

currentpicture := currentpicture shifted (0,-100);

rightanchor := rightanchor + (points1 - z11 + (30,45)) + (0,-100);

tr_ := tr_ shifted (0,-90);

%currentpicture := currentpicture shifted - (points1 - z11 + (30,45)) ;

%draw pic; % shifted (points1 - z11 + (30,45));




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
defchar(behshape.fina.expa,-1,-1,-1,-1);
%%backgroundimage:images/beh.expa.max.jpg,-461.114,-1259.89,1,0,0,1
%%beginparams
rightVerticalRatio := 6;
z33 = (-352.421,-125.105);

%%beginverbatim
righttatweel := righttatweel -0.5;
lefttatweel := lefttatweel + 1;

%%backgroundimage:images/beh.expa.min.jpg,-140.057,-910.498,1,0,0,1

minLeft := 0;
maxLeft := 10;

x33 := - maxLeft *nuqta;

%%endverbatim

%%beginpaths

fill
bldmvi((191.89,207.3),z33) .. tension 0.75 and 2.91359 ..
 {right}bldmvi((194.688,217.664),(-1174.63,-131.964)) .. tension 2.22691 and 1 ..
bldmvi((251.466,183.985),(-1172.66,-121.429)) .. tension 1 and 1 ..
bldmvi((393.497,104.54),(-1108.21,-120.363)) .. tension 1 and 1 ..
bldmvi((551.6,58.099),(-1018.77,-138.832)) .. tension 1 and 1 ..
 {right}bldmvi((723.374,44.67),(-841.31,-155.725)) .. tension 1 and 1 ..
bldmvi((953.788,79.0947),(-364.07,-124.672)) .. tension 1 and 1 ..
bldmvi((1064.57,121.136),(-86.8416,-48.88)) .. tension 1 and 1 ..
bldmvi((1139.74,183.186),(-32.1492,-19.5254)) .. tension 1 and bldmvi(1.74281,2.29594) ..
 {right}bldmvi((1165.92,219.463),(-4.53318,12.7315)) .. tension 2.02492 and 1 ..
 {down}bldmvi((1171.28,199.103),(0,0)) .. tension 3.62578 and 1 ..
bldmvi((1166.22,171.315),(0,0)) .. tension 1 and 1 ..
bldmvi((1013.85,20.2547),(-15.3984,-10.0944)) .. tension 1 and 1 ..
bldmvi((834.914,-32.2755),(-262.483,-86.4813)) .. tension 1 and 1 ..
 {left}bldmvi((642.383,-38.1909),(-789.201,-143.267)) .. tension 1 and 1 ..
bldmvi((425.801,6.98403),(-1162.65,-86.104)) .. tension 1 and 1 ..
bldmvi((283.677,86.1593),(-1169.33,-101.616)) .. tension 1 and 1 ..
bldmvi((217.205,159.234),(-1169.3,-136.16)) .. tension 1 and 1.84475 ..
 {up} cycle
;
fill
brdmvi((1090.17,100.372),(-5.83333,-3.33333)) .. tension 1 and 1.95677 ..
 {right}(1120.94,128.113) .. tension 1.92486 and 1 ..
(1138.72,119.859) .. tension 1 and 1 ..
brdmvi((1152.87,111.104),(59.1667,-30.8333)) ..
brdmvi((1167.42,105.558),(217.5,-85)) ..
brdmvi((1199.5,97.7524),(591.287,-132.88)) link
brdmvi((1191.59,9.4811),nuqta * (7,-7 / rightVerticalRatio)) ..
brdmvi((1180.19,12.0704),(565,-130.833)) ..
brdmvi((1149.33,31.226),(201.667,-83.3333)) ..
brdmvi((1120.21,55.948),(50,-34.1667)) .. tension 1 and 1 ..
brdmvi((1094.97,87.7884),(-2.5,-7.5)) .. tension 1 and 1 ..
 cycle
;

%%endpaths
%%beginbody

show lefttatweel;
enddefchar;
defchar(twoseen,-1,-1,-1,-1);
%%backgroundimage:images/basmala_naml.jpeg,-5961.72,-1373.48,1,0,0,1
%%beginparams
z.twoseen.1 = (37.0185,93.9037);
z.twoseen.2 = (450.019,127.904);

%%beginverbatim




%%endverbatim

%%beginpaths

fill
(153.503,-6.0986) .. tension 0.996186 and 0.868663 ..
 {dir 135}(39,62) .. tension 1 and 0.980757 ..
 {dir 51.3402}z.twoseen.1 .. tension 0.988304 and 1.50036 ..
 {dir -29.7449}(67,101) .. tension 1 and 1.26685 ..
 {right}(163.333,64.5) .. tension 1 and 0.919731 ..
 {dir 37.9987}(273,106) .. tension 1 and 6.21871 ..
 {right}(331,192) .. tension 1 and 1 ..
 {down}(339,185) .. tension 1 and 1.618 ..
 {down}(319.376,115.589) .. tension 1.5565 and 3.09958 ..
 {right}(389,76) .. tension 1.684 and 1.00273 ..
 {dir 69.8637}(432.589,139) .. tension 1 and 3.92973 ..
 {right}(467.589,215.035) .. tension 1.57003 and 1 ..
 {down}(475.071,190.248) .. tension 3.60557 and 1.7612 ..
 {left}(367,-1) .. tension 0.75 and 4.68197 ..
 {left}(273,48) .. tension 5.08452 and 0.75 ..
 {left} cycle
;

%%endpaths
%%beginbody
leftbearing:=0;
enddefchar;
defchar(sad.init.kii,-1,-1,-1,-1);
%%backgroundimage:images/sad.init.jpg,-2302.75,-1465.38,1,0,0,1
%%beginparams
z.mark1 = (500,-99);
z1 = (144.461,77.3435);
z2 = (170.103,156.484);

%%beginverbatim







%%endverbatim

%%beginpaths

fillc
z1 kashida_iii
z2 {dir -313.018} .. tension atleast 1 ..
(182.712,172.701) .. tension 1 and 2.59952 ..
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
 {left}(156.25,84.375) .. tension 13.1865 and 1.32084 ..
 {dir -138.221} cycle
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
defchar(hah.init.iii,-1,-1,-1,-1);
%%backgroundimage:images/jeem.init.jpg,-1524.1,-1041.26,1,0,0,1
%%beginparams
z1 = (333.352,134.633);
z2 = (304.797,180.108);

%%beginverbatim


lefttatweel := lefttatweel ;

%%endverbatim

%%beginpaths

fillc
z1 kashida_iii
z2 .. tension 0.982496 and 3.1366 ..
(330.746,198.027) {up} .. tension 13.0439 and 1.63017 ..
 {left}(169.824,249.324) .. tension 0.93697 and 2.31909 ..
(77.7784,231.606) .. tension 1 and 2.50292 ..
 {left}(41.4548,203.233) .. tension 0.882523 and 1 ..
 {up}(37.3118,213.235) .. tension 1.45052 and 1.3939 ..
 {right}(177.975,344.714) .. tension 1.91368 and 0.841609 ..
(325.913,279.015) .. tension 1 and 2.30651 ..
 {right}(551.866,239.183) .. tension 1 and 2.3242 ..
 {right}(634.823,245.015) .. tension 1 and 1 ..
 {down}(639.731,240.065) .. tension 5.83816 and 4.05417 ..
 {left}(595.75,154.995) .. tension 1.26894 and 1 ..
 {left}(449.803,173.616) .. tension 1.15877 ..
 cycle
;

%%endpaths
enddefchar;
defchar(heh.fina.kii,-1,-1,-1,-1);
%%backgroundimage:images/tehmarbouta.fina.jpg,-29.2335,-1361.9,1,0,0,1

%%beginverbatim



%%endverbatim

%%beginpaths

fillc
(91.9445,101.667) .. tension 1 and 1 ..
(184.604,274.726) .. tension 1 and 8.57661 ..
 {up}(295.417,353.056) .. tension 0.910087 and 3.11944 ..
 {right}(315.426,483.754) .. tension 0.985899 and 1 ..
 {down}(323.889,470.417) .. tension 1 and 2.20226 ..
(366.259,238.286) .. tension 1 and 1.38434 ..
(400.867,162.373) kashida_ii
(347.091,128.347) .. tension 1 and 4.23371 ..
 {left}(330.544,175.863) .. tension 6.79971 and 1 ..
(290.237,114.377) .. tension 1.13506 and 0.75 ..
 {left}(176.186,81.8004) .. tension 1 and 2.69002 ..
 {up} cycle
;
fill
(221.111,162.778) .. tension 1 and 1 ..
 {up}(322.5,218.333) .. tension 3.47055 and 12.31 ..
 {left}(302.361,298.194) .. tension 20.1423 and 3.43297 ..
 {down}(144.709,176.919) .. tension 1.68523 and 1 ..
 {right} cycle
;

%%endpaths
enddefchar;
defchar(hah.init.kii,-1,-1,-1,-1);
%%backgroundimage:images/jeem.init.jpg,-1078.14,-1041.34,1,0,0,1
%%beginparams
z1 = (280.472,73.8188);
z2 = (289.964,148.302);

%%beginverbatim


lefttatweel := lefttatweel + 2 ;

%%endverbatim

%%beginpaths

fillc
z1 kashida_ii
z2 .. tension 0.982496 and 3.1366 ..
(342.302,180.11) {up} .. tension 13.0439 and 1.63017 ..
 {left}(181.38,231.407) .. tension 0.93697 and 2.31909 ..
(89.334,213.689) .. tension 1 and 2.50292 ..
 {left}(53.0104,185.316) .. tension 0.882523 and 1 ..
 {up}(48.8674,195.318) .. tension 1.45052 and 1.3939 ..
 {right}(189.531,326.797) .. tension 1.91368 and 0.841609 ..
(337.469,261.098) .. tension 1 and 2.30651 ..
 {right}(563.422,221.266) .. tension 1 and 2.3242 ..
 {right}(646.379,227.098) .. tension 1 and 1 ..
 {down}(651.287,222.148) .. tension 5.83816 and 4.05417 ..
 {left}(607.306,137.078) .. tension 1.26894 and 1 ..
 {left}(461.359,155.699) .. tension 1.15877 ..
 cycle
;

%%endpaths
%%beginbody
show rightPath_,_bottomTime,_upTime,finalPoint;
enddefchar;
defchar(dal.fina.kii,-1,-1,-1,-1);
%%backgroundimage:images/dal.fina.jpg,-774.703,-1077.49,1,0,0,1
%%beginparams
z1 = (453.802,89.4173);
z2 = (514.974,120.184);

%%beginverbatim

righttatweel := righttatweel ;
%%endverbatim

%%beginpaths

fillc
(89.1712,90.4503) .. tension 1 and 5.58004 ..
 {right}(101.526,141.887) .. tension 5.11224 and 1 ..
(129.713,125.285) .. tension 1 and 1 ..
 {right}(188.403,117.199) .. tension 1 and 2.45586 ..
 {up}(395,222.7) .. tension 10.598 and 1 ..
(356.773,337.074) .. tension 1 and 1 ..
 {up}(341.759,441.273) .. tension 1 and 4.56942 ..
 {right}(354.587,492.826) .. tension 10.1375 and 1 ..
(387.316,374.449) .. tension 1.94655 and 2.41325 ..
(427.8,256.855) .. tension 0.998234 and 1.01303 ..
z2 kashida_ii
z1 .. tension 0.928459 and 29.0369 ..
 {left}(407.761,186.549) .. tension 24.4711 and 1 ..
(383.194,129.638) .. tension 1 and 1.35948 ..
 {left}(169.267,29.2361) .. tension 0.885773 and 1 ..
 {up} cycle
;

%%endpaths
enddefchar;
defchar(alef.fina.kii,-1,-1,-1,-1);
%%backgroundimage:images/testheight.jpg,-12431.7,-1434.75,1,0,0,1
%%beginparams
defdeltas[0][5][0] := (-2.43396,0.757231);
defdeltas[0][5][1] := (-2.43396,0.757231);
defdeltas[0][5][2] := (-2.43396,0.757231);
rightdeltas[0][11][1] := (17.3611,0.694444);
rightdeltas[0][11][2] := (-8.50857,-3.21249);
rightdeltas[0][4][1] := (-2.7827,-0.422084);
rightdeltas[0][4][2] := (7.03286,-23.7778);
rightdeltas[0][5][0] := (11.132,-16.5886);
rightdeltas[0][5][1] := (16.5644,-25.575);
rightdeltas[0][5][2] := (10.3646,-8.3511);
z1 = (161.199,104.041);
z2 = (120.406,47.5289);

%%beginverbatim






%%endverbatim

%%beginpaths

fillc
(25.1151,668.464) .. tension 1.29322 and 10.6712 ..
 {right}(54.1787,728.424) .. tension 3.6286 and 1.61658 ..
(83.546,619.056) .. tension 3.28624 and 2.76364 ..
(119.896,296.456) .. tension 1.51205 and 1 ..
(142.57,152.512) .. tension 1 and 0.75 ..
 {dir -55.9357}z1 kashida_ii
z2 .. tension 2.70964 and 0.75 ..
(58.7528,498.117) .. tension 1 and 1.50214 ..
(33.6605,640.757) .. tension 1 and 1.57007 ..
 {up} cycle
;

%%endpaths
%%beginbody


currentpicture := calcDeltasPic(currentpicture);




enddefchar;
defchar(behshape.medi.expa.ii,-1,-1,-1,-1);
%%backgroundimage:images/expasinna.iii.jpg,-2341.64,-1179.69,1,0,0,1
%%beginparams
leftdeltas[0][0][2] := (-4.13765,-0.382446);
leftdeltas[0][10][0] := (38.5471,20.1809);
leftdeltas[0][10][1] := (42.4717,22.4388);
leftdeltas[0][10][2] := (25.9677,-4.71296);
leftdeltas[0][11][0] := (10.0828,-16.7593);
leftdeltas[0][11][1] := (10.9877,-16.3741);
leftdeltas[0][11][2] := (4.30162,-5.43969);
leftdeltas[0][12][1] := (4.93489,-6.05646);
leftdeltas[0][12][2] := (-0.158108,-0.136608);
leftdeltas[0][1][0] := (-70.1057,-21.9729);
leftdeltas[0][1][1] := (-59.7055,-21.3058);
leftdeltas[0][1][2] := (0.437888,99.8591);
leftdeltas[0][7][0] := (-42.6411,-16.1406);
leftdeltas[0][7][1] := (-62.6448,40.9848);
leftdeltas[0][7][2] := (-16.2049,-9.27543);
leftdeltas[0][8][0] := (12.1669,4.14551);
leftdeltas[0][8][1] := (-9.63611,-16.5591);
leftdeltas[0][8][2] := (32.3456,23.8065);
leftdeltas[0][9][0] := (44.045,31.6509);
leftdeltas[0][9][1] := (38.8528,31.587);
leftdeltas[0][9][2] := (48.879,31.7119);
z.mark1 = (105.509,331);
z1 = (225.912,98.639);
z2 = (262.804,159.207);
z3 = (202.651,101.4);
z4 = (191.183,177.742);

%%beginverbatim




%%endverbatim

%%beginpaths

fillc
(220.044,132.644) {dir -115.334} .. tension 0.837561 and 1 ..
z3 kashida_ii
z4 .. tension 0.932615 and 1 ..
(218.684,208.227) .. tension 1 and 1.18468 ..
 {right}(230.505,216.238) .. tension 1 and 1 ..
(240.142,208.544) .. tension 1 and 1.72888 ..
(245.803,195.953) .. tension 0.75 and 1 ..
z2 kashida_ii
z1 .. tension 1 ..
 {dir 95.2044} cycle
;

%%endpaths
%%beginbody
%labels(1,2,3,4);


currentpicture := calcDeltasPic(currentpicture);
show leftfactor;
show rightfactor;
%leftdeltas[7][0] := (-87.6969,-92.0045);
%leftdeltas[7][2] := (-194.982,100.626);

x.mark1 := x.mark1 - lefttatweel * nuqta * 0.1;

setAnchor(onedotup,z.mark1);

enddefchar;
defchar(behshape.init.kii,-1,-1,-1,-1);
%%backgroundimage:images/beh.init.jpg,-1375.87,-1296.63,1,0,0,1

%%beginverbatim

lefttatweel := lefttatweel +0;






%%endverbatim

%%beginpaths

fillc
(301.915,112.409) kashida_i
(267.261,143.76) .. tension 1 and 1.58967 ..
 {up}(274.915,153.085) .. tension 0.9544 and 11.9251 ..
(194.346,253.587) .. tension 5.61953 and 5.94319 ..
 {right}(211.539,337.069) .. tension 5.85287 and 1 ..
(277.778,282.208) .. tension 1.13818 and 1.06645 ..
 {down}(309.595,172.8) .. tension atleast 1 ..
(306.217,129.985) .. tension atleast 1 ..
 cycle
;

%%endpaths
%%beginbody


currentpicture := calcDeltasPic(currentpicture);



enddefchar;
defchar(behshape.init.bseen.basmala,-1,-1,-1,-1);
%%backgroundimage:images/basmala.4.jpg,-10576.9,-993.264,1,0,0,1
%%beginparams
z.mark1 = (139.669,104.202);
z1 = (280.109,67.9173);

%%beginverbatim

lefttatweel := lefttatweel +0;






%%endverbatim

%%beginpaths

fillc
(119.644,32.7569) {dir -206.913} .. tension 3.35106 and 1 ..
 {dir 45}z.mark1 {dir -22.584} .. tension 0.852509 and 1.2775 ..
 {right}z1 .. tension 1.18171 and 1.51368 ..
 {up}(302.6,80.0391) .. tension 1.58647 and 1 ..
(260.861,194.732) .. tension 0.762144 and 3.08027 ..
 {dir 133.393}(181.809,305.266) {dir -145.534} .. tension 0.75 and 1.43213 ..
 {left}(171.623,297.856) .. tension 3.63053 and 11.9251 ..
(116.885,365.049) .. tension 9.06204 and 5.94319 ..
 {right}(135.629,471.865) .. tension 5.85287 and 1 ..
(234.292,359.757) .. tension 1.12129 and 0.88406 ..
 {down}(333.911,98.0731) .. tension 8.94193 and 0.75 ..
 {left}(276.522,-20.1989) .. tension atleast 1 ..
 cycle
;

%%endpaths
%%beginbody

setCursiveAnchor(behshapeseen,z.mark1 ,ExitAnchorRTL);



enddefchar;
defchar(seen.medi.basmala,-1,-1,-1,-1);
%%backgroundimage:images/basmala.4.jpg,-10073.9,-1121.49,1,0,0,1
%%beginparams
leftVerticalRatio := 9.35;
z.mark1 = (642.169,232.349);

%%beginverbatim






%%endverbatim

%%beginpaths

fill
(286.222,55.2094) .. tension 1.11405 and 0.959001 ..
(191.092,120.762) .. tension 1 and 7.10621 ..
 {left}(184.889,141.62) .. tension 17.8548 and 1 ..
bldmvi((170.445,132.124),(-23.1288,-5.2064)) ..
bldmvi((126.898,111.872),(-181.081,-37.8486)) ..
bldmvi((103.298,106.19),(-690.124,-86.5863)) ..
bldmvi((87.4313,105.46),nuqta * (-7,-7 / leftVerticalRatio)) link
bldmvi((130.216,193.855),(-696.121,-87.6473)) ..
bldmvi((169.408,199.662),(-190.504,-40.267)) ..
bldmvi((234.338,227.975),(-12.6959,-4.27021)) .. tension 7.93898 and bldmvi(1.62963,0.75) ..
 {right}(238.493,228.761) .. tension 1 and 1 ..
 {down}(249.136,220.242) .. tension 1.23162 and 2.42392 ..
 {down}(230.318,178.544) .. tension 1.62638 and 1 ..
 {right}(303.362,139.541) .. tension 1 and 1 ..
(384.295,150.425) .. tension 0.75 and 1.71558 ..
(461.619,197.014) .. tension 1.66726 and 1 ..
(480.3,224.103) .. tension 1 and 1.44043 ..
 {right}(513.724,267.826) .. tension 0.75 and 1 ..
 {down}(518.853,256.329) .. tension 1.47574 and 2.57963 ..
 {down}(493.039,183.788) .. tension 5.9891 and 1.57094 ..
 {right}(589.541,152.441) .. tension 5.74865 and 1 ..
(614.39,197.724) .. tension 0.75 and 1.85118 ..
 {right}(659.595,276.456) .. tension 3.33023 and 1 ..
(631.936,169.619) .. tension 1 and 2.65892 ..
 {left}(568.625,62.3112) .. tension 1.76604 and 1.11477 ..
(471.648,89.8643) .. tension 1.14001 and 3.16224 ..
 {left}(443.041,107.366) .. tension 5.7691 and 1 ..
(403.514,81.2515) .. tension 1.64816 and 0.75 ..
 {left} cycle
;

%%endpaths
%%beginbody

setCursiveAnchor(behshapeseen,z.mark1 ,EntryAnchorRTL);
show rightanchor;
show leftanchor;
%showAnchors:=1;
which_in_baseline := 3;

enddefchar;
defchar(kashida.param,-1,-1,-1,-1);
%%backgroundimage:images/basmala.4.jpg,-9890.51,-1000.91,1,0,0,1
%%beginparams
leftMinHeight := 42;
leftMinWidth := 200;
leftVerticalRatio := 10;
z.p = (229.8,-19.2584);
z.q = (230.497,50.3789);
z_10 := (458.425,179.263);
z_11 := (203.813,116.684);

%%beginverbatim
%%beginverbatim

%lefttatweel := lefttatweel + 13;


numeric width_, height_, tatweel_;

tatweel_ := lefttatweel*nuqta;

width_ := leftMinWidth + tatweel_;
height_ := leftMinHeight + tatweel_ / leftVerticalRatio;

pair leftFirstRatio, leftLastRatio;

%leftFirstRatio := (2,1.33);
%leftLastRatio := (1.2,1.04);
leftFirstRatio := z_10;
leftLastRatio := z_11;



z1 = z.p + (-width_/(xpart leftFirstRatio/100),-height_/(ypart leftFirstRatio/100));
z2 = z.p + (-width_/(xpart leftLastRatio/100),-height_/(ypart leftLastRatio/100));
z3 = z.p + (-width_,-height_) ;
z4 = z2 + joinvector;
z5 =  z1 + joinvector;

z.dir1 = (z1 - z.p) rotated 5;
z.dir2 = (z.q - z5) rotated 5;


%%endverbatim

%%beginpaths

fillc
z.p {z.dir1} .. tension atleast 1 ..
(0,0) + z1 .. tension atleast 1 ..
z2 .. tension atleast 1 ..
z3 link
z4 .. tension atleast 1 ..
z5 .. tension atleast 1 ..
 {z.dir2}z.q {dir -28.4411} .. tension atleast 1 ..
 {dir -169.108} cycle
;

%%endpaths
%%beginbody





show tatweel_;
show x_0;
enddefchar;
defchar(behshape.medi.basmala,-1,-1,-1,-1);
%%backgroundimage:images/basmala.4.jpg,-2859.82,-940.951,1,0,0,1
%%beginparams
leftVerticalRatio := 6;
z.mark1 = (170.509,-122);
z1 = (172,131);

%%beginverbatim

%%endverbatim

%%beginpaths

fill
(195.742,156.111) .. tension 1.74567 and 1 ..
(147.893,69.9612) .. tension 1 and 1 ..
bldm((103.204,34.9645),(-48.8057,-29.6825)) ..
bldm((60.5803,13.8041),(-805.69,-278.19)) ..
bldm((-12.9554,-4.75253),(-1782.72,-393.12)) ..
bldm((-33.7049,-6.08523),nuqta * (-21,-21 / leftVerticalRatio)) link
bldm((3.31284,83.6499),(-1848.96,-396)) ..
bldm((78.9243,106.392),(-856.691,-285.338)) ..
bldm((126.541,129.963),(-284.709,-137.548)) .. tension 1 and 1 ..
(161.692,152.362) .. tension 1 and 2.6577 ..
 {right}(187.455,168.197) .. tension 1 and 1 ..
 {down} cycle
;

%%endpaths
%%beginbody
setCursiveAnchor(hahbehshape,z1,EntryAnchorRTL);
setAnchor(twodotsdown,z.mark1);
%showAnchors:=1;
enddefchar;
defchar(meem.fina.basmala,-1,-1,-1,-1);
%%backgroundimage:images/basmala.4.jpg,-7058.63,-1179.17,1,0,0,1
%%beginparams
rightAngleRot := 5;
rightAngleRotRatio := 10000;
rightFirstRatio := (202.4,191.8);
rightLastRatio := (122.4,164.8);
rightMinHeight := 31;
rightMinWidth := 200;
rightVerticalRatio := 9.4;
z.mark1 = (297.4,-158.2);
z1 = (412.035,54.5316);
z2 = (417.006,143.806);

%%beginverbatim

righttatweel := righttatweel - 0.0;



%%endverbatim

%%beginpaths

fill
(168.086,66.0635) .. tension 2.23132 and 0.75 ..
 {up}(156.499,121.939) .. tension 1.13344 and 2.93404 ..
 {right}(262.357,259.921) .. tension 1.5746 and 1 ..
(332.529,203.798) .. tension 0.970458 and 1.4554 ..
brdm((361.497,178.875),(21.614,-11.1083)) .. tension 1.41875 and 1 ..
brdm(z2,(221.729,-59.626)) ..
brdm((445.138,130.613),(1630.89,-253.703)) ..
brdm((468.553,124.142),(2361.75,-267.78)) link
brdm((462.987,34.8056),nuqta * (21,-21 / rightVerticalRatio)) ..
brdm((455.021,35.6325),(2302.2,-267.963)) ..
brdm((445.435,39.018),(1585.55,-247.826)) ..
brdm(z1,(59.9477,-19.4696)) .. tension 1 and 1 ..
brdm((401.848,60.522),(0.547765,-0.156504)) .. tension 2.67114 and 2.67114 ..
 {left}(400.807,61.1237) .. tension 27.6548 and 2.68826 ..
 {left}(384.203,0.7577) .. tension 1.58419 and 4.56919 ..
 {dir 117.425}(190.297,46.3734) {dir -102.461} .. tension 0.75 and 5.43489 ..
(131.465,-44.274) .. tension 1.17645 and 1.72332 ..
 {left}(-91.6088,-131.325) .. tension 1.8144 and 1 ..
 {up}(-98.024,-129.035) .. tension 8.27514 and 1 ..
(83.8981,-12.5363) .. tension 1.73988 and 6.20895 ..
 {up} cycle
;
fill
(257.818,206.334) .. tension 2.06872 and 2.35875 ..
 {down}(187.806,148.86) .. tension 6.28125 and 31.1391 ..
 {up}(335.974,99.3684) .. tension 141.37 and 1 ..
(292.957,153.554) .. tension 0.811254 and 1.77235 ..
 {left} cycle
;

%%endpaths
%%beginbody
%%beginbody
leftbearing :=- 50;

setMarkAnchor(buttonmark,z.mark1);

enddefchar;
defchar(hah.init.basmala,-1,-1,-1,-1);
%%backgroundimage:images/basmala.4.jpg,-3049.4,-958.334,1,0,0,1
%%beginparams
z1 = (-20.0402,147.686);

%%beginverbatim


%%endverbatim

%%beginpaths

fill
(-37.0604,100.575) {dir -252.1} .. tension 1.74486 and 1.85096 ..
 {dir 45}z1 {dir -45} .. tension 5.68827 and 0.75 ..
 {right}(67.7083,114.005) .. tension 1 and 36.9116 ..
 {dir 25.8348}(382.46,201.475) {dir -242.953} .. tension 12.5779 and 1.65326 ..
 {left}(180.454,255.94) .. tension 0.75 and 1.21121 ..
(77.9196,229.406) .. tension 1 and 6.40635 ..
 {left}(45.583,208.46) .. tension 1 and 1 ..
 {up}(38.3177,218.749) .. tension 4.72612 and 0.750683 ..
 {right}(182.379,354.361) .. tension 1.0997 and 1 ..
(336.186,297.964) .. tension 1 and 1.73076 ..
 {right}(632.687,243.357) .. tension 1 and 5.56899 ..
 {down}(702.364,241.338) .. tension 3.90761 and 17.8421 ..
 {left}(658.858,162.778) .. tension 3.18506 and 0.75 ..
 {left}(581.221,173.437) .. tension 2.41416 and 0.75 ..
(513.068,169.494) .. tension 1.05124 and 3.64317 ..
 {left}(109.532,33.0764) .. tension 1.33992 and 5.87146 ..
 {up} cycle
;

%%endpaths
%%beginbody

setCursiveAnchor(hahbehshape,z1,ExitAnchorRTL);
enddefchar;
