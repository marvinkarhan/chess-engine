ifeq ($(OS),Windows_NT)
	RM_DIR_CMD = if exist $(BUILD_PATH) rmdir /q /s
	FixPath = $(subst /,\,$1)
else
	RM_DIR_CMD = rm -rf
	FixPath = $1
endif

BUILD_PATH = build
BUILD_TMP_PATH = $(call FixPath,$(BUILD_PATH)/tmp)
BUILD_BIN_PATH = $(call FixPath,$(BUILD_PATH)/bin)

# Compiler settings
CPP_STANDARD = 17
CXXFLAGS = -std=c++$(CPP_STANDARD) -O3
CC = g++

# BUILD_PATHS

UCI_EXE_NAME = uci-engine
UCI_SRC_FILE = ./MainUCI.cpp
UCI_OBJ_FILE = $(BUILD_TMP_PATH)/MainUCI.o

MAIN_EXE_NAME = main-engine
MAIN_SRC_FILE = ./MainEngine.cpp
MAIN_OBJ_FILE = $(BUILD_TMP_PATH)/MainEngine.o

ENGINE_FOLDER = ./engine
# Match all src file from the engine folder
ENGINE_SRC_FILES = $(wildcard $(ENGINE_FOLDER)/*.cpp)
# changed the .cpp ending to .o for every file
OBJ_FILES = $(patsubst $(ENGINE_FOLDER)/%.cpp,$(BUILD_TMP_PATH)/%.o,$(ENGINE_SRC_FILES)) \

all: build-step uci-engine main-engine clean

debug: CXXFLAGS = -std=c++$(CPP_STANDARD) -O1 -g
debug: build-step uci-engine clean

uci-engine: $(OBJ_FILES) $(UCI_OBJ_FILE) 
	@echo Link uci engine
	$(CC) $(CXXFLAGS) $(OBJ_FILES) $(UCI_OBJ_FILE) -o $(BUILD_BIN_PATH)/$(UCI_EXE_NAME)

main-engine: $(OBJ_FILES) $(MAIN_OBJ_FILE)
	@echo Main uci engine
	$(CC) $(CXXFLAGS) $(OBJ_FILES) $(MAIN_OBJ_FILE) -o $(BUILD_BIN_PATH)/$(MAIN_EXE_NAME)

wasm-engine:
	@echo Compile wasm
	emcc $(CXXFLAGS) wasm/test.cpp $(ENGINE_SRC_FILES) -o $(BUILD_BIN_PATH)/wasm-engine.js -s ASSERTIONS=1 -s EXPORTED_FUNCTIONS='["_main","_processCommand"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]' -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1

build-step:
	@echo Build directories
	$(RM_DIR_CMD) $(BUILD_PATH)
	mkdir $(BUILD_PATH) $(BUILD_TMP_PATH) $(BUILD_BIN_PATH)
clean:
	@echo Cleanup
	$(RM_DIR_CMD) $(BUILD_TMP_PATH)

$(UCI_OBJ_FILE): $(UCI_SRC_FILE)
	@echo Compile uci engine main
	$(CC) $(CXXFLAGS) -c $< -o $@ 

$(MAIN_OBJ_FILE): $(MAIN_SRC_FILE)
	@echo Compile uci engine main
	$(CC) $(CXXFLAGS) -c $< -o $@ 

# Compile every engine src into the the build engine folder for multiple use
$(BUILD_TMP_PATH)/%.o: $(ENGINE_FOLDER)/%.cpp
	@echo Compile engine sources
	$(CC) $(CXXFLAGS) -c $< -o $@ 





	