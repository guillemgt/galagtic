@echo off
set CC=emcc
set SOURCES=..\code\main.cpp
set ALL_FILES=..\code\* .\* ../run_tree/*
set EXPORTS_FILE=makefile_exports.txt
set INC_PATH=freetype_headers\
set LDFLAGS=-O2 --llvm-opts 2
set OUTPUT=build\index.html

pushd lag-wasm

%CC% %SOURCES% --bind -std=c++11 %LDFLAGS% -I%INC_PATH% -o %OUTPUT% --preload-file ../run_tree/Textures/ -s WASM=1 -s ASSERTIONS=2 -s TOTAL_MEMORY=131072000 --js-library ./library.js --shell-file ./shell.html -s EXPORTED_FUNCTIONS="['_main']" --use-preload-plugins -g -s USE_WEBGL2=1 -s USE_SDL=2

popd