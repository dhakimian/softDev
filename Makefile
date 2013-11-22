CXX = clang++
SDL = -framework SDL2 -framework SDL2_image -framework SDL2_mixer

# You may need to change -std=c++11 to -std=c++0x if your compiler is a bit older
CXXFLAGS = -Wall -c -std=c++0x
LDFLAGS = $(SDL)

SRC = src
BUILD = build
VPATH = $(SRC):$(BUILD)
SOURCES=$(notdir $(wildcard $(SRC)/*.cpp))
OBJECTS=$(SOURCES:.cpp=.o)
EXE = our_game

all: $(SOURCES) $(BUILD) $(EXE)
		
$(EXE): $(OBJECTS)
		$(CXX) $(addprefix $(BUILD)/,$(notdir $^)) $(LDFLAGS) -o $@

.cpp.o:
		$(CXX) $< $(CXXFLAGS) -o $(BUILD)/$@

# Make sure the build directory exists
$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm $(BUILD)/*.o; rm $(EXE)
