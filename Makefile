# Makefile for building bininfo from main.cpp

# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++11

# Target and source files
TARGET := bininfo
SRCS := main.cpp
OBJS := $(SRCS:.cpp=.o)

# MXE settings (uncomment if using MXE)
# MXE_PATH := /path/to/mxe/usr/bin
# CXX := $(MXE_PATH)/i686-w64-mingw32.static-g++
# TARGET := bininfo.exe

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
