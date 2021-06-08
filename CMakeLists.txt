cmake_minimum_required(VERSION 3.19)
project(c-webbug)

set(CMAKE_CXX_STANDARD 14)

add_executable(c-webbug main.cpp request.h request.cpp BeautifulSoup.h BeautifulSoup.cpp)

target_link_libraries(c-webbug ssl)
target_link_libraries(c-webbug crypto)

target_link_libraries(c-webbug iconv)
target_link_libraries(c-webbug Ws2_32)
