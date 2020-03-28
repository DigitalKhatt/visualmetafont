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
#include "QByteArrayOperator.h"

QByteArray &operator<<(QByteArray &l, quint8 r)
{
	l.append(r);
	return l;
}

QByteArray &operator<<(QByteArray &l, qint8 r)
{
	l.append(r);
	return l;
}

QByteArray &operator<<(QByteArray &l, quint16 r)
{
	return l << quint8(r >> 8) << quint8(r);
}

QByteArray &operator<<(QByteArray &l, qint16 r)
{
	return l << qint8(r >> 8) << qint8(r);
}

QByteArray &operator<<(QByteArray &l, quint32 r)
{
	return l << quint16(r >> 16) << quint16(r);
}

QByteArray &operator<<(QByteArray &l, qint32 r)
{
	return l << qint16(r >> 16) << qint16(r);
}


