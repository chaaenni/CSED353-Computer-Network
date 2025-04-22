file (GLOB LIB_SOURCES "*.cc" "util/*.cc" "tcp_helpers/*.cc")
add_library (sponge STATIC ${LIB_SOURCES})
set(CMAKE_CXX_STANDARD 17)  #c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)