# Make file v0.0

# compiler
CC = gcc

BUILD_DIR = obj

#Binary filename
TARGET = gl_egl_extension_test

#required package
REQ_PKGS = \
			glesv2	\
			x11     \
			egl	\

# complie option
CFLAGS = -o2 -fPIC -g `pkg-config --cflags $(REQ_PKGS)`

# link libraries 
LIBRARY = `pkg-config --libs $(REQ_PKGS)` -lm -lpthread

# C source file list
C_SOURCES = main.c

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LIBRARY)  -o $@ 
#	mv *.o $(BUILD_DIR)	

clean : 
	rm *.o rm -rf $(BUILD_DIR)/*.o $(TARGET)
