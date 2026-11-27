/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#include <QByteArray>

QByteArray &operator<<(QByteArray &l, quint8 r);

QByteArray &operator<<(QByteArray &l, qint8 r);

QByteArray &operator<<(QByteArray &l, quint16 r);

QByteArray &operator<<(QByteArray &l, qint16 r);

QByteArray &operator<<(QByteArray &l, quint32 r);

QByteArray &operator<<(QByteArray &l, qint32 r);

QByteArray &operator<<(QByteArray &l, qint64 r);

QByteArray &operator<<(QByteArray &l, quint64 r);

template<typename T>
QByteArray& operator<<(QByteArray& s, const QVector<T>& v)
{
	for (typename QVector<T>::const_iterator it = v.begin(); it != v.end(); ++it)
		s << *it;
	return s;
}

template<typename T>
QByteArray& operator<<(QByteArray& s, const QList<T>& v)
{
	for (typename QList<T>::const_iterator it = v.begin(); it != v.end(); ++it)
		s << *it;
	return s;
}

