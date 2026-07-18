// the project_strings.cpp for the strings
#include "gllobal_strings.h"
namespace GlobalStrings {
  const char* textHeader = R"tex(\documentclass{article}
\usepackage[paperwidth=215.9mm,paperheight=279.4mm, left=10mm,right=10mm,top=5mm,bottom=15mm]{geometry}
\usepackage{graphicx}
\usepackage{bookmark}
\usepackage{setspace}
\usepackage{luacode}
\usepackage{fontspec}
\begin{luacode}
function newglue(parameters)
    local g = node.new("glue")
    local tmp_spec
    if node.has_field(g,"spec") then
        g.spec = node.new("glue_spec")
        tmp_spec = g.spec
    else
        tmp_spec = g
    end
    for k,v in pairs(parameters) do
        tmp_spec[k] = v
    end
    return g
end
function mknodes( text,features )
  local current_font = font.current()
  local font_parameters = font.getfont(current_font).parameters
  local n  
  local glyph_features = {}
  for k, v in string.gmatch(features, "(%w+)=(%w+)") do
   glyph_features[k] = v
  end  
  for s in string.utfvalues( text ) do
    local char = unicode.utf8.char(s)
    if unicode.utf8.match(char,"%s") then
      -- its a space
      n = newglue({width = font_parameters.space,shrink  = font_parameters.space_shrink, stretch = font_parameters.space_stretch})
    else -- a glyph
      n = node.new("glyph")
      n.font = current_font
      n.subtype = 1
      n.char = s
      n.lang = tex.language
      n.uchyph = 1
      node.setproperty(n,{ glyph_features = glyph_features })
    end
    node.write(n);
  end
end
\end{luacode}
\newcommand{\setfea}[2]{\directlua{mknodes("#1","#2")}}
)tex";
}
