CC=emcc
SOURCES:=$(wildcard ../code/main.cpp ../code/basecode/os_wasm.cpp)
ALL_FILES:=$(wildcard ../code/* ./* ../run_tree/*)
EXPORTS_FILE=makefile_exports.txt
INC_PATH=freetype_headers/
LDFLAGS=-O2 --llvm-opts 2
OUTPUT=build/play.html

all: $(SOURCES) $(OUTPUT)

$(OUTPUT): $(ALL_FILES)
	$(CC) $(SOURCES) --bind -s STB_IMAGE=1 -std=c++11 $(LDFLAGS) -I$(INC_PATH) -o $(OUTPUT) --preload-file ../run_tree -s WASM=1 -s ASSERTIONS=2 -s USE_FREETYPE=1 -s TOTAL_MEMORY=65536000 -s USE_SDL=1 --js-library library.js --shell-file shell.html -s EXPORTED_FUNCTIONS="['_main', '_jsResizeWindow', '_c_close_menu', '_c_open_menu', '_set_option']" -s USE_WEBGL2=1 --use-preload-plugins
    $(clean)

clean: rm $(OUTPUT)
