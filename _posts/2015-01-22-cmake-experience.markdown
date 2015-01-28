---
layout: post
title: CMake经验
date: 2015-01-22 22:32:11
categories: CMake
description: 使用CMake的一些经验
---


## 源文件收集
一般情况下，我们通过逐个列出的方式，设定源文件集(`set(SRC_LIST src/main.c src/other.c)`)。这样的好处是可以**明确控制那些文件会被加入到工程中**。
但是有些时候，如果源文件较多，一个一个列出来的话可能会有些麻烦，对于这种情况，可以使用CMake提供的`file`命令来自动收集源文件。使用方法如下。

```
file(GLOB_RECURSE SRC_CORE src/*.c)
```
通过上面这行代码，会生成一个源文件列表，每一项是一个**源文件的全路径**。可以通过下面的代码来剔除一些不想加入工程的源文件。

```
foreach(rm_file ${SRC_CORE})
	string(REGEX MATCH ".*/filename1.c|.*/filename2.c" need_remove_file ${rm_file})
	if(need_remove_file)
		list(REMOVE_ITEM SRC_CORE ${need_remove_file})
	endif(need_remove_file)
endforeach(rm_file)
```
上面使用了CMake的正则表达式，匹配两个文件，分别为`.*/filename1.c`和`.*filename2.c`，其中`.*`代表了任意字符串，`filename1.c`是文件名。


## 同一个工程同时生成静态库和动态库
有了如标题所述的需求，在网上搜索了一下，发现大部分博客都是讲通过设置`OUTPUT_NAME`，使两个工程(一个动态库，一个静态库)输出同名二进制文件。但是我不希望同样的源码，要创建两个工程。
可以使用下面的方法为使用一个工程同时生成动态库和静态库。

```
# 添加控制选项
# BUILD_SHARED_LIBS 是CMake内置变量
# 针对WIN32可以

if(WIN32)
	option(BUILD_SHARED_LIBS "Build Shared libs" ON)
	option(BUILD_AS_DLL "Build as dll" ${BUILD_SHARED_LIBS})
endif()

add_library(libHello ${SRC_LIST})
target_link_libraries(libHello ${EXTRA_LIB})

if(BUILD_AS_DLL)
	set_target_properties(libHello PROPERTIES COMPILE_DEFINITIONS BUILD_AS_DLL)
endif()
```


## 设置头文件目录和静态库
很多时候，我们会使用第三方库，因此，我们要在工程里面包含第三方库的头文件，链接第三方提供的静态链接库(如果有的话)。

```
# 包含头文件
# include_directories 具有继承性
# 当前 CMakeLists 里面包含的目录，会被 add_subdirectory 里面的 CMakeLists 继承
include_directories ( "D:/Program Files (x86)/Lua/include" )

# 链接库
set(LUA_LIB "D:/Program Files (x86)/Lua/lib/lua51.lib")
target_link_libraries(target ${LUA_LIB})
```


## 设置安装路径前缀
使用`install`命令安装项目的时候，如果路径给出的是**全路径**，那么会直接使用这个路径，如果给出的是相对路径，那么会加上`CMAKE_INSTALL_PREFIX`，默认情况下这个值为`C:/Program Files (x86)`。可以直接在CMakeLists里面设置这个值，也可以在CMake GUI里面设置。

```
set(CMAKE_INSTALL_PREFIX "D:/Program Files (x86)/" CACHE PATH "Project install directory" FORCE)
```
