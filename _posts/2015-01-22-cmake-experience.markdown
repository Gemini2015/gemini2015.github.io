---
layout: post
title: CMake经验
date: 2015-01-22 22:32:11
categories: CMake
description: 使用CMake的一些经验
codelang: cmake
---


## 命令

### 源文件收集
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


### 同一个工程同时生成静态库和动态库
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


### 设置头文件目录和静态库
很多时候，我们会使用第三方库，因此，我们要在工程里面包含第三方库的头文件，链接第三方提供的静态链接库(如果有的话)。

```
# 包含头文件
# include_directories 具有继承性
# 当前 CMakeLists 里面包含的目录，会被 add_subdirectory 里面的 CMakeLists 继承
include_directories ( "D:/Program Files (x86)/Lua/include" )

# 库目录
# eg. 链接 Linux 下的 X11
link_directories(/usr/include/X11)
SET(SYS_LIB_LIST "x11")

# 链接库
set(LUA_LIB "D:/Program Files (x86)/Lua/lib/lua51.lib")
target_link_libraries(target ${LUA_LIB})
```

### 生成配置头文件
一般可以用CMake来进行平台检测，那么CMake的检测结果如何告诉C++工程呢？CMake提供了`configure_file`命令。
`configure_file`命令输入一个文本文件，处理文本文件中包含的CMake的指令，然后生成一个文本文件。

```
configure_file(
	"${PROJECT_SOURCE_DIR}/platform/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
	)
```

```
/*
	config.h.in 文件
*/
#cmakedefine PLATFORM_WIN32
#cmakedefine PLATFORM_LINUX
```
如果 CMakeLists.txt 中定义了`PLATFORM_WIN32`变量，那么`#cmakedefine PLATFORM_WIN32`就会变成C++的宏定义语句`#define PLATFORM_WIN32`。

### 生成Visual Studio筛选器
可以通过一个宏来为收集的文件建立筛选器。代码如下：

```
# file(GLOB_RECURSE all_files *.*)
# create_filters(all_files)
# add_executable(app ${all_files})

macro(create_filters source_files)
	if(MSVC)
		# 获取当前目录
		set(current_dir ${CMAKE_CURRENT_SOURCE_DIR})
		foreach(src_file ${${source_files}})
			# 求出相对路径
			string(REPLACE ${current_dir}/ "" rel_path_name ${src_file})
			# 删除相对路径中的文件名部分
			string(REGEX REPLACE "(.*)/.*" \\1 rel_path ${rel_path_name})
			# 比较是否是当前路径下的文件
			string(COMPARE EQUAL ${rel_path_name} ${rel_path} is_same_path)
			# 替换成Windows平台的路径分隔符
			string(REPLACE "/" "\\" rel_path ${rel_path})
			if(is_same_path)
				set(rel_path "\\")
			endif(is_same_path)

			# CMake 命令
			source_group(${rel_path} FILES ${src_file})
		endforeach(src_file)
	endif(MSVC)
endmacro(create_filters)
```

## 编译选项

### Linux使用 C++ 11
在Linux下编译下面的代码时出错。

```
typedef std::map<key, value> Dict_Map;
Dict_Map::iterator it = DictMap.begin();
while(it != DictMap.end())
{
	it = DictMap.erase(it);
}
```
出错提示是`erase函数没有返回值`。
查看源码，发现`#if __cplusplus >= 201103L`时，`iterator erase(const_iterator __position)`。否则，`void erase(iterator __position)`。
改成使用 C++ 11 就好了。

```
# Use C++11 in linux
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
```

### MSVC 禁用特定警告
**一般情况下，不要禁用任何警告，而应该积极解决每一个警告。**
但是有些情况下，当你充分了解某些警告可能造成的后果时，也可以禁用该警告。
比如要禁用警告4819(该文件包含不能在当前代码页中表示的字符)，可以采用下面的语句：

```
if(MSVC)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819")

endif()
```

## 调试运行

### Visual Studio设置工作目录
使用Visual Studio开发的时候，假设编译构建生成的程序在`binarypath/bin/debug`目录下。我们调试时，会有一些数据要使用，假设这些数据在`binarypath/data`目录下，如果程序查找数据文件时，只在当前目录下查找，那么在这种情况下，就无法找到数据。要解决这个问题，可以在Visual Studio中设置工作目录，这样程序就能找到数据文件。
Visual Studio的工作目录设置是保存在`projectname.vcxproj.user`文件中，CMake没有提供专门的命令来设置工作目录，但是我们可以通过类似配置文件的方式来手动生成`projectname.vcxproj.user`文件。
**perconfig.vcxproj.user.in**

```
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='@USERFILE_CONFIGNAME@|@USERFILE_PLATFORM@'">
	<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
	<LocalDebuggerCommandArguments>@USERFILE_COMMAND_ARGUMENTS@</LocalDebuggerCommandArguments>
	<LocalDebuggerWorkingDirectory>@USERFILE_WORKING_DIRECTORY@</LocalDebuggerWorkingDirectory>
</PropertyGroup>
```
**vcxproj.user.in**

```
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="@USERFILE_VC_VERSION@" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
@USERFILE_CONFIGSECTIONS@
</Project>
```
CMake 模块

```
file(READ "${_launchermoddir}/perconfig.vcxproj.user.in" _perconfig)
set(USERFILE_CONFIGSECTIONS)
# 对于每一个配置（Debug, Release, RelWithDebInfo, MinSizeRel），生成对应的配置项
foreach(USERFILE_CONFIGNAME ${CMAKE_CONFIGURATION_TYPES})
	string(CONFIGURE "${_perconfig}" _temp @ONLY ESCAPE_QUOTES)
	string(CONFIGURE
		"${USERFILE_CONFIGSECTIONS}${_temp}"
		USERFILE_CONFIGSECTIONS
		ESCAPE_QUOTES)
endforeach()

configure_file("${_launchermoddir}/vcxproj.user.in"
	${VCPROJNAME}.vcxproj.user
	@ONLY)
```
**perconfig.vcxproj.user.in**中出现的`@USERFILE_CONFIGNAME@`、`@USERFILE_PLATFORM@`、`@USERFILE_PLATFORM@`、`@USERFILE_COMMAND_ARGUMENTS@`、`@USERFILE_WORKING_DIRECTORY@`等，都是在CMake模块中预先设置好的变量，通过`string(CONFIGURE "${_perconfig}" _temp @ONLY ESCAPE_QUOTES)`语句填到配置里面去的。

### 拷贝运行时库
对于需要动态链接的库，一般是直接在可执行程序目录下查找的，所以对于项目中使用的第三方动态链接库，我们需要在用CMake构建项目的时候，将这些动态链接库拷贝到最终可执行文件目录下。

**拷贝动态链接库**，用于调试时运行。

```
macro(copy_dll depdir dllname)
	foreach(configuration ${CMAKE_CONFIGURATION_TYPES})
		# 对应于不同的构建版本
		set(dllpath "${depdir}/bin/${configuration}/${dllname}")
		if(EXISTS ${dllpath})
			configure_file(${dllpath} ${CMAKE_BINARY_DIR}/bin/${configuration}/${dllname} COPYONLY)
		endif()
	endforeach()
endmacro()
```

**安装动态链接库**，用于最终应用程序安装部署。

```
macro(install_dll depdir dllname)
	foreach(configuration ${CMAKE_CONFIGURATION_TYPES})
		set(dllpath "${depdir}/bin/${configuration}/${dllname}")
		if(EXISTS ${dllpath})
			install(FILES ${dllpath} DESTINATION bin/${configuration} CONFIGURATIONS ${configuration})
		endif()
	endforeach()
endmacro()
```


## 安装

### 设置安装路径前缀
使用`install`命令安装项目的时候，如果路径给出的是**全路径**，那么会直接使用这个路径，如果给出的是相对路径，那么会加上`CMAKE_INSTALL_PREFIX`，默认情况下这个值为`C:/Program Files (x86)`。可以直接在CMakeLists里面设置这个值，也可以在CMake GUI里面设置。

```
set(CMAKE_INSTALL_PREFIX "D:/Program Files (x86)/" CACHE PATH "Project install directory" FORCE)
```

### 根据配置设置安装目录
CMake预定义了四种编译配置: Debug、Release、RelWithDebInfo、MinSizeRel。大多数时候，我们只会安装Release版本。但是在某些情况下，比如A项目链接B项目的静态库，当A项目采用Debug编译时，需要链接B项目的Debug版本，当A项目采用Release编译时，需要链接B项目的Release版本。

假设我们使用了`link_directories(${PROJECT_BINARY_DIR}/lib)`命令设置了静态库查找目录，那么在Visual Studio中，实际查找目录是`/projectpath/lib`和`/projectpath/lib/$(Configuration)`，`$(Configuration)`表示编译配置。

因此，我们在设置安装目录时，也可以采用`lib/Debug`和`lib/Release`这种结构。使用下面的代码可以完成这个功能。

```
foreach(lib_file platform)
	install(TARGETS ${lib_file}
		RUNTIME DESTINATION "bin/Debug" CONFIGURATIONS Debug
		LIBRARY DESTINATION "lib/Debug"  CONFIGURATIONS Debug
		ARCHIVE DESTINATION "lib/Debug" CONFIGURATIONS Debug
		)
	install(TARGETS ${lib_file}
		RUNTIME DESTINATION "bin/Release" CONFIGURATIONS Release
		LIBRARY DESTINATION "lib/Release"  CONFIGURATIONS Release
		ARCHIVE DESTINATION "lib/Release" CONFIGURATIONS Release
		)
	install(TARGETS ${lib_file}
		RUNTIME DESTINATION "bin/RelWithDebInfo" CONFIGURATIONS RelWithDebInfo
		LIBRARY DESTINATION "lib/RelWithDebInfo"  CONFIGURATIONS RelWithDebInfo
		ARCHIVE DESTINATION "lib/RelWithDebInfo" CONFIGURATIONS RelWithDebInfo
		)
	install(TARGETS ${lib_file}
		RUNTIME DESTINATION "bin/MinSizeRel" CONFIGURATIONS MinSizeRel
		LIBRARY DESTINATION "lib/MinSizeRel"  CONFIGURATIONS MinSizeRel
		ARCHIVE DESTINATION "lib/MinSizeRel" CONFIGURATIONS MinSizeRel
		)
endforeach()
```

## 工具

作为一个项目构建工具，CMake用一种优雅的方式，描述了项目构建的步骤。
但是对于大型项目而言，光是使用CMake简化项目构建过程还是不够的，还需要配备一套程序配合CMake来完成项目构建的过程。

### 编译加速
CMake只完成了从源文件到建立项目工程的过程，而真正编译，则还是要依赖于特定的编译工具，比如Visual Studio、gcc等。
对于大型工程而言，工程的编译时间也是一件值得优化的事。目前，有一些工具可以加速基于CMake构建的工程的编译过程。

*	[Electric Cloud Free CMake Accelerator](https://electric-cloud.com/plugins/build-automation/cmake-acceleration/)：似乎和 [Incredibuild](https://www.incredibuild.com/) 类似，利用本地集群分布式编译。