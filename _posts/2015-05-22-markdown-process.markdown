---
layout:	post
title:	Markdown后期处理
date:	2015-05-22 21:34:56 
categories:	web
description: 对由Markdown渲染出的HTML进行后期处理的一些实践
codelang: js
---

Markdown是一个轻量级的标记语言，同强大的HTML相比，它书写简单，冗余符号较少，易于学习。在保证最简单的语法的同时，对主流常用的文本格式均提供了支持。
我是从搭建这个博客时开始接触Markdown，至今已有一年有余。已经使用Markdown写了十几篇博文，爱不释手。
本文的内容主要是记录对由Markdown渲染出来的HTML进行后期处理的一些实践。更准确的来说，**本文主要是对基于Github Pages的博文进行后期处理。**

## 使用Markdown

1.  首先，使用Markdown提供的标记书写出文章，一般Markdown文件的后缀名为`.md`或者`.markdown`。
2.  然后可以通过其它转换工具将Markdown文章渲染出来，或者转换成其它格式，如HTML，LaTex等。
3.  如果生成的是HTML，再搭配一份CSS，就可以将文章渲染成一个优雅的页面。

Markdown存在着很多风格，不同的风格对标准Markdown语法的解释有些不一样。**本文是基于`Github风格`的Markdown文本。**
不同的转换工具也会提供对Markdown非语法层面的扩展，比如增加一些特定的`tag`来扩展新的格式。**我提倡尽量减少对非语法层面扩展的依赖。**也就是说尽量不使用这些转换器提供的`tag`，而是**通过js脚本来实现扩展**。

## 结构

### 目录导航
Markdown中存在多级标题，我们可以为每一级标题生成一个目录，方便阅读的人快速定位感兴趣的内容。
Markdown中的标题对应于HTML中的`<h1> - <h6>`标签。假设我们要提取`<h2>`建立目录，基本思想就是利用js标本从文章开头，依次遍历每一个`<h2>`标签，提取标签内容，为标签设置`id`，然后将提取到的内容建立一个链接列表即可。
在我的博客中，我只对2级标题提取了目录，如果要对每一级都提取目录，可能要相对复杂一些，但是基本思想都是一样的。

```
// 添加一个返回顶部的按钮
$('body').attr('id','body-top');
var top = '<button type="button" class="btn btn-success" data="body-top">Top</button>';
$('#navbar-list').append(top);

// 遍历每一个 h2 标签
var counter = 0;
$('article').children('h2').each(function(index, elem){
	$(elem).attr('id', 'section-h2-' + counter);
    var btn = '<button type="button" class="btn btn-primary" data="' + $(elem).attr('id') + '">'+ $(elem).text() +'</button>';
    $('#navbar-list').append(btn);
    counter++;
});

// 设置点击动画
$('#navbar-list').children('button').each(function(index, elem){
    $(elem).click(function(){
        var id = $(elem).attr('data');
        var dst = $('#'+id).offset().top - $('.navbar-title').outerHeight(true) - 10;
        if(dst < 0)
        {
            dst = 0;
        }
        $("html,body").animate({scrollTop:dst},600);
    });
});

// navbar-list 为一个div，我使用了一个第三方的库stickUp
// 使其随滚动条移动
$('.navbar-title').stickUp();
```