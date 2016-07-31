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
Android插件一般以`.jar`的形式提供。假设名为`MyClass.java`的文件，所在包为`com.icodeten.dev`，导出为`libFunc.jar`。
其中`MyClass`的实现如下：

```
public class MyClass
{
	public void Func1(int intValue, String strValue)
	{
	}
	
	public String Func2()
	{
		return "Func2";
	}
	
	public static void Func3()
	{
	}
}
```
可按照下面的步骤，在Unity中调用上述方法：

1.	将`libFunc.jar`放到`Assets/Plugins/Android`目录下
2. 创建一个桥接C#脚本，内容如下：

```
public class AndroidWrap
{
	#if UNITY_ANDROID

	struct ClassDefine
	{
		public const string ClassName = "com.icodeten.dev.MyClass";
		public const string Func1 = "Func1";
		public const string Func2 = "Func2";
		public const string Func3 = "Func3";
	}

	private static AndroidJavaObject javaObj = null;

	private static AndroidJavaObject GetInstance()
	{
		if(javaObj == null)
		{
			// java class 对象
			// var javaClass = new AndroidJavaClass(ClassDefine.ClassName);
			javaObj = new AndroidJavaObject(ClassDefine.ClassName);
		}
		return javaObj;
	}

	public static void Func1(int intValue, string strValue)
	{
		var obj = GetInstance();
		obj.Call(ClassDefine.Func1, intValue, strValue);
	}

	public static string Func2()
	{
		var obj = GetInstance();
		return obj.Call<string>(ClassDefine.Func2);
	}

	public static void Func3()
	{
		// 调用静态方法
		var obj = GetInstance();
		obj.CallStatic(ClassDefine.Func3);
	}

	#endif
}

```


## 开发
很多情况下，我们不得不自己开发插件。比如一些原生的系统调用，Unity没有提供对应的接口，那么我们就必须编写插件来实现调用。
经常有一些第三方服务只提供了`iOS`或`Android`的SDK，并没有直接提供Unity SDK，这个时候，就需要我们对这些SDK进行一些简单的封装，使得我们可以在Unity中调用这些SDK提供的接口。
以下以访问系统剪切板为例，来简要介绍一下`iOS`和`Android`插件的开发过程。

### iOS

1.	创建一个Xcode Project，项目模版选为“Cocoa Touch Static Library”。取名为`libClipboard`。
2. 删除自动创建的`.h`文件（留着也可以）。将`.m`文件的后缀名改为`.mm`。
3. `.mm`文件内容如下
4.  执行Build。
5. 将`.h`文件（如果没有删掉）和`.a`文件拷贝到Unity工程下`Assets/Plugins/iOS`使用。

```
#import <UIKit/UIKit.h>

extern "C"
{
    void _SetTextToClipBoard(const char* plainText);
    const char* _GetTextFromClipBoard();
}

// 传进来的字符串，如果需要在闭包里面使用的话，需要进行一次深拷贝
void _SetTextToClipBoard(const char* plainText)
{
    [UIPasteboard generalPasteboard].string = [[NSString alloc]initWithUTF8String:plainText];
}

// 传给Unity的字符串，也要进行深拷贝，其内存交由Mono进行管理
const char* _GetTextFromClipBoard()
{
    NSString *plainText = [UIPasteboard generalPasteboard].string;
    if(plainText == nil)
        return NULL;

    const char* plainTextStr = [plainText UTF8String];
    char *ret = strdup(plainTextStr);
    return ret;
}
```



#### Tips

1.	使用`const char*`来对应`C#`里面的`string`类型，需要小心`const char*`的内存管理。否则容易出现访问野指针的情况。
2. 当Xcode提示`extern`出错时，别忘了将`.m`文件改为`.mm`。
3. Unity工程，导出Xcode工程，编译链接时，提示找不到`.a`中导出的函数，可以用`file libClipboard.a`命令看看`libClipboard.a`支持的架构。**如果要在真机上运行，那么静态库必须支持`armv7`和`arm64`架构。**插入真机设备，然后将调试设备设为真机，然后再执行Build，应该可以保证生成的`.a`文件支持`armv7`和`arm64`架构。
4. 使用`NSString *result ＝ [NSString stringWithFormat:@"%@", nsStr]`时，如果`nsStr`为`NULL`时，那么`result`会是`(null)`，而不是空字符串。
5. Unity提供了在`iOS`插件中向Unity发送消息的机制，以实现异步事件。

```
// 由Unity提供的函数
// 第一个参数为场景上某个GameObject的名称
// 第二个参数为该GameObject上挂载的某个脚本里面的方法名称
// 第三个为自定义的字符串参数
extern void UnitySendMessage(const char*, const char*, const char*);

// 假设为某个异步回调
void MyCallBack(int intValue, const char* strValue, NSString* nsStrValue)
{
	NSString *paramString = [NSString stringWithFormat:@"%d|%s|%@", intValue, strValue, nsStrValue);
	UnitySendMessage("GameObjectName", "MethodName", paramString);
}
```

### Android


## 参考
[Unity Plugins](http://docs.unity3d.com/Manual/Plugins.html 'unity plugins')