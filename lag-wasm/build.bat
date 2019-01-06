@echo off
set CC=emcc
set SOURCES=..\code\main.cpp ..\code\basecode\os_wasm.cpp
set ALL_FILES=..\code\* .\* ../run_tree\*
set EXPORTS_FILE=makefile_exports.txt
set INC_PATH=freetype_headers\
set LDFLAGS=-O2 --llvm-opts 2
set OUTPUT=build\play.html

pushd lag-wasm

%CC% %SOURCES% --bind -s STB_IMAGE=1 -std=c++11 %LDFLAGS% -I%INC_PATH% -o %OUTPUT% --preload-file ../run_tree -s WASM=1 -s ASSERTIONS=2 -s USE_FREETYPE=1 -s TOTAL_MEMORY=65536000 -s USE_SDL=1 --js-library library.js --shell-file shell.html -s EXPORTED_FUNCTIONS="['_main', '_jsResizeWindow', '_c_close_menu', '_c_open_menu', '_set_option']" -s USE_WEBGL2=1 --use-preload-plugins

popd