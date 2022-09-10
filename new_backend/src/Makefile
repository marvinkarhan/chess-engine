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
CXXFLAGS = -std=c++$(CPP_STANDARD) -O3 -m64
CCX = g++

# Add support for simd
# only support x86-64-modern right now
# can be omitted by using SIMD=no 
# Intel Streaming SIMD Extensions
sse = yes
# Intel Streaming SIMD Extensions 2
sse2 = yes
# Intel Supplemental Streaming SIMD Extensions 3
ssse3 = yes
# Intel Streaming SIMD Extensions 4.1
sse41 = yes
# INTEL Advanced Vector Extensions 2
avx2 = yes
# INTEL Advanced Vector Extensions 512
avx512 = no
# INTEL Vector Neural Network Instructions extension of AVX-512
vnni256 = no
# INTEL Vector Neural Network Instructions extension of AVX-512
vnni512 = no
# ARM SIMD
neon = no

ifeq ($(SIMD), no)
	sse = no
	sse2 = no
	ssse3 = no
	sse41 = no
	avx2 = no
	avx512 = no
	vnni256 = no
	vnni512 = no
	neon = no
endif

ifeq ($(sse), yes)
	CXXFLAGS += -msse
endif

ifeq ($(sse2), yes)
	CXXFLAGS += -DUSE_SSE2
	CXXFLAGS += -msse2
endif

ifeq ($(ssse3), yes)
	CXXFLAGS += -DUSE_SSSE3
	CXXFLAGS += -mssse3
endif

ifeq ($(sse41), yes)
	CXXFLAGS += -DUSE_SSE41
	CXXFLAGS += -msse4.1
endif

ifeq ($(avx2), yes)
	CXXFLAGS += -DUSE_AVX2
	CXXFLAGS += -mavx2
endif

ifeq ($(avx512), yes)
	CXXFLAGS += -DUSE_AVX512
	CXXFLAGS += -mavx512f -mavx512bw
endif

ifeq ($(vnni256), yes)
	CXXFLAGS += -DUSE_VNNI
	CXXFLAGS += -mavx512f -mavx512bw -mavx512vnni -mavx512dq -mavx512vl -mprefer-vector-width=256
endif

ifeq ($(vnni512), yes)
	CXXFLAGS += -DUSE_VNNI
	CXXFLAGS += -mavx512vnni -mavx512dq -mavx512vl
endif

ifeq ($(neon), yes)
	CXXFLAGS += -DUSE_NEON
	CXXFLAGS += -mfpu=neon
endif

# BUILD_PATHS

UCI_EXE_NAME = uci-engine
UCI_SRC_FILE = ./MainUCI.cpp
UCI_OBJ_FILE = $(BUILD_TMP_PATH)/MainUCI.o

MAIN_EXE_NAME = main-engine
MAIN_SRC_FILE = ./MainEngine.cpp
MAIN_OBJ_FILE = $(BUILD_TMP_PATH)/MainEngine.o

ENGINE_FOLDER = ./engine
NNUE_FOLDER = ./engine/nnue
# Match all src file from the engine folder
ENGINE_SRC_FILES = $(wildcard $(NNUE_FOLDER)/*.cpp) $(wildcard $(ENGINE_FOLDER)/*.cpp)
# changed the .cpp ending to .o for every file
OBJ_FILES = $(patsubst $(ENGINE_FOLDER)/%.cpp,$(BUILD_TMP_PATH)/%.o,$(ENGINE_SRC_FILES))

all: build-step uci-engine main-engine clean
# all: build-step uci-engine main-engine wasm-engine clean

debug: CXXFLAGS = -std=c++$(CPP_STANDARD) -O1 -g
debug: build-step uci-engine main-engine clean

uci-engine: $(OBJ_FILES) $(UCI_OBJ_FILE) 
	@echo Link uci engine
	$(CCX) $(CXXFLAGS) $(OBJ_FILES) $(UCI_OBJ_FILE) -o $(BUILD_BIN_PATH)/$(UCI_EXE_NAME)

main-engine: $(OBJ_FILES) $(MAIN_OBJ_FILE)
	@echo Main uci engine
	$(CCX) $(CXXFLAGS) $(OBJ_FILES) $(MAIN_OBJ_FILE) -o $(BUILD_BIN_PATH)/$(MAIN_EXE_NAME)

# wasm-engine:
# 	@echo Compile wasm
# 	emcc $(CXXFLAGS) wasm/test.cpp $(ENGINE_SRC_FILES) -o $(BUILD_BIN_PATH)/wasm-engine.js -s ASSERTIONS=1 -s EXPORTED_FUNCTIONS='["_main","_processCommand"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]' -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1

build-step:
	@echo Build directories
	$(RM_DIR_CMD) $(BUILD_PATH)
	mkdir $(BUILD_PATH) $(BUILD_TMP_PATH) $(BUILD_TMP_PATH)/nnue $(BUILD_BIN_PATH)

clean:
	@echo Cleanup
	$(RM_DIR_CMD) $(BUILD_TMP_PATH)

$(UCI_OBJ_FILE): $(UCI_SRC_FILE)
	@echo Compile uci engine main
	$(CCX) $(CXXFLAGS) -c $< -o $@ 

$(MAIN_OBJ_FILE): $(MAIN_SRC_FILE)
	@echo Compile uci engine main
	$(CCX) $(CXXFLAGS) -c $< -o $@ 

# Compile every engine src into the the build engine folder for multiple use
$(BUILD_TMP_PATH)/%.o: $(ENGINE_FOLDER)/%.cpp
	@echo Compile engine sources
	$(CCX) $(CXXFLAGS) -c $< -o $@ 





	