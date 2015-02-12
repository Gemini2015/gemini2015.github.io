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

## PPI & DPI

- 	PPI(pixel per inch，每英寸像素数)
	**PPI主要针对于显示领域**，描述的是显示屏每英寸拥有的像素数，比如iPhone 6 Plus的5.5寸(对角线)屏，采用了1920 * 1080的分辨率，其对应的PPI是401。一般情况下，PPI越高，显示效果越好。
-	DPI(dot per inch，每英寸点数)
	**DPI主要用在印刷领域**，描述的是打印设备每英寸打印的点数，常见的冲印设备在150DPI到300DPI之间。
	可以这样理解DPI，假设我们有一张800 * 600像素的照片要去打印，打印店的打印机分辨率最高为200DPI，若是我们将像素点一一对应到打印点，这能保证最佳的打印质量，那么打印的最大尺寸就是4 * 3英寸。如果我们执意要打8 * 6英寸的，那么必须要通过插值来计算增加的点的颜色，势必会造成失真。如果我们打2 * 1.5英寸的，打印机也会将两个像素点合成一个打印点，其结果也不一定会提升打印质量。

对于照片而言，通过提升打印机分辨率和图象像素数，可以获得更好的打印效果。但是对于字体而言，其一个点是固定为1/72英寸，也就是每英寸打72个点。通过将字体分辨率固定，可以保证同一个字号，在多台打印机上打印出的字符大小相同。


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
	当一个新的`FT_Face`对象建立时，对于可伸缩字体格式，`FT_Face`中`size`会默认设置为`(10, 10)`。而对于固定尺寸字体格式，这个大小是未定义的。
	对于可伸缩字体格式，你可以将`size`设置成任意合理的值，对于固定尺寸格式，若是设置的`size`不在`FT_Face`的`available_sizes`数组中，则会引发错误。

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

	// 获取字形索引
	FT_Face face;
	unsigned long charcode = '程';
	unsigned int glyph_index = FT_Get_Char_Index(face, charcode);
	```

5.	**装载字形**
	一旦获得了字形索引，便可以装载对应的字形映像。对于固定尺寸字体格式，每个字形都是一个位图。对于可伸缩字体格式，则使用名为轮廓的矢量形状来描述每一个字形。当然，也存在一些特殊的方式来表示字形。
	字形映像存储在字形槽中，一个`FT_Face`只有一个字形槽。所以每次只能获取一个字符串中的一个字符对应的字形。
	对于固定尺寸的字体格式，由于获取到的字形是位图，所以可以直接使用，而对于可伸缩格式的字体，装载的是一个轮廓，因此还必须通过`FT_Render_Glyph`函数将轮廓渲染成位图，方可使用。
	获取到位图之后，可以通过`face->glyph->bitmap`来访问位图数据，`face->glyph->bitmap_left`和`face->glyph->bitmap_top`用来指示起始位置。

	```
	// 装载字形
	FT_Face face;
	unsigned int glyph_index;
	signed int load_flags;
	FT_Error error = FT_Load_Glyph(face, glyph_index, load_flags);

	// 渲染轮廓
	FT_Render_Mode render_mode;
	FT_Error error = FT_Render_Glyph(face->glyph, render_mode);
	```

6.	**字形变换**
	可以通过调用`FT_Set_Transform`函数来对可伸缩字体进行仿射变换。
	需在字形由轮廓渲染成位图之前，设置字形变换。

	```
	FT_Face face;
	FT_Matrix matrix; // 仿射矩阵
	FT_Vector vector; // 仿射变换后进行平移
	FT_Error error = FT_Set_Transform(face, &matrix, &vector);
	```


# FreeType实践
***

## FreeType示例
[此处](/etc/FreeTypeExample.cpp)，提供一个本人写的简单示例，从控制台输入字体文件名和一个字符，程序打印出字符对应的字形位图。

## 使用策略
前面讲到一个`FT_Face`只有一个字形槽，因此对于一个字符串来说，我们每次只能取到一个字符的字形。如果每当我们渲染一个字符串的时候，都一个一个从FreeType处获取字形的话，效率无疑是很低的。
**空间换时间**。一般情况下，我们很少会频繁变动字体的大小，所以，一个提高效率的办法是在设定字体大小之后，在内存中创建一个纹理，将所有字形都绘制到这张纹理上，同时，保存每个字形的UV，以便需要的时候从这张纹理上“抠”下字形。
那么，问题来了：怎样将字形绘制到纹理上？理想的情况下，我们最好能将字形紧凑的排列到纹理上，这样最节省空间。但是实际情况是，要想将字形紧凑的排列到纹理上，难度还蛮大的。同样是空间换时间，我们可以采取这样的策略：先找到宽高最大的那个字形，以这个字形为标准，将纹理分割成一个一个的格子，然后将每个字形填到格子上，这样不仅绘制起来简单，同时也便于计算UV。

> **OGRE的字符处理**
> OGRE也是按照字形最大的那个来分配空间，但是OGRE在计算内存用量的时候有个小技巧。
> 在求出最大宽高`max_width, max_height`之后，求出理想情况下的内存用量`rawSize`，假设为正方形的纹理，就可得出正方形的边长`tex_side`，然后为了保证不会出现字形跨行的情况，给`tex_side`加了一个宽高中的最大值。因为一般情况下纹理的宽高都会取2的幂，所以求出最接近`tex_side`的`roundUpSize`。有时候`roundUpSize * roundUpSize`可能会比`tex_side * tex_side`大很多，所以OGRE在这里判断了是否应该以`roundUpSize * 0.5`作为高。接下来就是将字形数据拷贝到纹理上。

## ZFXEngine整合FreeType
我们使用FreeType载入字体文件，获取字形映像，并保存到内存纹理中。[当前版本](https://github.com/Gemini2015/ZFXEngine/tree/ce38a5a0af5f50eaa56f8ed84f008080710b8290 'ZFXEngine Source Code')的ZFXEngine提供的`SkinManager`接口只能从文件载入纹理，所以必须修改接口以实现从内存中载入纹理。同时，应创建一个`Font`类对字体进行抽象，以及`FontManager`管理字体。

> 原始版本的ZFXEngine是使用D3D实现的，所以在接口的设计上主要考虑的D3D，所以在用OpenGL实现时，经常要修改接口。

### 是否载入保存全部字形
前面说到可以把所有字形都绘制到一张纹理上，当我开始实现的时候，发现有点不现实。
首先，对于英文还好，但是对于中文来说，字符集太大，而且OpenGL对于单张纹理的尺寸有限制（可以使用`glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size)`查询当前OpenGL实现支持的最大纹理尺寸，我的是16384 pixel。可以算出对于20000个中文字符，最大字号只能在100 pixel左右），所以不可能在一张纹理里面存储所有字形映像。
同时，考虑到实际运行情况，对于在游戏中出现的某些特大号的文字或字符串，其使用的字符数量毕竟不多，如果为了使用某一个尺寸的若干个字符，而将所有字符都绘制到纹理上存在内存中，未免有些浪费。
或许，我们可以假设对于游戏中出现的少数大字号字符串，会由美术直接出图，而不是用文字渲染。

> **OGRE的字符区间**
> OGRE用`CodePointRange(33, 255)`来表示一段字符区间，OGRE的每一个Font都有一个区间列表，表面对于某个字号的某个字体，有哪些字形被绘制到了内存纹理中。OGRE没有对字符数量和字号大小做检验，对于20000个汉字，设置100的大小，运行时内存分配失败，直接空指针。
> 个人感觉，对于中文来说，`CodePointRange`似乎也不太实用，Unicode中的中文（0x4E00 - 0x9FCC，20000多汉字。除这区间以外，其他区间还有大量扩展）没有按使用频率分段，所以无法精简到某个区间。

### 固定大小纹理
鉴于一张纹理无法存储所有汉字，我决定使用多张纹理来存储。
使用固定大小的纹理（我选择的是`2048 * 2048`），将一个大的字符区间转换成若干个小的字符区间。一张纹理上可以存储多个区间内的字符。
**实现**

```
struct Range
{
	DWORD from;	// 字符区间
	DWORD to;
	UINT nTexID;	// 保存该区间字符的纹理ID
};
typedef vector<Range> RangeList;

RangeList tempRangeList;	// 用户设置的字符区间表
RangeList InteralRangeList;	// 最后程序内部保存的字符区间表

// 首先遍历每个字符。获取最大宽高，作为填充格子的大小
int max_height = 0, max_width = 0;
GetCellSize(tempRangeList, &max_width, &max_height);

/*
 	开始填充格子
*/
// 先创建一个纹理ID，
UINT nTexID = CreateTexture();	
// 创建一个内存纹理，等内存纹理填充完毕之后，再传输到硬件缓冲上。
int width = 2048;	// 内存纹理宽度
int height = 2048;	// 内存纹理高度
Byte *pBuf = new Byte[width * height];

int penx = 0, peny = 0;	// 画笔，表示内存纹理的绘制位置

// 遍历区间
foreach range in tempRangeList
{
	// 遍历字符
	DWORD from = range.from;
	DWORD to = from;
	for(DWORD codepoint = range.from; codepoint <= range.to; codepoint++, to++)
	{
		// 调用FreeType载入字形
		FT_GlyphSlot slot = LoadGlyph(codepoint);

		// 当前内存纹理已填满
		if(IsImageBufferFull)
		{
			UpdateTexture(nTexID, pBuf);

			// 保存当前区间
			AddRange(InteralRangeList, from, to - 1, nTexID);
			from = to;

			// 创建一个新的纹理ID
			nTexID = CreateTexture();
			
			// 更新画笔位置
			penx = peny = 0;
		}

		// 一行填满，换行
		if(IsImageNeedChangeRow)
		{
			penx = 0;
			peny = peny + max_width;
		}

		// 填充字形数据
		CopyGlyphToImage(pBuf, penx, peny, slot);

		// 保存该字形UV数据
		AddCodePointUV(codepoint, penx, peny, slot);

		// 画笔移动到下一格
		penx = penx + max_width;
	}

	//一个区间结束，也应当保存
	AddRange(InteralRangeList, from, to - 1, nTexID);
}
```

对于上面给出的实现，如果出现区间重合的情况就会发生问题。一个字符只会存在一个UV数据，但是处于不同区间的字符会有不同的纹理ID。所以有可能会出现UV与纹理不对应的情况。目前还没有解决办法。

> **OGRE关于字符区间重合**
> OGRE对于用户输入的区间重合情况，没有做处理。但是OGRE只用一个纹理，重复的字符都会绘制到纹理上，后绘制的字符的UV数据会覆盖前面的。所以不会出现运行问题，只不过纹理利用率不高。

### 解决字符区间重合
因为有多个纹理ID，但是只有一个UV数据，所以才会出现UV与纹理不对应的情况。
针对这个问题，一个解决办法是将UV数据与纹理ID绑定。
之前的数据结构

```
// 字符与UV映射构成 字符映射表
typedef std::map<DWORD, UVInfo> Glyph_Map;
// 直接通过字符查找对于UV
Glyph_Map m_glyphMap;
```
修改之后的结构

```
// 字符与UV映射构成 字符映射表
typedef std::map<DWORD, UVInfo> Glyph_Map;
// 纹理ID与字符映射表 绑定
typedef std::map<UINT, Glyph_Map> TexGlyph_Map;
// 先通过纹理ID，获取字符映射表，在通过字符查找UV
TexGlyph_Map m_texGlyphMap;
```
通过两级映射，可以解决字符区间重合的问题，同时，冗余度也较小。但是，目前还不知道这种解决办法对性能的影响。

上面采用多纹理来存储字符映像，虽然可以解决单个字符区间太大的问题，但是可能会增加纹理切换的时间消耗。当然，极端情况下，可以将纹理尺寸设置足够大，以保证一张纹理就能把所有字符存储下，也可以解决纹理切换的问题，主要还是具体情况，具体取舍。

### 细节部分
今天花了一整天调试上面的实现，遇到了一些问题，在此记录一下。

1. 	Y轴方向问题
	在向内存纹理绘制字形的时候，逻辑上y轴是从上向下的，因此其UV坐标满足D3D的屏幕坐标系，但是OpenGL的屏幕坐标系Y轴为从下向上的。故在应用程序使用字形UV的时候，应该调整Y轴方向。

2.	字形基线
	在原始的实现中，我没有考虑基线，直接将字形位图的（0,0）点对应纹理格子的（0,0）点，UV也是这样计算的。因为一个字形的位图尺寸为能包裹字形的最小矩形框，所以在每个纹理格子中，字形位图都是偏左上的，最后应用程序绘制字符的时候，也会显得偏左上，显得不美观。比如字符“一”，本来正常情况下，在垂直方向上应该接近居中，但是如果不考虑基线直接绘制的话，就会绘制在格子顶部。
	在这个地方参考了OGRE的做法，得以保证字形在统一的基线上绘制。

3.	字符显示尺寸
	经过封装之后，应用程序在使用上述实现时，只能获取到字体大小Size，字符对应纹理，字符UV。
	很明显`O`与`i`的宽度是不等的，每一个字符的UV对应了字符的实际位图，也就是说，`O`的UV组成的矩形要比`i`的UV组成的矩形宽。
	怎样确定绘制出来的字符的宽度和高度？肯定不能使用(Size * Size)这样的尺寸，使用这样的尺寸会将`i`拉宽，因而失真。
	我采取的办法是根据字符的纵横比结合Size来计算最后显示的尺寸。

	```
	int x, y;		// pen pos
	float swidth;		// canvas size
	float sheight;
	
	float width = uv.right - uv.left;
	float height = uv.bottom - uv.top;
	float aspect = height / width;
	if(width > height)
	{
		// 比如 "一"
		float vertex[6][4] = {
			// triangle 1
			{ x / swidth, 			(y + size * (1 + aspect) / 2) / sheight, uv.left, uv.top },
			{ (x + size) / swidth, 	(y + size * (1 + aspect) / 2) / sheight, uv.right, uv.top },
			{ x / swidth, 			(y + size * (1 - aspect) / 2) / sheight, uv.left, uv.bottom },

			// triangle 2
			{ (x + size) / swidth, 	(y + size * (1 + aspect) / 2) / sheight, uv.right, uv.top },
			{ (x + size) / swidth, 	(y + size * (1 - aspect) / 2) / sheight, uv.right, uv.bottom },
			{ x / swidth, 			(y + size * (1 - aspect) / 2) / sheight, uv.left, uv.bottom },
		}
	}
	else
	{
		// 比如 "|"
		float vertex[6][4] = {
			// triangle 1
			{ x / swidth, 					(y + size) / sheight, uv.left, uv.top },
			{ (x + size * aspect) / swidth, (y + size) / sheight, uv.right, uv.top },
			{ x / swidth, 					y / sheight, uv.left, uv.bottom },

			// triangle 2
			{ (x + size * aspect) / swidth, (y + size) / sheight, uv.right, uv.top },
			{ (x + size * aspect) / swidth, y / sheight, uv.right, uv.bottom },
			{ x / swidth, 					y / sheight, uv.left, uv.bottom },
		}
	}
	```

4.	字符间距
	在求得字符的显示尺寸之后，字符能正常显示，但是又会出现一个问题，字符间距需要调整。字符间距与画笔的步进有关。画笔的步进由字幅宽度，字偶距（Kerning），自定义间距组成。
	一个解决办法是修改字符绘制函数，返回当前绘制的字符的宽度（垂直绘制的话，返回高度）。
	对上面的代码进行修改，即可获得画笔步进。

	```
	if(width > height)
	{
		// ...
		
		*advance = size;
	}
	else
	{
		// ...

		*advance = size * aspect;
	}
	```