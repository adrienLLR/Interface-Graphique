# STUDENT CMakeLists.txt
#
# tabs: 8 space

# CMake initialization

cmake_minimum_required(VERSION 2.8)
project("ProjetC_IG")

# Paths

get_filename_component(ROOT_DIR		${CMAKE_CURRENT_LIST_FILE} PATH)
set(HEADER_PATHS			"${ROOT_DIR}/include")
set(TESTS_SRC				"${ROOT_DIR}/tests")
set(DOCS_DIR				"${ROOT_DIR}/docs")
set(SRC					"${ROOT_DIR}/src")

set(LIB_FLAGS				-lfreeimage -lSDL2 -lSDL2_ttf -lSDL2_gfx)

# Platform specific definitions

if(IG_ASAN)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
endif(IG_ASAN)

set(WORDS_BIT_SIZE			32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(WORDS_BIT_SIZE		64)
endif()

if(${APPLE})
	set(HEADER_PATHS		${HEADER_PATHS}
					"/opt/local/include/SDL2"
					"/opt/local/include")
	set(PLATFORM_DIR		"${ROOT_DIR}/_osx")
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

	message(STATUS "Building for Linux with eibase: ${EIBASE}")

else()

	set(SDL				"C:/projetc/SDL2_windows")
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
    ${SRC}/ei_application.c
	${SRC}/ei_draw.c
	${SRC}/ei_event.c
	${SRC}/ei_geometrymanager.c
	${SRC}/ei_widgetclass.c
	${SRC}/ei_widget.c
	${SRC}/ei_struct_for_bind.c
	${SRC}/ei_utilsfct.c	)

add_library(ei STATIC			${LIB_EI_SOURCES})

# target minimal

add_executable(minimal 			${TESTS_SRC}/minimal.c)
target_link_libraries(minimal		${PLATFORM_LIB_FLAGS})

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

# target minesweeper

add_executable(minesweeper		${TESTS_SRC}/minesweeper.c)
target_link_libraries(minesweeper	ei ${PLATFORM_LIB_FLAGS})

# target test

add_executable(test		${TESTS_SRC}/test.c)
target_link_libraries(test	ei ${PLATFORM_LIB_FLAGS})

# target parsing

add_executable(parsing			${TESTS_SRC}/parsing.c)
target_link_libraries(parsing		ei ${PLATFORM_LIB_FLAGS})

# target to build the documentation

add_custom_target(doc doxygen		${DOCS_DIR}/doxygen.cfg WORKING_DIRECTORY ${ROOT_DIR})
