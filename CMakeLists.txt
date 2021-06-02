cmake_minimum_required(VERSION 3.19)
project(juanjuan)

set(CMAKE_CXX_STANDARD 14)

add_executable(juanjuan main.cpp request.h request.cpp BeautifulSoup.h BeautifulSoup.cpp)

target_link_libraries(juanjuan ssl)
target_link_libraries(juanjuan crypto)
