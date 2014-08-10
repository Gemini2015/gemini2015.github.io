---
layout: post
title:  "iVerilog 安装"
date:   2014-01-04 21:24:21
categories: Verilog
---

开始学习Verilog HDL了，别人推荐了iverilog这个小型仿真工具。比起大型的IDE，个人感觉初期还是用这些简洁的工具要好一些。  

[iverilog](http://iverilog.icarus.com/)运行于终端模式下，安装完成之后通过iverilog执行编译，生成的文件通过vvp执行仿真。
配合gtkwave可以实现图形化的波形显示。  

关于iverilog的安装，官网给出了教程[传送门](http://iverilog.wikia.com/wiki/Installation_Guide)  
另外，网上的教程比较多。  

## 三个平台：  
### PC:
没装过，所以不知道好不好装  

### Linux/Ubuntu：
官网给出的教程有点复杂，我试了一下直接用下面的命令就可以安装。  
`sudo apt-get install iverilog`  
`sudo apt-get install gtkwave`  
可以给Ubuntu添加一些国内的源，我这个是在163的源上下载的。  
PS.我的Ubuntu是12.04。  

### Mac OS X:
因为我用的MacBook，所以我在Mac下面也装了一个。  
系统是10.9 Mavericks，装下来花了好长时间。  

首先我想自己编译，然后从GitHub上下载了源码  
在本机上编译，好像是因为10.9的c编译器是llvm，不是GNU gcc  
所以按照官网给的步骤自己编译没成功  
（期间提示缺少autoconf，又得用brew安装一个autoconf，相当麻烦。）  

然后没办法，还是直接通过macports或是homebrew安装吧。  
`macports`和`homebrew`，个人感觉有点类似于Linux上的apt-get。  
homebrew安装的时候，会要求安装`xcode-select`。  

#### Macports安装iverilog:
`sudo ports -d -v install iverilog`  
`sudo ports -d -v install gtkwave`  

#### homebrew安装iverilog:
`brew install icarus-verilog`  

不得不吐槽一下校园网，用brew安装直接失败（应该是网速的问题），用Mac ports安装也是试了n多次，花了好长时间才安装好。两个加起来才几十M的软件啊，花了我几个小时。  

安装完成之后，可以运行下面的命令检测是否安装好了  
`which iverilog`  
`which vvp`  
`which gtkwave`  
会分别提示安装的位置。  

至此，iverilog的安装就完成了。  
