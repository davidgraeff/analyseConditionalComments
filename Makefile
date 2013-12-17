ROOT_DIR := $(shell pwd)
BUILD_DIRECTORY := ${ROOT_DIR}/build
CMAKE_FILES := $(shell find -name CMakeLists.txt)
CMAKE_ARGS := ${CMAKE_USER_ARGS}

ifeq (${NINJA},1)
BUILDCMD := ninja
CMAKE_ARGS += -G"Ninja"
else
BUILDCMD := make --no-print-directory
endif

all:

configure ${BUILD_DIRECTORY}/Makefile: ${CMAKE_FILES}
	@mkdir -p ${BUILD_DIRECTORY}
	@cd ${BUILD_DIRECTORY}; cmake ${CMAKE_ARGS} ${ROOT_DIR}

mrproper:
	rm -rf ${BUILD_DIRECTORY}

%::
	@if [ ! -e ${BUILD_DIRECTORY} ]; then make configure BUILD_DIRECTORY=${BUILD_DIRECTORY}; fi
	+@cd ${BUILD_DIRECTORY}; $(BUILDCMD) $@

run:
	${BUILD_DIRECTORY}/analyseCSourceFiles
	
.PHONY: configure mrproper
