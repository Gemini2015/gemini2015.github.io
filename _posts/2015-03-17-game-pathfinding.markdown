---
layout: post
title: 浅谈游戏寻路
date: 2015-03-17 20:09:52
categories: game dev 
description: 关于游戏中寻路的一些简单思路
---


今天在工作中产生了一个需求：在一组地图中，找出距指定地图最近的那张地图。一个很典型最短路径问题，可以使用Floyd-Warshall算法轻松解决。在解决这个问题的过程中，对游戏寻路进行了一些简单的思考。

## 基本概念
考察一下现在市面上的游戏，可以发现，大部分游戏都是采用**多地图**机制，少部分游戏只使用一张特大地图，如GTA、武装突袭等。
对于采用多地图机制的游戏，寻路可以分为**地图间寻路**和**地图内寻路**两个部分。地图间寻路可以从宏观的角度上确定从起点到目的点需要经过的地图序列。而地图内寻路则从微观上确定从一个地图走到另一个地图，或到达当前地图指定点的行走路线，行走线路也可以称之为路径。
需要注意的是，现在游戏中大多有**传送**机制，也就是说，两个非相邻的地图也可以连通。
在谈论路径的时候，一般会考虑路径的代价，也可以称为路径的**权**。
**寻路**，也就是*找出一组可以连通起点和目标点的路径，并且这些路径的权值之和最小*。
一般在一张地图中，会有若干个区域，当玩家走到这些区域时，会通过某些机制，将玩家移动到另一张地图上，我们称这些区域为**边界区**。

对于地图间寻路和地图内寻路都可以应用各种经典最短路径算法，如Dijkstra算法、A*算法等。但是考虑到实际情况，对于地图间寻路和地图内寻路还是可以做不同的处理。
一般情况下，游戏中的地图数量不会太多，而且较为固定，所以对于地图间寻路来说，可以在游戏初始化的时候，求出任意两张地图的最短路径，并且保存起来，只在发生影响地图连通性的变化（如某个地图消失，或地图边界消失）之后再重新计算。
而地图内寻路就不同了，没有必要保存所有的寻路可能性，所以一般地图内寻路都是动态计算的。

地图内寻路一般采用A*算法，下文重点考虑地图间寻路，在某些假设的情况下，实现细节。

## 基于简单假设
在简单假设中，**不考虑传送，地图间寻路与地图内寻路各自独立。**
在这样的情况下进行地图间寻路时，我们不考虑从一张地图走到另一张地图的代价，也就是说，可以认为任意两张相邻地图间的权值为 1 。如果两张地图不相邻，则初始权值为 无穷大（下文用 $ 表示 无穷大）。
对应于这种情况的Floyd-Warshall算法：

```
// 最大地图数量
int MAX_NUM;

// 任意两张地图x, y对应的权值为 weight[x][y]
// 初始化为无穷大
int weight[MAX_NUM][MAX_NUM] = { $ };
// 保存最短路径
int path[MAX_NUM][MAX_NUM] = { 0 };

// 填充初始数据
for(int i = 0; i < MAX_NUM; i++)
{
	for(int j = 0; j < MAX_NUM; j++)
	{
		if(地图i, j相邻)
		{
			weight[i][j] = 1;
            path[i][j] = j;
		}
	}
}

// 计算最短路径
for(int k = 0; k < MAX_NUM; k++)
{
	for(int i = 0; i < MAX_NUM; i++)
	{
		for(int j = 0; j < MAX_NUM; j++)
		{
            if(i == k || j == k || i == j)
                continue;
			if(weight[i][k] + weight[k][j] < weight[i][j])
			{
				weight[i][j] = weight[i][k] + weight[k][j];
                path[i][j] = path[i][k];
			}
		}
	}
}
```

## 基于一般假设
在较精确的情况下，**需要考虑传送，同时地图间寻路与地图内寻路进行联合考虑。**
在地图间寻路与地图内寻路独立时，我们认为两张相邻地图的权值为 1 。但是若将地图间寻路与地图内寻路联合考虑的话，可以将**从当前地图起点（一般是地图的边界区）走到相邻地图的代价**作为两张地图间的权值。
从宏观上来讲，**传送**的作用是使两张本来不连通或不相邻的地图，变成相邻地图。对于由传送生成的这种相邻情况，我们可以取**从当前地图起点走到传送点的代价**作为权值。
对应的伪代码：

```
// 最大地图数量
int MAX_NUM;

// 通过地图内寻路计算出来的权值
int WeightInMap[MAX_NUM][MAX_NUM];

// 任意两张地图x, y对应的权值为 weight[x][y]
// 初始化为无穷大
int weight[MAX_NUM][MAX_NUM] = { $ };
// 保存最短路径
int path[MAX_NUM][MAX_NUM] = { 0 };

// 填充初始数据
for(int i = 0; i < MAX_NUM; i++)
{
	for(int j = 0; j < MAX_NUM; j++)
	{
		if(地图i,j相邻 || 地图i,j可传送)
		{
			weight[i][j] = WeightInMap[i][j];
            path[i][j] = j;
		}
	}
}

// 计算最短路径
for(int k = 0; k < MAX_NUM; k++)
{
	for(int i = 0; i < MAX_NUM; i++)
	{
		for(int j = 0; j < MAX_NUM; j++)
		{
			if(i == k || j == k || i == j)
                continue;
            if(weight[i][k] + weight[k][j] < weight[i][j])
            {
                weight[i][j] = weight[i][k] + weight[k][j];
                path[i][j] = path[i][k];
            }
		}
	}
}
```