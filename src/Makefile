#CXX=clang++
#CXXFLAGS=-g -DDEBUG
#LDFLAGS=`sdl2-config --libs --cflags` -fsanitize=address
#
#TARGET=main.cpp
#
#all:
#	${CXX} ${TARGET} ${CXXFLAGS} -o 21p ${LDFLAGS}
#
#run:
#	./21p
#

CXX=clang++
CXXFLAGS=-O0 -g --std=c++17 -DDEBUG `sdl2-config --cflags`
LDFLAGS=-fsanitize=address `sdl2-config --libs`

#GLFW STUFF
GL_DIR=$(shell pwd)/../GL

GL_INCLUDE_DIR=${GL_DIR}/include
GL_LIBS_DIR=${GL_DIR}/src

#BUILD
BUILD_DIR=./build

IMGUI_DIR = ../imgui
IMPLOT_DIR = ../implot
IMGUI_ADDONS_DIR = ../ImGui-Addons/FileBrowser

CXXFLAGS += -I${GL_INCLUDE_DIR}
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMPLOT_DIR) -I$(IMGUI_ADDONS_DIR) -I$(IMGUI_DIR)/backends -DIMGUI_IMPL_OPENGL_LOADER_GLAD
LDFLAGS += -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -L${GL_LIBS_DIR}

SOURCES = $(GL_DIR)/src/glad.c $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMPLOT_DIR)/implot.cpp $(IMPLOT_DIR)/implot_items.cpp

all: build_dir shader_lib imgui_addons
	${CXX} main.cpp ${SOURCES} ${CXXFLAGS} ${BUILD_DIR}/*.o -o ${BUILD_DIR}/main ${LDFLAGS}

build_dir:
	mkdir ${BUILD_DIR} || true

shader_lib:
	${CXX} -c shader.c ${CXXFLAGS} -o ${BUILD_DIR}/shader.o

imgui_addons:
	${CXX} -c ${IMGUI_ADDONS_DIR}/ImGuiFileBrowser.cpp ${CXXFLAGS} -o ${BUILD_DIR}/filebrowser.o

run:
	${BUILD_DIR}/main

.PHONY: clean
clean:
	rm -rf ${BUILD_DIR} || true
