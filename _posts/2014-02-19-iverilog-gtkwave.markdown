---
layout: post
title:  "iVerilog & gtkwave 使用说明"
date:   2014-02-19 16:40:36
categories: Verilog
---

[参考链接](http://blog.csdn.net/liming0931/article/category/923723)

Step
--
以计数器（counter）举例  

1.  使用文本编辑工具（eg:vim)编写源代码，后缀名为`.v`。  
    源代码包括：  
    * 模块：counter.v
    * 激励文件：counter_tb.v

2.  使用iverilog进行编译，具体代码如下：  
    终端（命令行）模式下：  
    `iverilog -o counter_design counter_tb.v counter.v`
    （与gcc编译语法类似）

3.  使用vvp运行，具体代码如下：  
    终端（命令行）模式下：  
    `vvp counter_design`（结果将以文本的方式，在终端中显示）  
    再输入 `finish` 结束vvp的运行，或者在执行vvp的时候添加 -n 参数（即 `vvp -n counter_design`)。  

In Addition
--
如果想再gtkwave中观察波形图，需要额外的以下步骤：  

1. 在激励文件中添加如下代码：
    ```
initial
begin
$dumpfile("test.lxt");
$dumpvars(0,test);
end
    ```

2. 在执行vvp是添加参数 `-lxt2`  
`vvp -n counter_design -lxt2`  
终端将显示运行结果，同时生成名为 test.lxt 的波形文件。  

3. 使用gtkwave打开 test.lxt 文件  
`gtkwave test.lxt`  
gtkwave的使用方法，[点此进入](http://blog.chinaunix.net/uid-25148957-id-3180303.html)  


PS
--
vvp参数简介  
使用 `man vvp` 可以看到vvp命令的介绍  
其中，在命令格式的最后有个 `extended-args` （扩展参数）  
该类型的参数有：  

参数 | 作用  
----|----  
-vcd | 将wave dump format（波形镜像格式）设置成 vcd。默认参数。特点是生成文件大，但能最好的适应第三方工具  
-lxt, -lxt-speed, -lxt-space | 将波形镜像格式设置成 lxt。顾名思义 lxt-speed拥有较高的执行与读取速度，lxt-space 占用较小的空间  
-lxt2 | 比lxt慢，但是比vcd快，比lxt节省空间。递增式写入，即可以一边仿真，一边读取波形数据，进行显示。  
-none | 禁止一切波形镜像的生成，可以加快仿真速度。  
