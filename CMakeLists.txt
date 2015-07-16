project(dfoam)

cmake_minimum_required(VERSION 2.6)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(USE_OPENMP true CACHE BOOL "Use OpenMP")

set(ADDITIONAL_LIBS "curl")

find_package(CURL REQUIRED)

find_package(OpenMP)
if(USE_OPENMP)
	if(OPENMP_FOUND)
		find_package(Threads REQUIRED)
		add_definitions( -DOPENMP_FOUND )
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	endif()
endif()

find_package(Boost COMPONENTS
  program_options
  regex
  system
  random
  REQUIRED)

file(GLOB bot_headers "*_bot.h")

foreach(bot_header ${bot_headers})

    string(REGEX REPLACE "^.*/([^/]+)_bot.h$" "\\1" bot_name "${bot_header}")
    set(bot_header "${bot_name}_bot.h")
    set(bot_src "${bot_name}_bot.cpp")
    set(bot_bin "client_${bot_name}")
    string(TOUPPER "BOT${bot_name}" bot_definition)

    message(STATUS "++ ${bot_name} ${bot_src} ${bot_header} ${bot_bin} ${bot_definition}")

    add_executable(${bot_bin}
        position.cpp
        utils.cpp
        game.cpp
		state.cpp
        ${bot_src}
        options.cpp
        network.cpp
        tiles.cpp
        client.cpp
        )

    set_target_properties(${bot_bin}
        PROPERTIES COMPILE_DEFINITIONS "BOTINCLUDE=\"${bot_header}\";${bot_definition}"
        )

    target_link_libraries(${bot_bin}
        ${Boost_REGEX_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_RANDOM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        ${ADDITIONAL_LIBS}
        )
endforeach()
