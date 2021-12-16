---
layout: post
title: LRU算法在UI缓存策略上的应用
categories: Game
description: 将LRU（最近最少使用）算法应用到UI缓存策略上
codelang: lua
---

**时间与空间的取舍总是会出现在软件开发的各个环节之中。**

## 前言

一个UI的生命周期可以概括为以下几个阶段：

1.  静态资源文件：不占用内存空间
1.  创建UI：由静态资源文件创建UI对象，需要分配内存
1.  打开&更新UI：激活UI，使之处于可见状态
1.  关闭UI：使UI处于不可见状态
1.  销毁UI：销毁UI对象，释放内存

当前项目中存在大量的UI，既有复杂的，也有简单的。
相比于**打开&更新UI**，**创建UI**一般是相当耗时的，所以从**时间角度**上来考虑的话，你可能倾向于**不销毁UI**（即缓存UI），以便下次需要使用该UI的时候，直接打开，更新。
但是从**空间角度**上考虑的话，所有UI都不销毁，势必会造成**内存的占用过多**。在手机设备上，内存占用过多，会导致可用内存不足，而可用内存不足的后果可能就是闪退。
所以，这个时候就需要选择一个UI缓存策略，来达到一个时间和空间上的平衡。

前段时间和同事讨论UI缓存策略的时候，同事提到了一个方案，我思考了一下感觉有点类似LRU算法，因此写下这篇文章简要描述一下LRU在UI缓存上的应用。

## 几个UI缓存策略

*   全部缓存：空间占用最大，时间消耗最少
*   全部不缓存：空间占用最小，时间消耗最多
*   FIFO（First In First Out，先进先出）：空间占用较少，缓存命中率较低
*   LFU（Least-Frequently Used，最近最常使用）：空间占用较少，缓存命中率较高
*   LRU（Least Recently Used，最近最少使用）：空间占用较少，缓存命中率较高

LFU与LRU类似，但LFU每次命中都会将对应数据的命中次数加1，在需要抛弃数据时，将命中次数最少的数据抛弃，而LRU则是将距离当前时间最久的数据抛弃。
当缓存数量为3时，考虑一种这样的访问顺序：A, B, A, B, C, D, C, D
LFU算法会导致C, D来回替换，而LRU不存在这样的问题。

UI访问路径一般会呈现出一种树状结构，而且会频繁的在叶子节点与其父节点之间来回访问，比如道具列表界面和道具详情界面，所以考虑到上述LFU存在的问题，故使用LRU要合适一些。
同时，UI可以分为**全屏**和**非全屏**两种，非全屏UI一般依附于某个全屏UI，在访问路径上，可以认为是某个全屏UI的子节点，一般情况下，你可能不会希望在某个全屏UI下，打开一个非全屏UI，因为使用了LFU算法，导致这个全屏UI被关闭的情况出现。

LFU的一个特点是，可以保证那些累计访问次数较多的UI，不会被被销毁掉。当使用LRU算法时，对于那些能预测到的会频繁访问的UI，可以不使用LRU算法来管理，强制缓存，比如背包界面，人物详情界面等……

## LRU(Least Recently Used)

关于LRU的描述，可以参考一下以下链接

*   [Cache_replacement_policies](https://en.wikipedia.org/wiki/Cache_replacement_policies)
*   [LRU-百度百科](https://baike.baidu.com/link?url=BjOGTObL-7beUmxVcrPT_58YCCgw4htHKJabyFEbnlSl9tFIARCk-XF9rcE3_nWhL-cMVaiTaOeVhaPaewgu3K)

## 一个简单的LRU实现

下面是一个LRU算法的Lua实现，只是演示用，效率上还可以提升一下。
```
local lru = {
    forms = {},
    counter = 0,
    max_size = 5,
}
local this = lru

function lru.setup(max_size, ondiscard)
    this.counter = 0
    this.max_size = max_size or 5
    this.ondiscard = ondiscard
end

function lru.visit(key)
    if this.try_hit(key) then
        return
    end

    local slot_index = #this.forms + 1
    if #this.forms == this.max_size then
        slot_index = this.discard()
    end

    this.forms[slot_index] = {
        key = key,
        value = this.counter
    }

    this.counter = this.counter + 1
end

function lru.try_hit(key)
    for _, v in ipairs(this.forms) do
        if v.key == key then
            v.value = this.counter
            this.counter = this.counter + 1
            return true
        end
    end
    return false
end

function lru.discard()
    if #this.forms == 0 then
        return 1
    end
    local target_index = 1
    local target = this.forms[target_index]
    for index, v in ipairs(this.forms) do
        if v.value < target.value then
            target_index = index
            target = v
        end
    end

    if this.ondiscard ~= nil then
        this.ondiscard(target.key)
    end

    return target_index
end

return lru
```

测试程序
```
local lru = require('lru')

lru.setup(5, function(key)
    print('discard ' .. key)
end)

for i = 0, 20 do
    local a = math.random(1, 10)
    print('load ' .. a)
    lru.visit(a)
end

```