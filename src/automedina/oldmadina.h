#pragma once
#include "automedina.h"

class OldMadina : public Automedina {
public:
  OldMadina(OtLayout* layout, MP mp, bool extended);
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
