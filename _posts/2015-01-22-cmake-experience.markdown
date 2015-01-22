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


