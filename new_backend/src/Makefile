# Compiler settings
CPP_STANDARD = 17
CXXFLAGS = -std=c++$(CPP_STANDARD) -O3
CC = g++

# BUILD_PATHS
BUILD_PATH = build

UCI_EXE_NAME = uci-engine
BUILD_TMP_PATH = $(BUILD_PATH)/tmp
BUILD_BIN_PATH = $(BUILD_PATH)/bin
UCI_SRC_FILE = ./MainUCI.cpp
UCI_OBJ_FILE = $(BUILD_TMP_PATH)/MainUCI.o

ENGINE_FOLDER = ./engine/
# Match all src file from the engine folder
ENGINE_SRC_FILES = $(wildcard $(ENGINE_FOLDER)/*.cpp)
# changed the .cpp ending to .o for every file
OBJ_FILES = $(patsubst $(ENGINE_FOLDER)/%.cpp,$(BUILD_TMP_PATH)/%.o,$(ENGINE_SRC_FILES)) \

.PHONY: clean uci-engine
uci-engine: clean $(OBJ_FILES) $(UCI_OBJ_FILE)
	@echo Compile and link uci engine
	$(CC) $(CXXFLAGS) $(OBJ_FILES) $(UCI_OBJ_FILE) -o $(BUILD_BIN_PATH)/$(UCI_EXE_NAME)

clean:
	rm -rf $(BUILD_PATH)
	mkdir $(BUILD_PATH)
	mkdir $(BUILD_TMP_PATH)
	mkdir $(BUILD_BIN_PATH)

$(UCI_OBJ_FILE): $(UCI_SRC_FILE)
	@echo Compile uci engine main
	$(CC) $(CXXFLAGS) -c $< -o $@ 

# Compile every engine src into the the build engine folder for multiple use
$(BUILD_TMP_PATH)/%.o: $(ENGINE_FOLDER)%.cpp
	@echo Compile engine sources
	$(CC) $(CXXFLAGS) -c $< -o $@ 





	