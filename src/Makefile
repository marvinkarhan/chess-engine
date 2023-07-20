# Compiler settings
CPP_STANDARD = 17
CXXFLAGS = -std=c++$(CPP_STANDARD) -O3 -Wall -Wextra
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

ifeq ($(SIMD), no)
	sse = no
	sse2 = no
	ssse3 = no
	sse41 = no
	avx2 = no
	avx512 = no
	vnni256 = no
	vnni512 = no
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

# wasm does not support avx2 and up
ifneq ($(MAKECMDGOALS), wasm)
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
endif

EXE = uci-engine
WASM = wasm-engine
WASM_SRC = wasm/wasm.cpp

# Match all src file from the engine folder
SRCS = $(wildcard ./nnue/*.cpp) $(wildcard ./*.cpp)
# changed the .cpp ending to .o for every file
OBJ_FILES = $(patsubst %.cpp,%.o,$(SRCS))

%.o: %.cpp
	$(CCX) $(CXXFLAGS) -c $< -o $@ 

all: engine

debug: CXXFLAGS = -std=c++$(CPP_STANDARD) -O3 -g -fno-omit-frame-pointer
debug: all

profile: CXXFLAGS = -std=c++$(CPP_STANDARD) -O3 -pg -ggdb3 -fno-omit-frame-pointer
profile: all

engine: $(OBJ_FILES) 
	$(CCX) $(CXXFLAGS) $(OBJ_FILES) -o $(EXE)

wasm: FORCE
	@echo $(MAKECMDGOALS)
	emcc $(CXXFLAGS) -DWASM $(WASM_SRC) $(SRCS) -o $(WASM).js -s ASSERTIONS=1 -s EXPORTED_FUNCTIONS='["_main","_processCommand"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]' -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s TOTAL_MEMORY=64MB --embed-file default.nnue -msimd128
	@rm -f ../frontend/src/assets/wasm/$(WASM)*
	@mv wasm-engine* ../frontend/src/assets/wasm/

FORCE: ;

clean:
ifeq ($(OS),Windows_NT)
	for /R . %%G in (*.o) do del /Q /F "%%G"
	del /Q /F $(EXE)*
	del /Q /F $(WASM)*
else
	@rm -f *.o ./nnue/*.o $(EXE)* $(WASM)*
endif


.PHONY: all clean