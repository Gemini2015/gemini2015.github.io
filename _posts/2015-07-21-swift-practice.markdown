---
layout: post
title: Swift实践
date: 2015-07-21 23:19:38
categories: Swift
description: 使用Swift过程中的一些经验
codelang: swift
---


## 代码片段

### 比特枚举
为了表示一些配置或标记，在C++中，我们经常会使用下面的枚举

```
enum Item
{
    Apple = 1 << 0,
    Orange = 1 << 1,
    Peach = 1 << 2,
}
int itemSet = Apple | Orange;
if itemSet & Apple == Apple
{
    // do
}
else if itemSet & Peach == 0
{
    // do
}

itemSet |= Peach;   // itemSet: Apple, Orange, Peach
itemSet &= ~Apple;  // itemSet: Orange, Peach
```
**Objective-C**中也可以通过`enum`来实现这样的功能，但是在**Swift**中却不能通过`enum`来实现，而是通过`struct`来实现。***Swift 2.0***中代码如下。

```
struct Item: OptionSetType
{
    let rawValue: Int
    init(rawValue: Int) { self.rawValue = rawValue }

    static let Apple = Item(rawValue: 1 << 0)
    static let Orange = Item(rawValue: 1 << 1)
    static let Peach = Item(rawValue: 1 << 2)
}
var itemSet: Item = [.Apple, .Orange]
if itemSet.contains(.Apple)
{
    // do
}
else if !itemSet.contains(.Peach)
{
    // do
}

itemSet.unionInPlace(.Peach)    // itemSet: Apple, Orange, Peach
itemSet.subtractInPlace(.Apple) // itemSet: Orange, Peach
```