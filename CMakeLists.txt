cmake_minimum_required(VERSION 3.19)
project(cpp_reptile)

set(CMAKE_CXX_STANDARD 17)

add_executable(cpp_reptile main.cpp request.h request.cpp BeautifulSoup.h BeautifulSoup.cpp ThreadPool.h)

target_link_libraries(cpp_reptile ssl)
target_link_libraries(cpp_reptile crypto)

target_link_libraries(cpp_reptile iconv)
target_link_libraries(cpp_reptile Ws2_32)
