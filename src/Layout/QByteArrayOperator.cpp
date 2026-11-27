/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
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

QByteArray &operator<<(QByteArray &l, qint64 r){
  return l << qint32(r >> 32) << qint32(r);
}

QByteArray &operator<<(QByteArray &l, quint64 r){
  return l << quint32(r >> 32) << quint32(r);
}


