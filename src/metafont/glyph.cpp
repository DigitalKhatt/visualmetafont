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

#include "qstringbuilder.h"

#include "glyph.hpp"
#include "font.hpp"
#include "commands.h"

#include "GlyphParser/glyphdriver.h"
#include "qcoreevent.h"
#include  <cmath>

#include "metafont.h"



Glyph::Glyph(QString code, MP mp, Font * parent) : QObject((QObject*)parent) {
	edge = NULL;
	this->mp = mp;
	this->font = parent;
	m_unicode = -1;
	m_charcode = -1;
	m_lefttatweel = 0;
	m_righttatweel = 0;

	this->setSource(code);

	m_undoStack = new QUndoStack(this);
}

Glyph::~Glyph() {

}
void  Glyph::setBeginMacroName(QString macro) {
	m_beginmacroname = macro;
}
QString  Glyph::beginMacroName() {
	return m_beginmacroname;
}

void Glyph::setSource(QString source, bool structureChanged) {

	isSetSource = true;
	blockSignals(true);
	isDirty = true;

	ldirections.clear();
	rdirections.clear();
	ltensions.clear();
	rtensions.clear();
	params.clear();

	QList<QByteArray> dynamicProperties = dynamicPropertyNames();
	for (int i = 0; i < dynamicProperties.length(); i++) {
		QByteArray propname = dynamicProperties[i];
		QVariant variant;
		setProperty(propname, variant);
	}

	QHash<Glyph*, ComponentInfo>::iterator comp;
	for (comp = m_components.begin(); comp != m_components.end(); ++comp) {
		Glyph* glyph = comp.key();
		disconnect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);
	}

	m_components.clear();
	controlledPaths.clear();

	m_body = "";
	m_verbatim = "";

	glyphparser::Driver driver(*this);

	driver.parse_string(source.toStdString(),name().toStdString());

	edge = NULL;
	isDirty = true;

	m_source = this->source();

	isSetSource = false;

	blockSignals(false);

	//auto gg = receivers(SIGNAL(valueChanged(QString)));

	emit valueChanged("source", structureChanged);
}
QString Glyph::source() {

	if (isDirty) {
		edge = NULL;
		QString source;

		//source = QString("%1(%2,%3,%4,%5,%6,%7,%8);\n").arg(beginMacroName()).arg(name()).arg(unicode()).arg(width()).arg(height()).arg(depth()).arg(leftTatweel()).arg(rightTatweel());
		source = QString("%1(%2,%3,%4,%5,%6);\n").arg(beginMacroName()).arg(name()).arg(unicode()).arg(width()).arg(height()).arg(depth());
		//source = source % QString("%%glyphname:%1\n").arg(name());
		if (!image().path.isEmpty()) {
			Glyph::ImageInfo imageInfo = image();
			QTransform transform = imageInfo.transform;
			source = source % QString("%%backgroundimage:%1,%2,%3,%4,%5,%6,%7\n")
				.arg(imageInfo.path)
				.arg(imageInfo.pos.x())
				.arg(imageInfo.pos.y())
				.arg(transform.m11())
				.arg(transform.m12())
				.arg(transform.m21())
				.arg(transform.m22());
		}

		if (!m_components.isEmpty()) {
			source = source % "%%begincomponents\n";
			QHash<Glyph*, ComponentInfo>::iterator i;
			for (i = m_components.begin(); i != m_components.end(); ++i) {
				ComponentInfo component = i.value();
				QTransform transform = component.transform;
				source = source % QString("drawcomponent(%1,%2,%3,%4,%5,%6,%7);\n").arg(i.key()->name())
					.arg(component.pos.x())
					.arg(component.pos.y())
					.arg(transform.m11())
					.arg(transform.m12())
					.arg(transform.m21())
					.arg(transform.m22());
			}
			source = source % "%%endcomponents\n";
		}

		if (params.count() > 0) {
			source = source % "%%beginparams\n";
			QPointF valpoint;
			QString typepoint;
			double valdouble;
			QMapIterator<QString, Param> i(params);
			while (i.hasNext()) {
				i.next();
				Param param = i.value();
				QString propname = param.name;
				QString affect = param.isEquation ? "=" : ":=";
				switch (param.type) {
				case point:
					valpoint = property(propname.toLatin1()).toPointF();
					source = source % QString("%1 %2 (%3,%4);\n").arg(QString(propname)).arg(affect).arg(valpoint.x()).arg(valpoint.y());
					break;
				case direction:
					valdouble = property(propname.toLatin1()).toDouble();
					typepoint = param.position == left ? "ldir" : "rdir";
					source = source % QString("%1 %2 %3; % %4 (%5,%6)\n").arg(QString(propname)).arg(affect).arg(valdouble).arg(typepoint).arg(param.applytosubpath).arg(param.applytopoint);
					break;
				case tension:
					valdouble = property(propname.toLatin1()).toDouble();
					typepoint = param.position == left ? "ltens" : "rtens";
					source = source % QString("%1 %2 %3; % %4 (%5,%6)\n").arg(QString(propname)).arg(affect).arg(valdouble).arg(typepoint).arg(param.applytosubpath).arg(param.applytopoint);
					break;
				case control:
					valpoint = property(propname.toLatin1()).toPointF();
					typepoint = param.position == left ? "lcont" : "rcont";
					source = source % QString("%1 %2 (%3,%4); % %5 (%6,%7)\n").arg(QString(propname)).arg(affect).arg(valpoint.x()).arg(valpoint.y()).arg(typepoint).arg(param.applytosubpath).arg(param.applytopoint);
					break;
				case numeric:
					valdouble = property(propname.toLatin1()).toDouble();
					typepoint = param.position == left ? "lcont" : "rcont";
					source = source % QString("%1 %2 %3; % numeric\n").arg(QString(propname)).arg(affect).arg(valdouble);
					break;
				}
			}
		}

		if (verbatim() != "") {
			source = source % "\n%%beginverbatim\n";
			source = source % verbatim();
			source = source % "%%endverbatim\n";
		}

		if (!controlledPaths.isEmpty()) {
			source = source % "\n%%beginpaths\n\n";
			QMapIterator<int, QMap<int, Glyph::Knot*> > j(controlledPaths);
			while (j.hasNext()) {
				j.next();
				QMap<int, Knot*> path = j.value();
				const Knot * firstpoint = path.first();
				const Knot * previouspoint = firstpoint;
				if (controlledPathNames[j.key()] == "fill") {
					source = source % QString("fill\n");
				}
				else {
					source = source % QString("controlledPath (%1,%2)(%3)(\n").arg(j.key()).arg(path.firstKey()).arg(controlledPathNames[j.key()]);
				}
				
				QMapIterator<int, Glyph::Knot*> h(path);
				h.next();
				while (h.hasNext()) {
					h.next();
					const Knot * currentpoint;
					currentpoint = h.value();

					if (previouspoint->isConstant) {
						if (!previouspoint->value.isEmpty()) {
							source = source % QString("((%1,%2)%3)").arg(previouspoint->x).arg(previouspoint->y).arg(previouspoint->value);
						}
						else {
							source = source % QString("(%1,%2)").arg(previouspoint->x).arg(previouspoint->y);
						}
						
					}
					else {
						source = source % QString("%1").arg(previouspoint->value);
					}


					if (previouspoint->rightValue.type == mpgui_explicit) {
						source = source % QString(" .. controls ");
						if (previouspoint->rightValue.isControlConstant) {
							source = source % QString("(%1,%2) ").arg(previouspoint->rightValue.x).arg(previouspoint->rightValue.y);
						}
						else {
							source = source % QString("%1 ").arg(previouspoint->rightValue.controlValue);
						}
						if (!previouspoint->rightValue.isEqualAfter) {
							source = source % QString("and ");
							if (currentpoint->leftValue.isControlConstant) {
								source = source % QString("(%1,%2) ").arg(currentpoint->leftValue.x).arg(currentpoint->leftValue.y);
							}
							else {
								source = source % QString("%1 ").arg(currentpoint->leftValue.controlValue);
							}
						}
						source = source % QString(" ..\n");
					}
					else if (previouspoint->rightValue.type == mpgui_curl) {
						source = source % QString(" %1\n").arg(previouspoint->rightValue.value);
					}
					else {
						if (previouspoint->rightValue.type == mpgui_given) {
							if (previouspoint->rightValue.isDirConstant) {
								source = source % QString(" {dir %1}").arg(previouspoint->rightValue.x);
							}
							else {
								source = source % QString(" {%1}").arg(previouspoint->rightValue.value);
							}
						}
						if (previouspoint->rightValue.y == 1 && currentpoint->leftValue.y == 1 && !previouspoint->rightValue.isControlConstant && !currentpoint->leftValue.isControlConstant) {

							source = source % QString(" ..\n");
						}
						else {
							if (previouspoint->rightValue.isControlConstant) {
								if (previouspoint->rightValue.isAtleast) {
									source = source % QString(" .. tension atleast %1").arg(previouspoint->rightValue.y);
								}
								else {
									source = source % QString(" .. tension %1").arg(previouspoint->rightValue.y);
								}

							}
							else {
								if (previouspoint->rightValue.isAtleast) {
									source = source % QString(" .. tension atleast %1").arg(previouspoint->rightValue.controlValue);
								}
								else {
									source = source % QString(" .. tension %1").arg(previouspoint->rightValue.controlValue);
								}
							}

							if (!previouspoint->rightValue.isEqualAfter) {
								if (currentpoint->leftValue.isControlConstant) {
									if (currentpoint->leftValue.isAtleast) {
										source = source % QString(" and atleast %1").arg(currentpoint->leftValue.y);
									}
									else {
										source = source % QString(" and %1").arg(currentpoint->leftValue.y);
									}
								}
								else {
									if (currentpoint->leftValue.isAtleast) {
										source = source % QString(" and atleast %1").arg(currentpoint->leftValue.controlValue);
									}
									else {
										source = source % QString(" and %1").arg(currentpoint->leftValue.controlValue);
									}
								}
							}
							source = source % QString(" ..\n");
						}

						if (currentpoint->leftValue.type == mpgui_given) {
							if (currentpoint->leftValue.isDirConstant) {
								source = source % QString(" {dir %1}").arg(currentpoint->leftValue.x);
							}
							else {
								source = source % QString(" {%1}").arg(currentpoint->leftValue.value);
							}
						}
					}
					previouspoint = h.value();
				}
				source = source % QString(" cycle\n");
				if (controlledPathNames[j.key()] != "fill"){
					source = source % QString(");\n");
				}
				else {
					source = source % QString(";\n");
				}
				
			}
			/*
			for (int i = 0; i < controlledPaths.size(); ++i) {
				QVector<Knot*> path = controlledPaths.at(i);
				const Knot * firstpoint = path.constFirst();
				const Knot * previouspoint = firstpoint;
				source = source % QString("fill\n");
				for (int j = 1; j < path.size(); ++j) {
					const Knot * currentpoint;
					currentpoint = path.at(j);

					if (previouspoint->isConstant) {
						source = source % QString("(%1,%2)").arg(previouspoint->x).arg(previouspoint->y);
					}
					else {
						source = source % QString("%1").arg(previouspoint->x).arg(previouspoint->value);
					}


					if (previouspoint->rightValue.type == mpgui_explicit) {
						source = source % QString(" .. controls ");
						if (previouspoint->rightValue.isControlConstant) {
							source = source % QString("(%1,%2) ").arg(previouspoint->rightValue.x).arg(previouspoint->rightValue.y);
						}
						else {
							source = source % QString("%1 ").arg(previouspoint->rightValue.value);
						}
						if (!previouspoint->rightValue.isEqualAfter) {
							source = source % QString("and ");
							if (currentpoint->leftValue.isControlConstant) {
								source = source % QString("(%1,%2) ").arg(currentpoint->leftValue.x).arg(currentpoint->leftValue.y);
							}
							else {
								source = source % QString("%1 ").arg(currentpoint->leftValue.value);
							}
						}
						source = source % QString(" ..\n");
					}
					else if (previouspoint->rightValue.type == mpgui_curl) {
						source = source % QString(" --\n");
					}
					else {
						if (previouspoint->rightValue.type == mpgui_given) {
							if (previouspoint->rightValue.isDirConstant) {
								source = source % QString(" {dir %1}").arg(previouspoint->rightValue.x);
							}
							else {
								source = source % QString(" {%1}").arg(previouspoint->rightValue.value);
							}
						}
						if (previouspoint->rightValue.y == 1 && currentpoint->leftValue.y == 1 && !previouspoint->rightValue.isControlConstant && !currentpoint->leftValue.isControlConstant) {

							source = source % QString(" ..\n");
						}
						else{
							if (previouspoint->rightValue.isControlConstant) {
								if (previouspoint->rightValue.isAtleast) {
									source = source % QString(" .. tension atleast %1").arg(previouspoint->rightValue.y);
								}
								else {
									source = source % QString(" .. tension %1").arg(previouspoint->rightValue.y);
								}

							}
							else {
								if (previouspoint->rightValue.isAtleast) {
									source = source % QString(" .. tension atleast %1").arg(previouspoint->rightValue.value);
								}
								else {
									source = source % QString(" .. tension %1").arg(previouspoint->rightValue.value);
								}
							}

							if (!previouspoint->rightValue.isEqualAfter) {
								if (currentpoint->leftValue.isControlConstant) {
									if (currentpoint->leftValue.isAtleast) {
										source = source % QString(" and atleast %1").arg(currentpoint->leftValue.y);
									}
									else {
										source = source % QString(" and %1").arg(currentpoint->leftValue.y);
									}
								}
								else {
									if (currentpoint->leftValue.isAtleast) {
										source = source % QString(" and atleast %1").arg(currentpoint->leftValue.value);
									}
									else {
										source = source % QString(" and %1").arg(currentpoint->leftValue.value);
									}
								}
							}
							source = source % QString(" ..\n");
						}

						if (currentpoint->leftValue.type == mpgui_given) {
							if (currentpoint->leftValue.isDirConstant) {
								source = source % QString(" {dir %1}").arg(currentpoint->leftValue.x);
							}
							else {
								source = source % QString(" {%1}").arg(currentpoint->leftValue.value);
							}
						}
					}
					previouspoint = path.at(j);

				}


				source = source % QString("cycle;\n");




			}*/
			source = source % "\n%%endpaths\n";
		}


		/*
		QList<QByteArray> dynamicProperties = dynamicPropertyNames();
		for (int i = 0; i < dynamicProperties.length(); i++) {
			QByteArray propname = dynamicProperties[i];
			QVariant val = property(propname);
			if (QMetaType::QPoint == val.type()) {
				QPointF point = val.toPointF();
				source = source % QString("%1 := (%2,%3);\n").arg(QString(propname)).arg(point.x()).arg(point.y());
			}

		}*/

		if (body() != "") {
			source = source % "%%beginbody\n";
			source = source % body();
		}

		if (m_beginmacroname == "beginchar") {
			source = source % "endchar;\n";
		}
		else {
			source = source % "enddefchar;\n";
		}
		

		m_source = source;
		isDirty = false;
	}
	return m_source;
}
void Glyph::setName(QString name) {

	if (name == m_name)
		return;

	font->glyphperName.remove(name);
	m_name = name;
	isDirty = true;
	font->glyphperName.insert(m_name, this);
	if (m_unicode == -1) {
		m_charcode = -1;
	}
	emit valueChanged("name");
}
QString Glyph::name() const {
	return m_name;
}
void Glyph::setUnicode(int unicode) {
	if (unicode != m_unicode) {
		m_unicode = unicode;
		m_charcode = unicode;
		isDirty = true;
		if (edge) {
			edge->charcode = charcode();
		}
		emit valueChanged("unicode");
	}
}
int Glyph::unicode() const {
	return m_unicode;
}
int Glyph::charcode() {
	if (m_charcode == -1) {
		QString command("show " + name() + ";");

		QByteArray commandBytes = command.toLatin1();
		//mp->history = mp_spotless;
		int status = mp_execute(mp, (char *)commandBytes.constData(), commandBytes.size());
		mp_run_data * results = mp_rundata(mp);
		QString ret(results->term_out.data);
		ret.trimmed();
		if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
			mp_finish(mp);
			throw "Could not get charcode !\n" + ret;
		}

		m_charcode = ret.mid(3).toInt();
	}
	return m_charcode;
}


void Glyph::setWidth(double width) {
	m_width = width;
	isDirty = true;
	emit valueChanged("width");
}
double Glyph::width() const {
	return m_width;
}
void Glyph::setHeight(double height) {
	m_height = height;
	isDirty = true;
	emit valueChanged("height");
}
double Glyph::height() const {
	return m_height;
}
void Glyph::setDepth(double depth) {
	m_depth = depth;
	isDirty = true;
	emit valueChanged("depth");
}
double Glyph::depth() const {
	return m_depth;
}
void Glyph::setleftTatweel(double lefttatweel) {
	m_lefttatweel = lefttatweel;
	isDirty = true;
	emit valueChanged("leftTatweel");
}
double Glyph::leftTatweel() const {
	return m_lefttatweel;
}
void Glyph::setrightTatweel(double righttatweel) {
	m_righttatweel = righttatweel;
	isDirty = true;
	emit valueChanged("rightTatweel");
}
double Glyph::rightTatweel() const {
	return m_righttatweel;
}
void Glyph::setImage(Glyph::ImageInfo image) {
	ImageInfo old = m_image;
	m_image = image;
	isDirty = true;
	if (old.path != m_image.path) {
		emit valueChanged("image");
	}
	else if (old.pos != m_image.pos | old.transform != m_image.transform) {
		emit valueChanged("imagetransform");
	}
}
Glyph::ImageInfo Glyph::image() const {
	return m_image;
}
void Glyph::setComponents(QHashGlyphComponentInfo components) {

	isDirty = true;

	QHash<Glyph*, ComponentInfo>::iterator comp;
	for (comp = m_components.begin(); comp != m_components.end(); ++comp) {
		Glyph* glyph = comp.key();
		disconnect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);
	}

	m_components.clear();

	for (comp = components.begin(); comp != components.end(); ++comp) {
		Glyph* glyph = comp.key();
		connect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);
	}

	m_components = components;

	emit valueChanged("components");

}
Glyph::QHashGlyphComponentInfo Glyph::components() const {
	return m_components;
}
void Glyph::setBody(QString body, bool autoParam) {

	/*
	if (autoParam) {
		static int  tempindex = 1;
		int adjust = 0;

		QRegularExpression regpair("\\(\\s*(\\d*\\.?\\d*)\\s*,\\s*(\\d*\\.?\\d*)\\s*\\)", QRegularExpression::DotMatchesEverythingOption);
		QRegularExpressionMatchIterator i = regpair.globalMatch(body);
		while (i.hasNext()) {
			QRegularExpressionMatch match = i.next();
			Param param;
			param.name = QString("tmpp%1").arg(tempindex++);
			param.type = point;
			params.append(param);
			double x = match.captured(1).toDouble();
			double y = match.captured(2).toDouble();
			setProperty(param.name.toLatin1(), QPointF(x, y));
			QString replace = " " + param.name + " ";
			body.replace(match.capturedStart(0) + adjust, match.capturedLength(0), replace);
			adjust += replace.length() - match.capturedLength(0);
		}
	}*/

	m_body = body;
	isDirty = true;
	emit valueChanged("body");
}
QString Glyph::body() const {
	return m_body;
}
void Glyph::setVerbatim(QString verbatim) {
	m_verbatim = verbatim;
	isDirty = true;
	emit valueChanged("verbatim");
}
QString Glyph::verbatim()const {
	return m_verbatim;
}
void Glyph::setComponent(QString name, double x, double y, double t1, double t2, double t3, double t4) {
	isDirty = true;

	ComponentInfo component;
	Glyph* glyph = font->glyphperName[name];
	component.pos = QPointF(x, y);
	QTransform transform(t1, t2, t3, t4, 0, 0);
	component.transform = transform;
	m_components[glyph] = component;
	connect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);

	emit valueChanged("components");
}
void Glyph::parseComponents(QString componentSource) {

	/*
	isDirty = true;

	QHash<Glyph*, ComponentInfo>::iterator comp;
	for (comp = m_components.begin(); comp != m_components.end(); ++comp) {
		Glyph* glyph = comp.key();
		disconnect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);
	}

	m_components.clear();

	QRegularExpression regcomponents("^drawcomponent\\((.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?)\\);");
	regcomponents.setPatternOptions(QRegularExpression::MultilineOption);
	QRegularExpressionMatchIterator i = regcomponents.globalMatch(componentSource);
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		ComponentInfo component;
		Glyph* glyph = font->glyphperUnicode[match.captured(1).toLong()];
		component.pos = QPointF(match.captured(2).toDouble(), match.captured(3).toDouble());
		QTransform transform(match.captured(4).toDouble(), match.captured(5).toDouble(),
			match.captured(6).toDouble(), match.captured(7).toDouble(), 0, 0);
		component.transform = transform;
		m_components[glyph] = component;
		connect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);
	}

	emit valueChanged("components");*/
}
void Glyph::parsePaths(QString pathsSource) {
	/*
	QRegularExpression regpaths("fill\\s+(.+?)\\s+((?:\\.\\.\\.|\\.\\.|--).*);");
	QRegularExpression regpair("\\(\\s*([+-]?\\d*\\.?\\d*)\\s*,\\s*([+-]?\\d*\\.?\\d*)\\s*\\)");
	//QRegularExpression regcontrols("(?:\\{\\(.*?)\\})?(\\.\\.\\s*controls()\\s\\.\\.)(?:\\{\\(.*?)\\})?(.+?)(?:\\.\\.\\.|\\.\\.|--)?");
	//QRegularExpression regcontrols("(\\.\\.\\s*controls()\\s\\.\\.)(.+?)(?:\\.\\.\\.|\\.\\.|--)?");
	QString direction = "(?:\\s*\\{(.*?)\\}\\s*)?";
	QString controltension = "(?:\\.\\.\\s*(controls|tension)(?:((?:(?!\\.\\.).)*?)\\s*and\\s*(.*?)|(.*?))\\.\\.)";
	QString regcontrolsstr = direction + "(?:" + controltension + "|(--)" + ")" + direction;
	QRegularExpression regcontrols(regcontrolsstr);
	regpaths.setPatternOptions(QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
	QRegularExpressionMatchIterator i = regpaths.globalMatch(pathsSource);
	int nbpath = 0;
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		Knot knot;
		QMap<int, Knot> newpath;
		QString value = match.captured(1);
		QRegularExpressionMatch pairmatch = regpair.match(value);
		if (pairmatch.hasMatch()) {
			newpath[0].isConstant = true;
			newpath[0].x = pairmatch.captured(1).toDouble();
			newpath[0].y = pairmatch.captured(2).toDouble();
		}
		else {
			newpath[0].isConstant = false;
			newpath[0].value = value;
		}

		regcontrols.setPatternOptions(QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
		QRegularExpressionMatchIterator icontrols = regcontrols.globalMatch(match.captured(2));
		while (icontrols.hasNext()) {
			QRegularExpressionMatch match = icontrols.next();
		}
		controlledPaths[nbpath++] = newpath;

	}

	emit valueChanged("paths");*/
}
void Glyph::componentChanged(QString name, bool structureChanged) {
	emit valueChanged("component", false);
}
void Glyph::setParameter(QString name, Glyph::ParameterType type, double x, double y, Glyph::ParameterPosition position, int numsubpath, int numpoint, bool isEquation) {

	Param param = {};

	param.name = name;
	param.type = type;
	param.position = position;
	param.applytosubpath = numsubpath;
	param.applytopoint = numpoint;
	param.isEquation = isEquation;
	if (param.type == point) {
		setProperty(param.name.toLatin1(), QPointF(x, y));
	}
	else {
		setProperty(param.name.toLatin1(), x);
	}

	QString key = QString(param.applytosubpath) + "_" + QString(param.applytopoint);
	if (param.type == direction) {
		if (param.position == left) {
			ldirections[key] = param;
		}
		else {
			rdirections[key] = param;
		}
	}
	else if (param.type == tension) {
		if (param.position == left) {
			ltensions[key] = param;
		}
		else {
			rtensions[key] = param;
		}
	}

	params.insert(param.name, param);
}
void Glyph::setParameters(QString parameters) {
	isDirty = true;
	/*
	ldirections.clear();
	rdirections.clear();
	ltensions.clear();
	rtensions.clear();
	params.clear();

	QList<QByteArray> dynamicProperties = dynamicPropertyNames();
	for (int i = 0; i < dynamicProperties.length(); i++) {
		QByteArray propname = dynamicProperties[i];
		QVariant variant;
		setProperty(propname, variant);
	}

	QRegularExpression regnumeric("\\s(.*?)\\s*:=\\s*(\\-?\\d*\\.?\\d*)\\s*;\\s*%\\s*numeric");
	QRegularExpressionMatchIterator i = regnumeric.globalMatch(parameters);
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		Param param;
		param.name = match.captured(1);
		param.type = numeric;
		setProperty(param.name.toLatin1(), match.captured(2).toDouble());
		params.append(param);
	}

	QRegularExpression regpair("\\s(.*?)\\s:=\\s*\\(\\s*(.*?)\\s*,\\s*(.*?)\\s*\\)");
	i = regpair.globalMatch(parameters);
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		Param param;
		param.name = match.captured(1);
		param.type = point;
		double x = match.captured(2).toDouble();
		double y = match.captured(3).toDouble();
		setProperty(param.name.toLatin1(), QPointF(x, y));
		params.append(param);
	}

	QRegularExpression regdir("\\s(.*?)\\s:=\\s*(.*?)\\s*;\\s*%\\s*(ldir|rdir|ltens|rtens)\\s*\\((.*?),(.*?)\\)");
	i = regdir.globalMatch(parameters);
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		Param param;
		param.name = match.captured(1);
		double val = match.captured(2).toDouble();
		param.applytosubpath = match.captured(4).toInt();
		param.applytopoint = match.captured(5).toInt();
		setProperty(param.name.toLatin1(), val);
		QString key = QString(param.applytosubpath) + "_" + QString(param.applytopoint);
		if (match.captured(3) == "ldir") {
			param.type = direction;
			param.position = left;
			ldirections[key] = param;
		}
		else if (match.captured(3) == "rdir") {
			param.type = direction;
			param.position = right;
			rdirections[key] = param;
		}
		else if (match.captured(3) == "ltens") {
			param.type = tension;
			param.position = left;
			ltensions[key] = param;
		}
		else {
			param.type = tension;
			param.position = right;
			rtensions[key] = param;
		}
		params.append(param);

	}

	m_parameters = parameters;

	emit valueChanged("parameters");*/
}
QString Glyph::parameters() const {
	return m_parameters;
}

QString Glyph::getError()
{
	mp_run_data * results = mp_rundata(mp);
	QString ret(results->term_out.data);
	return ret.trimmed();
}

QString Glyph::getLog() {
	mp_run_data * results = mp_rundata(mp);
	QString ret(results->log_out.data);
	return ret.trimmed();
}

mp_edge_object* Glyph::getEdge(bool resetExpParams)
{
	if (edge && !isDirty)
		return edge;

	auto data = source();

	if (!resetExpParams) {
		data = QString("ignore_exp_parameters:=1;lefttatweel:=%1;righttatweel:=%2;").arg(leftTatweel()).arg(rightTatweel()) + data;
	}

	QByteArray commandBytes = data.toLatin1();
	mp->history = mp_spotless;
	int status = mp_execute(mp, (char *)commandBytes.constData(), commandBytes.size());
	mp_run_data* _mp_results = mp_rundata(mp);
	if (status >= mp_error_message_issued) {
		//QString error = getError();
        edge = NULL;
		return edge;
	}
	mp_edge_object * p = _mp_results->edges;
	while (p) {
		if (p->charcode == charcode()) {
			edge = p;
			break;
		}
		p = p->next;
	}

	//int res = mp_svg_gr_ship_out(edge, 0, false);

	//auto svg = _mp_results->ship_out.data;

	return edge;
}

Glyph::ComputedValues Glyph::getComputedValues() {
	mp_edge_object* mp_edge = getEdge();
	ComputedValues values;

	if (mp_edge) {
		values.charcode = mp_edge->charcode;
		values.width = mp_edge->width;
		values.height = mp_edge->height;
		values.depth = mp_edge->depth;
		values.bbox.llx = mp_edge->minx;
		values.bbox.urx = mp_edge->maxx;
		values.bbox.lly = mp_edge->miny;
		values.bbox.ury = mp_edge->maxy;

		if (!std::isnan(mp_edge->xleftanchor)) {
			values.leftAnchor = QPoint(mp_edge->xleftanchor, mp_edge->yleftanchor);
		}
		if (!std::isnan(mp_edge->xrightanchor)) {
			values.rightAnchor = QPoint(mp_edge->xrightanchor, mp_edge->yrightanchor);
		}
	}
	

	return values;
}

bool Glyph::event(QEvent *e) {
	if (e->type() == QEvent::DynamicPropertyChange) {
		QDynamicPropertyChangeEvent *pe = static_cast<QDynamicPropertyChangeEvent *>(e);
		if (!isSetSource) {
			isDirty = true;
			emit valueChanged(pe->propertyName());
		}
	}
	return QObject::event(e); // don't forget this
}
QUndoStack* Glyph::undoStack() const
{
	return m_undoStack;
}
void Glyph::setPropertyWithUndo(const QString &name, const QVariant &value) {
	ChangeGlyphPropertyCommand* command = new ChangeGlyphPropertyCommand(this, name, value);
	m_undoStack->push(command);

}
QPainterPath Glyph::getPathFromEdge(mp_edge_object*  h) {

	QPainterPath localpath;
	

	if (h) {
		mp_graphic_object* body = h->body;


		if (body) {

			do {
				switch (body->type)
				{
				case mp_fill_code: {
					QPainterPath subpath = mp_dump_solved_path(((mp_fill_object *)body)->path_p);
					localpath.addPath(subpath);

					break;
				}
				default:
					break;
				}

			} while (body = body->next);
		}
	}

	return localpath;
}
QPainterPath Glyph::mp_dump_solved_path(mp_gr_knot h) {
	mp_gr_knot p, q;
	QPainterPath path;
	//path.setFillRule(Qt::OddEvenFill);
	if (h == NULL) return path;

	path.moveTo(h->x_coord, h->y_coord);
	p = h;
	do {
		q = p->next;
		path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);

		p = q;
	} while (p != h);
	if (h->data.types.left_type != mp_endpoint)
		path.closeSubpath();

	return path;
}
QPainterPath Glyph::getPath() {
	QPainterPath localpath;
	QHash<Glyph*, Glyph::ComponentInfo>::iterator comp;
	for (comp = m_components.begin(); comp != m_components.end(); ++comp) {
		Glyph* compglyph = comp.key();
		QPainterPath compath = compglyph->getPath();
		compath.translate(comp.value().pos);
		compath = comp.value().transform.map(compath);
		localpath.addPath(compath);
	}

	localpath.addPath(getPathFromEdge(getEdge()));

	return localpath;
}
