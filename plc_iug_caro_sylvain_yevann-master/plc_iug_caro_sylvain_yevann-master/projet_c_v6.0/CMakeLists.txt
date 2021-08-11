# STUDENT CMakeLists.txt
#
# tabs: 8 space

# CMake initialization

cmake_minimum_required(VERSION 3.0)
project("ProjetC_IG")

# Paths

get_filename_component(ROOT_DIR		${CMAKE_CURRENT_LIST_FILE} PATH)
set(HEADER_PATHS			"${ROOT_DIR}/include")
set(TESTS_SRC				"${ROOT_DIR}/tests")
set(DOCS_DIR				"${ROOT_DIR}/docs")
set(SRC					"${ROOT_DIR}/src")

set(LIB_FLAGS				-lfreeimage -lSDL2 -lSDL2_ttf)

# Platform specific definitions

set(WORDS_BIT_SIZE			32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(WORDS_BIT_SIZE		64)
endif()

if(${APPLE})
	set(HEADER_PATHS		${HEADER_PATHS}
					"/opt/local/include/SDL2"
					"/opt/local/include")
	set(PLATFORM_DIR		"${ROOT_DIR}/_macos")
	set(EIBASE			${PLATFORM_DIR}/libeibase.a)
	set(PLATFORM_LIB_FLAGS		${EIBASE}
					-L/opt/local/lib ${LIB_FLAGS})

	message(STATUS "Building for MacOS with eibase: ${EIBASE}")

elseif(${UNIX})

	set(HEADER_PATHS		${HEADER_PATHS}
					"/usr/include/SDL2")
	set(PLATFORM_DIR		"${ROOT_DIR}/_x11")
	set(EIBASE			${PLATFORM_DIR}/libeibase${WORDS_BIT_SIZE}.a)
	set(PLATFORM_LIB_FLAGS		${EIBASE}
					-L${PLATFORM_DIR} ${LIB_FLAGS} -lm)

	set(CMAKE_C_FLAGS_DEBUG "-g -msse4.1")

	set(CMAKE_C_FLAGS_RELEASE "-O3 -msse4.1")

	message(STATUS "Building for Linux with eibase: ${EIBASE}")

else()

	set(SDL				"C:/projet_c_iug_syc/SDL2_windows")
	set(HEADER_PATHS		${HEADER_PATHS}
					"${SDL}/include")
	add_definitions(-D__WIN__=1 -D_CRT_SECURE_NO_WARNINGS=1)

	if(${WORDS_BIT_SIZE} EQUAL 64)
		set(ARCH		x64)
		add_definitions(-D_AMD64_=1)
	else(${WORDS_BIT_SIZE} EQUAL 64)
		set(ARCH		x86)
		add_definitions(-D_X86_=1)
	endif(${WORDS_BIT_SIZE} EQUAL 64)

	set(PLATFORM_DIR		"${ROOT_DIR}/_win")
	set(EIBASE			${PLATFORM_DIR}/eibase${WORDS_BIT_SIZE}.lib)
	set(PLATFORM_LIB_FLAGS		${EIBASE}
					${SDL}/lib/${ARCH}/SDL2.lib
					${SDL}/lib/${ARCH}/SDL2_gfx.lib
					${SDL}/lib/${ARCH}/SDL2_ttf.lib
					${SDL}/lib/${ARCH}/FreeImage.lib)

	message(STATUS "Building for Windows with eibase: ${EIBASE}")

endif(${APPLE})

# General definitions for all targets

include_directories(${HEADER_PATHS})


# target ei (libei)

set(LIB_EI_SOURCES
	${SRC}/clipper.c
	${SRC}/ei_draw_polygon.c
	${SRC}/ei_draw.c
	${SRC}/ei_application.c
	${SRC}/ei_widget.c
	${SRC}/ei_widget_frame.c
	${SRC}/ei_widget_toplevel.c
	${SRC}/ei_widget_button.c
	${SRC}/ei_placer.c
	${SRC}/widget_text.c
	${SRC}/ei_event.c
	${SRC}/picker.c
	${SRC}/hash_table.c
	${SRC}/picking_widget.c
	${SRC}/relief.c)

#
add_library(ei STATIC			${LIB_EI_SOURCES})

# target minimal

add_executable(minimal 			${TESTS_SRC}/minimal.c)
target_link_libraries(minimal		ei ${PLATFORM_LIB_FLAGS})

# target minimal_draw_point

add_executable(minimal_draw_point 			${TESTS_SRC}/minimal_draw_point.c)
target_link_libraries(minimal_draw_point		ei ${PLATFORM_LIB_FLAGS})

# target clipper_test

add_executable(clipper_test 			${TESTS_SRC}/clipper_test.c)
target_link_libraries(clipper_test		ei ${PLATFORM_LIB_FLAGS})

# target lines

add_executable(lines			${TESTS_SRC}/lines.c)
target_link_libraries(lines		ei ${PLATFORM_LIB_FLAGS})

# target frame

add_executable(frame			${TESTS_SRC}/frame.c)
target_link_libraries(frame		ei ${PLATFORM_LIB_FLAGS})

# target button

add_executable(button			${TESTS_SRC}/button.c)
target_link_libraries(button		ei ${PLATFORM_LIB_FLAGS})

# target hello_world

add_executable(hello_world		${TESTS_SRC}/hello_world.c)
target_link_libraries(hello_world	ei ${PLATFORM_LIB_FLAGS})

# target puzzle

add_executable(puzzle			${TESTS_SRC}/puzzle.c)
target_link_libraries(puzzle		ei ${PLATFORM_LIB_FLAGS})

# target two048

add_executable(two048			${TESTS_SRC}/two048.c)
target_link_libraries(two048		ei ${PLATFORM_LIB_FLAGS})

# target MineSweeper

add_executable(minesweeper			${TESTS_SRC}/minesweeper.c)
target_link_libraries(minesweeper		ei ${PLATFORM_LIB_FLAGS})

# target to build the documentation

add_custom_target(doc doxygen		${DOCS_DIR}/doxygen.cfg WORKING_DIRECTORY ${ROOT_DIR})
