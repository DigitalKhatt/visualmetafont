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


#include <string>
#include "OtLayout.h"
#include "qregularexpression.h"
#include "GlyphVis.h"
#include "automedina/automedina.h"
//#include "qfile.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
#include <unordered_map>
#include <fstream>
#include <math.h>



#if defined DIGITALKHATT_WEBLIB && defined  EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/bind.h>
#endif
#include <QtCore\qmath.h>

struct PageResult {
	QList<LineLayoutInfo> page;
	QStringList originalPage;
};

class QuranShaper {
public:

	QuranShaper() {

		int status = initilizeMetapost();

		if (status == 0) {
			status = executeMetapost("input medinafont.mp;");

			layout = new OtLayout(mp);

			loadLookupFile("lookups.json");
		}


	}

	int initilizeMetapost() {
		MP_options* _mp_options = mp_options();
		//MP_options _mp_options;
		_mp_options->noninteractive = 1;
		_mp_options->command_line = NULL;
		_mp_options->ini_version = true;
		_mp_options->math_mode = mp_math_double_mode;
		//_mp_options->interaction = mp_nonstop_mode;
		//_mp_options->mem_name = "plain";
		//_mp_options->mem_name = "automedina";
		//_mp_options -> main_memory = 1000000;

		mp = mp_initialize(_mp_options);

		if (!mp) return 5;

		//std::string commandBytes = "MPGUI:=1;input mpguifont.mp;input medinafont.mp;";

		std::string commandBytes = "MPGUI:=1;input mpguifont.mp;";

		int status = executeMetapost(commandBytes);

		delete _mp_options;

		std::cout << "Metapost initilized with status " << status << '\n';

		return status;
	}

	int executeMetapost(std::string code) {

		int status = mp_execute(mp, (char*)code.c_str(), code.size());

		if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
			mp_run_data* results = mp_rundata(mp);
			std::string ret(results->term_out.data);
			//ret.trimmed();
			std::cout << "Could not initialize MetaPost library instance!\n" + ret << '\n';
			mp_finish(mp);
			//throw "Could not initialize MetaPost library instance!\n" + ret;

		}

		return status;

	}


	void initLayout() {
		layout = new OtLayout(mp);
	}

	void initLookup(std::string fileName) {
		loadLookupFile(fileName);
	}

	~QuranShaper() {
		delete layout;
	}

	std::string getGlyphName(int codechar) {

		return layout->glyphNamePerCode.value(codechar).toStdString();

	}

	int getGlyphCode(std::string  name) {

		QString nn = QString::fromStdString(name);

		return layout->glyphCodePerName.value(nn);

	}

	QList<SuraLocation> getSuraLocations(bool tex) {
		if (tex) {
			if (texSuraLocations.isEmpty()) {
				readTexPages();
			}

			return texSuraLocations;
		}
		else {
			if (medinaSuraLocations.isEmpty()) {
				readMedinaPages();
			}

			return medinaSuraLocations;
		}



	}
#if defined DIGITALKHATT_WEBLIB && defined  EMSCRIPTEN
	void drawPath(std::string glyphName, emscripten::val ctx) {

		if (!mp) {
			std::cout << "cannot initilize mp";
		}
		mp_run_data* run = mp_rundata(mp);

		mp_edge_object* p = run->edges;

		while (p != NULL) {
			if (p->charname == glyphName) {
				mp_graphic_object* body = p->body;

				if (body) {
					edgetoHTML5Path(body, ctx);
				}

				return;
			}
			else {
				p = p->next;
			}

		}

		std::cout << "no char";


	}

	double  shapeText(std::string text, int lineWidth, float fontScalePerc, bool applyJustification, bool tajweedColor, emscripten::val ctx) {

		layout->applyJustification = applyJustification;

		QString input = QString::fromStdString(text);

		auto lines = input.split(10, QString::SkipEmptyParts);

		auto justification = LineJustification::Distribute;

		if (lineWidth == 0) {
			justification = LineJustification::Center;
		}

		int fontScale = (1 << OtLayout::SCALEBY) * fontScalePerc;

		double scale = 72. / (4800 << OtLayout::SCALEBY);

		lineWidth = lineWidth / scale;

		auto page = layout->justifyPage(fontScale, lineWidth, lineWidth, lines, justification, true, tajweedColor);

		int currentyPos = 0;
		int margin = 0;
		int InterLineSpacing = layout->InterLineSpacing << OtLayout::SCALEBY;

		double maxWidth = 0;

		for (int lineIndex = 0; lineIndex < page.size(); ++lineIndex) {

			auto& line = page[lineIndex];

			int currentxPos = 0; // lineWidth + margin - line.xstartposition;

			QPoint lastPos{ currentxPos ,currentyPos };

			ctx.call<void>("save");

			ctx.call<void>("scale", scale, scale);

			ctx.call<void>("transform", 1, 0, 0, -1, lastPos.x(), lastPos.y());

			for (int glyphIndex = 0; glyphIndex < line.glyphs.size(); glyphIndex++) {

				auto& glyph = line.glyphs[glyphIndex];

				if (glyph.color) {
					auto color = glyph.color;

					ctx.set("fillStyle", emscripten::val("rgb(" + std::to_string(((color >> 24) & 0xff)) + "," + std::to_string(((color >> 16) & 0xff)) + "," + std::to_string(((color >> 8) & 0xff)) + ")"));
				}

				QPoint pos;
				currentxPos -= glyph.x_advance;
				pos.setX(currentxPos + (glyph.x_offset));
				pos.setY(currentyPos - (glyph.y_offset));

				QPoint diff = pos - lastPos;
				lastPos = pos;

				ctx.call<void>("translate", diff.x(), -diff.y());

				ctx.call<void>("save");
				ctx.call<void>("scale", fontScale, fontScale);

				displayGlyph(glyph.codepoint, glyph.lefttatweel, glyph.righttatweel, ctx);
				ctx.call<void>("restore");

				if (glyph.color) {
					ctx.set("fillStyle", "rgb(0,0,0)");
				}
			}

			if (currentxPos < maxWidth)
				maxWidth = currentxPos;

			currentyPos += InterLineSpacing;

			ctx.call<void>("restore");

		}

		clearAlternates();

		return -maxWidth * scale;

	}

#endif
	PageResult  shapePage(int pageNumber, float fontScalePerc, bool applyJustification, int lineIndex, bool texFormat, bool tajweedColor) {

		//if (cachedPages.find(pageNumber) != cachedPages.end()) {
		//	std::cout << "Cached page number " << pageNumber << "\n";
		//	return cachedPages.at(pageNumber);
		//}

		int lineWidth = pageWidth;

		layout->applyJustification = applyJustification;

		QStringList lines;

		if (texFormat) {
			if (texPages.isEmpty()) {
				readTexPages();
			}

			if (texPages.size() <= pageNumber) {
				std::cout << "Out of range Tex pageNumber " << pageNumber << '\n';
				return PageResult{};
			}

			lines = texPages[pageNumber];
		}
		else {
			/*
			QString textt = QString::fromUtf8(qurantext[pageNumber] + 1);

			textt = textt.replace(QRegularExpression(" *" + QString("۞") + " *"), QString("۞") + " ");

			lines = textt.split(10, QString::SkipEmptyParts);*/

			if (medinaPages.isEmpty()) {
				readMedinaPages();
			}

			if (medinaPages.size() <= pageNumber) {
				std::cout << "Out of range Medina pageNumber " << pageNumber << '\n';
				return PageResult{};
			}

			lines = medinaPages[pageNumber];

		}

		int fontScale = (1 << OtLayout::SCALEBY) * fontScalePerc;

		if (lineIndex >= 0) {
			lines = QStringList{ lines[lineIndex] };
		}

		auto justification = LineJustification::Distribute;
		int beginsura = OtLayout::TopSpace << OtLayout::SCALEBY;

		if (pageNumber == 0 || pageNumber == 1) {
			justification = LineJustification::Center;
			beginsura = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 3)) << OtLayout::SCALEBY;
			if (lineIndex > 0) {
				double diameter = pageWidth * 1; // 0.9;
				if (pageNumber == 0) {
					diameter = pageWidth * 1; // 0.9;
				}

				int index = lineIndex - 1;
				//index = index % 4;
				// 22.5 = 180 / 8
				double degree = lineIndex * 22.5 * M_PI / 180;
				lineWidth = diameter * std::sin(degree);
				//std::cout << "lineIndex=" << lineIndex << ", lineWidth=" << lineWidth << std::endl;
			}
			else {
				lineWidth = 0;
			}

		}

		auto page = layout->justifyPage(fontScale, lineWidth, pageWidth, lines, justification, false, tajweedColor);

		for (int i = 0; i < page.size(); ++i) {



			auto& currentLine = page[i];

			// check if suran name or bism
			auto match = surabism.match(lines[i]);
			if (match.hasMatch()) {

				auto temp = layout->justifyPage(fontScale, 0, pageWidth, QStringList{ lines[i] }, LineJustification::Center, false, tajweedColor);

				if (match.captured(0).startsWith("سُ")) {
					temp[0].type = LineType::Sura;
				}
				else {
					temp[0].type = LineType::Bism;
				}

				page[i] = temp[0];
			}
			else {
				// check if sajda
				match = sajdaRe.match(lines[i]);
				if (match.hasMatch()) {

					//sajdamatched++;



					int startOffset = match.capturedStart(match.lastCapturedIndex()); // startOffset == 6
					int endOffset = match.capturedEnd(match.lastCapturedIndex()) - 1; // endOffset == 9




					while (layout->glyphGlobalClasses[lines[i][endOffset].unicode()] == OtLayout::MarkGlyph)
						endOffset--;

					bool beginDone = false;

					auto& glyphs = currentLine.glyphs;

					for (auto& glyphLayout : glyphs) {

						if (glyphLayout.cluster == startOffset && !beginDone) {
							glyphLayout.beginsajda = true;
							beginDone = true;;
							//beginsajda++;

						}
						else if (glyphLayout.cluster == endOffset) {
							glyphLayout.endsajda = true;
							//endsajda++;
							break;
						}

					}

				}
			}
			if (i == 0 && (pageNumber == 0 || pageNumber == 1)) {
				page[i].type = LineType::Sura;
				page[i].ystartposition = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 1)) << OtLayout::SCALEBY;
			}
			else {
				page[i].ystartposition = beginsura;
				beginsura += OtLayout::InterLineSpacing << OtLayout::SCALEBY;

			}


		}

		//cachedPages.insert({ pageNumber, page });
		//PageResult resutl;
		//resutl.page = page;

		return { page, lines };

	}
#if defined DIGITALKHATT_WEBLIB && defined  EMSCRIPTEN
	void displayGlyph(int glyphIndex, double leftTatweel, double righttatweel, emscripten::val ctx) {

		GlyphVis* glyph = layout->getGlyph(glyphIndex, leftTatweel, righttatweel);

		if (glyph) {
			generateGlyph(*glyph, ctx);
		}

	}
#endif
	void clearAlternates() {
		layout->clearAlternates();
	}

	MP mp;
	OtLayout* layout;

private:

	QList<QStringList> texPages;
	QList<QStringList> medinaPages;

	QList<SuraLocation> texSuraLocations;
	QList<SuraLocation> medinaSuraLocations;

	QString suraWord = "سُورَةُ";
	QString bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";

	QString surapattern = "^("
		+ suraWord + " .*|"
		+ bism
		+ "|" + "بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"
		+ ")$";

	QRegularExpression surabism = QRegularExpression(surapattern, QRegularExpression::MultilineOption);

	bool newface = true;


	QString sajdapatterns = "(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدࣰا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعࣰا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدࣰا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)"; // sajdapatterns.replace("\u0657", "\u08F0").replace("\u065E", "\u08F1").replace("\u0656", "\u08F2");
	QRegularExpression sajdaRe = QRegularExpression(sajdapatterns, QRegularExpression::MultilineOption);

	int pageWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

	std::unordered_map<int, QList<LineLayoutInfo>> cachedPages;

	void readTexPages() {
		double size;
		QList<QString> suraNamebyPage;

		QByteArray array = readFile("texpages.dat");

		QDataStream in(array);

		/*
	QFile file("texpages.dat");
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);   // we will serialize the data into the file*/

		in >> size >> texPages >> suraNamebyPage >> texSuraLocations;

		delete[] array.data();
	}

	void readMedinaPages() {
		double size;
		QList<QString> suraNamebyPage;

		/*
		QFile file("medinapages.dat");
		file.open(QIODevice::ReadOnly);
		QDataStream in(&file);   // we will serialize the data into the file*/

		QByteArray array = readFile("medinapages.dat");
		QDataStream in(array);

		in >> size >> medinaPages >> suraNamebyPage >> medinaSuraLocations;

		delete[] array.data();
	}

#if defined DIGITALKHATT_WEBLIB && defined  EMSCRIPTEN

	std::string getPath(std::string glyphName) {
		if (!mp) {
			std::cout << "cannot initilize mp";
			return "error";
		}
		mp_run_data* run = mp_rundata(mp);

		mp_edge_object* p = run->edges;

		while (p != NULL) {
			if (p->charname == glyphName) {
				std::stringstream ret;
				ret << "function(ctx) {\n";
				mp_graphic_object* body = p->body;

				if (body) {

					ret << "\tctx.beginPath();\n";
					do {
						switch (body->type)
						{
						case mp_fill_code: {
							filltoHTML5Path(((mp_fill_object*)body)->path_p, ret);

							break;
						}
						default:
							break;
						}

					} while (body = body->next);

					ret << "\tctx.fill();\n";
				}
				ret << "}";
				return ret.str();
			}

			p = p->next;
		}

		std::cout << "no char";
		return "error";
	}

	void filltoHTML5Path(mp_gr_knot h, std::stringstream& out)
	{
		mp_gr_knot p, q;

		out << "\tctx.moveTo(" << h->x_coord << "," << h->y_coord << ");\n";
		p = h;
		do {
			q = p->next;
			out << "\tctx.bezierCurveTo(" << p->right_x << "," << p->right_y << "," << q->left_x << "," << q->left_y << "," << q->x_coord << "," << q->y_coord << ");\n";

			p = q;
		} while (p != h);
		if (h->data.types.left_type != mp_endpoint)
			out << "\tctx.closePath()\n";


	}

	void filltoHTML5Path(mp_gr_knot h, emscripten::val ctx)
	{
		mp_gr_knot p, q;

		ctx.call<void>("moveTo", h->x_coord, h->y_coord);

		p = h;
		do {
			q = p->next;

			ctx.call<void>("bezierCurveTo", p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);

			p = q;
		} while (p != h);
		if (h->data.types.left_type != mp_endpoint) {
			ctx.call<void>("closePath");
		}

	}

	void getImageStream(GlyphVis& glyph, emscripten::val ctx) {
		auto edge = glyph.edge();

		if (edge != nullptr) {
			mp_graphic_object* body = edge->body;
			if (body) {
				do {
					switch (body->type)
					{
					case mp_fill_code: {
						auto fillobject = (mp_fill_object*)body;
						ctx.call<void>("beginPath");

						filltoHTML5Path(fillobject->path_p, ctx);
						if (fillobject->color_model == mp_rgb_model) {
							ctx.set("fillStyle", emscripten::val("rgb(" + std::to_string(fillobject->color.a_val * 255) + "," + std::to_string(fillobject->color.b_val * 255) + "," + std::to_string(fillobject->color.c_val * 255) + ")"));
							//out << "\tctx.fillStyle = 'rgb(" << fillobject->color.a_val * 255 << "," << fillobject->color.b_val * 255 << "," << fillobject->color.c_val * 255 << ")';\n";

						}
						//out << "\tctx.fill();\n";
						ctx.call<void>("fill");
						ctx.set("fillStyle", emscripten::val("rgb(0,0,0)"));
						//out << "\tctx.fillStyle = 'rgb(0,0,0)';\n";

						break;
					}
					default:
						break;
					}

				} while (body = body->next);
			}
		}

	}

	void edgetoHTML5Path(mp_graphic_object* body, emscripten::val ctx)
	{

		if (body) {

			ctx.call<void>("beginPath");
			do {
				switch (body->type)
				{
				case mp_fill_code: {
					filltoHTML5Path(((mp_fill_object*)body)->path_p, ctx);


					break;
				}
				default:
					break;
				}

			} while (body = body->next);

			ctx.call<void>("fill");
		}

	}

	void generateGlyph(GlyphVis& glyph, emscripten::val ctx) {


		if (glyph.name == "endofaya") { //||  glyph->name == "rubelhizb" glyph->name == "placeofsajdah" ||
			getImageStream(glyph, ctx);
		}
		else if (glyph.charcode >= Automedina::AyaNumberCode && glyph.charcode <= Automedina::AyaNumberCode + 286) {
			int ayaNumber = (glyph.charcode - Automedina::AyaNumberCode) + 1;

			int digitheight = 120;

			//out << "\tglyphs['endofaya'](ctx);\n";
			auto ayaGlyph = &layout->glyphs["endofaya"]; generateGlyph(*ayaGlyph, ctx);


			//out << "\tctx.save();\n";
			ctx.call<void>("save");

			if (ayaNumber < 10) {
				auto& onesglyph = layout->glyphs[layout->glyphNamePerCode[1632 + ayaNumber]];

				auto position = layout->glyphs["endofaya"].width / 2 - (onesglyph.width) / 2;

				//out << "\tctx.translate(" << position << "," << digitheight << ");\n";
				ctx.call<void>("translate", position, digitheight);

				//out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";
				auto tempGlyph = &layout->glyphs[onesglyph.name]; generateGlyph(*tempGlyph, ctx);

			}
			else if (ayaNumber < 100) {
				int onesdigit = ayaNumber % 10;
				int tensdigit = ayaNumber / 10;

				auto& onesglyph = layout->glyphs[layout->glyphNamePerCode[1632 + onesdigit]];
				auto& tensglyph = layout->glyphs[layout->glyphNamePerCode[1632 + tensdigit]];



				auto position = layout->glyphs["endofaya"].width / 2 - (onesglyph.width + tensglyph.width + 40) / 2;

				//out << "\tctx.translate(" << position << "," << digitheight << ");\n";
				ctx.call<void>("translate", position, digitheight);

				//out << "\tglyphs['" << tensglyph.name << "'](ctx);\n";
				auto tempGlyph = &layout->glyphs[tensglyph.name]; generateGlyph(*tempGlyph, ctx);

				//out << "\tctx.translate(" << tensglyph.width + 40 << "," << 0 << ");\n";
				ctx.call<void>("translate", tensglyph.width + 40, 0);
				//out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";
				tempGlyph = &layout->glyphs[onesglyph.name]; generateGlyph(*tempGlyph, ctx);

			}
			else {
				int onesdigit = ayaNumber % 10;
				int tensdigit = (ayaNumber / 10) % 10;
				int hundredsdigit = ayaNumber / 100;

				auto& onesglyph = layout->glyphs[layout->glyphNamePerCode[1632 + onesdigit]];
				auto& tensglyph = layout->glyphs[layout->glyphNamePerCode[1632 + tensdigit]];
				auto& hundredsglyph = layout->glyphs[layout->glyphNamePerCode[1632 + hundredsdigit]];

				auto position = layout->glyphs["endofaya"].width / 2 - (onesglyph.width + tensglyph.width + hundredsglyph.width + 80) / 2;

				//out << "\tctx.translate(" << position << "," << digitheight << ");\n";
				ctx.call<void>("translate", position, digitheight);

				//out << "\tglyphs['" << hundredsglyph.name << "'](ctx);\n";
				auto tempGlyph = &layout->glyphs[hundredsglyph.name]; generateGlyph(*tempGlyph, ctx);

				//out << "\tctx.translate(" << hundredsglyph.width + 40 << "," << 0 << ");\n";
				ctx.call<void>("translate", tensglyph.width + 40, 0);
				//out << "\tglyphs['" << tensglyph.name << "'](ctx);\n";
				tempGlyph = &layout->glyphs[tensglyph.name]; generateGlyph(*tempGlyph, ctx);

				//out << "\tctx.translate(" << tensglyph.width + 40 << "," << 0 << ");\n";
				ctx.call<void>("translate", tensglyph.width + 40, 0);
				//out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";
				tempGlyph = &layout->glyphs[onesglyph.name]; generateGlyph(*tempGlyph, ctx);

			}

			ctx.call<void>("restore");
		}
		else {
			edgetoHTML5Path(glyph.copiedPath, ctx);
		}

	}
#endif
	void loadLookupFile(std::string fileName) {

		//QJsonModel * model = new QJsonModel;

		std::ifstream lookupStream(fileName, std::ios::binary);

		if (lookupStream) {
			// get length of file:
			lookupStream.seekg(0, lookupStream.end);
			int length = lookupStream.tellg();
			lookupStream.seekg(0, lookupStream.beg);

			char* buffer = new char[length];

			lookupStream.read(buffer, length);

			if (!lookupStream) {
				std::cout << "Problem reading file." << fileName;
			}
			else {
				QJsonDocument mDocument = QJsonDocument::fromJson(QByteArray::fromRawData(buffer, length));
				layout->readJson(mDocument.object());

			}

			lookupStream.close();
			delete[] buffer;

		}

		std::ifstream parametersStream("parameters.json", std::ios::binary);

		if (parametersStream) {
			// get length of file:
			parametersStream.seekg(0, parametersStream.end);
			int length = parametersStream.tellg();
			parametersStream.seekg(0, parametersStream.beg);

			char* buffer = new char[length];

			parametersStream.read(buffer, length);

			if (!parametersStream) {
				std::cout << "Problem reading file." << fileName;
			}
			else {
				QJsonDocument mDocument = QJsonDocument::fromJson(QByteArray::fromRawData(buffer, length));
				layout->readParameters(mDocument.object());

			}

			parametersStream.close();
			delete[] buffer;

		}


		/*
		QFile file(fileName);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QByteArray myarray = file.readAll();
			QJsonDocument mDocument = QJsonDocument::fromJson(myarray);
			//model->loadJson(mDocument);
			layout->readJson(mDocument.object());
		}*/

		/*
		QFile file2("parameters.json");
		if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QByteArray myarray = file2.readAll();
			QJsonDocument mDocument = QJsonDocument::fromJson(myarray);
			layout->readParameters(mDocument.object());
		}*/
	}


	QByteArray readFile(std::string fileName) {

		QByteArray ret;

		std::ifstream stream(fileName, std::ios::binary);

		if (stream) {
			// get length of file:
			stream.seekg(0, stream.end);
			int length = stream.tellg();
			stream.seekg(0, stream.beg);

			char* buffer = new char[length];

			stream.read(buffer, length);

			if (!stream) {
				std::cout << "Problem reading file." << fileName;
			}
			else {
				ret = QByteArray::fromRawData(buffer, length);
			}

			stream.close();

		}

		return ret;

	}


};