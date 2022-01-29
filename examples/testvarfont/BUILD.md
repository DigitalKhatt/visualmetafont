Checkout the revision 7439 of LuaTeX. The password is anonsvn:
```
svn checkout https://serveur-svn.lri.fr/svn/modhel/luatex/trunk@7439 --username anonsvn
```
Patch the svn version:
```
patch -p0 -i luatex.v2.patch
```
Build luahbtex: 
```
./build.sh --luahb --parallel
```
Update luahbtex with the new build:
```
mv ~/texlive/2021/bin/x86_64-linux/luahbtex ~/texlive/2021/bin/x86_64-linux/luahbtex.bkp
cp ~/projects/luatex/trunk/build/texk/web2c/luahbtex ~/texlive/2021/bin/x86_64-linux/luahbtex
```
Regenerate the lualatex format:
```
fmtutil-sys --byfmt luahbtex
fmtutil-sys --byfmt lualatex
```
Set TEXMFDOTDIR to use files from luaotfload local folder :
```
export TEXMFDOTDIR=.//
```
Generate TeX example with justification :
```
lualatex --output-directory=output -jobname=quran_tex_just -just quran.tex
```
Generate Madina example with justification :
```
lualatex --output-directory=output -jobname=quran_madina_just -madina -just quran.tex
```
