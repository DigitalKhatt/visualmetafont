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

#include "automedina.h"
#include "Subtable.h"
#include "GlyphVis.h"

class Defaulbaseanchorfortop : public AnchorCalc {
public:
  Defaulbaseanchorfortop(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis* curr = &_y.glyphs[glyphName];



    if (lefttatweel != 0.0 || righttatweel != 0.0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      curr = curr->getAlternate(parameters);
    }



    if (curr->conatinsAnchor(className, GlyphVis::AnchorType::MarkAnchor)) {
      QPoint anchor = curr->getAnchor(className, GlyphVis::AnchorType::MarkAnchor) + adjust;
      return anchor;
    }
    else {
      GlyphVis* originalglyph;

      QPoint adjustoriginal = getAdjustment(_y, _subtable, curr, className, adjust, lefttatweel, righttatweel, &originalglyph);

      QPoint anchor;


      anchor = caclAnchor(*originalglyph, *curr, className) + adjustoriginal + adjust;




      return anchor;
    }

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
  QPoint caclAnchor(GlyphVis& glyph, GlyphVis& curr, QString className) {

    int height = std::max((int)curr.height + _y.spacebasetotopmark, className == "shadda" ? _y.shaddamarkheight : _y.markheigh);    

    int width = glyph.width * 0.4;

    if (glyph.originalglyph == "noon.isol.expa" || glyph.originalglyph == "behshape.fina.expa") {
      width = glyph.width * 0.5;
    }
    else if (glyph.name == "seen.isol.expa" || glyph.originalglyph == "seen.isol.expa"
      || glyph.name == "sad.isol.expa" || glyph.originalglyph == "sad.isol.expa"
      || glyph.name == "qaf.isol.expa" || glyph.originalglyph == "qaf.isol.expa"
      || glyph.name == "feh.isol.expa" || glyph.originalglyph == "feh.isol.expa") {
      width = glyph.width - 250;
    }
    else if (glyph.name == "qaf.fina.expa" || glyph.originalglyph == "qaf.fina.expa"
      || glyph.name == "sad.fina.expa" || glyph.originalglyph == "sad.fina.expa"
      || glyph.name == "seen.fina.expa" || glyph.originalglyph == "seen.fina.expa"
      || glyph.name == "feh.fina.expa" || glyph.originalglyph == "feh.fina.expa") {
      width = glyph.width - 350;
    }
    else if (glyph.originalglyph == "alef.fina") {
      width = 100;
    }

    return QPoint(width, height);
  }
};

class Defaulbaseanchorforlow : public AnchorCalc {
public:
  Defaulbaseanchorforlow(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {


    GlyphVis* curr = &_y.glyphs[glyphName];

    if (lefttatweel != 0.0 || righttatweel != 0.0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      curr = curr->getAlternate(parameters);
    }

    GlyphVis* originalglyph;

    QPoint adjustoriginal = getAdjustment(_y, _subtable, curr, className, adjust, lefttatweel, righttatweel, &originalglyph);

    if (curr->originalglyph.contains("isol.expa")) {
      originalglyph = curr;
      adjustoriginal = {};
    }

    int depth = std::max((int)-originalglyph->depth + _y.spacebasetobottommark, _y.markdepth);

    QPoint anchor = QPoint{ (int)(originalglyph->width * 0.5),-depth } + adjustoriginal + adjust;

    return anchor;


  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};

class Defaultopmarkanchor : public AnchorCalc {
public:
  Defaultopmarkanchor(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis* curr = &_y.glyphs[glyphName];

    auto ori_width = curr->width;

    if (lefttatweel != 0.0 || righttatweel != 0.0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      curr = curr->getAlternate(parameters);
    }
    int width = curr->width * 0.5;
    int height = 1;



    if (glyphName == "fatha") {
      height -= (curr->width - ori_width) / 7;
    }



    width = width + adjust.x();
    height = height + adjust.y();


    return QPoint(width, height);
  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};


class Defaullowmarkanchor : public AnchorCalc {
public:
  Defaullowmarkanchor(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis* glyph = &_y.glyphs[glyphName];

    if (lefttatweel != 0.0 || righttatweel != 0.0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      glyph = glyph->getAlternate(parameters);
    }

    int width = glyph->width * 0.5;
    int height = glyph->height;


    if (!adjust.isNull()) {
      std::cout << _subtable.getLookup()->name.toStdString() << "::" << _subtable.name.toStdString() << "::" << className.toStdString() << "::" << glyphName.toStdString() << std::endl;
    }

    width = width + adjust.x();
    height = height + adjust.y();


    return QPoint(width, height);

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};

class Defaultmarkabovemark : public AnchorCalc {
public:
  Defaultmarkabovemark(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis& curr = _y.glyphs[glyphName];

    int width = curr.width * 0.5;
    int height = curr.height;

    width = width + adjust.x();
    height = height + adjust.y();


    return QPoint(width, height);

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};

class Defaultmarkbelowmark : public AnchorCalc {
public:
  Defaultmarkbelowmark(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis& curr = _y.glyphs[glyphName];


    int width = curr.width * 0.5;
    int height = -curr.depth - 20;

    width = width + adjust.x();
    height = height - adjust.y();


    return QPoint(width, height);

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};

class Defaulbaseanchorforsmallalef : public AnchorCalc {
public:
  Defaulbaseanchorforsmallalef(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {



    GlyphVis* curr = &_y.glyphs[glyphName];


    if (lefttatweel != 0.0 || righttatweel != 0.0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      curr = curr->getAlternate(parameters);
    }

    GlyphVis* originalglyph = curr;
    QPoint adjustoriginal;

    // QPoint adjustoriginal = getAdjustment(_y, _subtable, curr, className, adjust, lefttatweel, righttatweel, &originalglyph);

    //if (curr->name == "alternatechar" || curr->name.contains(".added_")) {
    if (curr->expanded) {
      originalglyph = &_y.glyphs[curr->originalglyph];
      adjustoriginal = _subtable.classes[className].baseparameters[curr->originalglyph];
      if (curr->leftAnchor) {
        double xshift = curr->matrix.xpart - originalglyph->matrix.xpart;
        double yshift = curr->matrix.ypart - originalglyph->matrix.ypart;

        adjustoriginal += QPoint(xshift / 2, yshift);
      }
    }


    QPoint anchor = caclAnchor(originalglyph) + adjustoriginal + adjust;

    return anchor;

    /*
    int height;
    int width;

    QPoint anchor;

    if (!curr.originalglyph.isEmpty() && (curr.charlt != 0 || curr.charrt != 0)) {
      QPoint adjustoriginal;
      QString originalName = curr.originalglyph;
      if (_subtable.classes[className].baseparameters.contains(originalName)) {
        adjustoriginal = _subtable.classes[className].baseparameters[originalName];
      }

      GlyphVis& original = _y.glyphs[originalName];
      double xshift = curr.matrix.xpart - original.matrix.xpart;
      double yshift = curr.matrix.ypart - original.matrix.ypart;

      anchor = caclAnchor(&original) + adjustoriginal + QPoint(xshift / 2, yshift / 2);
    }
    else {
      anchor = caclAnchor(&curr);
    }

    return anchor + adjust;*/
  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;

  QPoint caclAnchor(GlyphVis* glyph) {
    int height = 250;
    int width = 0; // glyph->width * 0.5;
    if (!glyph->name.contains("isol")) {
      width = glyph->width * 0.0;
    }
    /*
    if (glyph->name.contains("hah")) {
      width = glyph->width * 0;
    }*/

    return QPoint(width, height);
  }
};

class Defaulbaseanchorfortopdots : public AnchorCalc {
public:
  Defaulbaseanchorfortopdots(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis* curr = &_y.glyphs[glyphName];

    if (lefttatweel != 0.0 || righttatweel != 0.0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      curr = curr->getAlternate(parameters);
    }

    if (curr->conatinsAnchor(className, GlyphVis::AnchorType::MarkAnchor)) {
      QPoint anchor = curr->getAnchor(className, GlyphVis::AnchorType::MarkAnchor) + adjust;
      return anchor;
    }

    GlyphVis* originalglyph;

    QPoint adjustoriginal = getAdjustment(_y, _subtable, curr, className, adjust, lefttatweel, righttatweel, &originalglyph);

    int width = (int)(originalglyph->width * 0.5);

    if (originalglyph->name == "sad.isol.expa" || originalglyph->originalglyph == "sad.isol.expa") {
      width = originalglyph->width - 50;
    }
    else if (originalglyph->name == "sad.fina.expa" || originalglyph->originalglyph == "sad.fina.expa") {
      width = originalglyph->width - 250;
    }
    else if (originalglyph->name == "seen.isol.expa" || originalglyph->originalglyph == "seen.isol.expa") {
      width = originalglyph->width - 250;
    }
    else if (originalglyph->name == "seen.fina.expa" || originalglyph->originalglyph == "seen.fina.expa") {
      width = originalglyph->width - 300;
    }


    QPoint anchor = QPoint{ width,(int)(originalglyph->height + 80) } + adjustoriginal + adjust;

    return anchor;

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};

class Defaulbaseanchorforlowdots : public AnchorCalc {
public:
  Defaulbaseanchorforlowdots(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis* curr = &_y.glyphs[glyphName];

    if (lefttatweel != 0.0 || righttatweel != 0.0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      curr = curr->getAlternate(parameters);
    }


    if (curr->conatinsAnchor(className, GlyphVis::AnchorType::MarkAnchor)) {
      QPoint anchor = curr->getAnchor(className, GlyphVis::AnchorType::MarkAnchor) + adjust;
      return anchor;
    }
    else if ((curr->name == "behshape.fina.expa" || curr->originalglyph == "behshape.fina.expa") && curr->conatinsAnchor("dotbelow", GlyphVis::AnchorType::MarkAnchor)) {
      QPoint anchor = curr->getAnchor("dotbelow", GlyphVis::AnchorType::MarkAnchor) + adjust;
      return anchor;
    }
    else {
      QPoint adjustoriginal = getAdjustment(_y, _subtable, curr, className, adjust, lefttatweel, righttatweel, &curr);

      QPoint anchor = QPoint{ (int)(curr->width * 0.5),(int)(curr->depth - 50) } + adjustoriginal + adjust;

      return anchor;
    }

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};

class Joinedsmalllettersbaseanchor : public AnchorCalc {
public:
  Joinedsmalllettersbaseanchor(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {



    GlyphVis* curr = &_y.glyphs[glyphName];

    if (lefttatweel != 0.0 || righttatweel != 0.0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      curr = curr->getAlternate(parameters);
    }

    //QPoint adjustoriginal = getAdjustment(_y, _subtable, originalglyph, className, adjust, lefttatweel, righttatweel, &originalglyph);

    // TODO يُضَٰهِـُٔونَ different from standard
    if (curr->name.contains("added")) {
      adjust = _subtable.classes[className].baseparameters[curr->originalglyph];
    }



    if (className == "jsl") {
      int width = curr->width * 0.5;
      int height = 200;

      auto value = QPoint{ width , height };

      return value + adjust;
    }
    else if (className == "smallhighwaw") {
      auto anchor = curr->getAnchor("smallhighwaw", GlyphVis::AnchorType::MarkAnchor);

      auto value = anchor + adjust;

      int diff = value.x() - 300;

      if (diff > 0) {
        value.setX(value.x() - diff / 2);
      }

      return value;

    }
    else if (className == "beforeheh") {
      int width = 300;
      int height = 200;

      auto value = QPoint{ width , height };

      return value + adjust;
    }
    else if (className == "beforewaw") {
      int width = 100;
      int height = 200;

      auto value = QPoint{ width , height };

      return value + adjust;
    }
    else if (className == "beforesad") {
      int width = 420;
      int height = 200;

      auto value = QPoint{ width , height };

      return value + adjust;
    }
    else if (className == "beforetah") {
      int width = 225;
      int height = 200;

      auto value = QPoint{ width , height };

      return value + adjust;
    }




    return adjust;

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};
