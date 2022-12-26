# LuaLaTeX Package
Please see the [readme.tex](readme.tex) as an example of use.
To compile the example execute the command
```
lualatex --shell-escape readme.tex
```
The `--shell-escape` command line argument is necessay since the package uses a native Lua module. For more details please see the file [readme.pdf](readme.pdf).

The package was tested using the last version of TeX Live 2022 (as of 2022-12-20) on Windows 10 and Debian 10.