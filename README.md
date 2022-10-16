# c++_reptile

c++爬虫

c++实现了requets，BeautifulSoup

# 基本环境需求
## linux
从github上下载openssl，然后
```
./Configure
make
make test
make install
```
##windows
安装msys2(国内注意换源)

在msys2中安装gcc，make，mingw
```
pacman -S gcc
pacman -S make
pacman -S iconv
pacman -S mingw-w64-i686-toolchain
pacman -S mingw-w64-x86_64-toolchain
```

从github下载openssl
然后
```
./Configure
make
make test
make install
```

# windows
需要在CMakeLists中添加下面的链接库
```
target_link_libraries(c-webbug iconv)
target_link_libraries(c-webbug Ws2_32)
```

注意有的时候网页的编码是utf-8，在windows上编程的时候注意要将编码从UTF-8转为GBK

requests现在还没有能自动处理301重定向，还需要自己注意，之后再补这个功能