#include "sourcelinks.hpp"
#include <QRegularExpression>

namespace SourceLinks {
namespace {
struct Token { QString value; int begin, end; };
QVector<Token> tokens(const QString& source) {
  QVector<Token> result;
  for (int i=0; i<source.size();) {
    if (source[i].isSpace()) { ++i; continue; }
    if (source[i] == '%') { while (i<source.size() && source[i] != '\n') ++i; continue; }
    if (source[i] == '"') {
      ++i; while (i<source.size() && source[i] != '"') ++i;
      if (i<source.size()) ++i;
      continue;
    }
    const int begin=i;
    if (source[i].isLetter() || source[i] == '_') {
      ++i;
      while (i<source.size() && (source[i].isLetterOrNumber() || source[i]=='_' ||
          (source[i]=='.' && i+1<source.size() && source[i+1].isLetterOrNumber()))) ++i;
    } else if (source[i].isDigit() || (source[i]=='.' && i+1<source.size() && source[i+1].isDigit())) {
      while (i<source.size() && source[i].isDigit()) ++i;
      if (i<source.size() && source[i]=='.' && (i+1==source.size() || source[i+1]!='.')) {
        ++i; while (i<source.size() && source[i].isDigit()) ++i;
      }
    } else if (source.mid(i,2)==".." || source.mid(i,2)=="--") {
      const QChar c=source[i]; i+=2; if (i<source.size() && source[i]==c) ++i;
    } else if (source.mid(i,2)==":=") i+=2;
    else ++i;
    result.append({source.mid(begin,i-begin),begin,i});
  }
  return result;
}
bool identifier(const QString& token) { return !token.isEmpty() && (token[0].isLetter() || token[0]=='_'); }
int balanced(const QVector<Token>& t, int i, const QString& open, const QString& close) {
  if (i>=t.size() || t[i].value!=open) return -1;
  int depth=0;
  for (;i<t.size();++i) {
    if (t[i].value==open) ++depth;
    if (t[i].value==close && !--depth) return i+1;
  }
  return -1;
}
// The expression boundaries follow the glyph editor's editable path grammar.
// Balanced function arguments can contain nested expressions and commas.
int primary(const QVector<Token>& t,int i) {
  if (i>=t.size()) return -1;
  const auto value=t[i].value;
  if (value=="+" || value=="-" || value=="dir") return primary(t,i+1);
  if (value=="(") return balanced(t,i,"(",")");
  if (identifier(value)) {
    if (i+1<t.size() && t[i+1].value=="(") return balanced(t,i+1,"(",")");
    return i+1;
  }
  if (value[0].isDigit() || (value[0]=='.' && value!=".." && value!="...")) return i+1;
  return -1;
}
int expression(const QVector<Token>& t,int i) {
  int end=primary(t,i);
  while (end>0 && end<t.size() && (t[end].value=="+" || t[end].value=="-" || t[end].value=="*" || t[end].value=="/"))
    end=primary(t,end+1);
  return end;
}
QStringList references(const QVector<Token>& t,int begin,int end) {
  QStringList keys;
  for (int i=begin;i<end;++i) if (identifier(t[i].value)) keys.append(parameterKey(t[i].value));
  keys.removeDuplicates();
  return keys;
}
int advance(const QString& macro) {
  if (macro.startsWith("leftjoin") || macro=="rightjoin") return 4;
  if (macro=="setranchor" || macro=="link") return 3;
  if (macro=="endlink") return 2;
  return 1;
}
}
QStringList expressionKeys(const QString& expression) {
  const auto t=tokens(expression);
  return references(t,0,t.size());
}
QVector<Declaration> declarations(const QString& source,const QMap<int,QSet<int>>& topology) {
  QVector<Declaration> result;
  const auto t=tokens(source);
  QMap<QString,QStringList> dependencies;
  // Named assignments may span lines. Strings/comments cannot create false declarations.
  for (int i=0;i+2<t.size();++i) {
    if (!identifier(t[i].value) || (t[i+1].value!="=" && t[i+1].value!=":=")) continue;
    int end=i+2;
    while (end<t.size() && t[end].value!=";") ++end;
    if (end==t.size()) continue;
    const auto key=parameterKey(t[i].value);
    const auto refs=references(t,i+2,end);
    result.append({key,t[i].begin,t[end].end,{}});
    dependencies[key]=refs;
  }
  const QMap<QString,int> types{{"MarkAnchor",1},{"EntryAnchor",2},{"ExitAnchor",3},
      {"EntryAnchorRTL",4},{"ExitAnchorRTL",5},{"Anchor",0}};
  for (int i=0;i+3<t.size();++i) {
    const auto command=t[i].value;
    if (command!="setMarkAnchor" && command!="setCursiveAnchor" && command!="setAnchor" && command!="defAnchor") continue;
    if (t[i+1].value!="(" || !identifier(t[i+2].value)) continue;
    int end=balanced(t,i+1,"(",")");
    if (end<0) continue;
    int type=command=="setMarkAnchor" ? 1 : 0;
    QStringList refs;
    if (t[i+3].value==",") {
      if (command=="setCursiveAnchor" || command=="defAnchor") {
        if (end<3 || !types.contains(t[end-2].value)) continue;
        type=types[t[end-2].value];
      }
      refs=references(t,i+4,end-1);
    } else if (t[i+3].value==")") {
      // MetaPost also permits separate argument groups: (name)(value)(type).
      const int valueEnd=balanced(t,end,"(",")");
      if (valueEnd<0) continue;
      refs=references(t,end+1,valueEnd-1); end=valueEnd;
      if (command=="setCursiveAnchor" || command=="defAnchor") {
        const int typeEnd=balanced(t,end,"(",")");
        if (typeEnd<0 || !types.contains(t[typeEnd-2].value)) continue;
        type=types[t[typeEnd-2].value]; end=typeEnd;
      }
    } else continue;
    QSet<QString> visited;
    for (int j=0;j<refs.size();++j) if (!visited.contains(refs[j])) {
      visited.insert(refs[j]);
      for (const auto& dep:dependencies.value(refs[j])) if (!refs.contains(dep)) refs.append(dep);
    }
    const int stop=end<t.size() && t[end].value==";" ? end : end-1;
    result.append({anchorKey(t[i+2].value,type),t[i].begin,t[stop].end,refs});
  }

  const auto beginSection=QRegularExpression("(?m)^\\s*%%beginpaths\\s*$").match(source);
  const auto endSection=QRegularExpression("(?m)^\\s*%%endpaths\\s*$").match(source);
  if (!beginSection.hasMatch() || !endSection.hasMatch()) return result;
  int sequentialPath=0;
  QSet<int> seenPaths;
  for (int i=0;i<t.size();++i) {
    if (t[i].begin<beginSection.capturedEnd() || t[i].begin>=endSection.capturedStart()) continue;
    int path=sequentialPath, point=0, start=i;
    if (t[i].value=="controlledPath") {
      if (i+8>=t.size() || t[i+1].value!="(" || t[i+3].value!=",") continue;
      path=t[i+2].value.toInt(); point=t[i+4].value.toInt();
      const int header=balanced(t,i+1,"(",")");
      const int name=header>0 ? balanced(t,header,"(",")") : -1;
      if (name<0 || name>=t.size() || t[name].value!="(") continue;
      start=name+1;
    } else if (t[i].value=="fill" || t[i].value=="fillc" || t[i].value=="draw") start=i+1;
    else continue;
    QVector<Declaration> pathDeclarations;
    QSet<int> points;
    int pos=start;
    const int firstPoint=point;
    bool valid=true, complete=false;
    Declaration pendingLeft;
    while (pos<t.size() && t[pos].begin<endSection.capturedStart()) {
      if (t[pos].value=="cycle") {
        if (!pendingLeft.key.isEmpty()) {
          pendingLeft.key=pointKey(path,firstPoint,"left"); pathDeclarations.append(pendingLeft);
        }
        ++pos; complete=true; break;
      }
      const int end=expression(t,pos);
      if (end<0) { valid=false; break; }
      points.insert(point);
      pathDeclarations.append({pointKey(path,point),t[pos].begin,t[end-1].end,references(t,pos,end)});
      if (!pendingLeft.key.isEmpty()) {
        pendingLeft.key=pointKey(path,point,"left"); pathDeclarations.append(pendingLeft); pendingLeft={};
      }
      pos=end;
      if (pos>=t.size()) { valid=false; break; }
      if (t[pos].value==";" || t[pos].value==")") { complete=true; break; }
      if (t[pos].value=="{") {
        const int stop=balanced(t,pos,"{","}");
        if (stop<0) { valid=false; break; }
        pathDeclarations.append({pointKey(path,point,"right"),t[pos].begin,t[stop-1].end,references(t,pos,stop)});
        pos=stop;
      }
      if (pos>=t.size()) { valid=false; break; }
      const auto join=t[pos++].value;
      if (join==".." && pos<t.size() && (t[pos].value=="tension" || t[pos].value=="controls")) {
        ++pos;
        if (pos<t.size() && t[pos].value=="atleast") ++pos;
        const int stop=expression(t,pos);
        if (stop<0) { valid=false; break; }
        Declaration right{pointKey(path,point,"right"),t[pos].begin,t[stop-1].end,references(t,pos,stop)};
        pathDeclarations.append(right);
        pos=stop;
        pendingLeft=right;
        if (pos<t.size() && t[pos].value=="and") {
          ++pos; if (pos<t.size() && t[pos].value=="atleast") ++pos;
          const int stop=expression(t,pos);
          if (stop<0) { valid=false; break; }
          pendingLeft={"pending",t[pos].begin,t[stop-1].end,references(t,pos,stop)};
          pos=stop;
        }
        if (pos>=t.size() || t[pos].value!="..") { valid=false; break; }
        ++pos;
      } else if (join!=".." && join!="--" && join!="..." && join!="---" && !identifier(join)) { valid=false; break; }
      if (pos<t.size() && t[pos].value=="{") {
        const int stop=balanced(t,pos,"{","}");
        if (stop<0) { valid=false; break; }
        pendingLeft={"pending",t[pos].begin,t[stop-1].end,references(t,pos,stop)};
        pos=stop;
      }
      point+=advance(join);
    }
    // A topology mismatch can be an unexecuted insertion/removal. Never shift
    // all later point links onto different graphical knots in that situation.
    if (valid && complete && topology.contains(path) && points==topology[path]) {
      for (auto& declaration:pathDeclarations) {
        QSet<QString> visited;
        for (int j=0;j<declaration.related.size();++j) if (!visited.contains(declaration.related[j])) {
          visited.insert(declaration.related[j]);
          for (const auto& dependency:dependencies.value(declaration.related[j]))
            if (!declaration.related.contains(dependency)) declaration.related.append(dependency);
        }
      }
      result+=pathDeclarations;
    }
    seenPaths.insert(path); sequentialPath=seenPaths.size();
    i=qMax(i,pos-1);
  }
  return result;
}
}
