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


## 相关术语
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