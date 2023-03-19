CC = g++

CPPFLAGS = -I./ -I./include/ -D _DEBUG -ggdb3 -std=c++2a -O0 -Wall -Wextra -Weffc++\
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

SFML_LIBS = -lsfml-window -lsfml-graphics -lsfml-system
GLM_LIBS = -libglm-dev

BLD_FOLDER = build
ASSET_FOLDER = assets
LOGS_FOLDER = logs

BUILD_LOG_NAME = build.log

MAIN_BLD_NAME = patcher
GUI_BLD_NAME = gui
BLD_VERSION = 0.1
BLD_PLATFORM = linux
BLD_TYPE = dev
BLD_FORMAT = .out

BLD_SUFFIX = _v$(BLD_VERSION)_$(BLD_TYPE)_$(BLD_PLATFORM)$(BLD_FORMAT)
MAIN_BLD_FULL_NAME = $(MAIN_BLD_NAME)$(BLD_SUFFIX)
GUI_BLD_FULL_NAME = $(GUI_BLD_NAME)$(BLD_SUFFIX)

PROJ_DIR = .

LIB_OBJECTS = lib/util/argparser.o 				\
			  lib/util/dbg/logger.o 			\
			  lib/util/dbg/debug.o 				\
			  lib/alloc_tracker/alloc_tracker.o	\
			  lib/speaker.o   					\
			  lib/util/util.o

all: main

MAIN_OBJECTS = src/main.o 				\
			   src/utils/main_utils.o 	\
			   src/utils/common_utils.o \
			   lib/patch_parser/patch_parser.o $(LIB_OBJECTS)
main: asset $(addprefix $(PROJ_DIR)/, $(MAIN_OBJECTS))
	@echo "Started"
	@mkdir -p $(BLD_FOLDER)
	$(CC) $(addprefix $(PROJ_DIR)/, $(MAIN_OBJECTS)) $(CPPFLAGS) -o $(BLD_FOLDER)/$(MAIN_BLD_FULL_NAME)

GUI_OBJECTS = src/gui.o 				\
			  src/utils/gui_utils.o 	\
			  src/utils/common_utils.o $(LIB_OBJECTS)
gui: asset main $(addprefix $(PROJ_DIR)/, $(GUI_OBJECTS))
	@mkdir -p $(BLD_FOLDER)
	$(CC) $(addprefix $(PROJ_DIR)/, $(GUI_OBJECTS)) $(CPPFLAGS) -o $(BLD_FOLDER)/$(GUI_BLD_FULL_NAME) $(SFML_LIBS)

asset:
	@mkdir -p $(BLD_FOLDER)
	@cp -r $(ASSET_FOLDER)/. $(BLD_FOLDER)

run: asset
	@cd $(BLD_FOLDER) && exec ./$(MAIN_BLD_FULL_NAME) $(ARGS)

run_gui: asset
	@cd $(BLD_FOLDER) && exec ./$(GUI_BLD_FULL_NAME) $(ARGS)

src/gui.o: src/gui.cpp
	@echo Building special file $^
	$(CC) $(CPPFLAGS) -D'PATCHER_NAME="./$(MAIN_BLD_FULL_NAME)"' -c $^ -o $@

%.o: %.cpp
	@echo Building file $^
	@$(CC) $(CPPFLAGS) -c $^ -o $@

clean:
	@find . -type f -name "*.o" -delete
	@rm -rf ./$(LOGS_FOLDER)/$(BUILD_LOG_NAME)

rmbld:
	@rm -rf $(BLD_FOLDER)
	@rm -rf $(TEST_FOLDER)

rm:
	@make clean
	@make rmbld