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
project(qt-test LANGUAGES C CXX VERSION 1.4.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(QT_MAJOR_VERSION 5)
find_package(PythonLibs REQUIRED)

#See if the environment var is set
if(DEFINED ENV{Qt${QT_MAJOR_VERSION}_HOME})
    message(STATUS "Looking for Qt in: " $ENV{Qt${QT_MAJOR_VERSION}_HOME})
else()
    message(STATUS "Qt${QT_MAJOR_VERSION}_HOME environment variable not set. Checking default paths.")
endif()

find_package(Qt${QT_MAJOR_VERSION} COMPONENTS Core Widgets Gui Concurrent Network Qml Svg UiTools Xml LinguistTools OpenGL REQUIRED PATHS $ENV{Qt${QT_MAJOR_VERSION}_HOME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-DQT_NO_DEBUG_OUTPUT=1)
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_constexpr OUT_CONSTEXPR)
if (NOT "${OUT_CONSTEXPR}" STREQUAL "-1")
	add_definitions(-DHAS_CONSTEXPR=1)
endif()

set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};/usr/local/lib/cmake)

find_package(libm2k REQUIRED)
find_library(LIBUSB_LIBRARIES NAMES usb-1.0 usb)

find_package(PkgConfig)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

include_directories(
	${SCOPY_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)


FILE(GLOB SRC_LIST  src/*.cpp src/*.cc)

FILE(GLOB M2KSCOPE_UIS ui/*.ui)
qt_wrap_ui(m2kscope_FORMS_HEADERS ${M2KSCOPE_UIS})


add_definitions(-DQT_NO_KEYWORDS)
#dump_cmake_variables("iio")

string(TIMESTAMP TODAY "%Y-%m-%d")
string(TIMESTAMP NOW "%H:%M:%S")
cmake_host_system_information(RESULT BUILD_HOST QUERY HOSTNAME)


	add_executable(${PROJECT_NAME} WIN32
		${SRC_LIST}
		${m2kscope_FORMS_HEADERS}
        )

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
		Qt${QT_MAJOR_VERSION}::Widgets
		Qt${QT_MAJOR_VERSION}::Gui
		Qt${QT_MAJOR_VERSION}::Concurrent
		Qt${QT_MAJOR_VERSION}::Qml
		Qt${QT_MAJOR_VERSION}::UiTools
		Qt${QT_MAJOR_VERSION}::Network
		Qt${QT_MAJOR_VERSION}::Svg
		Qt${QT_MAJOR_VERSION}::Xml
		Qt${QT_MAJOR_VERSION}::OpenGL
		${LIBUSB_LIBRARIES}
		${PYTHON_LIBRARIES}
		libm2k::libm2k
)

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
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
)

if (NOT ENABLE_APPLICATION_BUNDLE)
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
endif()

#configure_file(qt.conf.cmakein ${CMAKE_CURRENT_BINARY_DIR}/qt.conf @ONLY)
