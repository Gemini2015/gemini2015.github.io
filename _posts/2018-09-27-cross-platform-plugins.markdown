---
layout: post
title: Unity跨平台Native插件开发
categories: Game
description: 使用C/C++开发跨平台的Unity插件
codelang: shell
---

## 前言
前一篇[文章](https://blog.icodeten.com/game/2016/07/20/unity-plugins/)介绍了如何使用Objective-C和Java开发iOS和Android平台的插件。使用OC和Java开发插件，主要目的是与系统API进行交互。这种方式基本上能满足大部分需求。
但是有些时候，需要使用一些C/C++开发的库，实现一些对性能要求很严格的代码，或是使用指针来操作数据的时候，就不得不使用C/C++来开发插件。本文主要介绍如何使用C/C++来编写跨平台代码，并使用CMake完成跨平台构建编译。

## 目标平台

*	Windows：针对Unity Windows平台，我们需要将C/C++代码编译成`dll`文件，并导出接口函数供C#代码调用，对应架构为`x64`。
*	iOS：针对iOS平台，需要将C/C++代码编译成`.a`文件，如果只是真机使用的话，只需要支持`armv7 armv7s arm64`三种架构，如果需要支持模拟器的话，则需要考虑支持`x86_64`。
*	Android：针对Android平台，需要将C/C++代码编译成`.so`文件，Android手机运行的话，需要支持`armeabi-v7a`，如果需要支持模拟器，那么还需要支持`x86`或`x86_64`。

## 条件编译

毕竟各个平台操作系统之间还是有一些差别的，比如不同平台需要包含的头文件不同，函数签名也不同，要想同一份代码在不同平台都可以正常编译执行，就不得不使用条件编译。
以下列出一些常见的预定义宏，可以在代码中用来判断当前操作系统。

*	`_WIN32`：使用MSVC进行C/C++开发的话，可以使用这个宏定义来判断当前是否是Windows平台。
*	`__APPLE__`：使用这个宏定义来判断是否是Mac平台。
*	`__ANDROID__`：使用这个宏定义来判断是否是Android系统。

需要注意的是，宏定义需要特定的编译器支持才有效果，比如对于`_WIN32`这个宏定义来说，MSVC和GCC都是支持的。

## 函数导出

可以使用如下方式导出函数，供C#端进行调用。

```
// my_plugin.h

#if _WIN32

// 针对Windows平台导出dll
#ifdef MyPlugin_EXPORTS
#define MYPLUGIN_API __declspec(dllexport)
#else
#define MYPLUGIN_API __declspec(dllimport)
#endif

#else

#define MYPLUGIN_API

#endif

extern "C" {
	MYPLUGIN_API bool MyFunc(const char* string_arg, bool bool_arg, int int_arg);
}
```

`dllexport`和`dllimport`用于声明目标函数是否需要从当前dll中导出，或是需要从其它dll中导入。`MyPlugin_EXPORTS`是CMake自动添加的。

## CMake

编写如下CMakeList.txt文件

```
# CMakeList.txt
project(MyPlugin)

set(SRC_LIST
    my_plugin.cpp
    )

set(INC_LIST
    my_plugin.h
    )

if(MSVC)
    # Windows 动态链接库
    add_library(${PROJECT_NAME} SHARED ${SRC_LIST} ${INC_LIST})
elseif(ANDROID)
    # Android 动态链接库
    add_library(${PROJECT_NAME} SHARED ${SRC_LIST} ${INC_LIST})
elseif(IOS)
    # iOS 静态库
    add_library(${PROJECT_NAME} STATIC ${SRC_LIST} ${INC_LIST})
else()
    # Mac 动态链接库
    add_library(${PROJECT_NAME} MODULE ${SRC_LIST} ${INC_LIST})
    set_target_properties(${PROJECT_NAME} PROPERTIES BUNDLE TRUE)
endif()
```

## 构建

### Windows

1.	在CMakeList.txt文件目录下，创建一个Build目录。
2.	命令行进入Build目录，`cmake -G "Visual Studio 14 Win64" ..`，会生成一个VS工程。
3.	VS打开生成的工程，编译生成的dll就可以供Unity项目使用了。

### Mac

1.  在CMakeList.txt文件目录下，创建一个Build目录。
2.  命令行进入Build目录，`cmake -G "Xcode" ..`，会生成一个Xcode工程。
3.  Xcode打开生成的工程，编译生成的.bundle就可以供Unity项目使用了。

### iOS

iOS使用CMake的话，需要用到一个cmake文件，这个cmake文件可以在[这里](https://github.com/leetal/ios-cmake)下载到。
将这个**ios.toolchain.cmake**放到CMakeList.txt同级目录下。

1.	在CMakeList.txt文件目录下，创建一个Build目录。
2.	命令行进入Build目录，`cmake .. -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DIOS_PLATFORM=OS`，会生成一个基于Makefile的工程。
3.	`make`一下，就可以生成对应的`.a`文件了。

### Android

Android使用CMake的话，要依赖Android NDK里面的一个cmake文件，所以构建之前需要先安装Android NDK，版本为**r17b**
在系统环境变量中设置Android NDK的路径，保存在`ANDROID_NDK`这个变量里。

1.	在CMakeList.txt文件目录下，创建一个Build目录。
2.	命令行进入Build目录，`cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI="armeabi-v7a" -DANDROID_ARM_NEON=ON -DANDROID_PLATFORM=android-14 ..`，会生成一个基于Makefile的工程。
3.	`make`一下，就可以生成对应的`.so`文件了。


