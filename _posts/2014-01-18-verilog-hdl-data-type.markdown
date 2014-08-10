---
layout: post
title:  "Verilog HDL语言要素 - 数据类型"
date:   2014-01-18 22:48:05
categories: Verilog
---

数据类型
--
Verilog的数据类型主要有两种：  
1. **连线类型**  
2. **寄存器类型**  


1. 连线类型（Net Type）  
    （类似于电路中的导线，不能存储数据值，实时改变）  
    **驱动方式**：  
    > 作为逻辑门或模块的输出端  
    > 用持续赋值语句assign对其进行赋值  
    **关键字**:
    ```
        wire,tri  普通连线
        wor,trior  具有线或特性的连接线
        wand,triand
        supply1,supply0  分别是电源（逻辑1），接地（逻辑0）
    ```
    eg:
    ```
        wire[n-1:0] a,b;   //or wire[n:1] a,b
        assign a=b;  //将b赋值给a
        wire[7:0] out;
        wire[3:0] in;
        assign out[5:2] = in
    ```

2. 寄存器类型（Register Type）  
    （可以存储值）  
    Register变量需要被明确的赋值  
    在设计过程中，必须将寄存器变量放在过程语句中，通过过程赋值语句赋值（initial，always）  
    **关键字**:
    ```
    reg
    integer
    real
    time
    parameter  //定义符号常量
    ```

寄存器&存储器
--
```
reg mybit;  //1位寄存器
reg[7:0] mybyte;  //8位寄存器
a = mybyte[7];   //按位寻址
b = mybyte[5:2];  //按域寻址

reg[7:0] mymem[1023:0];   //1024个字节，每个字节为8bit
mymem[8] = 1;
mymem[64] = 65;
```
**Tips**:  
不允许对存储器进行按位选择（即mymem[8][0]）与按域选择（即mymem[8][5:0]）  
可以将存储器的值赋给寄存器，然后对寄存器进行按位选择或域选择。  
