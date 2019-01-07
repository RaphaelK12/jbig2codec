#
# Copyright (c) 2018, Avia Systems Limited.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Avia Systems Limited or its parts including
#       Scan2CAD nor the names of its contributors may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# It is a standalone program based on JBIG2 codec from the Pdfium project
# which is intended to decode JBIG2 images.
#
# Verbose output of the build process can be activated using 'make VERBOSE=1'.
#
cmake_minimum_required(VERSION 3.2)
project(jbig2codec)

# Program version
set(JBIG2C_VERSION "0.1.4")

# Program name
set(JBIG2C "jbig2codec")
set(JBIG2C_LIB "libjb2")
set(JBIG2C_LIB_NAME "jb2")

# Load modules from our source tree too
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Filename.cmake)

add_definitions(-DJBIG2C_VERSION="${JBIG2C_VERSION}")

# Add JBIG2 codec defitions.
#
# These definitions are necessary to make it work while being extracted from
# the Pdfium codebase.
#
# Set C++11 standard (introduced in CMake 3.1)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	# If the _FX_OS_ equals 4 and it means that we are building the
	# program on the GNU/Linux.
	#
	# There is a set of macros defined in core/fxcrt/fx_system.h, but
	# no specific one for FreeBSD. This is why it has been defined as for
	# GNU/Linux for compatibility.
	add_definitions(-D_FX_OS_=4)
endif()
add_definitions(-DMEMORY_TOOL_REPLACES_ALLOCATOR=1)

include(CheckFunctionExists)
include(CheckIncludeFiles)

# Set flags here
if (CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "[OPTION] Building ${JBIG2C} ${JBIG2C_VERSION}, debug version")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
	if (CMAKE_COMPILER_IS_GNUCC OR
			${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		add_definitions(" -g -std=c++11")
		#add_definitions(" -g -Wall -pedantic -std=c++11")
		#add_definitions(" -Wshadow -Wpointer-arith -Wcast-qual")
		#add_definitions(" -Wcast-align -Wstrict-prototypes")
		#add_definitions(" -Wmissing-prototypes -Wconversion -Wsign-compare")
	endif()
else()
	message(STATUS "[OPTION] Building ${JBIG2C} ${JBIG2C_VERSION}, release version")
	if (CMAKE_COMPILER_IS_GNUCC OR
			${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		add_definitions("-std=c++11")
		#add_definitions(" -Wall -pedantic -std=c++11")
		#add_definitions(" -Wshadow -Wpointer-arith -Wcast-qual")
		#add_definitions(" -Wcast-align -Wstrict-prototypes")
		#add_definitions(" -Wmissing-prototypes -Wconversion -Wsign-compare")
	endif()
endif()

# Set sources here
set(JBIG2C_LIB_SRC	core/fxcodec/jbig2/JBig2_ArithDecoder.cpp
			core/fxcodec/jbig2/JBig2_ArithIntDecoder.cpp
			core/fxcodec/jbig2/JBig2_BitStream.cpp
			core/fxcodec/jbig2/JBig2_Context.cpp
			core/fxcodec/jbig2/JBig2_GrdProc.cpp
			core/fxcodec/jbig2/JBig2_GrrdProc.cpp
			core/fxcodec/jbig2/JBig2_HtrdProc.cpp
			core/fxcodec/jbig2/JBig2_HuffmanDecoder.cpp
			core/fxcodec/jbig2/JBig2_HuffmanTable.cpp
			core/fxcodec/jbig2/JBig2_Image.cpp
			core/fxcodec/jbig2/JBig2_PatternDict.cpp
			core/fxcodec/jbig2/JBig2_PddProc.cpp
			core/fxcodec/jbig2/JBig2_SddProc.cpp
			core/fxcodec/jbig2/JBig2_Segment.cpp
			core/fxcodec/jbig2/JBig2_SymbolDict.cpp
			core/fxcodec/jbig2/JBig2_TrdProc.cpp
			core/fxcodec/codec/ccodec_jbig2module.cpp
			core/fxcodec/codec/ccodec_faxmodule.cpp
			core/fxcodec/codec/ccodec_scanlinedecoder.cpp)
file(GLOB_RECURSE FXCRT_CPPSRC "core/fxcrt/*.cpp")
file(GLOB_RECURSE FXCRT_CCSRC "core/fxcrt/*.cc")
list(APPEND JBIG2C_LIB_SRC ${FXCRT_CPPSRC})
list(APPEND JBIG2C_LIB_SRC ${FXCRT_CCSRC})

file(GLOB_RECURSE TP_BASE_CPPSRC "third_party/base/*.cpp")
file(GLOB_RECURSE TP_BASE_CCSRC "third_party/base/*.cc")
list(APPEND JBIG2C_LIB_SRC ${TP_BASE_CPPSRC})
list(APPEND JBIG2C_LIB_SRC ${TP_BASE_CCSRC})

# Look for libraries, packages, etc.

# Set target libraries here

# Check functions, include files here
if (NOT MSVC)
	check_function_exists(fmax RESULT)
	if (NOT RESULT)
		unset(RESULT)
		list(APPEND TARGET_LIBS -lm)
	endif()
endif()

# Define include files and directories here
include_directories("./")

# Compile jbig2codec library and executable
add_library(${JBIG2C_LIB} STATIC ${JBIG2C_LIB_SRC})
add_executable(${JBIG2C} main.cpp)
set_target_properties(${JBIG2C_LIB} PROPERTIES OUTPUT_NAME ${JBIG2C_LIB_NAME})
define_filename_for_sources(${JBIG2C_LIB})

# Link jbig2codec executable
target_link_libraries(${JBIG2C} ${JBIG2C_LIB})

# Install jbig2codec executable
install(TARGETS ${JBIG2C} DESTINATION bin)
