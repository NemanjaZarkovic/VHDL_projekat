# ------------------ Compilation options ------------------------

# Compiler and linker settings
CC = g++

# Libraries
LIBS = -lsystemc -lm `pkg-config --libs opencv4`

# Compiler flags
CFLAGS = -Wall -O0 -march=x86-64 -mmmx -msse -msse2 -msse3 `pkg-config --cflags opencv4` -std=c++11

# Include path for SystemC headers (modify SYSTEMC_HOME to point to your SystemC installation)
SYSTEMC_HOME = /usr/local/systemc-2.3.3
INCLUDES = -I$(SYSTEMC_HOME)/include

# Library path for SystemC
LIBDIR = -L$(SYSTEMC_HOME)/lib-linux64  # Adjust this path based on your system architecture (lib-linux, lib-linux64, lib-macosx, etc.)

# Object files
SRCS = main.cpp image.cpp fasthessian.cpp imload.cpp
OBJS = main.o image.o fasthessian.o imload.o

# Executable file
EXE = surf_sim

# ------------------------ Rules --------------------------------

# Default target to build the executable
all: $(EXE)

# Rule to link the object files and create the executable
$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(INCLUDES) $(LIBDIR) $(LIBS)

# Compile source files into object files
main.o: main.cpp image.h fasthessian.h imload.h
	$(CC) -c $(CFLAGS) $(INCLUDES) main.cpp

image.o: image.cpp image.h
	$(CC) -c $(CFLAGS) $(INCLUDES) image.cpp

fasthessian.o: fasthessian.cpp fasthessian.h image.h
	$(CC) -c $(CFLAGS) $(INCLUDES) fasthessian.cpp

imload.o: imload.cpp imload.h image.h
	$(CC) -c $(CFLAGS) $(INCLUDES) imload.cpp

# Clean rule to remove compiled object files and executable
clean:
	-rm -f $(OBJS) $(EXE)

# Rule to run the executable
run: $(EXE)
	./$(EXE)

.PHONY: clean all run

