#pragma once
#include "automedina.h"
#include "GlyphVis.h"
#include "Subtable.h"
#include "Lookup.h"

class IndoPak : public Automedina {
public:
  IndoPak(OtLayout* layout, Font* font, bool extended);
  Lookup* getLookup(QString lookupName) override;
  CalcAnchor  getanchorCalcFunctions(QString functionName, Subtable* subtable) override;
  void generateSubstEquivGlyphs() override;
private:
  Lookup* defaultmarkposition();
  Lookup* defaultwaqfmarktobase();
  Lookup* forsmalllalef();
  Lookup* forsmallhighwaw();
  Lookup* forhamza();
  Lookup* forheh();
  Lookup* forwaw();  
  Lookup* cursivejoin();
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

