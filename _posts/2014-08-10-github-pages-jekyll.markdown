---
layout: post
title:  "Github Pages静态网站搭建记录"
date:   2014-08-10 21:14:00
categories: Github Pages
description: 介绍Github上搭建静态网站的步骤
---


声明
--
以下操作皆在 Mac OS X 10.9.4 下进行，应该也适用于 Linux 环境，但是 Windows 下，可能有些区别，Windows下的步骤，等我后续试验了之后，再补充吧。


环境简介
--

*   Github Pages本来为开发者准备的，用来介绍项目信息，以及开发者的个人展示。
    它只能展示静态页面。
    Github Pages有两类，一个是 **个人** 静态网页，其仓库名为 `UserName.github.io`。一个账户只有一个。
    另一个是 **项目** 静态网页，每一个项目可以建立一个，为该项目的 `gh-pages` 分支。

*   Jekyll可以配合Github Pages，用来生成一个静态网站。其功能包括博文的组织，解析（将用Markdown写的文章，转成HTML格式）

*   Markdown是一个轻量级的标记语言，易读易写。


准备工作
--

如果你想用Github Pages来搭建个人博客，首先请确认一下几点

- 最简单的英文应该能看懂吧
- 拥有一个 Github 账号，以及会使用基本的git命令，并且本机已经配置好git。
- 当然得会用 Markdown 写文章了


搭建步骤
--

1.  按照[Github Pages](https://pages.github.com/ "Github Pages")官网给出的步骤，建立一个仓库。
    在本地选一个文件夹，假设为 git。 命令行下 cd 到 git 目录。
    使用 git clone 命令，将前面建立的仓库 clone 到本地。
    eg:
    `git clone https://github.com/gemini2015/gemini2015.github.io`


2.  [这里](https://help.github.com/articles/using-jekyll-with-pages "Install Jekyll")有官网给出的安装Jekyll的步骤，但是我觉得有点啰嗦。所以在文章最后给出我总结的步骤。
    当Jekyll安装好之后，命令行下 cd 到前面 clone 的仓库文件夹内，执行以下命令。
    `jekyll new .` (后面的 . 表示当前目录)
    如果出现提示，说是 ***创建失败，因为当前目录非空*** 的话，那就随便找一个空文件夹，使用上面的命令，然后将生成的文件和文件夹拷贝到仓库文件夹内。
    可以在仓库文件夹内使用`jekyll serve`命令，然后会提示服务器已启动，接着在浏览器输入`localhost:4000`命令，来测试是否创建成功。


3.  以上两步，已经创建了一个 Jekyll 的模板了，在我们第一次 commit, push之前，还有一些事要做。
    1.  在仓库文件夹内创建一个名为`.gitignore`的文件，在里面写上 `_site/`(这么做的目的是告诉git，要忽略这个文件夹，因为这个文件夹是本地Build出来的结果，不需要放到github上)
    2.  打开`_config.yaml`，修改里面的信息，诸如 title, email, desription等。

4.  到此，就可以commit, push了


博文编写
--

Jekyll 对博文的文件名，内容格式有一定的要求。

1.  在 仓库文件夹 内有个 `_post` 文件夹，进入这个文件夹，创建一个文件。
    文件的命名方式为 `YY-MM-DD-file-name.markdown`（如：1992-05-19-I-came-to-this-world.markdown)
2.  该文件的开头一般为一段 *Frontmatter*,形如：
    <pre>
        <code>
---
layout: post
title:  "Verilog HDL语言要素 - 数据类型"
date:   2014-01-18 22:48:05
categories: Verilog
---
        </code>
    </pre>

    **layout**: 代表使用的模板。
    **title**: 代表文章的标题。
    **date**: 代表文章的时间信息。
    **categories**: 代表文章的分类。
    这些内容可以自己修改。
    在这些内容之后，就可以用 markdown 写文章了。
3.  写完之后，可以用 `jekyll serve` 在本地先调试一下，没问题了就可以 push 到 github 上了。
4.  在浏览器上输入 `UserName.github.io` 如(`gemini2015.github.io`)就可以，浏览博客了。当然，你也可以将自己的域名解析到这个页面上。


Jekyll 安装
--

1.  确认本机是否装有 Ruby ，可以通过执行 `ruby --version` 命令。
2.  安装 Bundler，命令行下执行 `gem install bundler` 命令，如果提示需要管理员权限的话，就 sudo.
3.  随便找个文件夹，创建一个名为 `gemfile` 的文件。在文件里写上 ` gem 'github-pages' `，然后在此文件夹下执行 `bundler install`命令。
4.  至此，Jekyll 就安装好了。

更多内容，可以到[Jekyll官网](https://jekyllrb.com/)查阅。


## Windows平台
在Windows平台上搭建，首先需要安装Ruby。[RubyGems](https://rubygems.org/ 'RubyGems')上最新版的`github-pages`对Ruby的版本要求是**1.9.3以上**。关于Windows平台上Ruby的安装，可以在[RubyInstaller](https://rubyinstaller.org/downloads/)这个网站上找到资源。
安装好Ruby之后，就可以参阅上面的Jekyll安装步骤，安装Jekyll了。

具体步骤，可以参考外国网友[juthilo的博客](https://jekyll-windows.juthilo.com/)。





