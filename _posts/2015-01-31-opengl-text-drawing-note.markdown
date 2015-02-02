---
layout: post
title: OpenGL文字绘制笔记
date: 2015-01-31 22:28:41
categories: game dev 
description: 记录实现OpenGL文字绘制的过程
---


与D3D相比，OpenGL本身并不支持文字绘制。因此，对于文字绘制功能的实现，并不能像D3D一样，只需对相关API进行简单的封装。对于OpenGL而言，开发人员必须自己实现一套文字绘制功能。
经过对OGRE相关代码的简单分析，以及在网上进行了一些搜索，发现大部分涉及到文字显示的项目都会使用**freetype**这个开源的字体引擎。

使用freetype的大致流程是：**先通过freetype载入字体文件，然后将要显示的字符和对应的参数传给freetype，freetype就会生成相应的位图，然后应用程序再将这个位图绘制到输出设备上。**

以下主要记录了使用freetype过程中的一些笔记。


## FreeType相关术语
对于相关术语，此处只列出条目，具体含义可以参考freetype官网上的[相关内容](http://www.freetype.org/freetype2/docs/glyphs/glyphs-1.html)。

- 字体(font)
- 字体家族(font family)
- 字体外观(font face)
- 字符映像(character image) == 字形(glyph)
- 字符规格(character metric)
- 字符编码(character encoding)
- 字形索引(glyph index)
- 字符映射表(character map)
- DPI(每英寸点数, dots per inch): `1 point = 1/72 inch`
- PPI(每英寸像素点数, pixels per inch)
- 设备分辨率(device resolution): `pixel_size = point_size * resolution / 72`
- 轮廓(contour)
- em
- 网格单元(grid unit): `pixel_coord = grid_coord * pixel_size / EM_size`
- 网格对齐(grid-fitting)
- 隐式网格对齐(hinting)
- 基线(baseline)
- 笔(pen)
- 布局(layout)
- 印刷规格(Typographic metric)
- 上行高度(ascent)
- 下行高度(descent)
- 行距(linegap): `linespace = ascent - descent + linegap`
- 边界框(bounding box)
- Internal leading: `internal leading = ascent - descent - EM_size`
- External leading
- Bearing
- Advance

## FreeType使用步骤
可以将freetype当成这样一个黑箱：我们的应用程序负责提供字体文件，需要绘制的字符，以及相应的参数，freetype返回对应字符的位图，然后我们的应用程序再将位图绘制到输出设备上。
可以按照下面提供的基本步骤来使用freetype：

1.	**初始化库**
	freetype的大部分操作都依赖于一个库(`FT_Library`)实例，一个库关联着若干个模块(`FT_Module`)和字体外观(`FT_Face`)。

	```
	// 创建一个变量
	FT_Library library;

	// 初始化库，该函数同时会注册一些缺省的模块
	FT_Error error = FT_Init_FreeType(&library)

	/*
	或者也可以创建一个空库，然后手动注册模块
	在这种情况下可以使用下面的函数
	FT_New_Memory
	FT_New_Library
	FT_Add_Module
	*/
	```

2. 	**载入字体文件**
	一个字体文件可能对应若干个字体外观(`FT_Face`)，一般情况下，在载入字体文件的时候，总是选择第一个(index == 0)字体外观，然后根据字体外观的`num_faces`来获取该文件中的外观数。
	使用`FT_New_Face`可以从一个指定文件载入字体，也可以使用`FT_New_Memory_Face`从一个内存地址处载入字体。

	```
	// 外观变量
	FT_Face face;
	char* font_file = "Arial.ttf";
	int face_index = 0;
	FT_Error error = FT_New_Face(library, font_file, face_index, &face);

	/*
	从内存中载入
	FT_Error error = FT_New_Memory_Face(library, buf, size, face_index, &face);
	*/
	```

3.	**设置像素尺寸**
	载入字体之后，在获取字符映像之前，我们首先要设置像素尺寸。
	当一个新的`FT_Face`对象建立时，对于可伸缩字体格式，`FT_Face`中`size`会默认设置为`(10, 10)`。而对于定长字体格式，这个大小是未定义的。
	对于可伸缩字体格式，你可以将`size`设置成任意合理的值，对于定长格式，若是设置的`size`不在`FT_Face`的`available_sizes`数组中，则会引发错误。

	```
	FT_Face face;
	int width = 0; // 为零表示与height相同
	int height = 16 * 64; // 以 1/64 点为单位的字符高度
	int ResolutionX = 300; // 水平分辨率
	int ResolutionY = 300; // 垂直分辨率
	FT_Error error = FT_Set_Char_Size(face, width, height, ResolutionX, ResolutionY);

	/*
	或者可以直接设置像素大小
	FT_Face face;
	int WidthInPixel = 0;
	int HeightInPixel = 16;
	FT_Error error = FT_Set_Pixel_Sizes(face, WidthInPixel, HeightInPixel);
	*/
	```

4.	**字符码到字形索引**
	字符码指的是某个字符在某种编码下的数值。比如`A`在ASCII中的字符码为`64`。字形索引是字体文件内部用来查找字形的索引。可以通过字体文件提供的字符映射表来将字符码转换成对应的字形索引。通常一个字体文件会包含多个字符映射表，以提供对多种常用的字符编码的支持。
	当一个`FT_Face`对象创建时，会默认选择Unicode字符表，如果字体没包含Unicode字符表，FreeType会尝试在字形名的基础上模拟一个(对于某些字体，其模拟效果可能不尽人意)。
	`FT_Face`中的`charmaps`表，记录了当前字体提供的字符映射表，可以使用预定义的一些枚举值来调用`FT_Select_CharMap`来选中某个字符映射表，也可以手动遍历`charmaps`，以符合要求的`charmap`调用`FT_Set_CharMap`来设置字符映射表。

	```
	// 获取字形索引
	FT_Face face;
	unsigned long charcode = '程';
	unsigned int glyph_index = FT_Get_Char_Index(face, charcode);

	// 选择字符映射表
	FT_Face face;
	FT_Error error = FT_Select_CharMap(face, FT_ENCODING_BIG5);

	// 手动选择字符映射表
	FT_Face face;
	FT_CharMap dest = NULL;
	FT_CharMap charmap = NULL;
	for(int i = 0; i < face->num_charmaps; i++)
	{
		charmap = face->charmaps[i];
		if(charmap->platform_id = dest_platform_id
			&& charmap->encoding_id == dest_encoding_id)
			{
				dest = charmap;
				break;
			}
	}
	if(dest != NULL)
	{
		FT_Error error = FT_Set_CharMap(face, dest);
	}
	```
