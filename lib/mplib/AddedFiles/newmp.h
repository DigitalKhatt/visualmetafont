/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#pragma once

typedef struct MP_instance*MP;
//void mymplib_shipout_backend(MP mp, void*voidh);

typedef struct mp_edge_object mp_edge_object;

typedef struct Transform {
	double xpart;
	double ypart;
} Transform;

void setAnchors(MP mp, mp_edge_object* hh);

void mp_gr_toss_objects_extended(mp_edge_object* hh);

//AnchorPoint getAnchor(MP mp, int charcode, int anchorIndex);
//unsigned int getTotalAnchors(MP mp, int charcode);
//Transform getMatrix(MP mp, int charcode);

void getPointParam(MP mp, int index,double*x, double*y);

