#include <iostream>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H

#pragma comment(lib, "freetype.lib")
using namespace std;

void Draw_Bitmap(unsigned char* bitmap, int width, int height);

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cout << "Useage: " << argv[0] << " fontfile character" << endl;
		return 0;
	}
	string font(argv[1]);
	char charcter = argv[2][0];

	cout << "font: " << font << "\tcharcter: " << charcter<<endl;

	// 初始化Freetype库
	FT_Library library;
	FT_Error error = FT_Init_FreeType(&library);
	if (error)
	{
		cout << "error in init freetype" << endl;
	}

	// 创建字体外观
	FT_Face face;
	error = FT_New_Face(library, font.c_str(), 0, &face);
	if (error == FT_Err_Unknown_File_Format)
	{
		cout << "error in file format" << endl;
	}
	else if (error)
	{
		cout << "error in create face" << endl;
	}

	// 设置字体大小
	error = FT_Set_Pixel_Sizes(face, 30, 0);
	if (error)
	{
		cout << "error in set pixel size" << endl;
	}

	// 获取字符索引
	unsigned int index = FT_Get_Char_Index(face, charcter);

	// 载入字形
	error = FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
	if (error)
	{
		cout << "error in load glyph" << endl;
	}

	// 渲染轮廓
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (error)
	{
		cout << "error in render glyph" << endl;
	}

	// bitmap.width 字形图象一行有多少个像素
	// bitmap.pitch 表示字形图象一行有多少个字节
	Draw_Bitmap(face->glyph->bitmap.buffer, face->glyph->bitmap.pitch,
		face->glyph->bitmap.rows);

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return 0;
}

void Draw_Bitmap(unsigned char* bitmap, int width, int height)
{
	cout << endl;
	if (bitmap == NULL)
		return;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int color = bitmap[i * width + j];
			if (color == 0)
			{
				cout << " ";
			}
			else
			{
				cout << "*";
			}
		}
		cout << endl;
	}
}