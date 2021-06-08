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

并且Windows下默认的字符集就是GBK，不需要转UTF8
要注释掉下面函数
```
GBKToUTF8
```