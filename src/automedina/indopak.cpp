#include "indopak.h"
#include "qstring.h"
#include "Subtable.h"
#include "Lookup.h"
#include "font.hpp"
#include "GlyphVis.h"
#include <algorithm>
#include "qregularexpression.h"
#include "defaultmarkpositions.h"
#include "metafont.h"
#include "qdebug.h"


using namespace std;

namespace indopak {

  void IndoPak::generateGlyphs() {

    mp_edge_object* edges = font->getEdges();

    glyphs.clear();

    while (edges) {

      auto name = QString(edges->charname);

      if (name != "alternatechar") {
        GlyphVis& glyph = *glyphs.insert(name, GlyphVis(m_layout, edges));

        if (edges->glyphtype != (int)GlyphType::GlyphTypeColored && edges->glyphtype != (int)GlyphType::GlyphTypeTemp) {
          m_layout->glyphNamePerCode[glyph.charcode] = glyph.name;
          m_layout->glyphCodePerName[glyph.name] = glyph.charcode;
          m_layout->unicodeToGlyphCode.insert(glyph.charcode, glyph.charcode);

          if (edges->glyphtype == (int)GlyphType::GlyphTypeMark) {
            classes["marks"].insert(glyph.name);
            m_layout->glyphGlobalClasses[glyph.charcode] = OtLayout::MarkGlyph;
          }
          else if (classes["marks"].contains(glyph.name)) {
            m_layout->glyphGlobalClasses[glyph.charcode] = OtLayout::MarkGlyph;
          }
          else if (edges->glyphtype == (int)GlyphType::GlyphTypeBase || edges->glyphtype == (int)GlyphType::GlyphTypeLigature) {
            classes["bases"].insert(glyph.name);
            m_layout->glyphGlobalClasses[glyph.charcode] = (OtLayout::GDEFClasses)edges->glyphtype;
          }
          else if (edges->glyphtype == (int)GlyphType::GlyphTypeComponent) {
            m_layout->glyphGlobalClasses[glyph.charcode] = OtLayout::ComponentGlyph;
          }

          for (int i = 0; i < edges->numAnchors; i++) {
            AnchorPoint anchor = edges->anchors[i];
            if (anchor.anchorName) {
              switch (anchor.type)
              {
              case 1:
                markAnchors[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
                break;
              case 2:
                entryAnchors[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
                break;
              case 3:
                exitAnchors[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
              case 4:
                entryAnchorsRTL[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
                break;
              case 5:
                exitAnchorsRTL[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
              default:
                break;
              }
            }
          }
        }
      }

      edges = edges->next;
    }

    auto addFake = [this](QString glyphName, quint16 unicode, quint16 codechar) {
      auto code = unicode; //codechar; //layout.glyphNamePerCode.lastKey();
      GlyphVis& glyph = *glyphs.insert(glyphName, GlyphVis());
      glyph.name = glyphName;
      glyph.charcode = code;

      m_layout->glyphNamePerCode[glyph.charcode] = glyph.name;
      m_layout->glyphCodePerName[glyph.name] = glyph.charcode;
      m_layout->unicodeToGlyphCode.insert(unicode, glyph.charcode);


      };
    addFake("alef.maddahabove.isol", 0x0622, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("alef.hamzaabove.isol", 0x0623, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("waw.hamzaabove.isol", 0x0624, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("alef.hamzabelow.isol", 0x0625, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("alefmaksura.hamzaabove.isol", 0x0626, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("behshape.onedotdown.isol", 0x0628, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("heh.twodotsup.isol", 0x0629, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("behshape.twodotsup.isol", 0x062A, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("behshape.three_dots.isol", 0x062B, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("hah.onedotdown.isol", 0x062C, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("hah.onedotup.isol", 0x062E, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("dal.onedotup.isol", 0x0630, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("reh.onedotup.isol", 0x0632, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("seen.three_dots.isol", 0x0634, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("sad.onedotup.isol", 0x0636, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("tah.onedotup.isol", 0x0638, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("ain.onedotup.isol", 0x063A, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("alef.wasla.isol", 0x0671, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("noon.onedotup.isol", 0x0646, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("feh.onedotup.isol", 0x0641, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("qaf.twodotsup.isol", 0x0642, m_layout->glyphNamePerCode.lastKey() + 1);
    addFake("yehfarsi.isol", 0x06CC, m_layout->glyphNamePerCode.lastKey() + 1);

    m_layout->glyphs = glyphs;


  }

  void IndoPak::generateAyas(QString ayaName, bool colored, int unicode) {
    int codechar = unicode - 1;
    for (int ayaNumber = 1; ayaNumber <= 286; ayaNumber++) {
      codechar = unicode == -1 ? -1 : codechar + 1;
      QString setcolored;
      if (colored) {
        setcolored = QString("coloredglyph:=\"%1.colored%2\"").arg(ayaName).arg(ayaNumber);
      }
      QString data = QString("beginchar(%1%2,%4,-1,1,-1);\n%%beginbody\ngenAyaNumber(%1, %2);%3;endchar;").arg(ayaName).arg(ayaNumber).arg(setcolored).arg(codechar);
      m_layout->font->executeMetaPost(data);
      if (colored) {
        data = QString("beginchar(%1.colored%2,-1,-1,5,-1);\n%%beginbody\ngenAyaNumber(%1.colored, %2);endchar;").arg(ayaName).arg(ayaNumber);
        m_layout->font->executeMetaPost(data);
      }
    }
  }

  void IndoPak::addchars() {
    generateAyas("endofaya", false, 0xF500);
  }

  IndoPak::IndoPak(OtLayout* layout, Font* font, bool extended) :Automedina{ layout,font, extended } {

    //m_metafont = layout->m_font;
    classes["marks"] = {
      //"cgj",
      "onedotup",
      "onedotdown",
      "twodotsup",
      "twodotsdown",
      "three_dots",
      "fathatanidgham",
      "kasratanidgham",
      "dammatanidgham",
      "fatha",
      "damma",
      "kasra",
      "shadda",
      "headkhah",
      "sukun",
      "dammatan",
      "maddahabove",
      "fathatan",
      "kasratan",
      "smallalef",
      "smallalef.joined",
      "meemiqlab",
      "smalllowmeem",
      "smallhighyeh",
      "smallhighwaw",
      "wasla",
      "hamzaabove",
      "hamzaabove.lamalef",
      "hamzaabove.small",
      "hamzaabove.joined",
      "hamzabelow",
      "smallhighroundedzero",
      "rectangularzero",
      "smallhighseen",
      "smallhighnoon",
      "waqf.meem",
      "waqf.lam",
      "waqf.jeem",
      "waqf.sad",
      "waqf.smallhighthreedots",
      "roundedfilledhigh",
      "roundedfilledlow",
      "space.ii",
      "maddawaajib",
      "smallhightah",
      "smallhighsad",
      "smallhighain",
      "smallhighqaf",
      "smallhighzain",
      "inverteddamma",
      "subalef",
      "takhallus",
      "sakta",
      "sajda",
      "smalllownoon",
      "waqf.qif",
      "waqfa",
      "hamzabelow.joined"
    };

    classes["topmarks"] = {
      //"onedotup",
      //"twodotsup",
      //"three_dots",
      "fathatanidgham",
      "dammatanidgham",
      "fatha",
      "damma",
      "shadda",
      "headkhah",
      "sukun",
      "dammatan",
      "maddahabove",
      "fathatan",
      "smallalef",
      "smallalef.joined",
      "meemiqlab",
      "smallhighyeh",
      "smallhighwaw",
      "wasla",
      "hamzaabove",
      "hamzaabove.joined",
      //"hamzaabove.lamalef",
      "smallhighroundedzero",
      "rectangularzero",
      "smallhighseen",
      "smallhighnoon",
      "roundedfilledhigh",
      "hamzaabove.joined",
      "maddawaajib",
      "inverteddamma"        
    };

    classes["lowmarks"] = {
      //"onedotdown",
      //"twodotsdown",
      "kasratanidgham",
      "kasra",
      "kasratan",
      "hamzabelow",
      "roundedfilledlow",
      "smalllowmeem",
      "subalef",
      "smalllownoon"
    };

    classes["kasras"] = {
        "kasratanidgham",
        "kasra",
        "kasratan"
    };

    classes["waqfmarks"] = {
        "takhallus",
        "waqf.lam",
        "waqf.jeem",
        "waqf.sad",
        "waqf.qif",
        "sakta",
        "sajda",
        "waqf.meem",
        "waqf.smallhighthreedots",
        "smallhightah",
        "smallhighsad",
        "smallhighain",
        "smallhighqaf",
        "smallhighzain",
        "waqfa",
        "meemiqlab",
    };

    classes["waqfmarksfina"] = classes["waqfmarks"];
    classes["waqfmarksfina"].remove("sajda");
    classes["waqfmarksaya"] = classes["waqfmarks"];

    classes["dotmarks"] = {
        "onedotup",
        "onedotdown",
        "twodotsup",
        "twodotsdown",
        "three_dots"
    };

    classes["topdotmarks"] = {
        "onedotup",
        "twodotsup",
        "three_dots"
    };

    classes["downdotmarks"] = {
        "onedotdown",
        "twodotsdown"
    };

    classes["digits"] = {
        "zeroindic",
        "oneindic",
        "twoindic",
        "threeindic",
        "fourindic",
        "fiveindic",
        "sixindic",
        "sevenindic",
        "eightindic",
        "nineindic"
    };

    initchar = {
      "behshape" ,
      //"teh" ,
      //"tehmarbuta" ,
      //"theh" ,
      //"jeem" ,
      "hah" ,
      //"khah" ,
      //"dal" ,
      //"thal" ,
      //"reh" ,
      //"zain" ,
      "seen" ,
      //"sheen" ,
      "sad" ,
      //"dad" ,
      "tah" ,
      //"zah" ,
      "ain" ,
      //"ghain" ,
      "fehshape" ,
      //"qaf" ,
      "kaf" ,
      "lam" ,
      "meem" ,
      //"noon" ,
      "heh" ,
      //"waw" ,
      //"yeh" ,
      //"yehwithoutdots" ,
      //"alefmaksura"
    };

    medichar = {
      //"alef" ,
      "behshape" ,
      //"teh" ,
      //"tehmarbuta" ,
      //"theh" ,
      //"jeem" ,
      "hah" ,
      //"khah" ,
      //"dal" ,
      //"thal" ,
      //"reh" ,
      //"zain" ,
      "seen" ,
      //"sheen" ,
      "sad" ,
      //"dad" ,
      "tah" ,
      //"zah" ,
      "ain" ,
      //"ghain" ,
      "fehshape" ,
      //"qaf" ,
      "kaf" ,
      "lam" ,
      "meem" ,
      //"noon" ,
      "heh" ,
      //"waw" ,
      //"yeh" ,
      //"yehwithoutdots" ,
      //"alefmaksura"
    };

    addchars();

    generateGlyphs();

    //Expandable glyphs
    //Alef
    layout->expandableGlyphs["alef.isol"] = { 20,-2,0,0 };
    layout->expandableGlyphs["alef.fina"] = { 20.0,-2,20,-0.5 };


    //Behshape
    layout->expandableGlyphs["behshape.isol"] = { 2,-2,0,0 };
    layout->expandableGlyphs["behshape.isol.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["behshape.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["behshape.init.beforenoon"] = { 20,-1,0,0 };
    layout->expandableGlyphs["behshape.medi"] = { 20,-1,20,-1 };
    layout->expandableGlyphs["behshape.medi.beforeseen"] = { 20,-1,20,-1 };
    layout->expandableGlyphs["behshape.medi.beforereh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["behshape.medi.beforenoon"] = { 20,-0.5,20,-0.5 };
    layout->expandableGlyphs["behshape.medi.expa"] = { 20,-1,20,-1 };
    layout->expandableGlyphs["behshape.medi.beforeyeh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["behshape.fina"] = { 0.0,0.0,20,-0.5 };
    layout->expandableGlyphs["behshape.fina.expa"] = { 20,0,20,-0.5 };

    //Seen
    layout->expandableGlyphs["seen.isol.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["seen.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["seen.medi"] = { 20,-0.5,20,-0.5 };
    layout->expandableGlyphs["seen.medi.beforeyeh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["seen.medi.beforereh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["seen.fina"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["seen.fina.expa"] = { 20,0,0,0 };


    //Lam
    layout->expandableGlyphs["lam.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["lam.medi"] = { 20,-0.5,20,-0.5 };
    layout->expandableGlyphs["lam.medi.afterkaf"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["lam.medi.beforeyeh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["lam.medi.beforeheh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["lam.medi.laf"] = { 0.0,0.0,20,-0.5 };
    layout->expandableGlyphs["lam.fina"] = { 0.0,0.0,20,-0.5 };

    //Meem
    layout->expandableGlyphs["meem.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["meem.medi"] = { 20,-0.5,20,-0.5 };
    layout->expandableGlyphs["meem.medi.afterhah"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["meem.medi.beforeyeh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["meem.fina"] = { 0.0,0.0,20,-0.5 };
    layout->expandableGlyphs["meem.fina.ii"] = { 0.0,0.0,20,-0.5 };
    layout->expandableGlyphs["meem.fina.basmala"] = { 0,0,20,-0.5 };

    //Noon
    layout->expandableGlyphs["noon.isol.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["noon.fina"] = { 0.0,0.0,20,-0.5 };
    layout->expandableGlyphs["noon.fina.expa"] = { 20,0,20,-0.5 };
    layout->expandableGlyphs["noon.fina.expa.afterbeh"] = { 20,0,0,0 };
    layout->expandableGlyphs["noon.fina.basmala"] = { 20,0,20,-0.5 };

    //Qaf
    layout->expandableGlyphs["qaf.isol.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["qaf.fina.expa"] = { 20,0,0,0 };

    //Feh
    layout->expandableGlyphs["feh.isol"] = { 7,-1,0,0 };
    layout->expandableGlyphs["feh.isol.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["fehshape.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["fehshape.medi"] = { 20,-0.5,20,-0.5 };
    layout->expandableGlyphs["fehshape.medi.beforeyeh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["feh.fina"] = { 20,-0.5,20,-0.5 };
    layout->expandableGlyphs["feh.fina.expa"] = { 20,0,0,0 };

    //Hah
    layout->expandableGlyphs["hah.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["hah.medi"] = { 20,-0.5,20,-0.5 };
    layout->expandableGlyphs["hah.medi.ii"] = { 20,-1,20,-0.5 };
    layout->expandableGlyphs["hah.medi.afterbeh"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["hah.medi.lam_hah"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["hah.medi.aftermeem"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["hah.medi.afterfeh"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["hah.medi.beforeyeh"] = { 0,0,20,-1 };
    layout->expandableGlyphs["hah.fina"] = { 0.0,0.0,20,-0.5 };

    //Dal 
    layout->expandableGlyphs["dal.fina"] = { 0.0,0.0,20,-0.5 };

    //Reh 
    layout->expandableGlyphs["reh.fina"] = { 0.0,0.0,20,-0.5 };

    //Heh
    layout->expandableGlyphs["heh.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["heh.medi"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["heh.medi.beforeyeh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["heh.fina"] = { 0.0,0.0,20,-0.5 };

    //Sad

    layout->expandableGlyphs["sad.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["sad.medi"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["sad.isol.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["sad.fina.expa"] = { 20,0,0,0 };

    //Ain

    layout->expandableGlyphs["ain.init"] = { 20,-0.5,0,0 };
    layout->expandableGlyphs["ain.medi"] = { 20,-0.5,20,-0.5 };
    layout->expandableGlyphs["ain.medi.beforeyeh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["ain.fina"] = { 0,0,20,-0.5 };

    //Tah
    layout->expandableGlyphs["tah.init"] = { 20,-0.4,0,0 };
    layout->expandableGlyphs["tah.medi"] = { 20,-0.4,20,-0.5 };
    layout->expandableGlyphs["tah.medi.beforeyeh"] = { 0,0,20,-0.4 };

    //Yeh
    layout->expandableGlyphs["alefmaksura.isol"] = { 2,-2,0,0 };
    layout->expandableGlyphs["yehshape.isol"] = { 2,-2,0,0 };
    layout->expandableGlyphs["alefmaksura.isol.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["yehshape.isol.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["yehshape.fina.afterbeh.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["yehshape.fina.ii.expa"] = { 20,0,0,0 };
    layout->expandableGlyphs["yehshape.fina.expa"] = { 20,0,0,0 };

    //Kaf
    layout->expandableGlyphs["kaf.isol"] = { 20,-2,0,0 };
    layout->expandableGlyphs["kaf.init"] = { 20,-2,0,0 };
    layout->expandableGlyphs["kaf.init.ascent"] = { 20,-2,0,0 };
    layout->expandableGlyphs["kaf.init.ii"] = { 20,-2.5,0,0 };
    layout->expandableGlyphs["kaf.medi"] = { 20,-1,20,-1 };
    layout->expandableGlyphs["kaf.medi.beforelam"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["kaf.medi.beforemeem"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["kaf.medi.beforeyeh"] = { 0,0,20,-0.5 };
    layout->expandableGlyphs["kaf.medi.ii"] = { 20,-2,20,-0.5 };
    layout->expandableGlyphs["kaf.fina"] = { 0.0,0.0,20,-0.5 };
    layout->expandableGlyphs["kaf.fina.expa"] = { 20,0,20,-0.5 };
    layout->expandableGlyphs["kaf.fina.afterlam.expa"] = { 20,0,0,0 };


    //Marks
    layout->expandableGlyphs["fatha"] = { 10,-1.5,0,0 };
    layout->expandableGlyphs["kasra"] = layout->expandableGlyphs["fatha"];
    layout->expandableGlyphs["space"] = { 20,-0.8,0.0,0.0 };

    //Basmala
    layout->expandableGlyphs["behshape.medi.basmala"] = { 20,-1,0,0 };
    layout->expandableGlyphs["seen.medi.basmala"] = { 20,-1,0,0 };


  }

  CalcAnchor  IndoPak::getanchorCalcFunctions(QString functionName, Subtable* subtable) {
    if (functionName == "defaultmarkabovemark") {
      return Defaultmarkabovemark(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "defaultopmarkanchor") {
      return Defaultopmarkanchor(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "defaultmarkbelowmark") {
      return Defaultmarkbelowmark(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "defaullowmarkanchor") {
      return Defaullowmarkanchor(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "waqffinabasemark") {
      return Waqffinabasemark(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "waqffinamark") {
      return Waqffinamark(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "defaultbaseanchorforlow") {
      return Defaulbaseanchorforlow(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "defaulbaseanchorfortop") {
      return Defaulbaseanchorfortop(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "joinedsmalllettersbaseanchor") {
      return Joinedsmalllettersbaseanchor(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "waqfbasebelow") {
      return Waqfbasebelow(*this, *(MarkBaseSubtable*)(subtable));
    }
    else if (functionName == "waqfmarkabove") {
      return Waqfmarkabove(*this, *(MarkBaseSubtable*)(subtable));
    }
  }

  Lookup* IndoPak::getLookup(QString lookupName) {

    if (lookupName == "defaultmarkpositioncpp") {
      return defaultmarkposition();
    }
    else if (lookupName == "waqfmarkpositioning") {
      return waqfMarkPositioning();
    }
    else if (lookupName == "waqfmkmkpositioning") {
      return waqfMkmkPositioning();
    }
    else if (lookupName == "forhamza") {
      return forhamza();
    }
    else if (lookupName == "forheh") {
      return forheh();
    }
    else if (lookupName == "cursivejoin") {
      return cursivejoin();
    }
    else if (lookupName == "cursivejoinrtl") {
      return cursivejoinrtl();
    }
    else if (lookupName == "rehwawcursivecpp") {
      return rehwawcursivecpp();
    }
    else if (lookupName == "defaultdotmarks") {
      return defaultdotmarks();
    }
    else if (lookupName == "pointmarks") {
      return pointmarks();
    }
    else if (lookupName == "defaultmarkdotmarks") {
      return defaultmarkdotmarks();
    }
    else if (lookupName == "defaultmkmk") {
      return defaultmkmk();
    }
    else if (lookupName == "ayanumbers") {
      return ayanumbers();
    }
    else if (lookupName == "ayanumberskern") {
      return ayanumberskern();
    }
    else if (lookupName == "shrinkstretchlt") {
      return shrinkstretchlt();
    }
    else if (lookupName == "forsmallhighwaw") {
      return forsmallhighwaw();
    }
    else if (lookupName == "populatecvxx") {
      return populatecvxx();
    }
    else if (lookupName == "glyphalternates") {
      return glyphalternates();
    }

    return nullptr;
  }

  Lookup* IndoPak::rehwawcursivecpp() {
    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "rehwawcursivecpp";
    lookup->feature = "curs";
    lookup->type = Lookup::cursive;
    lookup->flags = Lookup::Flags::IgnoreMarks; // | Lookup::Flags::RightToLeft;

    int kern = 0;

    class CustomCursiveSubtable : public CursiveSubtable {
    public:
      CustomCursiveSubtable(Lookup* lookup) : CursiveSubtable(lookup) {}

      virtual QPoint calculateEntry(GlyphVis* originalglyph, GlyphVis* extendedglyph, QPoint defaultEntry) {

        QPoint entry = QPoint(extendedglyph->width, 0);

        return entry;

      }

    };


    CursiveSubtable* rehfinaafterbehshape = new CursiveSubtable(lookup);
    lookup->subtables.append(rehfinaafterbehshape);
    rehfinaafterbehshape->name = "rehfinaafterbehshape";
    rehfinaafterbehshape->anchors[glyphs["reh.fina.afterbehshape"].charcode].exit = QPoint(kern, 0);

    CursiveSubtable* rehfinaafterseen = new CursiveSubtable(lookup);
    lookup->subtables.append(rehfinaafterseen);
    rehfinaafterseen->name = "rehfinaafterseen";
    rehfinaafterseen->anchors[glyphs["reh.fina.afterseen"].charcode].exit = QPoint(kern, 0);

    CursiveSubtable* rehisol = new CursiveSubtable(lookup);
    lookup->subtables.append(rehisol);
    rehisol->name = "rehisol";
    rehisol->anchors[glyphs["reh.isol"].charcode].exit = QPoint(kern, 0);

    CursiveSubtable* wawisol = new CursiveSubtable(lookup);
    lookup->subtables.append(wawisol);
    wawisol->name = "wawisol";
    wawisol->anchors[glyphs["waw.isol"].charcode].exit = QPoint(kern, 0);

    CursiveSubtable* rehfina = new CustomCursiveSubtable(lookup);
    lookup->subtables.append(rehfina);
    rehfina->name = "rehfina";

    auto glyphcodes = m_layout->classtoUnicode("^reh.fina$|^reh.fina[.]added");

    for (auto glyphcode : glyphcodes) {
      rehfina->anchors[glyphcode].exit = QPoint(kern, 0);
    }


    CursiveSubtable* wawfina = new CustomCursiveSubtable(lookup);
    lookup->subtables.append(wawfina);
    wawfina->name = "wawfina";

    glyphcodes = m_layout->classtoUnicode("^waw.fina$|^waw.fina[.]added");

    for (auto glyphcode : glyphcodes) {
      wawfina->anchors[glyphcode].exit = QPoint(kern, 0);
    }

    glyphcodes = m_layout->classtoUnicode("[.]isol|[.]init"); //"((?<!reh|waw)[.]isol)|init"

    for (auto glyphcode : glyphcodes) {

      QString glyphName = m_layout->glyphNamePerCode[glyphcode];
      auto& glyph = glyphs[glyphName];

      rehisol->anchors[glyphcode].entry = QPoint(glyph.width, 0);
      wawisol->anchors[glyphcode].entry = QPoint(glyph.width, 0);
      rehfina->anchors[glyphcode].entry = QPoint(glyph.width, 0);
      wawfina->anchors[glyphcode].entry = QPoint(glyph.width, 0);
      rehfinaafterbehshape->anchors[glyphcode].entry = QPoint(glyph.width, 0);
      rehfinaafterseen->anchors[glyphcode].entry = QPoint(glyph.width, 0);

    }

    return lookup;

  }
  Lookup* IndoPak::cursivejoin() {

    auto lookup = new Lookup(m_layout);
    lookup->name = "cursivejoin";
    lookup->feature = "curs";
    lookup->type = Lookup::cursive;
    lookup->flags = Lookup::Flags::IgnoreMarks;


    for (auto it = entryAnchors.constBegin(); it != entryAnchors.constEnd(); ++it) {
      QString cursiveName = it.key();
      auto entries = it.value();
      auto exits = exitAnchors[cursiveName];

      CursiveSubtable* newsubtable = new CursiveSubtable(lookup);
      lookup->subtables.append(newsubtable);
      newsubtable->name = cursiveName;

      for (auto anchor = entries.constBegin(); anchor != entries.constEnd(); ++anchor) {
        newsubtable->anchors[anchor.key()].entry = anchor.value();
      }

      for (auto anchor = exits.constBegin(); anchor != exits.constEnd(); ++anchor) {
        newsubtable->anchors[anchor.key()].exit = anchor.value();
      }
    }


    return lookup;

  }
  Lookup* IndoPak::cursivejoinrtl() {
    auto lookup = new Lookup(m_layout);
    lookup->name = "cursivejoinrtl";
    lookup->feature = "curs";
    lookup->type = Lookup::cursive;
    lookup->flags = Lookup::Flags::IgnoreMarks | Lookup::Flags::RightToLeft;

    for (auto it = entryAnchorsRTL.constBegin(); it != entryAnchorsRTL.constEnd(); ++it) {
      QString cursiveName = it.key();
      auto entries = it.value();
      auto exits = exitAnchorsRTL[cursiveName];

      CursiveSubtable* newsubtable = new CursiveSubtable(lookup);
      lookup->subtables.append(newsubtable);
      newsubtable->name = cursiveName;

      for (auto anchor = entries.constBegin(); anchor != entries.constEnd(); ++anchor) {
        newsubtable->anchors[anchor.key()].entry = anchor.value();
      }

      for (auto anchor = exits.constBegin(); anchor != exits.constEnd(); ++anchor) {
        newsubtable->anchors[anchor.key()].exit = anchor.value();
      }
    }

    return lookup;

  }
  Lookup* IndoPak::defaultmarkposition() {
    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "defaultmarkposition";
    lookup->feature = "mark";
    lookup->type = Lookup::mark2base;
    lookup->flags = 0;

    auto topmarks = classes["topmarks"];
    topmarks.remove("smallalef.joined");
    topmarks.remove("smallhighyeh");
    topmarks.remove("smallhighwaw");
    topmarks.remove("smallhighnoon");
    topmarks.remove("roundedfilledhigh");
    topmarks.remove("hamzaabove");
    topmarks.remove("hamzaabove.small");
    topmarks.remove("hamzaabove.joined");
    topmarks.remove("wasla");
    topmarks.remove("maddahabove");
    topmarks.remove("smallhighseen");
    topmarks.remove("shadda");
    topmarks.remove("meemiqlab");


    auto lowmarks = classes["lowmarks"];
    lowmarks.remove("hamzabelow");
    lowmarks.remove("hamzaabove.joined");


    //meem.fina.afterkaf

    MarkBaseSubtable* newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "meemfinaafterkaf";
    newsubtable->base = { "meem.fina.afterkaf" };
    newsubtable->classes["sukun"].mark = { "sukun" };
    newsubtable->classes["sukun"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["sukun"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //tah
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "tah";
    newsubtable->base = { "^tah" };

    newsubtable->classes["fathadamma"].mark = { "fatha", "damma","shadda", "sukun" };
    newsubtable->classes["fathadamma"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["fathadamma"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    newsubtable->classes["fathatandammatan"].mark = { "fathatan", "dammatan" };
    newsubtable->classes["fathatandammatan"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["fathatandammatan"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    newsubtable->classes["idgham"].mark = { "fathatanidgham", "dammatanidgham" };
    newsubtable->classes["idgham"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["idgham"].markfunction = Defaultopmarkanchor(*this, *newsubtable);


    //topmarks
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "topmarks";
    newsubtable->base = { "bases" };
    newsubtable->classes["topmarks"].mark = topmarks;
    newsubtable->classes["topmarks"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["topmarks"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //lowmarks
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "lowmarks";
    newsubtable->base = { "bases" };
    newsubtable->classes["lowmarks"].mark = lowmarks;
    newsubtable->classes["lowmarks"].basefunction = Defaulbaseanchorforlow(*this, *newsubtable);
    newsubtable->classes["lowmarks"].markfunction = Defaullowmarkanchor(*this, *newsubtable);

    //smallletters
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "smallletters";
    newsubtable->base = { "bases" };
    newsubtable->classes["smallletters"].mark = { "smallalef.joined","smallhighwaw" };
    newsubtable->classes["smallletters"].basefunction = Defaulbaseanchorforsmallalef(*this, *newsubtable);
    newsubtable->classes["smallletters"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //joinedmarks
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "joinedmarks";
    newsubtable->base = { "bases" };
    newsubtable->classes["hamzaabove"].mark = { "hamzaabove.joined" };
    newsubtable->classes["hamzaabove"].basefunction = Defaulbaseanchorforsmallalef(*this, *newsubtable);
    newsubtable->classes["hamzaabove"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //hamzabelow.joined
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "hamzabelow.joined";
    newsubtable->base = { ".init|.medi" };
    newsubtable->classes["hamzabelow"].mark = { "hamzabelow.joined" };
    newsubtable->classes["hamzabelow"].basefunction = [this](QString glyphName, QString className, QPoint adjust, double lefttatweel, double righttatweel) -> QPoint {
      GlyphVis* curr = &glyphs[glyphName];

      if (lefttatweel != 0.0 || righttatweel != 0.0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = lefttatweel;
        parameters.righttatweel = righttatweel;

        curr = curr->getAlternate(parameters);
      }

      int width = 100 + adjust.x();
      int height = curr->depth + adjust.y();

      return QPoint(width, height);
      };
    newsubtable->classes["hamzabelow"].markfunction = [this](QString glyphName, QString className, QPoint adjust, double lefttatweel, double righttatweel) -> QPoint {
      GlyphVis* curr = &glyphs[glyphName];      
      return QPoint(adjust.x(), adjust.y() + 100 + curr->height);
      };

    //default
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "smallhighyeh";
    newsubtable->base = { "bases" }; // TODO minimize

    newsubtable->classes["smallhighyeh"].mark = { "smallhighyeh" };
    newsubtable->classes["smallhighyeh"].basefunction = Defaulbaseanchorforsmallalef(*this, *newsubtable);
    newsubtable->classes["smallhighyeh"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //shadda

    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "shadda";
    newsubtable->base = { "bases" };
    newsubtable->classes["shadda"].mark = { "shadda" };
    newsubtable->classes["shadda"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["shadda"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //maddahabove

    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "maddahabove";
    newsubtable->base = { "bases" };
    newsubtable->classes["maddahabove"].mark = { "maddahabove" };
    newsubtable->classes["maddahabove"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["maddahabove"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //hamzaabove

    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "hamzaabove";
    newsubtable->base = { "alef|waw|yehshape|behshape" };
    newsubtable->classes["hamzaabove"].mark = { "hamzaabove","hamzaabove.small" };
    newsubtable->classes["hamzaabove"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["hamzaabove"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //roundedfilledhigh

    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "roundedfilledhigh";
    newsubtable->base = { "alef[.]isol.*|meem[.]init.*" };
    newsubtable->classes["roundedfilledhigh"].mark = { "roundedfilledhigh" };
    newsubtable->classes["roundedfilledhigh"].basefunction = Defaulbaseanchorforsmallalef(*this, *newsubtable);
    newsubtable->classes["roundedfilledhigh"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //smallhighnoon
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "smallhighnoon";
    newsubtable->base = { "behshape[.]init.*" };
    newsubtable->classes["smallhighnoon"].mark = { "smallhighnoon" };
    newsubtable->classes["smallhighnoon"].basefunction = Defaulbaseanchorforsmallalef(*this, *newsubtable);
    newsubtable->classes["smallhighnoon"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //smallhighseen
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "smallhighseen";
    newsubtable->base = { "sad[.]medi|^alef.fina|^heh.fina|^lam.fina|^noon.fina" };
    newsubtable->classes["smallhighseen"].mark = { "smallhighseen" };
    newsubtable->classes["smallhighseen"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["smallhighseen"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //hamzaabove.lamalef
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "hamzaabove.lamalef";
    newsubtable->base = { "lam.init.lamalef","^lam.medi.laf" };
    newsubtable->classes["hamzaabove"].mark = { "hamzaabove.lamalef" };
    newsubtable->classes["hamzaabove"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["hamzaabove"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //hamzabelow
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "hamzabelow";
    newsubtable->base = { "^alef[.]" };
    newsubtable->classes["hamzabelow"].mark = { "hamzabelow" };
    newsubtable->classes["hamzabelow"].basefunction = Defaulbaseanchorforlow(*this, *newsubtable);
    newsubtable->classes["hamzabelow"].markfunction = Defaullowmarkanchor(*this, *newsubtable);

    //wasla
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "wasla";
    newsubtable->base = { "^alef[.]" };
    newsubtable->classes["wasla"].mark = { "wasla" };
    newsubtable->classes["wasla"].basefunction = Defaulbaseanchorfortop(*this, *newsubtable);
    newsubtable->classes["wasla"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    //takhallus
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "takhallus";
    newsubtable->base = { "bases" };
    newsubtable->classes["takhallus"].mark = { "takhallus" };
    newsubtable->classes["takhallus"].basefunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];

      auto disp = 1000;

      int width = adjust.x() + curr.width / 2;
      int height = disp  + adjust.y();

      return QPoint(width, height);
      };
    newsubtable->classes["takhallus"].markfunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];
      return QPoint(adjust.x() + curr.width / 2, adjust.y());
      };

    //aya waqf
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "waqfsubtable";
    newsubtable->base = { "aya" };
    newsubtable->classes["waqfmarksaya"].mark = { "waqfmarksaya" };
    newsubtable->classes["waqfmarksaya"].basefunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];

      int width = curr.width / 2 + adjust.x();
      int height = 50 + curr.height + adjust.y();

      return QPoint(width, height);
      };
    newsubtable->classes["waqfmarksaya"].markfunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];
      return QPoint(adjust.x() + curr.width / 2, adjust.y());
      };


    //fina waqf
    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "waqfsubtable";
    newsubtable->base = { "waqfbase.isol","disputedeoa" };
    newsubtable->classes["waqfmarksfina"].mark = { "waqfmarksfina" };
    newsubtable->classes["waqfmarksfina"].basefunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];

      auto disp = glyphName == "waqfbase.isol" ? 330 : 30;

      int width = adjust.x() + curr.width / 2;
      int height = disp + curr.height + adjust.y();

      return QPoint(width, height);
      };
    newsubtable->classes["waqfmarksfina"].markfunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];
      int width = adjust.x() + curr.width / 2;
      int height = adjust.y();
      return QPoint(width, height);
      };

    return lookup;
  }
  Lookup* IndoPak::waqfMarkPositioning() {
    return nullptr;
  }
  Lookup* IndoPak::waqfMkmkPositioning() {

    auto endWordClass = "space|isol|fina|smallwaw|smallyeh";
    auto waqfCodes = m_layout->classtoUnicode("waqfmarksfina");
    auto endWordCodes = m_layout->classtoUnicode(endWordClass);
    auto ayaCodes = m_layout->classtoUnicode("aya");


    auto addLookup = [this, &waqfCodes, &endWordCodes, &endWordClass](int numMarks) -> void {
      Lookup* lookup = new Lookup(m_layout);
      lookup->name = QString("waqfmkmkpositioning.l%1").arg(numMarks);
      lookup->feature = "";
      lookup->type = Lookup::chainingpos;
      lookup->flags = Lookup::Flags::UseMarkFilteringSet;
      lookup->markGlyphSetIndex = m_layout->addMarkSet(waqfCodes.toList());
      m_layout->addLookup(lookup);

      QVector<QVector<quint16>> waqfSequences{ {} };

      for (int i = 1; i <= numMarks; i++) {
        auto tempWaqfSequences = waqfSequences;
        waqfSequences.clear();
        for (auto code : waqfCodes) {
          for (auto seq : tempWaqfSequences) {
            if (!seq.contains(code)) {
              seq.append(code);
              waqfSequences.append(seq);
            }
          }
        }
      }

      int subtableNum = 0;

      for (auto& seq : waqfSequences) {

        int maxWidth = 0;

        for (auto glyphCode : seq) {
          GlyphVis* curr = m_layout->getGlyph(glyphCode);

          maxWidth = std::max((int)curr->width, maxWidth);
        }

        ChainingSubtable* subtable = new ChainingSubtable(lookup);
        lookup->subtables.append(subtable);
        subtable->name = QString("subtable%1").arg(++subtableNum);
        subtable->compiledRule = ChainingSubtable::CompiledRule();
        subtable->compiledRule.input = { endWordCodes,{seq[0]} };

        auto lkernName = QString("lkern%1").arg(subtableNum);
        auto lmarkName = QString("lmark%1").arg(subtableNum);

        subtable->compiledRule.lookupRecords.append({ 0,lkernName });
        subtable->compiledRule.lookupRecords.append({ 1,lmarkName });

        for (quint16 i = 2; i <= numMarks; i++) {
          subtable->compiledRule.input.append({ seq[i - 1] });
          subtable->compiledRule.lookupRecords.append({ i,QString("waqfmkmkpositioning.lmkmk") });
        }

        Lookup* sublookup = new Lookup(m_layout);
        sublookup->name = lookup->name + "." + lkernName;
        sublookup->feature = "";
        sublookup->type = Lookup::singleadjustment;
        m_layout->addLookup(sublookup);

        SingleAdjustmentSubtable* singleadjsubtable = new SingleAdjustmentSubtable(sublookup);
        sublookup->subtables.append(singleadjsubtable);

        singleadjsubtable->name = sublookup->name;

        for (auto endWordCode : endWordCodes) {
          singleadjsubtable->singlePos[endWordCode] = { (qint16)maxWidth ,0,(qint16)maxWidth,0 };
        }

        sublookup = new Lookup(m_layout);
        sublookup->name = lookup->name + "." + lmarkName;
        sublookup->feature = "";
        sublookup->type = Lookup::mark2base;
        m_layout->addLookup(sublookup);



        auto markGlyph = m_layout->getGlyph(seq[0]);

        int waqfKern = 100 + markGlyph->width + (maxWidth - markGlyph->width) / 2;
        int waqfHeight = seq.size() == 1 ? 200 : seq.size() == 2 ? 300 : seq.size() == 3 ? 400 : 500;

        auto basefunction = [this, waqfKern, waqfHeight](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
          GlyphVis& curr = glyphs[glyphName];

          int height = waqfHeight;
          auto width = -waqfKern; // curr.bbox.llx;


          width = width + adjust.x();
          height = height + adjust.y();

          return QPoint(width, height);
          };

        auto markfunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
          GlyphVis& curr = glyphs[glyphName];

          int height = 0;
          int width = 0;


          width = width + adjust.x();
          height = height + adjust.y();

          return QPoint(width, height);
          };

        MarkBaseSubtable* markSubtabe = new MarkBaseSubtable(sublookup);
        sublookup->subtables.append(markSubtabe);

        markSubtabe->name = "subtable1";
        auto markName = m_layout->glyphNamePerCode[seq[0]];
        markSubtabe->base = { endWordClass };


        markSubtabe->classes["waqfmarksfina"].mark = { markName };
        markSubtabe->classes["waqfmarksfina"].basefunction = basefunction;
        markSubtabe->classes["waqfmarksfina"].markfunction = markfunction;

      }



      };

    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "waqfmkmkpositioning";
    lookup->feature = "mkmk";
    lookup->type = Lookup::chainingpos;
    lookup->flags = Lookup::Flags::UseMarkFilteringSet;
    lookup->markGlyphSetIndex = m_layout->addMarkSet(waqfCodes.toList());

    /*
    auto subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = QString("waqffina4");
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    subtable->compiledRule.input = { endWordCodes,waqfCodes,waqfCodes,waqfCodes,waqfCodes };
    subtable->compiledRule.lookupRecords.append({ 0,QString("l4") });
    addLookup(4);
    */

    /*
    auto subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = QString("waqffina3");
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    subtable->compiledRule.input = { endWordCodes ,waqfCodes ,waqfCodes ,waqfCodes };
    subtable->compiledRule.lookupRecords.append({ 0,QString("l3") });
    addLookup(3);*/

    auto subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = QString("waqffina2");
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    subtable->compiledRule.input = { endWordCodes,waqfCodes,waqfCodes };
    subtable->compiledRule.lookupRecords.append({ 0,QString("l2") });
    addLookup(2);

    subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = QString("waqffina1");
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    subtable->compiledRule.input = { endWordCodes,waqfCodes };
    subtable->compiledRule.lookupRecords.append({ 0,QString("l1") });
    addLookup(1);

    Lookup* sublookup = new Lookup(m_layout);
    sublookup->name = "waqfmkmkpositioning.lmkmk";
    sublookup->feature = "";
    sublookup->type = Lookup::mark2mark;
    m_layout->addLookup(sublookup);

    auto basefunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];

      int height = 0; // (int)curr.height + spacebasetotopmark;
      int width = curr.width / 2;


      width = width + adjust.x();
      height = height + adjust.y();

      return QPoint(width, height);
      };

    auto markfunction = [this](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];

      int height = (int)curr.height + 50;
      int width = curr.width / 2;


      width = width + adjust.x();
      height = height + adjust.y();

      return QPoint(width, height);
      };

    auto markSubtabe = new MarkBaseSubtable(sublookup);
    sublookup->subtables.append(markSubtabe);

    markSubtabe->name = "subtable1";
    markSubtabe->base = { "waqfmarksfina" };


    markSubtabe->classes["waqfmarksfina"].mark = { "waqfmarksfina" };
    markSubtabe->classes["waqfmarksfina"].basefunction = basefunction;
    markSubtabe->classes["waqfmarksfina"].markfunction = markfunction;

    return lookup;
  }
  Lookup* IndoPak::defaultdotmarks() {
    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "defaultdotmarks";
    lookup->feature = "mark";
    lookup->type = Lookup::mark2base;
    lookup->flags = 0;


    MarkBaseSubtable* newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "onedotup";
    newsubtable->base = { "^behshape|^hah|^fehshape|^dal|^reh|^sad|^tah|^ain|^noon|^feh[.]" };
    newsubtable->classes["onedotup"].mark = { "onedotup" };
    newsubtable->classes["onedotup"].basefunction = Defaulbaseanchorfortopdots(*this, *newsubtable);
    newsubtable->classes["onedotup"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "twodotsup";
    newsubtable->base = { "^behshape|^fehshape|^heh|^qaf" };
    newsubtable->classes["twodotsup"].mark = { "twodotsup" };
    newsubtable->classes["twodotsup"].basefunction = Defaulbaseanchorfortopdots(*this, *newsubtable);
    newsubtable->classes["twodotsup"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "three_dots";
    newsubtable->base = { "^behshape|^seen" };
    newsubtable->classes["three_dots"].mark = { "three_dots" };
    newsubtable->classes["three_dots"].basefunction = Defaulbaseanchorfortopdots(*this, *newsubtable);
    newsubtable->classes["three_dots"].markfunction = Defaultopmarkanchor(*this, *newsubtable);

    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "onedotdown";
    newsubtable->base = { "^behshape|^hah" };
    newsubtable->classes["onedotdown"].mark = { "onedotdown" };
    newsubtable->classes["onedotdown"].basefunction = Defaulbaseanchorforlowdots(*this, *newsubtable);
    newsubtable->classes["onedotdown"].markfunction = Defaullowmarkanchor(*this, *newsubtable);

    newsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(newsubtable);
    newsubtable->name = "twodotsdown";
    newsubtable->base = { "^behshape" };
    newsubtable->classes["twodotsdown"].mark = { "twodotsdown" };
    newsubtable->classes["twodotsdown"].basefunction = Defaulbaseanchorforlowdots(*this, *newsubtable);
    newsubtable->classes["twodotsdown"].markfunction = Defaullowmarkanchor(*this, *newsubtable);

    return lookup;

  }
  Lookup* IndoPak::defaultmkmk() {

    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "defaultmkmk";
    lookup->feature = "mkmk";
    lookup->type = Lookup::mark2mark;
    lookup->flags = 0;

    MarkBaseSubtable* subtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(subtable);

    subtable->name = "defaultmkmktop";
    subtable->base = { "hamzaabove","hamzaabove.small","hamzaabove.joined","hamzaabove.lamalef", "shadda","smallalef","smallalef.joined","smallhighseen", "smallhighwaw","smallhighyeh","smallhighnoon"};

    subtable->classes["topmarks"].mark = { "topmarks" };
    subtable->classes["topmarks"].basefunction = Defaultmarkabovemark(*this, *subtable);
    subtable->classes["topmarks"].markfunction = Defaultopmarkanchor(*this, *subtable);

    subtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(subtable);

    subtable->name = "defaultmkmkbottom";
    subtable->base = { "hamzabelow","hamzabelow.joined", "hamzaabove.joined","smallhighyeh" };

    subtable->classes["lowmarks"].mark = { "lowmarks" };
    subtable->classes["lowmarks"].basefunction = Defaulbaseanchorforlow(*this, *subtable);
    subtable->classes["lowmarks"].markfunction = Defaullowmarkanchor(*this, *subtable);
    
    subtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(subtable);

    subtable->name = "sukunmaddahabove";
    subtable->base = { "sukun","inverteddamma","fatha"};

    subtable->classes["maddahabove"].mark = { "maddahabove","maddawaajib"};
    subtable->classes["maddahabove"].basefunction = Defaultmarkabovemark(*this, *subtable);
    subtable->classes["maddahabove"].markfunction = Defaultopmarkanchor(*this, *subtable);

    subtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(subtable);

    subtable->name = "noonmeemiqlab";
    subtable->base = { "onedotup" };

    subtable->classes["meemiqlab"].mark = { "meemiqlab" };
    subtable->classes["meemiqlab"].basefunction = Defaultmarkabovemark(*this, *subtable);
    subtable->classes["meemiqlab"].markfunction = Defaultopmarkanchor(*this, *subtable);

    subtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(subtable);

    subtable->name = "meemiqlabsukun";
    subtable->base = { "meemiqlab" };

    subtable->classes["sukun"].mark = { "sukun" };
    subtable->classes["sukun"].basefunction = Defaultmarkabovemark(*this, *subtable);
    subtable->classes["sukun"].markfunction = Defaultopmarkanchor(*this, *subtable);

    subtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(subtable);

    subtable->name = "defaultmkmksmalllowmeem";
    subtable->base = { "kasratan" };

    subtable->classes["smalllowmeem"].mark = { "smalllowmeem" };
    subtable->classes["smalllowmeem"].basefunction = Defaultmarkbelowmark(*this, *subtable);
    subtable->classes["smalllowmeem"].markfunction = Defaullowmarkanchor(*this, *subtable);

    return lookup;

  }
  Lookup* IndoPak::defaultmarkdotmarks() {
    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "defaultmarkdotmarkstop";
    lookup->feature = "mkmk";
    lookup->type = Lookup::mark2mark;
    lookup->flags = 0;


    MarkBaseSubtable* topsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(topsubtable);

    topsubtable->name = "defaultmarkdotmarkstop";
    topsubtable->base = { "topdotmarks" };

    auto basetopfunction = [this, topsubtable](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];


      int width = curr.width * 0.5;
      int height = (int)curr.height + 80;



      width = width + adjust.x();
      height = height + adjust.y();

      return QPoint(width, height);
      };

    auto topmarks = classes["topmarks"];
    topmarks.remove("shadda");

    topsubtable->classes["topmarks"].mark = topmarks;
    topsubtable->classes["topmarks"].basefunction = basetopfunction;
    topsubtable->classes["topmarks"].markfunction = Defaultopmarkanchor(*this, *topsubtable);

    topsubtable->classes["shadda"].mark = { "shadda" };
    topsubtable->classes["shadda"].basefunction = basetopfunction;
    topsubtable->classes["shadda"].markfunction = Defaultopmarkanchor(*this, *topsubtable);

    m_layout->addLookup(lookup);

    lookup = new Lookup(m_layout);
    lookup->name = "defaultmarkdotmarksbottom";
    lookup->feature = "mkmk";
    lookup->type = Lookup::mark2mark;
    lookup->flags = 0;
    lookup->setGlyphSet({ "downdotmarks","lowmarks" });

    MarkBaseSubtable* bottomsubtable = new MarkBaseSubtable(lookup);
    lookup->subtables.append(bottomsubtable);

    bottomsubtable->name = "defaultmarkdotmarksbottom";
    bottomsubtable->base = { "downdotmarks" };

    auto basedownfunction = [this, bottomsubtable](QString glyphName, QString className, QPoint adjust, double, double) -> QPoint {
      GlyphVis& curr = glyphs[glyphName];

      int depth = -(int)curr.depth + 50;
      int width = curr.width * 0.5;


      width = width + adjust.x();
      depth = depth - adjust.y();

      return QPoint(width, -depth);
      };

    bottomsubtable->classes["lowmarks"].mark = { "lowmarks" };
    bottomsubtable->classes["lowmarks"].basefunction = basedownfunction;
    bottomsubtable->classes["lowmarks"].markfunction = Defaullowmarkanchor(*this, *bottomsubtable);


    return lookup;

  }
  Lookup* IndoPak::pointmarks() {

    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "pointmarks";
    lookup->feature = "mark";
    lookup->type = Lookup::chainingpos;
    lookup->flags = 0;

    for (auto pointmark : classes["dotmarks"]) {

      QString sublookupName = pointmark;

      Lookup* sublookup = new Lookup(m_layout);
      sublookup->name = lookup->name + "." + sublookupName;
      sublookup->feature = "";
      sublookup->type = Lookup::mark2base;
      sublookup->flags = 0;

      m_layout->addLookup(sublookup);

      MarkBaseSubtable* marksubtable = new MarkBaseSubtable(sublookup);
      sublookup->subtables.append(marksubtable);

      marksubtable->name = sublookup->name;
      marksubtable->base = { "bases" };

      marksubtable->classes["topmarks"].mark = { "topmarks" };
      marksubtable->classes["topmarks"].basefunction = Defaulbaseanchorfortop(*this, *marksubtable);
      marksubtable->classes["topmarks"].markfunction = Defaultopmarkanchor(*this, *marksubtable);


      marksubtable->classes["lowmarks"].mark = { "lowmarks" };
      marksubtable->classes["lowmarks"].basefunction = Defaulbaseanchorforlow(*this, *marksubtable);
      marksubtable->classes["lowmarks"].markfunction = Defaullowmarkanchor(*this, *marksubtable);


      ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
      lookup->subtables.append(newsubtable);



      newsubtable->name = "pointmarks_" + pointmark;

      newsubtable->compiledRule = ChainingSubtable::CompiledRule();

      newsubtable->compiledRule.backtrack.append({ classtoUnicode("bases") });
      newsubtable->compiledRule.input.append(QSet{ (quint16)glyphs[pointmark].charcode });
      newsubtable->compiledRule.input.append(classtoUnicode("marks"));

      newsubtable->compiledRule.lookupRecords.append({ 1,sublookupName });
    }



    return lookup;


  }

  Lookup* IndoPak::ayanumberskern() {

    short digitheight = 120;
    short digitKern = 40;
    auto& ayaGlyph = glyphs["endofaya"];
    //main lokkup

    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "ayanumberskern";
    lookup->feature = "kern";
    lookup->type = Lookup::chainingpos;
    lookup->flags = 0;

    auto getDigitName = [](int code) {
      switch (code) {
      case 0: return "zeroindic";
      case 1: return "oneindic";
      case 2: return "twoindic";
      case 3: return "threeindic";
      case 4: return "fourindic";
      case 5: return "fiveindic";
      case 6: return "sixindic";
      case 7: return "sevenindic";
      case 8: return "eightindic";
      case 9: return "nineindic";
      default: return "";
      };
      };


    for (int ayaNumber = 286; ayaNumber >= 1; ayaNumber--) {
      if (ayaNumber < 10) {

        Lookup* sublookup = new Lookup(m_layout);
        sublookup->name = QString("ayanumberskern.l%1").arg(ayaNumber);
        sublookup->feature = "";
        sublookup->type = Lookup::singleadjustment;
        m_layout->addLookup(sublookup);

        SingleAdjustmentSubtable* singleadjsubtable = new SingleAdjustmentSubtable(sublookup);
        sublookup->subtables.append(singleadjsubtable);

        singleadjsubtable->name = sublookup->name;


        auto& onesglyph = glyphs[getDigitName(ayaNumber)];
        auto position = (short)(ayaGlyph.width / 2 - (onesglyph.width) / 2);

        auto disp = (short)(-onesglyph.width - position);

        //singleadjsubtable->singlePos[onesglyph.charcode] = { disp ,digitheight,(short)-onesglyph.width,0 };
        singleadjsubtable->singlePos[onesglyph.charcode] = { disp ,digitheight,(short)-onesglyph.width,0 };

        ChainingSubtable* subtable = new ChainingSubtable(lookup);
        lookup->subtables.append(subtable);
        subtable->name = singleadjsubtable->name;
        subtable->compiledRule = ChainingSubtable::CompiledRule();
        subtable->compiledRule.input = { {(uint16_t)ayaGlyph.charcode},{(uint16_t)onesglyph.charcode} };
        subtable->compiledRule.lookupRecords.append({ 1,QString("l%1").arg(ayaNumber) });

      }
      else if (ayaNumber < 100) {

        int onesdigit = ayaNumber % 10;
        int tensdigit = ayaNumber / 10;


        Lookup* sublookup1 = new Lookup(m_layout);
        sublookup1->name = QString("ayanumberskern.l%1.1").arg(ayaNumber);
        sublookup1->feature = "";
        sublookup1->type = Lookup::singleadjustment;
        m_layout->addLookup(sublookup1);

        SingleAdjustmentSubtable* singleadjsubtable1 = new SingleAdjustmentSubtable(sublookup1);
        sublookup1->subtables.append(singleadjsubtable1);

        singleadjsubtable1->name = sublookup1->name;

        Lookup* sublookup2 = new Lookup(m_layout);
        sublookup2->name = QString("ayanumberskern.l%1.2").arg(ayaNumber);
        sublookup2->feature = "";
        sublookup2->type = Lookup::singleadjustment;
        m_layout->addLookup(sublookup2);

        SingleAdjustmentSubtable* singleadjsubtable2 = new SingleAdjustmentSubtable(sublookup2);
        sublookup2->subtables.append(singleadjsubtable2);

        singleadjsubtable2->name = sublookup2->name;

        auto& onesglyph = glyphs[getDigitName(onesdigit)];
        auto& tensglyph = glyphs[getDigitName(tensdigit)];

        auto digitswidth = onesglyph.width + tensglyph.width + digitKern;

        auto position = (short)(ayaGlyph.width / 2 - digitswidth / 2);

        auto disp = (short)(-digitswidth - position);

        singleadjsubtable2->singlePos[tensglyph.charcode] = { disp ,digitheight,disp,0 };
        singleadjsubtable1->singlePos[onesglyph.charcode] = { digitKern ,digitheight,(short)(digitKern + position),0 };

        ChainingSubtable* subtable = new ChainingSubtable(lookup);
        lookup->subtables.append(subtable);
        subtable->name = QString("ayanumberskern.l%1").arg(ayaNumber);
        subtable->compiledRule = ChainingSubtable::CompiledRule();
        subtable->compiledRule.input = { {(uint16_t)ayaGlyph.charcode} ,{(uint16_t)tensglyph.charcode},{(uint16_t)onesglyph.charcode} };
        subtable->compiledRule.lookupRecords.append({ 1,QString("l%1.2").arg(ayaNumber) });
        subtable->compiledRule.lookupRecords.append({ 2,QString("l%1.1").arg(ayaNumber) });


      }
      else {

        Lookup* sublookup1 = new Lookup(m_layout);
        sublookup1->name = QString("ayanumberskern.l%1.1").arg(ayaNumber);
        sublookup1->feature = "";
        sublookup1->type = Lookup::singleadjustment;
        m_layout->addLookup(sublookup1);
        SingleAdjustmentSubtable* singleadjsubtable1 = new SingleAdjustmentSubtable(sublookup1);
        sublookup1->subtables.append(singleadjsubtable1);
        singleadjsubtable1->name = sublookup1->name;

        Lookup* sublookup2 = new Lookup(m_layout);
        sublookup2->name = QString("ayanumberskern.l%1.2").arg(ayaNumber);
        sublookup2->feature = "";
        sublookup2->type = Lookup::singleadjustment;
        m_layout->addLookup(sublookup2);
        SingleAdjustmentSubtable* singleadjsubtable2 = new SingleAdjustmentSubtable(sublookup2);
        sublookup2->subtables.append(singleadjsubtable2);
        singleadjsubtable2->name = sublookup2->name;

        Lookup* sublookup3 = new Lookup(m_layout);
        sublookup3->name = QString("ayanumberskern.l%1.3").arg(ayaNumber);
        sublookup3->feature = "";
        sublookup3->type = Lookup::singleadjustment;
        m_layout->addLookup(sublookup3);
        SingleAdjustmentSubtable* singleadjsubtable3 = new SingleAdjustmentSubtable(sublookup3);
        sublookup3->subtables.append(singleadjsubtable3);
        singleadjsubtable3->name = sublookup3->name;

        int onesdigit = ayaNumber % 10;
        int tensdigit = (ayaNumber / 10) % 10;
        int hundredsdigit = ayaNumber / 100;

        auto& onesglyph = glyphs[getDigitName(onesdigit)];
        auto& tensglyph = glyphs[getDigitName(tensdigit)];
        auto& hundredsglyph = glyphs[getDigitName(hundredsdigit)];

        auto digitswidth = onesglyph.width + tensglyph.width + hundredsglyph.width + 2 * digitKern;

        auto position = (short)(ayaGlyph.width / 2 - digitswidth / 2);

        auto disp = (short)(-digitswidth - position);

        singleadjsubtable3->singlePos[hundredsglyph.charcode] = { disp ,digitheight,disp,0 };
        singleadjsubtable2->singlePos[tensglyph.charcode] = { digitKern ,digitheight,digitKern,0 };
        singleadjsubtable1->singlePos[onesglyph.charcode] = { digitKern ,digitheight,(short)(digitKern + position),0 };


        ChainingSubtable* subtable = new ChainingSubtable(lookup);
        lookup->subtables.append(subtable);
        subtable->name = QString("ayanumberskern.l%1").arg(ayaNumber);
        subtable->compiledRule = ChainingSubtable::CompiledRule();
        subtable->compiledRule.input = { {(uint16_t)ayaGlyph.charcode},{(uint16_t)hundredsglyph.charcode} ,{(uint16_t)tensglyph.charcode} ,{(uint16_t)onesglyph.charcode} };
        subtable->compiledRule.lookupRecords.append({ 1,QString("l%1.3").arg(ayaNumber) });
        subtable->compiledRule.lookupRecords.append({ 2,QString("l%1.2").arg(ayaNumber) });
        subtable->compiledRule.lookupRecords.append({ 3,QString("l%1.1").arg(ayaNumber) });
      }

    }

    return lookup;
  }

  /*
  Lookup* IndoPak::ayanumberskern() {



    auto& ayaGlyph = glyphs["endofaya"];
    auto digitySet = classtoUnicode("digits");
    qint16 yoffset = 120;

    // three digits
    Lookup* sublookup = new Lookup(m_layout);
    sublookup->name = "ayanumberskern.l1";
    sublookup->feature = "";
    sublookup->type = Lookup::singleadjustment;
    m_layout->addLookup(sublookup);

    SingleAdjustmentSubtable* singleadjsubtable = new SingleAdjustmentSubtable(sublookup);
    sublookup->subtables.append(singleadjsubtable);

    singleadjsubtable->name = sublookup->name;

    for (auto digit : digitySet) {
      auto& onesglyph = glyphs[m_layout->glyphNamePerCode.value(digit)];
      qint16 kern = -(ayaGlyph.width / 2 - onesglyph.width / 2);
      singleadjsubtable->singlePos[digit] = { 700,yoffset,0,0 };
    }

    // two digits
    sublookup = new Lookup(m_layout);
    sublookup->name = "ayanumberskern.l2";
    sublookup->feature = "";
    sublookup->type = Lookup::singleadjustment;
    m_layout->addLookup(sublookup);

    singleadjsubtable = new SingleAdjustmentSubtable(sublookup);
    sublookup->subtables.append(singleadjsubtable);

    singleadjsubtable->name = sublookup->name;

    for (auto digit : digitySet) {
      singleadjsubtable->singlePos[digit] = { 500,yoffset,0,0 };
    }

    // 1 digit
    sublookup = new Lookup(m_layout);
    sublookup->name = "ayanumberskern.l3";
    sublookup->feature = "";
    sublookup->type = Lookup::singleadjustment;
    m_layout->addLookup(sublookup);

    singleadjsubtable = new SingleAdjustmentSubtable(sublookup);
    sublookup->subtables.append(singleadjsubtable);

    singleadjsubtable->name = sublookup->name;

    for (auto digit : digitySet) {
      auto& onesglyph = glyphs[m_layout->glyphNamePerCode.value(digit)];
      int leftBearing = 0;
      qint16 kern = leftBearing + (ayaGlyph.width - leftBearing) / 2 + onesglyph.width / 2;
      singleadjsubtable->singlePos[digit] = { kern,yoffset,0,0 };
    }

    // up
    sublookup = new Lookup(m_layout);
    sublookup->name = "ayanumberskern.up";
    sublookup->feature = "";
    sublookup->type = Lookup::singleadjustment;
    m_layout->addLookup(sublookup);

    singleadjsubtable = new SingleAdjustmentSubtable(sublookup);
    sublookup->subtables.append(singleadjsubtable);

    singleadjsubtable->name = sublookup->name;

    for (auto digit : digitySet) {
      singleadjsubtable->singlePos[digit] = { 0,yoffset,0,0 };
    }





    //main lokkup



    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "ayanumberskern";
    lookup->feature = "kern";
    lookup->type = Lookup::chainingpos;
    lookup->flags = 0;
    //lookup->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["smallalef"].charcode });
    //lookup->flags = lookup->flags | Lookup::Flags::IgnoreMarks;




    ChainingSubtable* subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = "ayanumbers3digits";
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    //subtable->compiledRule.backtrack = {{(int16_t)ayaGlyph.charcode}};
    subtable->compiledRule.input = { {(uint16_t)ayaGlyph.charcode}, digitySet,digitySet,digitySet };
    subtable->compiledRule.lookupRecords.append({ 1,"l1" });
    subtable->compiledRule.lookupRecords.append({ 2,"l1" });
    subtable->compiledRule.lookupRecords.append({ 3,"l1" });

    subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = "ayanumbers2digits";
    //subtable->compiledRule.backtrack = {{(int16_t)ayaGlyph.charcode}};
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    subtable->compiledRule.input = { {(uint16_t)ayaGlyph.charcode}, digitySet,digitySet };
    subtable->compiledRule.lookupRecords.append({ 1,"l2" });
    subtable->compiledRule.lookupRecords.append({ 2,"l2" });

    subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = "ayanumbers1digit";
    //subtable->compiledRule.backtrack = {{(quint16)ayaGlyph.charcode}};
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    subtable->compiledRule.input = { {(uint16_t)ayaGlyph.charcode}, digitySet };
    subtable->compiledRule.lookupRecords.append({ 1,"l3" });

    return lookup;


  }*/

  Lookup* IndoPak::ayanumbers() {

    QString ayaName = "endofaya";

    quint16 endofaya = m_layout->glyphCodePerName[ayaName];

    // ligature
    Lookup* ligature = new Lookup(m_layout);
    ligature->name = "ayanumbers.l1";
    ligature->feature = "";
    ligature->type = Lookup::ligature;
    m_layout->addLookup(ligature);

    LigatureSubtable* ligaturesubtable = new LigatureSubtable(ligature);
    ligature->subtables.append(ligaturesubtable);
    ligaturesubtable->name = ligature->name;

    for (quint16 i = 286; i > 99; i--) {
      quint16 code = m_layout->glyphCodePerName[QString("%1%2").arg(ayaName).arg(i)];

      int onesdigit = i % 10;
      int tensdigit = (i / 10) % 10;
      int hundredsdigit = i / 100;
      if (extended) {
        ligaturesubtable->ligatures.append({ code, {endofaya, (quint16)(m_layout->unicodeToGlyphCode.value(1632 + hundredsdigit)),
                                                     (quint16)(m_layout->unicodeToGlyphCode.value(1632 + tensdigit)),
                                                     (quint16)(m_layout->unicodeToGlyphCode.value(1632 + onesdigit)) } });
      }
      else {
        ligaturesubtable->ligatures.append({ code, { (quint16)(m_layout->unicodeToGlyphCode.value(1632 + onesdigit)),
                                                     (quint16)(m_layout->unicodeToGlyphCode.value(1632 + tensdigit)),
                                                     (quint16)(m_layout->unicodeToGlyphCode.value(1632 + hundredsdigit)),endofaya } });
        ligaturesubtable->ligatures.append({ code, {endofaya,(quint16)(m_layout->unicodeToGlyphCode.value(1632 + hundredsdigit)),
                                                    (quint16)(m_layout->unicodeToGlyphCode.value(1632 + tensdigit)),
                                                    (quint16)(m_layout->unicodeToGlyphCode.value(1632 + onesdigit)) } });
      }


    }

    // ligature
    ligature = new Lookup(m_layout);
    ligature->name = "ayanumbers.l2";
    ligature->feature = "";
    ligature->type = Lookup::ligature;
    m_layout->addLookup(ligature);

    ligaturesubtable = new LigatureSubtable(ligature);
    ligature->subtables.append(ligaturesubtable);
    ligaturesubtable->name = ligature->name;

    for (quint16 i = 99; i > 9; i--) {
      quint16 code = m_layout->glyphCodePerName[QString("%1%2").arg(ayaName).arg(i)];
      int onesdigit = i % 10;
      int tensdigit = i / 10;
      if (extended) {
        ligaturesubtable->ligatures.append({ code,{endofaya, (quint16)(m_layout->unicodeToGlyphCode.value(1632 + tensdigit)),
                                                    (quint16)(m_layout->unicodeToGlyphCode.value(1632 + onesdigit)) } });
      }
      else {
        ligaturesubtable->ligatures.append({ code,{(quint16)(m_layout->unicodeToGlyphCode.value(1632 + onesdigit)),
                                                    (quint16)(m_layout->unicodeToGlyphCode.value(1632 + tensdigit)),endofaya } });
        ligaturesubtable->ligatures.append({ code,{endofaya,(quint16)(m_layout->unicodeToGlyphCode.value(1632 + tensdigit)),
                                                   (quint16)(m_layout->unicodeToGlyphCode.value(1632 + onesdigit)) } });
      }

    }

    // ligature
    ligature = new Lookup(m_layout);
    ligature->name = "ayanumbers.l3";
    ligature->feature = "";
    ligature->type = Lookup::ligature;
    m_layout->addLookup(ligature);

    ligaturesubtable = new LigatureSubtable(ligature);
    ligature->subtables.append(ligaturesubtable);
    ligaturesubtable->name = ligature->name;

    for (int i = 1; i < 10; i++) {
      quint16 code = m_layout->glyphCodePerName[QString("%1%2").arg(ayaName).arg(i)];
      ligaturesubtable->ligatures.append({ code,{endofaya,(quint16)(m_layout->unicodeToGlyphCode.value(1632 + i))} });
      ligaturesubtable->ligatures.append({ code,{(quint16)(m_layout->unicodeToGlyphCode.value(1632 + i)),endofaya} });
    }

    //main lokkup



    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "ayanumbers";
    lookup->feature = "rlig";
    lookup->type = Lookup::chainingsub;
    lookup->flags = 0;

    auto digitySet = classtoUnicode("digits");

    auto digitySetplusendofaya = digitySet;
    digitySetplusendofaya.insert(endofaya);


    ChainingSubtable* subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = "ayanumbers3digits";
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    if (extended) {
      subtable->compiledRule.input = { {endofaya},digitySet,digitySet,digitySet };
    }
    else {
      //subtable->compiledRule.input = {digitySet,digitySet,digitySet,{endofaya} };
      subtable->compiledRule.input = { digitySetplusendofaya,digitySet,digitySet,digitySetplusendofaya };
    }

    subtable->compiledRule.lookupRecords.append({ 0,"l1" });

    subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = "ayanumbers2digits";
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    if (extended) {
      subtable->compiledRule.input = { {endofaya},digitySet,digitySet };
    }
    else {
      subtable->compiledRule.input = { digitySetplusendofaya,digitySet,digitySetplusendofaya };
    }
    subtable->compiledRule.lookupRecords.append({ 0,"l2" });

    subtable = new ChainingSubtable(lookup);
    lookup->subtables.append(subtable);
    subtable->name = "ayanumbers1digit";
    subtable->compiledRule = ChainingSubtable::CompiledRule();
    if (extended) {
      subtable->compiledRule.input = { {endofaya},digitySet };
    }
    else {

      subtable->compiledRule.input = { digitySetplusendofaya,digitySetplusendofaya };
    }
    subtable->compiledRule.lookupRecords.append({ 0,"l3" });

    return lookup;

  }
  Lookup* IndoPak::forheh() {

    Lookup* single = new Lookup(m_layout);
    single->name = "forheh.l1";
    single->feature = "";
    single->type = Lookup::single;
    m_layout->addLookup(single);

    SingleSubtable* singlesubtable = new SingleSubtable(single);
    single->subtables.append(singlesubtable);
    singlesubtable->name = single->name;


    for (auto& glyph : glyphs) {
      if (classes["haslefttatweel"].contains(glyph.name)) {
        QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + 2) * 100));
        if (glyphs.contains(destName)) {
          singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
        }
      }
      else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
        QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + 2) * 100));
        if (glyphs.contains(destName)) {
          singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
        }
      }
    }


    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "forheh";
    lookup->feature = "rlig";
    lookup->type = Lookup::chainingsub;
    lookup->flags = Lookup::Flags::IgnoreMarks;

    ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "forheh";

    newsubtable->compiledRule = ChainingSubtable::CompiledRule();

    auto keys = singlesubtable->subst.keys();
    if (!keys.isEmpty()) {
      newsubtable->compiledRule.input.append(QSet(keys.begin(), keys.end()));
    }


    //newsubtable->compiledRule.input.append({ (quint16)glyphs["heh.medi"].charcode,(quint16)glyphs["heh.medi.forsmalllalef"].charcode });
    newsubtable->compiledRule.lookahead.append(classtoUnicode("^heh.medi")); //  { (quint16)glyphs["heh.medi"].charcode });

    newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });

    return lookup;

  }
  Lookup* IndoPak::forhamza() {

    Lookup* single = new Lookup(m_layout);
    single->name = "forhamza.l1";
    single->feature = "";
    single->type = Lookup::single;

    m_layout->addLookup(single);

    SingleSubtable* singlesubtable = new SingleSubtable(single);
    single->subtables.append(singlesubtable);
    singlesubtable->name = single->name;

    int tatweel = 2;

    for (auto& glyph : glyphs) {
      if (classes["haslefttatweel"].contains(glyph.name)) {
        QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + tatweel) * 100));
        if (glyphs.contains(destName)) {
          singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
        }
      }
      else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
        QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + tatweel) * 100));
        if (glyphs.contains(destName)) {
          singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
        }
      }
    }

    // ligature
    Lookup* ligature = new Lookup(m_layout);
    ligature->name = "forhamza.l2";
    ligature->feature = "";
    ligature->type = Lookup::ligature;
    //ligature->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["hamzaabove"].charcode,(quint16)glyphs["smallhighyeh"].charcode,(quint16)glyphs["smallhighwaw"].charcode ,(quint16)glyphs["smallhighnoon"].charcode });
    //ligature->flags = ligature->flags | Lookup::Flags::UseMarkFilteringSet;
    m_layout->addLookup(ligature);

    LigatureSubtable* ligaturesubtable = new LigatureSubtable(ligature);
    ligature->subtables.append(ligaturesubtable);
    ligaturesubtable->name = ligature->name;

    ligaturesubtable->ligatures.append({ (quint16)glyphs["hamzaabove"].charcode,{ (quint16)glyphs["hamzaabove"].charcode  , 0x200D } });
    ligaturesubtable->ligatures.append({ (quint16)glyphs["hamzaabove.joined"].charcode,{ 0x200D,(quint16)glyphs["hamzaabove"].charcode } });
    ligaturesubtable->ligatures.append({ (quint16)glyphs["hamzaabove.joined"].charcode,{ (quint16)glyphs["tatweel"].charcode,(quint16)glyphs["hamzaabove"].charcode } });
    ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighyeh"].charcode,{ 0x200D,(quint16)glyphs["smallhighyeh"].charcode } });
    ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighyeh"].charcode,{ (quint16)glyphs["tatweel"].charcode,(quint16)glyphs["smallhighyeh"].charcode } });
    //ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighwaw"].charcode,{ 0x200D,(quint16)glyphs["smallhighwaw"].charcode } });
    //ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighwaw"].charcode,{ (quint16)glyphs["tatweel"].charcode,(quint16)glyphs["smallhighwaw"].charcode } });

    ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighnoon"].charcode,{ 0x200D,(quint16)glyphs["smallhighnoon"].charcode } });
    ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighnoon"].charcode,{ (quint16)glyphs["tatweel"].charcode,(quint16)glyphs["smallhighnoon"].charcode } });


    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "forhamza";
    lookup->feature = "rlig";
    lookup->type = Lookup::chainingsub;
    lookup->markGlyphSetIndex = m_layout->addMarkSet({
      //(quint16)glyphs["smallhighwaw"].charcode,
      (quint16)glyphs["hamzaabove"].charcode,
      (quint16)glyphs["smallhighyeh"].charcode,
      (quint16)glyphs["smallhighnoon"].charcode,
      (quint16)glyphs["roundedfilledhigh"].charcode
      });
    lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;
    //lookup->flags = lookup->flags | Lookup::Flags::IgnoreMarks;

    ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "forhamza";

    newsubtable->compiledRule = ChainingSubtable::CompiledRule();

    //newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
    auto keys = singlesubtable->subst.keys();
    if (!keys.isEmpty()) {
      newsubtable->compiledRule.input.append(QSet(keys.begin(), keys.end()));
    }


    newsubtable->compiledRule.input.append({ 0x200D ,  (quint16)glyphs["tatweel"].charcode });
    newsubtable->compiledRule.input.append({ (quint16)glyphs["hamzaabove"].charcode,(quint16)glyphs["smallhighyeh"].charcode, (quint16)glyphs["smallhighnoon"].charcode });

    newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });
    newsubtable->compiledRule.lookupRecords.append({ 1,"l2" });


    //roundedfilledhigh
    newsubtable = new ChainingSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "roundedfilledhigh";

    newsubtable->compiledRule = ChainingSubtable::CompiledRule();

    //newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
    keys = singlesubtable->subst.keys();
    if (!keys.isEmpty()) {
      newsubtable->compiledRule.input.append(QSet(keys.begin(), keys.end()));
    }

    newsubtable->compiledRule.input.append(QSet{ (quint16)glyphs["roundedfilledhigh"].charcode, });

    newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });


    return lookup;

  }
  Lookup* IndoPak::shrinkstretchlt() {



    Lookup* lookup;
    int count = 1;
    for (float i = -0.1; i >= -0.7; i = i - 0.1) {
      lookup = shrinkstretchlt(i, QString("shr%1").arg(count));
      m_layout->addLookup(lookup);
      count++;
    }

    return nullptr;

  }
  Lookup* IndoPak::shrinkstretchlt(float lt, QString featureName) {



    //m_layout->addLookup(forwaw(), false);

    QString lookupName;

    if (lt < 0) {
      lookupName = QString("minuslt_%1").arg(lt * -100);
    }
    else {
      lookupName = QString("pluslt_%1").arg(lt * -100);
    }

    Lookup* single = new Lookup(m_layout);
    single->name = lookupName + ".l1";
    single->feature = "";
    single->type = Lookup::single;

    m_layout->addLookup(single);

    SingleSubtable* singlesubtable = new SingleSubtable(single);
    single->subtables.append(singlesubtable);
    singlesubtable->name = single->name;

    for (auto& glyph : glyphs) {
      //QRegularExpression reg2("beginchar\\((.*?),(.*?),(.*?),(.*?)\\);");
      QRegularExpression regname("(.*)[.](minuslt|pluslt)_(.*)");
      QRegularExpressionMatch match = regname.match(glyph.name);
      if (match.hasMatch()) {
        QString name = match.captured(1);
        QString plusminus = match.captured(2);
        int value = match.captured(3).toInt();
      }
      else if (classes["haslefttatweel"].contains(glyph.name)) {
        if (lt < 0) {
          QString destName = QStringLiteral("%1.minuslt_%2").arg(glyph.name).arg((int)(lt * -100));
          if (glyphs.contains(destName)) {
            singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
          }
        }
        else {
          QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)(lt * 100));
          if (glyphs.contains(destName)) {
            singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
          }
        }

      }
      /*
                  if (match.hasMatch()) {
                          int w1 = match.captured(1).toInt();
                          double w2 = match.captured(2).toDouble();

                  if (classes["haslefttatweel"].contains(glyph.name)) {
                          QString destName = QStringLiteral("%1.minuslt_%2").arg(glyph.name).arg((int)(lt * 100));
                          if (glyphs.contains(destName)) {
                                  singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
                          }
                  }
                  else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
                          QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt - shrink) * 100));
                          if (glyphs.contains(destName)) {
                                  singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
                          }
                  }*/
    }


    Lookup* lookup = new Lookup(m_layout);
    lookup->name = lookupName;
    lookup->feature = featureName;
    lookup->type = Lookup::chainingsub;
    lookup->flags = 0;


    ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = lookupName;

    newsubtable->compiledRule = ChainingSubtable::CompiledRule();

    //newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
    auto keys = singlesubtable->subst.keys();
    if (!keys.isEmpty()) {
      newsubtable->compiledRule.input.append(QSet(keys.begin(), keys.end()));
    }

    newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });

    return lookup;

  }
  Lookup* IndoPak::forsmallhighwaw() {

    Lookup* single = new Lookup(m_layout);
    single->name = "forsmallhighwaw.l1";
    single->feature = "";
    single->type = Lookup::single;

    m_layout->addLookup(single);

    float tatweel = 1;

    SingleSubtable* singlesubtable = new SingleSubtable(single);
    single->subtables.append(singlesubtable);
    singlesubtable->name = single->name;

    for (auto& glyph : glyphs) {
      if (classes["haslefttatweel"].contains(glyph.name)) {
        QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + tatweel) * 100));
        if (glyphs.contains(destName)) {
          singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
        }
      }
      else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
        QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + tatweel) * 100));
        if (glyphs.contains(destName)) {
          singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
        }
      }
    }

    // ligature
    Lookup* ligature = new Lookup(m_layout);
    ligature->name = "forsmallhighwaw.l2";
    ligature->feature = "";
    ligature->type = Lookup::ligature;
    m_layout->addLookup(ligature);

    LigatureSubtable* ligaturesubtable = new LigatureSubtable(ligature);
    ligature->subtables.append(ligaturesubtable);
    ligaturesubtable->name = ligature->name;

    ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighwaw"].charcode,{ 0x034F,(quint16)glyphs["smallhighwaw"].charcode } });

    //main lookup
    Lookup* lookup = new Lookup(m_layout);
    lookup->name = "forsmallhighwaw";
    lookup->feature = "rlig";
    lookup->type = Lookup::chainingsub;
    lookup->markGlyphSetIndex = m_layout->addMarkSet(QList{ (quint16)glyphs["smallhighwaw"].charcode });
    lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;



    //forsmallalefwithmaddah
    ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
    lookup->subtables.append(newsubtable);

    newsubtable->name = "subtable1";

    newsubtable->compiledRule = ChainingSubtable::CompiledRule();

    //newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
    auto keys = singlesubtable->subst.keys();
    if (!keys.isEmpty()) {
      newsubtable->compiledRule.input.append(QSet(keys.begin(), keys.end()));
    }
    newsubtable->compiledRule.input.append(QSet{ (quint16)0x034F });
    newsubtable->compiledRule.input.append(QSet{ (quint16)glyphs["smallhighwaw"].charcode });

    newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });
    newsubtable->compiledRule.lookupRecords.append({ 1,"l2" });



    return lookup;

  }

  Lookup* IndoPak::populatecvxx() {
    int cvNumber = 1;

    for (auto alternates : cvxxfeatures) {
      Lookup* alternate = new Lookup(m_layout);
      alternate->name = QString("cv%1").arg(cvNumber, 2, 10, QLatin1Char('0'));
      alternate->feature = alternate->name;
      alternate->type = Lookup::alternate;

      m_layout->addLookup(alternate);

      AlternateSubtable* alternateSubtable = new AlternateSubtable(alternate);
      alternate->subtables.append(alternateSubtable);
      alternate->name = alternate->name;

      alternateSubtable->alternates = alternates;

      cvNumber++;
    }

    return nullptr;

  }

  Lookup* IndoPak::glyphalternates() {

    /*
    if (m_layout->isExtended()) {
      return nullptr;
    }*/

    bool isExtended = m_layout->isExtended();

    unordered_map<QString, QString> cv01mappings;


    cv01mappings.insert({ "noon.isol","noon.isol.expa" });
    cv01mappings.insert({ "behshape.isol","behshape.isol.expa" });
    cv01mappings.insert({ "feh.isol","feh.isol.expa" });
    cv01mappings.insert({ "qaf.isol","qaf.isol.expa" });
    cv01mappings.insert({ "seen.isol","seen.isol.expa" });
    cv01mappings.insert({ "sad.isol","sad.isol.expa" });
    cv01mappings.insert({ "yehshape.isol","yehshape.isol.expa" });
    cv01mappings.insert({ "alefmaksura.isol","alefmaksura.isol.expa" });
    cv01mappings.insert({ "kaf.isol","kaf.isol" });

    cv01mappings.insert({ "noon.fina","noon.fina.expa" });
    cv01mappings.insert({ "noon.fina.afterbeh","noon.fina.expa.afterbeh" });
    cv01mappings.insert({ "kaf.fina","kaf.fina.expa" });
    cv01mappings.insert({ "kaf.fina.afterlam","kaf.fina.afterlam.expa" });
    cv01mappings.insert({ "behshape.fina","behshape.fina.expa" });
    cv01mappings.insert({ "feh.fina","feh.fina.expa" });
    cv01mappings.insert({ "qaf.fina","qaf.fina.expa" });
    cv01mappings.insert({ "seen.fina","seen.fina.expa" });
    cv01mappings.insert({ "sad.fina","sad.fina.expa" });
    cv01mappings.insert({ "alef.fina","alef.fina" });
    cv01mappings.insert({ "yehshape.fina","yehshape.fina.expa" });
    cv01mappings.insert({ "yehshape.fina.ii","yehshape.fina.ii.expa" });

    struct AltFeature {
      struct Subst {
        QString glyph;
        QString substitute;
      };
      QString featureName;
      std::vector<Subst> alternates;
    };

    std::vector<AltFeature> altfeatures;

    altfeatures.push_back({ "cv10",{{"behshape.medi","behshape.medi.expa"}} });
    altfeatures.push_back({ "cv11",{{"heh.init.beforemeem","heh.init"},{"meem.fina.afterheh","meem.fina"}} });
    altfeatures.push_back({ "cv12",{{"behshape.init.beforehah","behshape.init"},{"hah.medi.afterbeh","hah.medi" }, {"hah.medi.afterbeh.beforeyeh","hah.medi.beforeyeh"}} });
    altfeatures.push_back({ "cv13",{{"meem.init.beforehah","meem.init" },{"hah.medi.aftermeem","hah.medi" }} });
    altfeatures.push_back({ "cv14",{{"fehshape.init.beforehah","fehshape.init" },{"hah.medi.afterfeh","hah.medi" }} });
    altfeatures.push_back({ "cv15",{{"lam.init.lam_hah","lam.init" },{"hah.medi.lam_hah","hah.medi" }} });
    altfeatures.push_back({ "cv16",{{"hah.init.ii","hah.init" },{"hah.medi.ii","hah.medi" },{"ain.init.finjani","ain.init"} } });
    altfeatures.push_back({ "cv17",{{"seen.init.beforereh","seen.init" },{"seen.medi.beforereh","seen.medi"}, {"reh.fina.afterseen","reh.fina"},{"sad.medi.beforereh","sad.medi"},{"sad.init.beforereh","sad.init"}} });
    altfeatures.push_back({ "cv18",{{"hah.init.beforemeem","hah.init" },{"meem.medi.afterhah","meem.medi" }, } });

    for (auto& feature : altfeatures) {
      Lookup* alternate = new Lookup(m_layout);
      alternate->name = feature.featureName;
      alternate->feature = alternate->name;
      alternate->type = Lookup::alternate;

      m_layout->addLookup(alternate);

      AlternateSubtableWithTatweel* alternateSubtable = new AlternateSubtableWithTatweel(alternate);
      alternate->subtables.append(alternateSubtable);
      alternate->name = alternate->name;

      for (auto mapping : feature.alternates) {

        QVector<ExtendedGlyph> alternates;
        int code = m_layout->glyphCodePerName[mapping.glyph];
        int substcode = m_layout->glyphCodePerName[mapping.substitute];
        ValueLimits valueLimits;

        if (m_layout->expandableGlyphs.contains(mapping.glyph)) {
          valueLimits = m_layout->expandableGlyphs[mapping.glyph];
        }

        if (code == 0 || substcode == 0) {
          throw new std::runtime_error("Glyph name invalid");
        }
        alternates.append({ substcode,0,0 });
        alternateSubtable->alternates[code] = alternates;

        for (double leftTatweel = 0.5; leftTatweel <= std::min(valueLimits.maxLeft, 6.0F); leftTatweel += 0.5) {
          GlyphParameters parameters;
          parameters.lefttatweel = leftTatweel;
          parameters.righttatweel = 0.0;
          GlyphVis* newglyph = m_layout->getAlternate(code, parameters, !isExtended, !isExtended);
          if (newglyph != nullptr) {
            QVector<ExtendedGlyph> alternates2;
            alternates2.append({ substcode,leftTatweel,0 });
            alternateSubtable->alternates[newglyph->charcode] = alternates2;
          }
        }
      }
    }



    //decomp
    unordered_map<QString, QString> mappingsdecomp;

    mappingsdecomp.insert({ "behshape.medi","behshape.medi.expa" });

    mappingsdecomp.insert({ "heh.init.beforemeem","heh.init" });
    mappingsdecomp.insert({ "meem.fina.afterheh","meem.fina" });

    mappingsdecomp.insert({ "behshape.init.beforehah","behshape.init" });
    mappingsdecomp.insert({ "hah.medi.afterbeh","hah.medi" });

    mappingsdecomp.insert({ "meem.init.beforehah","meem.init" });
    mappingsdecomp.insert({ "hah.medi.aftermeem","hah.medi" });

    mappingsdecomp.insert({ "fehshape.init.beforehah","fehshape.init" });
    mappingsdecomp.insert({ "hah.medi.afterfeh","hah.medi" });
    mappingsdecomp.insert({ "hah.medi.afterbeh.beforeyeh","hah.medi" });



    mappingsdecomp.insert({ "lam.init.lam_hah","lam.init" });
    mappingsdecomp.insert({ "hah.medi.lam_hah","hah.medi" });

    mappingsdecomp.insert({ "hah.init.ii","hah.init" });
    mappingsdecomp.insert({ "hah.medi.ii","hah.medi" });

    mappingsdecomp.insert({ "seen.init.beforereh","seen.init" });
    mappingsdecomp.insert({ "reh.fina.afterseen","reh.fina" });

    mappingsdecomp.insert({ "hah.init.beforemeem","hah.init" });
    mappingsdecomp.insert({ "meem.medi.afterhah","meem.medi" });
    mappingsdecomp.insert({ "sad.medi.beforereh","sad.medi" });
    mappingsdecomp.insert({ "sad.init.beforereh","sad.init" });

    mappingsdecomp.insert({ "ain.init.finjani","ain.init" });

    //kafs
    mappingsdecomp.insert({ "kaf.init","kaf.init.ii" });
    mappingsdecomp.insert({ "kaf.init.beforemeem","kaf.init.ii" });
    mappingsdecomp.insert({ "kaf.init.beforelam","kaf.init.ii" });
    mappingsdecomp.insert({ "kaf.init.ascent","kaf.init.ii" });
    //mappingsdecomp.insert({ "kaf.init.beforeyeh","kaf.init.ii" });
    mappingsdecomp.insert({ "kaf.medi","kaf.medi.ii" });
    mappingsdecomp.insert({ "kaf.medi.beforelam","kaf.medi.ii" });
    mappingsdecomp.insert({ "kaf.medi.beforemeem","kaf.medi.ii" });
    //mappingsdecomp.insert({ "kaf.medi.beforeyeh","kaf.medi.ii" });  
    mappingsdecomp.insert({ "lam.medi.afterkaf","lam.medi" });
    mappingsdecomp.insert({ "lam.fina.afterkaf","lam.fina" });
    mappingsdecomp.insert({ "alef.fina.afterkaf","alef.fina" });
    mappingsdecomp.insert({ "meem.fina.afterkaf","meem.fina" });

    Lookup* alternate = new Lookup(m_layout);
    alternate->name = "cv03";
    alternate->feature = alternate->name;
    alternate->type = Lookup::alternate;

    m_layout->addLookup(alternate);

    AlternateSubtableWithTatweel* alternateSubtable = new AlternateSubtableWithTatweel(alternate);
    alternate->subtables.append(alternateSubtable);
    alternate->name = alternate->name;

    for (auto mapping : mappingsdecomp) {

      QVector<ExtendedGlyph> alternates;
      int code = m_layout->glyphCodePerName[mapping.first];
      int substcode = m_layout->glyphCodePerName[mapping.second];

      ValueLimits valueLimits;

      if (m_layout->expandableGlyphs.contains(mapping.first)) {
        valueLimits = m_layout->expandableGlyphs[mapping.first];
      }

      if (code == 0 || substcode == 0) {
        throw new std::runtime_error("Glyph name invalid");
      }
      alternates.append({ substcode,0,0 });
      alternateSubtable->alternates[code] = alternates;

      for (double leftTatweel = 0.5; leftTatweel <= std::min(valueLimits.maxLeft, 6.0F); leftTatweel += 0.5) {
        GlyphParameters parameters;
        parameters.lefttatweel = leftTatweel;
        parameters.righttatweel = 0.0;
        GlyphVis* newglyph = m_layout->getAlternate(code, parameters, !isExtended, !isExtended);
        if (newglyph != nullptr) {
          QVector<ExtendedGlyph> alternates2;
          alternates2.append({ substcode,leftTatweel,0 });
          alternateSubtable->alternates[newglyph->charcode] = alternates2;
        }
      }
    }

    //cv01
    alternate = new Lookup(m_layout);
    alternate->name = "cv01";
    alternate->feature = alternate->name;
    alternate->type = Lookup::alternate;

    m_layout->addLookup(alternate);

    alternateSubtable = new AlternateSubtableWithTatweel(alternate);
    alternate->subtables.append(alternateSubtable);
    alternateSubtable->name = alternate->name;


    for (auto mapping : cv01mappings) {

      QVector<ExtendedGlyph> alternates;
      int code = m_layout->glyphCodePerName[mapping.first];
      int substcode = m_layout->glyphCodePerName[mapping.second];

      if (code == 0 || substcode == 0) {
        throw new std::runtime_error("Glyph name invalid");
      }
      auto sameSubst = mapping.first == "kaf.isol" || mapping.first == "alef.fina";
      if (!sameSubst) {
        alternates.append({ substcode,0,0 });
      }

      alternates.append({ substcode,1,0 });
      alternates.append({ substcode,2,0 });
      alternates.append({ substcode,3,0 });
      alternates.append({ substcode,4,0 });
      alternates.append({ substcode,5,0 });
      alternates.append({ substcode,6,0 });
      alternates.append({ substcode,7,0 });
      alternates.append({ substcode,8,0 });
      alternates.append({ substcode,9,0 });
      alternates.append({ substcode,10,0 });
      alternates.append({ substcode,11,0 });
      if (sameSubst) {
        alternates.append({ substcode,12,0 });
      }
      alternateSubtable->alternates[code] = alternates;
    }

    unordered_map<QString, QString> mappingLigaRightOnlys;


    mappingLigaRightOnlys.insert({ "ain.init.finjani","ain.init" });
    mappingLigaRightOnlys.insert({ "hah.init.ii", "hah.init" });
    mappingLigaRightOnlys.insert({ "hah.medi.ii","hah.medi" });
    mappingLigaRightOnlys.insert({ "behshape.init.beforereh","behshape.init" });
    mappingLigaRightOnlys.insert({ "lam.init.beforelam","lam.init" });



    for (auto mapping : mappingLigaRightOnlys) {

      QVector<ExtendedGlyph> alternates;
      int code = m_layout->glyphCodePerName[mapping.first];
      int substcode = m_layout->glyphCodePerName[mapping.second];

      if (code == 0 || substcode == 0) {
        throw new std::runtime_error("Glyph name invalid");
      }
      alternates.append({ substcode,1,0 });
      alternates.append({ substcode,2,0 });
      alternates.append({ substcode,3,0 });
      alternates.append({ substcode,4,0 });
      alternates.append({ substcode,5,0 });
      alternates.append({ substcode,6,0 });
      alternateSubtable->alternates[code] = alternates;
    }

    //behshape.medi
    auto valueLimits = m_layout->expandableGlyphs.at("behshape.medi");
    auto glyphCode = m_layout->glyphCodePerName["behshape.medi"];
    int substcode = m_layout->glyphCodePerName["behshape.medi.expa"];

    for (double leftTatweel = 0.5; leftTatweel <= std::min(valueLimits.maxLeft, 3.0F); leftTatweel += 0.5) {
      QVector<ExtendedGlyph> alternates;
      GlyphParameters parameters;
      parameters.lefttatweel = leftTatweel;
      parameters.righttatweel = 0.0;
      GlyphVis* newglyph = m_layout->getAlternate(glyphCode, parameters, !isExtended, !isExtended);
      for (double leftTatweel2 = leftTatweel + 1; leftTatweel2 <= std::min(valueLimits.maxLeft, 6.0F); leftTatweel2 += 1) {
        alternates.append({ substcode,leftTatweel2,0 });
      }
      alternateSubtable->alternates[newglyph->charcode] = alternates;
    }

    for (auto& glyph : m_layout->expandableGlyphs) {

      if (cv01mappings.find(glyph.first) != cv01mappings.end()) continue;

      if (glyph.first == "kasra") continue;

      auto glyphCode = m_layout->glyphCodePerName[glyph.first];
      auto valueLimits = glyph.second;

      if (valueLimits.maxLeft > 0) {
        for (double leftTatweel = 0; leftTatweel <= std::min(valueLimits.maxLeft, 3.0F); leftTatweel += 0.5) {
          QVector<ExtendedGlyph> alternates;
          GlyphParameters parameters;
          parameters.lefttatweel = leftTatweel;
          parameters.righttatweel = 0.0;
          GlyphVis* newglyph = m_layout->getAlternate(glyphCode, parameters, !isExtended, !isExtended);
          auto newCode = newglyph->charcode;
          if (leftTatweel == 0) {
            newCode = glyphCode;
          }
          auto leftTatweel2 = leftTatweel;
          for (int i = 1; i <= 6; i++) {
            leftTatweel2++;
            if (leftTatweel2 > 6) {
              leftTatweel2 = 6;
            }
            alternates.append({ glyphCode,leftTatweel2,0 });
          }
          /*
          for (double leftTatweel2 = leftTatweel + 1; leftTatweel2 <= std::min(valueLimits.maxLeft, 6.0F); leftTatweel2 += 1) {
            alternates.append({ glyphCode,leftTatweel2,0 });
          }*/
          alternateSubtable->alternates[newCode] = alternates;
        }
      }
    }

    alternate = new Lookup(m_layout);
    //alternate->name = QString("cv%1").arg(cvNumber++, 2, 10, QLatin1Char('0'));
    alternate->name = "cv02";
    alternate->feature = alternate->name;
    alternate->type = Lookup::alternate;

    m_layout->addLookup(alternate);

    alternateSubtable = new AlternateSubtableWithTatweel(alternate);
    alternate->subtables.append(alternateSubtable);
    alternate->name = alternate->name;

    for (auto& glyph : m_layout->expandableGlyphs) {

      auto glyphCode = m_layout->glyphCodePerName[glyph.first];
      auto valueLimits = glyph.second;

      if (valueLimits.maxRight > 0) {
        QVector<ExtendedGlyph> alternates;
        for (double righttatweel = 0.5; righttatweel <= std::min(valueLimits.maxRight, 6.0F); righttatweel += 0.5) {
          alternates.append({ glyphCode,0,righttatweel });
        }
        alternateSubtable->alternates[glyphCode] = alternates;
      };

      if (valueLimits.maxLeft > 0 && valueLimits.maxRight > 0) {
        for (double leftTatweel = 0.5; leftTatweel <= std::min(valueLimits.maxLeft, 3.0F); leftTatweel += 0.5) {
          QVector<ExtendedGlyph> alternates;
          GlyphParameters parameters;
          parameters.lefttatweel = leftTatweel;
          parameters.righttatweel = 0.0;
          GlyphVis* newglyph = m_layout->getAlternate(glyphCode, parameters, !isExtended, !isExtended);
          for (double righttatweel = 0.5; righttatweel <= std::min(valueLimits.maxRight, 6.0F); righttatweel += 0.5) {
            alternates.append({ glyphCode,leftTatweel,righttatweel });
          }
          alternateSubtable->alternates[newglyph->charcode] = alternates;
        }


      }
    }




    return nullptr;

  }
} // namespace
