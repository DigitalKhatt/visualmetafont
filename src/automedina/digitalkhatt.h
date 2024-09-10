#pragma once
#include "automedina.h"
#include "GlyphVis.h"
#include "Subtable.h"
#include "Lookup.h"

class digitalkhatt : public Automedina {
public:
  digitalkhatt(OtLayout* layout, Font* font, bool extended);
  Lookup* getLookup(QString lookupName) override;
  CalcAnchor  getanchorCalcFunctions(QString functionName, Subtable* subtable) override;
private:
  Lookup* defaultmarkposition();
  Lookup* defaultwaqfmarktobase();
  Lookup* forsmalllalef();
  Lookup* forsmallhighwaw();
  Lookup* forhamza();
  Lookup* forheh();
  Lookup* forwaw();
  Lookup* cursivejoin();
  Lookup* cursivejoinrtl();
  Lookup* pointmarks();
  Lookup* defaultwaqfmarkabovemarkprecise();
  Lookup* defaultdotmarks();
  Lookup* defaultmarkdotmarks();
  Lookup* defaultmkmk();
  Lookup* ayanumbers();
  Lookup* ayanumberskern();
  Lookup* rehwawcursivecpp();
  Lookup* tajweedcolorcpp();
  Lookup* populatecvxx();
  Lookup* glyphalternates();
  //Justification
  Lookup* shrinkstretchlt(float lt, QString featureName);
  Lookup* shrinkstretchlt();
  void addchars();
  void generateGlyphs();

};
class Defaultwaqfmarkabovemark : public AnchorCalc {
public:
  Defaultwaqfmarkabovemark(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis& curr = _y.glyphs[glyphName];

    int width = 0;
    int height = 0;

    return QPoint(width, height);

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};
class Defaultmarkbelowwaqfmark : public AnchorCalc {
public:
  Defaultmarkbelowwaqfmark(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) override {

    GlyphVis& curr = _y.glyphs[glyphName];

    int width = 0 + adjust.x();
    int height = curr.height + 50 + adjust.y();


    return QPoint(width, height);

  };
private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};
