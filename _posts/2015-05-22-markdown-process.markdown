---
layout: post
description: 对由Markdown渲染出的HTML进行后期处理的一些实践
title: Markdown后期处理
date: 2015-05-22 21:34:56
categories: web
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

## 渲染

### 代码着色
可以在两个阶段对代码进行着色：渲染时，渲染后。

1.	渲染时着色
	在使用Jekyll进行渲染时，可以使用Jekyll提供的标签<code>&#123;% highlight ruby %}</code> 和 <code>&#123;% endhighlight %}</code> 代替 <code>&#96;&#96;&#96;</code>包裹代码块，为代码块设置语言，同时实现着色。
	***这种方法使用了非标准的Markdown标记，所以我没有采用这种办法。***

2.	渲染后着色
	由<code>&#96;&#96;&#96;</code>标注的代码块会生成` <pre> `和` <code> `标签，因此可以使用js脚本，在浏览器本地进行着色。
	我选择使用`highlight.js`库。`highlight.js`可以尝试识别代码所属的语言。但是识别效果不咋地。所以，我们还需要标注代码块所用的语言。
	对于大部分文章，可以假设整篇文章中的代码都是采用同一种语言，因此，可以在文章的`Front Matter`中用一个变量保存当前文章的全局语言设置，然后在模板中引用该变量，保存到一个隐藏`<input>`标签中，最后通过js处理，将该隐藏`<input>`的值设置到每一个`<pre><code>`块中。

渲染后着色相关实现：

```
// front matter
---
layout: post
title:  "Verilog HDL语言要素 - 词法"
date:   2014-01-05 20:17:54
categories: Verilog
codelang: verilog
---
```

```
// post 模板
<input type="hidden" id="codelang" value="{{ page.codelang }}" />
```

```
// js 脚本
// set code language, default cpp
var codelang = $('#codelang').attr("value");
if(codelang == "") codelang = "cpp";
$('pre code').each(function(index, elem){
    $(elem).attr("class", codelang);
});

// init & apply highlight
hljs.configure({
    tabReplace: '    ', // replace a tab with 4 spaces
})
hljs.initHighlighting();
```