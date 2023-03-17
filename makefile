CC = g++

CFLAGS = -I./ -I./include/ -D _DEBUG -ggdb3 -std=c++2a -O0 -Wall -Wextra -Weffc++\
-Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations\
-Wcast-align -Wchar-subscripts -Wconditionally-supported\
-Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral\
-Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op\
-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith\
-Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo\
-Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn\
-Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default\
-Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast\
-Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers\
-Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector\
-fcheck-new\
-fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging\
-fno-omit-frame-pointer -fPIE -fsanitize=address,bool,${strip \
}bounds,enum,float-cast-overflow,float-divide-by-zero,${strip \
}integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,${strip \
}returns-nonnull-attribute,shift,signed-integer-overflow,undefined,${strip \
}unreachable,vla-bound,vptr\
-pie -Wlarger-than=65536 -Wstack-usage=8192 -lglut -lGLU -lGL

SFML_LIBS = -lsfml-window -lsfml-system

BLD_FOLDER = build
ASSET_FOLDER = assets

MAIN_BLD_NAME = patcher
BLD_VERSION = 0.1
BLD_PLATFORM = linux
BLD_TYPE = dev
BLD_FORMAT = .out

BLD_SUFFIX = _v$(BLD_VERSION)_$(BLD_TYPE)_$(BLD_PLATFORM)$(BLD_FORMAT)
MAIN_BLD_FULL_NAME = $(MAIN_BLD_NAME)$(BLD_SUFFIX)

LIB_OBJECTS = argparser.o logger.o debug.o alloc_tracker.o speaker.o util.o patch_parser.o

MAIN_OBJECTS = main.o main_utils.o common_utils.o $(LIB_OBJECTS)
all: asset $(MAIN_OBJECTS)
	mkdir -p $(BLD_FOLDER)
	$(CC) $(MAIN_OBJECTS) $(CFLAGS) -o $(BLD_FOLDER)/$(MAIN_BLD_FULL_NAME) $(SFML_LIBS)

asset:
	mkdir -p $(BLD_FOLDER)
	cp -r $(ASSET_FOLDER)/. $(BLD_FOLDER)

run: asset
	cd $(BLD_FOLDER) && exec ./$(MAIN_BLD_FULL_NAME) $(ARGS)

main.o:
	$(CC) $(CFLAGS) -c src/main.cpp

main_utils.o:
	$(CC) $(CFLAGS) -c src/utils/main_utils.cpp

common_utils.o:
	$(CC) $(CFLAGS) -c src/utils/common_utils.cpp

patch_parser.o:
	$(CC) $(CFLAGS) -c lib/patch_parser/patch_parser.cpp

alloc_tracker.o:
	$(CC) $(CFLAGS) -c lib/alloc_tracker/alloc_tracker.cpp

argparser.o:
	$(CC) $(CFLAGS) -c lib/util/argparser.cpp

logger.o:
	$(CC) $(CFLAGS) -c lib/util/dbg/logger.cpp

debug.o:
	$(CC) $(CFLAGS) -c lib/util/dbg/debug.cpp

speaker.o:
	$(CC) $(CFLAGS) -c lib/speaker.cpp

util.o:
	$(CC) $(CFLAGS) -c lib/util/util.cpp


clean:
	rm -rf *.o

rmbld:
	rm -rf $(BLD_FOLDER)
	rm -rf $(TEST_FOLDER)

rm: clean rmbld