# Copyright (c) 2019 Analog Devices Inc.
#
# This file is part of Scopy
# (see http://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.5)
project(GrFlowGraphRunner LANGUAGES C CXX VERSION 1.4.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(PythonLibs REQUIRED)

if(ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
endif()

find_package(Qt5 COMPONENTS Core Widgets Gui Concurrent OpenGL REQUIRED PATHS $ENV{Qt5_HOME})

if(ANDROID)
        find_package(Qt5AndroidExtras REQUIRED)
	find_path(LIBUSB_INCLUDE_DIRS NAMES libusb.h)
	find_library(LIBUSB_LIBRARIES NAMES usb-1.0 usb)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};/usr/local/lib/cmake)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

if(ANDROID)
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt_${ANDROID_ABI})
else()
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt)
endif()


include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
	${PYTHON_INCLUDE_DIRS}
	${QWT_INCLUDE_DIRS}
	${LIBUSB_INCLUDE_DIRS}
)


FILE(GLOB SRC_LIST  src/*.cpp src/*.cc)
FILE(GLOB UIS ui/*.ui)
qt5_wrap_ui(UI_HEADERS ${UIS})

add_definitions(-DQT_NO_KEYWORDS)

if(ANDROID)
    add_library(${PROJECT_NAME} SHARED
	${SRC_LIST}
	${UIS} )
else()
    add_executable(${PROJECT_NAME}
	${SRC_LIST}
	${UIS} )
endif()


target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
		Qt5::Widgets
		Qt5::Gui
		Qt5::Concurrent
		Qt5::OpenGL
		${PYTHON_LIBRARIES}
		${QWT_LIBRARIES}

)

if(ANDROID)
    target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
	${Qt5AndroidExtras_LIBRARIES}
	${LIBUSB_LIBRARIES}
	log
	android)
endif()

if (NOT WIN32)
	find_library(PTHREAD_LIBRARIES pthread)
	if (PTHREAD_LIBRARIES)
		target_link_libraries(${PROJECT_NAME} LINK_PRIVATE ${PTHREAD_LIBRARIES})
	endif()
endif()

# Compiler options
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -fPIC)

#List of warnings to be treated as errors
target_compile_options(${PROJECT_NAME} PUBLIC
	-Werror=return-type
	-Werror=uninitialized
	-Werror=init-self
	-Werror=switch
)

set_target_properties(${PROJECT_NAME} PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)
