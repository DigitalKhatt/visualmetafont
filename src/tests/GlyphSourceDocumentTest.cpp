#include "glyphsourcedocument.hpp"
#include <QCoreApplication>
#include <iostream>
#include <stdexcept>
static void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    const QByteArray first = "defchar(first,-1,500,700,0);\r\n% retain this comment\r\nenddefchar;";
    const QByteArray second = "beginchar(second,32,500,700,0);\r\n%%beginbody\r\nmessage \"endchar; defchar(fake,-1,0,0,0);\";\r\nendchar;";
    const QByteArray prefix = QByteArray("\xef\xbb\xbf") + QString::fromUtf8("% العربية: defchar(commented,1,0,0,0); enddefchar;\r\n").toUtf8();
    const auto raw = prefix + first + "\r\n% Between glyphs\r\n" + second + "\r\n% Footer\r\n";
    GlyphSourceDocument doc;
    QString error;
    require(doc.parse(raw, &error), qPrintable(error));
    require(doc.blocks.size() == 2 && doc.blocks[0].name == "first" && doc.blocks[1].name == "second", "ignore keywords inside comments and strings");
    require(QString::fromUtf8(raw.mid(3)).mid(doc.blocks[0].start, doc.blocks[0].length) == doc.blocks[0].source, "Qt character spans survive non-ASCII prefixes");
    require(doc.render({}) == raw, "unchanged source is byte-identical");
    const auto edited = doc.render({{0, "defchar(first,-1,600,700,0);\nenddefchar;"}});
    require(edited.startsWith(prefix) && edited.contains(second) && edited.endsWith("\r\n% Footer\r\n"), "preserve BOM, comments and untouched glyphs");
    require(edited.contains("600,700,0);\r\nenddefchar;"), "keep CRLF for edited glyphs");
    const auto removed = doc.render({{0, {}}});
    require(!removed.contains(first) && removed.contains(second) && removed.contains("% Between glyphs"), "moving removes only the source definition");
    GlyphSourceDocument appended;
    require(appended.parse(doc.render({}, {"defchar(third,-1,0,0,0);\nenddefchar;"})), "parse appended glyph");
    require(appended.blocks.size() == 3 && appended.blocks.last().name == "third", "append after existing definitions");
    require(!appended.parse("defchar(broken,-1,0,0,0);", &error), "reject incomplete glyphs");
    require(!appended.parse("defchar(broken,-1,0,0,0);endchar;", &error), "reject mismatched glyph endings");
    require(!appended.parse("message \"unfinished", &error), "reject incomplete strings");
    require(!appended.parse(QByteArray("\xff"), &error), "reject invalid UTF-8 without changing it");
    std::cout << "PASS glyph source documents: parsing, lossless text, comments, BOM, CRLF, editing and malformed input\n";
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
