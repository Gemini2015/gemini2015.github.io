---
layout: post
title: Unity3D插件开发Tips
categories: Game
description: 关于Unity3D插件开发的一些经验建议
codelang: java
---



使用Unity将近一年了，虽然还只是皮毛水平，但是凡是学习，总是有收获的，有收获就应该分享。
这一年来，项目进度一直处于很紧张的状态，经常是抽不出时间整理所学的知识，当有空闲时间时，又因为各种各样的原因，而未能将脑海中的片段整理成文字。
在接下来的一段时间里，将不定期更新关于Unity的实践经验，内容分为多个部分，从插件开发，性能优化，到自动化编译打包脚本的编写等。


## 使用插件
Unity本身已经很强大了，基本上大部分需求都有相应的C# API可以实现，但是某些特殊情况下，不得不通过编写插件的方式来扩展Unity的功能（比如文本拷贝）。
考虑到**Windows Phone**可怜的市场占有率，所以大部分游戏都很少发布WP版本，故而也不需要开发WP插件，因此一般所说的写插件主要是面向**iOS**和**Android**平台。
对于现成的插件，要想在Unity中使用的话，还是比较简单的。

### iOS
iOS插件一般以静态库的方式提供（后缀名为`.a`的文件），这个静态库里面一般会导出一些函数，供外部调用。
假设名为`libfunc.a`静态库中导出了如下两个方法:

```
extern "C" {
	void func1(int intValue, const char* strValue);
	const char* func2();
}
```
那么在Unity中可按照以下步骤来调用这两个函数：

1.	将`libfunc.a`放到`Assets/Plugins/iOS/`目录下
2.	创建一个桥接C#脚本，内容如下

```
using System.Runtime.InteropServices;
public class LibFuncWrap
{
	// 要使用条件编译
	#if UNITY_IOS

    [DllImport("__Internal")]
    private static extern void func1(int intValue, string strValue); // const char* 对应 string

	[DllImport("__Internal")]
    private static extern string func2();

    public static void Func1(int intValue, string strValue)
    {
		func1(intValue, strValue);
    }

    public static string Func2()
    {
		return func2();
    }

    #endif
}
```

### Android


## 开发

### iOS


### Android


## 参考
[Unity Plugins](http://docs.unity3d.com/Manual/Plugins.html 'unity plugins')