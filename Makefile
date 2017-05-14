RAY_FILE=raytracer
# RAS_FILE=rasteriser

########
#   Directories
S_DIR=Source
B_DIR=Build

########
#   Output
RAY_EXEC=$(B_DIR)/$(RAY_FILE)
RAS_EXEC=$(B_DIR)/$(RAS_FILE)

# default build settings
CC_OPTS=-c -pipe -Wall -Wno-switch -std=c++11 -pthread -O3
LN_OPTS=
CC=g++

########
#       SDL options
SDL_CFLAGS := $(shell sdl-config --cflags)
GLM_CFLAGS := -I$(GLMDIR)
SDL_LDFLAGS := $(shell sdl-config --libs)

########
#   This is the default action
all : raytracer # rasteriser


########
#   Object list
#
RAY_OBJ = $(B_DIR)/$(RAY_FILE).o
# RAS_OBJ = $(B_DIR)/$(RAS_FILE).o

########
#   Header file list
COMMON_HEADERS = Makefile $(S_DIR)/SDLauxiliary.h $(S_DIR)/TestModel.h $(S_DIR)/Primitive.h $(S_DIR)/Triangle.h $(S_DIR)/Pixel.h $(S_DIR)/Camera.h $(S_DIR)/Ray.h $(S_DIR)/Material.h
RAY_HEADERS = $(S_DIR)/Intersection.h $(S_DIR)/Light.h $(S_DIR)/Sphere.h
# RAS_HEADERS = $(S_DIR)/Interpolation.h $(S_DIR)/VertexShader.h $(S_DIR)/WireframeShader.h $(S_DIR)/PixelShader.h $(S_DIR)/PointLight.h $(S_DIR)/PostProcess.h

########
#   Objects
$(RAY_OBJ) : $(S_DIR)/$(RAY_FILE).cpp $(COMMON_HEADERS) $(RAY_HEADERS)
	$(CC) $(CC_OPTS) -o $(RAY_OBJ) $(S_DIR)/$(RAY_FILE).cpp $(SDL_CFLAGS) $(GLM_CFLAGS)

# $(RAS_OBJ) : $(S_DIR)/$(RAS_FILE).cpp $(COMMON_HEADERS) $(RAS_HEADERS)
# 	$(CC) $(CC_OPTS) -o $(RAS_OBJ) $(S_DIR)/$(RAS_FILE).cpp $(SDL_CFLAGS) $(GLM_CFLAGS)


########
#   Main build rule
raytracer : $(RAY_OBJ)
	$(CC) $(LN_OPTS) -o $(RAY_EXEC) $(RAY_OBJ) $(SDL_LDFLAGS)

# rasteriser : $(RAS_OBJ)
# 	$(CC) $(LN_OPTS) -o $(RAS_EXEC) $(RAS_OBJ) $(SDL_LDFLAGS)

clean:
	rm -f $(B_DIR)/*
