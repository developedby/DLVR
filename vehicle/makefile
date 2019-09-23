CXX       := g++
CXXFLAGS  := -Wall -pedantic-errors
LDFLAGS   := -lwiringPi -lstdc++ -lraspicam -lmmal -lmmal_core -lmmal_util -lpthread
BUILD     := ./build
OBJ_DIR   := $(BUILD)/objects
EXEC_DIR  := $(BUILD)/executables

INCLUDE   := -Iinclude/
SRC       := $(wildcard src/*.cpp)
TEST_SRC  := $(wildcard test/*.cpp)
OBJECTS   := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

