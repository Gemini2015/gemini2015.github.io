---              
layout: post
title: 图集制作
date: 2015-12-28 22:10:00
categories: Game
description: 关于图集制作的几种方式
codelang: csharp
---


**图集（Atlas）**：在计算机图形学中，图集是一种特殊类型的图像，其内容为一组图像的集合，按某种排布规则分布在图集图像中。

## 为什么制作图集
说到图集，就不得不提图形硬件API，现在主流的图形硬件API有两种：OpenGL 和 Direct3D。
以OpenGL为例，来说明如何绘制一个人物，假设人物图素包括：人体、帽子、衣服、裤子、鞋子等。

*	不使用图集
	1.	将所有图素输入到OpenGL中，为每一个图素创建纹理对象
	2.	绑定一个纹理对象，如人体
	3.	输入纹理坐标，将纹理绘制到缓冲区上
	4.	重复上面的2，3步骤，依次绘制所有图素

*	使用图集
	1.	将图集输入到OpenGL中，创建纹理对象
	2.	绑定图集对应的纹理对象
	3.	输入纹理坐标，如人体的纹理坐标，将对应的纹理绘制到缓冲区上
	4.	重复上面的第3步，依次绘制所有图素

从上面的步骤可以看出，使用图集将一组相关的图素打包成一个整体，可以避免频繁的状态切换（切换纹理对象），从而提高渲染速度。

## 制作图集
制作图集的方法有很多，从简陋的U3D函数，到专门的图形库，再到简单实用的图集工具等，都可以制作出图集。

下面介绍两种方法来生成图集
*	使用U3D的`Texture2D.PackTextures`函数
*	使用TexturePacker工具

### U3D API
代码正在完成中……

### TexturePacker
以TexturePacker（以下简称TP）导出图集到Unity为例，来演示TP的使用方法。
![texturepacker](http://7xip1j.com1.z0.glb.clouddn.com/post%2Fatlas%2Ftexturepacker.jpg 'texturepacker')

1.	导入图素
	将要制作成一张图集的所有图素收集到一个文件夹中，然后打开TP，将该文件夹拖入TP左边的Sprites栏目中，即完成了图素的导入。
	![icon_folder](http://7xip1j.com1.z0.glb.clouddn.com/post%2Fatlas%2Ficon_folder.png 'icon_folder')
	![import](http://7xip1j.com1.z0.glb.clouddn.com/post%2Fatlas%2Fimport_icon.png 'import')

1.	TP设置
	*	Data Format: **Unity - Texture2D sprite sheet**
	*	Data file: 生成的 .tpsheet 数据文件路径名
	*	Texture format: **PNG**
	*	Texture file: 与 .tpsheet 对应的图集名（.tpsheet 与 .png 文件名必须相同）
	*	Max size: 对于移动平台的话，考虑到兼容性，最大选**2048**
	*	Algorithm: **Basic** 和 **MaxRects** 两者之间，参照实际情况来选择
	*	Trim mode: 选择**None**

1.	保存TP工程
	建议将当前的TP工程进行保存，这样下次如果有新图素要加入图集的话，只需要将图素放入图集文件夹内，然后打开之前保存的TP工程，TP会自动加入新的图素。

1.	生成图集文件
	点击**Publish sprite sheet**会生成 .tpsheet 文件以及 .png 文件。

1.	导入Unity
	1.	在Unity中导入由[CodeAndWeb](https://www.codeandweb.com )提供的插件**TexturePackerImporter**（可在Asset Store中免费下载）。
	2.	将上面生成的 .tpsheet 文件和 .png 文件添加到Unity工程中，TexturePackerImporter 会自动分析 .tpsheet 文件，生成 .meta 文件，而 .meta 文件中则包含有图集中图素的信息。

注：**.png 文件和 .meta 文件应该加入到Unity工程的版本控制系统中，而 .tpsheet 文件可以不必加入到Unity工程中（生成 .meta 文件之后可以删除掉），当然也可以把 .tpsheet文件放到单独的目录进行版本管理**

## 参考

*	[What is a sprite sheet](https://www.codeandweb.com/what-is-a-sprite-sheet) & [what is a sprite sheet performance](https://www.codeandweb.com/what-is-a-sprite-sheet-performance)
