##
##  Copyright (C) 2015 Simon Boyé
##
##  This file is part of lof3.
##
##  lair is free software: you can redistribute it and/or modify it
##  under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  lair is distributed in the hope that it will be useful, but
##  WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##  General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with lair.  If not, see <http://www.gnu.org/licenses/>.
##

cmake_minimum_required(VERSION 2.8.12.1)

project(lof3)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS
	system
	filesystem
)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

set(LAIR_INCLUDE_DIR ../lair/include CACHE PATH "Lair include dir")
set(LAIR_LIBRARY ../lair/src/liblair.a CACHE FILEPATH "Lair library")

include_directories(
	"${EIGEN3_INCLUDE_DIR}"
	"${GLEW_INCLUDE_DIRS}"
	"${SDL2_INCLUDE_DIR}"
	"${Boost_INCLUDE_DIRS}"
	"${LAIR_INCLUDE_DIR}"
)

add_executable(lof3
	src/frame.cpp
	src/menu.cpp
	src/game.cpp
	src/main_state.cpp
	src/main.cpp
)

add_definitions(-D "'DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/assets\"'")

target_link_libraries(lof3
	${LAIR_LIBRARY}
	SDL2_image
	"${GLEW_LIBRARIES}"
	"${SDL2_LIBRARY}"
	"${Boost_LIBRARIES}"
	GL
)
