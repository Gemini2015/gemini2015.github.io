---
layout: post
title: 使用OpenGL实现ZFXEngine的渲染系统
date: 2014-12-04 21:18:10
categories: game dev 
description: 使用OpenGL实现ZFXEngine定义的ZFXRenderDevice
---


&emsp;&emsp;前一段时间，看了一下OpenGL，把OpenGL红宝书看了几章，实现了书里面的例程，虽然书里面的例程大部分都能看懂，但是总感觉还是不会使用。对于在游戏引擎的实现中，OpenGL（或是D3D）会在哪些地方使用到，怎么使用等问题，都还是非常模糊，因此我觉得有必要进行一下实践。刚好之前看的《3D Game Engine Programming》里面，采用D3D实现了一个简单的游戏引擎，所以我打算使用OpenGL来实现他的渲染系统。同时，也可以顺便观察一下OpenGL与D3D在具体使用过程中的区别。


基于使用角度分析的区别
--

1.  ***OpenGL中的矩阵采用列向量(Ogre等)，而D3D采用行向量***。这个差异的后果就是，*矩阵相乘的顺序*会发生改变，以及*变换矩阵中平移变换元素的位置*发生改变。
    在OpenGL中，矩阵与向量相乘的顺序是`ProjectionMatrix * ViewMatrix * ModelMatrix * VertexPosition`，而在D3D中则是`VertexPosition * ModelMatrix * ViewMatrix * ProjectionMatrix`，刚好相反，显然D3D的顺序更符合人类的认知习惯，但从计算机的角度来讲，使用列向量似乎可以获得一个好的运行效率。
    在OpenGL中，平移变量为`m[1][4] = x, m[2][4] = y, m[3][4] = z`，而在D3D中则是`m[4][1] = x, m[4][2] = y, m[4][3] = z`，对于比例，旋转变换，OpenGL与D3D没有什么区别。

2.  从个人的角度来讲，***OpenGL的编程属于C风格，D3D则偏向于C++风格***。
    在OpenGL中，实现各种功能主要是通过调用`gl`或是`wgl(Win平台)`开头的全局函数，设置参数，传入数据，绘制……
    而在D3D中，首先通过一个全局函数获取一个D3D的接口，通过这个接口创建一个D3D设备，并且获得设备接口，然后通过这个接口来实现绘制。

3.  ***实现多窗口渲染时的区别***。多窗口渲染的例子有游戏中的小地图，以及CAD中的3视图等。
    在D3D中，一个可行的方法是调用`CreateWindowEx`创建n个Window，然后为每个Window创建一个`SwapChain`（一个SwapChain有`Front`和`Back`两个Buffer，通过调用SwapChain的`Present`函数来进行切换），通过调用SwapChain的`GetBackBuffer`获取SwapChain的BackBuffer，然后调用D3D设备接口的`SetRenderTarget`函数，将BackBuffer设为当前绘制上下文。在不同的SwapChain之间切换，就可以实现多窗口渲染。

    >之前写的OpenGL程序都是直接使用的glut创建窗口，根本不知道OpenGL绘图的时候，需要创建`绘制上下文`，并且将绘制上下文与当前窗口的`DC`绑定，才能开始绘制绘制图形。由此可见，过度依赖于封装好的函数库，对于学习来说并不是一件好事。 

    在OpenGL中，要想绘制图形，必须首先创建一个`RC`（RenderContext），将RC与窗口的DC绑定，才能开始绘制。一个线程只能拥有一个RC，但是可以有多个DC。所以可以使用下面的方法来进行多窗口渲染。同样调用`CreateWindowEx`创建n个Window，获取每个Window的DC，并且将每个DC的像素格式(`PIXELFORMATDESCRIPTOR`)都设置成相同的（RC根据DC创建，一个RC要想与其他DC绑定，该DC必须与创建该RC的DC具有相同的像素格式）。使用`wglCreateContext`（Win平台前缀为`wgl`）创建一个RC，然后在渲染每个窗口之前使用`wglMakeCurrent(NULL, NULL)`解绑定，再调用`wglMakeCurrent(DC, RC)`来将目标DC与RC绑定，使用`SwapBuffers`切换目标DC的前后台缓冲。
    另一个办法是使用多线程，前面提到一个线程只能拥有一个RC，所以可以通过多线程，来创建多个RC，这样以来接可以避免RC与DC之间的街绑定/绑定问题，实际上，`wglMakeCurrent`函数还是比较耗时的。