---
layout: post
title: pbc支持64位整型
categories: Game
description: 修改pbc以支持protobuf 64位整型编解码
codelang: c
---

## 前言

在Unity游戏开发中，为了支持热更新，经常会选取[ulua](https://github.com/jarjin/uLua)或是[tolua](https://github.com/topameng/tolua)框架，使用lua来编写游戏逻辑。
同时，为了与服务器进行通信，可以使用框架中集成的云风编写的[pbc](https://github.com/cloudwu/pbc)来实现protobuf协议的编解码。

pbc是protobuf协议的C语言实现，里面包含了针对lua5.1和lua5.3的绑定。
因为lua5.1本身不支持64位整型，所以pbc针对lua5.1的绑定，也是默认不支持64位整型的。
本文描述了一种通过修改pbc以支持64位整型的方案。

## 思路

lua5.1本身是不支持64位整型的，所有的数值都是用double类型表示，当出现数值较大的数字时，便会出现精度丢失。
可以考虑***使用字符串来表示64位整型***，在pbc编码的时候，将lua传过来的字符串转换成数值，发送给服务器；解码的时候，将服务器发过来的数值转成字符串，传到lua里面。

### 优点

*   能保证64位整型的精度
*   直接支持判等操作

### 缺点

*   默认不支持数学运算（可以通过实现一套函数，来支持字符串的数学运算）

由于当前项目主要用64位整型来表示id，所以只需要支持判等，不要求支持运算，故该思路基本能满足当前项目要求。

## 实现

### 解码

在解码的过程中，我们需要注意一下几点：

*   pbc的解码，主要是在`pbc-lua.c`文件中的`push_value`函数中完成的。
*   pbc对protobuf支持的类型进行了映射，protobuf支持的类型在`context.h`文件中通过以`PTYPE_`开头的宏来定义的。pbc使用的类型则是在`pbc.h`文件中通过以`PBC_`开头的宏来定义的。
*   pbc在映射的时候，将`PTYPE_UINT32`和`PTYPE_UINT64`都映射成了`PBC_UINT`，我们需要增加一个`PBC_UINT64`的宏定义，将`PTYPE_UINT32`和`PTYPE_UINT64`区分开。


```
case PBC_UINT64: {
    uint64_t v64 = (uint64_t)(v->i.hi) << 32 | (uint64_t)(v->i.low);
    char buffer[32] = {0};
    sprintf(buffer, "%llu", v64);
    lua_pushstring(L, buffer);
    break;
}
```


### 编码

在编码的过程中，需要注意一下几点：

*   `protobuf.lua`中`_writer`定义了支持类型的编码函数。
*   需要在`_writer`中添加`uint64`、`uint64_repeated`来支持额外增加的`PBC_UINT64`类型的数值编码。
*   `protobuf.lua`通过调用`pbc-lua.c`中定义的`_wmessage_xxx`函数，来实现编码的过程。


```
static int
_wmessage_uint64(lua_State *L) {
	struct pbc_wmessage * m = (struct pbc_wmessage *)checkuserdata(L,1);
	const char * key = luaL_checkstring(L,2);
	switch (lua_type(L,3)) {
	case LUA_TSTRING : {
		size_t len = 0;
		const char * number = lua_tolstring(L,3,&len);
		uint64_t v64 = 0;
		if(len > 0)
		{
			v64 = strtoull(number, NULL, 10);
			if(v64 < 0)
				return luaL_error(L, "negative number : %s passed to unsigned field", number);
		}
		pbc_wmessage_integer(m, key, (uint32_t)v64 , (uint32_t)(v64>>32));
		break;
	}
	case LUA_TNUMBER : {
		int64_t number = (int64_t)(luaL_checknumber(L,3));
		uint32_t hi = (uint32_t)(number >> 32);
		pbc_wmessage_integer(m, key, (uint32_t)number, hi);
		break;
	}
	default :
		return luaL_error(L, "Need an int64 type");
	}

	return 0;
}
```


具体的实现过程，可以查看我的[提交记录](https://github.com/gemini2015/pbc.git)


## 集成

修改之后的pbc需要重新集成到ulua_runtime或是tolua_runtime中去，才能在Unity中使用，具体集成过程——略 :)