cmake_minimum_required(VERSION 3.16)
project(SeekerExternal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(SeekerExternal
    src/main.cpp
    src/Memory.cpp
    src/Game.cpp
    src/Offsets.cpp
    src/Entity.cpp
    src/Features.cpp
    src/Overlay.cpp
    src/Settings.cpp
    src/MenuOverlay.cpp
    src/Config.cpp
)

target_include_directories(SeekerExternal PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(SeekerExternal PRIVATE user32 gdi32)
if(NOT MSVC)
    target_link_options(SeekerExternal PRIVATE -static-libgcc -static-libstdc++)
endif()

# Console vs windowed (MinGW uses -mconsole/-mwindows, MSVC uses /SUBSYSTEM)
if(NOT DEFINED CONSOLE)
    set(CONSOLE ON)
endif()
if(MSVC)
    if(CONSOLE)
        set_target_properties(SeekerExternal PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    else()
        set_target_properties(SeekerExternal PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    endif()
else()
    if(CONSOLE)
        set_target_properties(SeekerExternal PROPERTIES LINK_FLAGS "-mconsole")
    else()
        set_target_properties(SeekerExternal PROPERTIES LINK_FLAGS "-mwindows")
    endif()
endif()
