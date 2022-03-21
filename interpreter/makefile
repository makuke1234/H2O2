BIN=bin
OBJ=obj
SRC=src
TARGET=water2c

CXX=g++
CDEFFLAGS=-std=c++20 -Wall -Wextra -Wpedantic -Wconversion -static
CFLAGS=-O3 -Wl,--strip-all,--build-id=none,--gc-sections -fno-ident -D NDEBUG
CDEBFLAGS=-g -O0 -D _DEBUG

SRCFILES=$(wildcard $(SRC)/*.cpp)

RELOBJFILES=$(SRCFILES:%.cpp=%.cpp.o)
RELOBJFILES:=$(RELOBJFILES:$(SRC)/%=$(OBJ)/%)

DEBOBJFILES=$(SRCFILES:%.cpp=%.cpp.d.o)
DEBOBJFILES:=$(DEBOBJFILES:$(SRC)/%=$(OBJ)/%)

default: release

rel: release
deb: debug


release: $(RELOBJFILES)
	$(CXX) $^ -o $(BIN)/$(TARGET).exe $(CFLAGS) $(CDEFFLAGS)

debug: $(DEBOBJFILES)
	$(CXX) $^ -o $(BIN)/deb$(TARGET).exe $(CDEFFLAGS)


$(OBJ)/%.cpp.o: $(SRC)/%.cpp $(OBJ) $(BIN)
	$(CXX) -c $< -o $@ $(CDEFFLAGS) $(CFLAGS)
$(OBJ)/%.cpp.d.o: $(SRC)/%.cpp $(OBJ) $(BIN)
	$(CXX) -c $< -o $@ $(CDEFFLAGS) $(CDEBFLAGS)

$(OBJ):
	mkdir $(OBJ)

$(BIN):
	mkdir $(BIN)

clean:
	rm -r -f $(OBJ)
	rm -f $(BIN)/*.exe
