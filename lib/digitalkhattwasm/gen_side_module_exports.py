#!/usr/bin/env python3
"""Compute the EXPORTED_FUNCTIONS list VisualMetaFontWasm's MAIN_MODULE build
needs so it exports only what its dlopen()'d side module(s) (madina_font)
actually resolve against, instead of MAIN_MODULE=1's "export every global
symbol" behavior (see digitalkhattwasm/CMakeLists.txt's MAIN_MODULE comment).

For each side module, a symbol it imports under the "env" (direct calls) or
"GOT.func"/"GOT.mem" (address-of, e.g. vtables/function pointers) import
modules must be resolvable from somewhere. Three cases, in order:

  1. The side module exports that same name itself (PIC codegen routes even
     a module's own globals through its GOT) -- resolves against its own
     export table at runtime, main module doesn't need to supply it.
  2. It's one of Emscripten's own JS-runtime ABI helpers (invoke_* signature
     trampolines, __cxa_*/exception-handling glue, getTempRet0/setTempRet0,
     __resumeException) -- these have no corresponding compiled wasm symbol
     anywhere; Emscripten's JS glue supplies them directly to every
     dynamically-loaded module. Requesting one via EXPORTED_FUNCTIONS is a
     hard build error ("undefined exported symbol"), so these must be
     excluded rather than requested.
  3. Otherwise, the main module must export it.

Usage:
  gen_side_module_exports.py --wasm-dis PATH -o out.json SIDE_MODULE.wasm [MORE.wasm ...]
"""
import argparse
import json
import re
import subprocess
import sys

STRUCTURAL = {"memory", "__indirect_function_table", "__memory_base", "__table_base", "__stack_pointer"}
JS_GLUE_NAMES = {"__resumeException", "getTempRet0", "setTempRet0"}

IMPORT_RE = re.compile(r'\s*\(import "(env|GOT\.func|GOT\.mem)" "([^"]+)"')
EXPORT_RE = re.compile(r'\s*\(export "([^"]+)"')


def is_js_glue(name):
    return name in JS_GLUE_NAMES or name.startswith("invoke_") or name.startswith("__cxa_")


def disassemble(wasm_dis, path):
    return subprocess.run(
        [wasm_dis, "--all-features", path], capture_output=True, text=True, check=True
    ).stdout


def imports_of(text):
    return {m.group(2) for line in text.splitlines() if (m := IMPORT_RE.match(line))} - STRUCTURAL


def exports_of(text):
    return {m.group(1) for line in text.splitlines() if (m := EXPORT_RE.match(line))}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("side_modules", nargs="+", help="side-module .wasm file(s), e.g. libmadina.wasm")
    ap.add_argument("-o", "--output", required=True)
    ap.add_argument("--wasm-dis", required=True, help="path to Binaryen's wasm-dis")
    args = ap.parse_args()

    required = set()
    for path in args.side_modules:
        text = disassemble(args.wasm_dis, path)
        needed = imports_of(text)
        self_resolved = needed & exports_of(text)
        required |= {n for n in (needed - self_resolved) if not is_js_glue(n)}

    exported_functions = sorted("_" + n for n in required)
    with open(args.output, "w") as f:
        json.dump(exported_functions, f)

    print(f"gen_side_module_exports: wrote {len(exported_functions)} required exports to {args.output}",
          file=sys.stderr)


if __name__ == "__main__":
    main()
