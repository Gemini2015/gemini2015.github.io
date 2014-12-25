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
    若是用`float m[16];`来表示一个4 × 4的矩阵，那么对于OpenGL和D3D来说，平移变换变量都是`m[12] = x, m[13] = y, m[14]= z`, 因为`m[12]`对于OpenGL来说代表`第一行第四列`,对于D3D来说则代表`第四行第一列`。

2.  从个人的角度来讲，***OpenGL的编程属于C风格，D3D则偏向于C++风格***。
    在OpenGL中，实现各种功能主要是通过调用`gl`或是`wgl(Win平台)`开头的全局函数，设置参数，传入数据，绘制……
    而在D3D中，首先通过一个全局函数获取一个D3D的接口，通过这个接口创建一个D3D设备，并且获得设备接口，然后通过这个接口来实现绘制。

3.  ***实现多窗口渲染时的区别***。多窗口渲染的例子有游戏中的小地图，以及CAD中的3视图等。
    在D3D中，一个可行的方法是调用`CreateWindowEx`创建n个Window，然后为每个Window创建一个`SwapChain`（一个SwapChain有`Front`和`Back`两个Buffer，通过调用SwapChain的`Present`函数来进行切换），通过调用SwapChain的`GetBackBuffer`获取SwapChain的BackBuffer，然后调用D3D设备接口的`SetRenderTarget`函数，将BackBuffer设为当前绘制上下文。在不同的SwapChain之间切换，就可以实现多窗口渲染。

    >之前写的OpenGL程序都是直接使用的glut创建窗口，根本不知道OpenGL绘图的时候，需要创建`绘制上下文`，并且将绘制上下文与当前窗口的`DC`绑定，才能开始绘制绘制图形。由此可见，过度依赖于封装好的函数库，对于学习来说并不是一件好事。 

    在OpenGL中，要想绘制图形，必须首先创建一个`RC`（RenderContext），将RC与窗口的DC绑定，才能开始绘制。一个线程只能拥有一个RC，但是可以有多个DC。所以可以使用下面的方法来进行多窗口渲染。同样调用`CreateWindowEx`创建n个Window，获取每个Window的DC，并且调用`ChoosePixelFormat`和`SetPixelFormat`将每个DC的像素格式(`PIXELFORMATDESCRIPTOR`)都设置成相同的（RC根据DC创建，一个RC要想与其他DC绑定，该DC必须与创建该RC的DC具有相同的像素格式）。使用`wglCreateContext`（Win平台前缀为`wgl`）创建一个RC，然后在渲染每个窗口之前使用`wglMakeCurrent(NULL, NULL)`解绑定，再调用`wglMakeCurrent(DC, RC)`来将目标DC与RC绑定，使用`SwapBuffers`切换目标DC的前后台缓冲。
    另一个办法是使用多线程，前面提到一个线程只能拥有一个RC，所以可以通过多线程，来创建多个RC，这样以来接可以避免RC与DC之间的街绑定/绑定问题，实际上，`wglMakeCurrent`函数还是比较耗时的。


碰到的一些问题
--

1.  上面提到了`ChoosePixelFormat`函数，当我在运行的时候，运行到这个函数时，程序直接宕掉了。这个函数的参数是`HDC`和`PIXELFORMATDESCRIPTOR`，我调试了一下，发现这两个参数应该都不会有问题。后来上网搜了一下，在一个英文网站上发现了类似的问题，上面的解释是这个函数会依赖`opengl32.dll`，如果之前没有调用任何gl函数的，程序就不会载入`opengl32.dll`，那么这个函数就会宕掉。解决方法是在这个函数之前随便调用一个gl函数就可以了，我试了一下，调用了一个`glLoadIdentity()`，果然就没问题了。但是后来，我把这个gl调用去掉之后，又不会宕掉，费解。

2.  记一个C++初始化问题，随手写了一个数组初始化，想不到竟然发生了越界，结果运行的时候，是这个地方没有提示异常，反倒是析构的时候报错，debug了半天才发现这个bug。
    
    ```
    class A
    {
        // 有一些成员
    };
    class B
    {
        A* m_aList[20];
    }
    class B::B()
    {
        memset(m_aList, 0, sizeof(A)*20); //随手清零
    }
    ```
    也许是初始化结构体数组习惯了，随手就用`memset`清零了，一个是指针的大小，一个是类的大小，结果当然就越界了。
    正确的是应该使用`memset(m_aList, 0, sizeof(A*) * 20)`。或者干脆用个for循环赋值。

3.  今晚，有个问题困扰了我好久，到现在还没有弄清楚：***OpenGL中的颜色字节序***。
    一般我们常用的颜色格式是`RGBA`，整型表示的话就是`4个Byte`，浮点表示的话就是`4个float`，建立结构体的话如下
    
    ```
    struct color
    {
        byte r;
        byte g;
        byte b;
        byte a;
    };
    ```
    在D3D中，往D3D里面传递颜色参数的时候需要用到D3D自带的`D3DCOLOR_RGBA`来生成一个DWORD数据，在这个DWORD数据里面，颜色值的顺序是`ARGB`。x86架构的计算机都是`Little Endian`，低字节在低地址上，那么内存中的顺序从低字节到高字节应该为`B,G,R,A`。
    再看OpenGL中，查资料也没有看到哪个地方说明了字节序，只是在OGRE的代码里面看到了一个`VET_COLOR_ABGR`的枚举型变量，注释说是OpenGL的顶点颜色类型，如果这就是OpenGL存储颜色的字节序，那么从低地址到高地址就应该为`R,G,B,A`了。
    有时间还是查查资料，这个搞清楚。

4.  由上面一个问题衍生出的一个问题。关于顶点缓冲数组的描述问题。
    顶点结构如下:

    ```
    // Untransformed and lit Vertex
    struct Light_Vertex
    {
        float x, y, z;
        DWORD clrDiffuse;
        float tu, tv;
    };
    // D3D Flexible Vertex Format
    #define LVERTEX_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX0)
    ```
    在D3D中，对于顶点缓冲数组中的`Diffuse Color`数据，是直接接受一个DWORD的，因此，直接调用`D3DCOLOR_RGBA`构造一个DWORD应该就可以了。
    但是在OpenGL中，我查到了一个`glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)`函数，第一个参数`size`的含义是每一个Color由多少个Component组成，**只能填3或4**，第二个参数`type`是指每个Component的数据类型。
    我的疑问是对于上面的结构体，我的参数组合应该是`size = 4, type = GL_UNSIGNED_INT`，还是`size = 4, type = GL_UNSIGNED_BYTE`，理论上，应该是后者，但是如果采用这种组合的话，OpenGL会不会将这个DWORD数据拆开成4个Byte来读取，从低到高分别读取`R,G,B,A`，那么在一开始创建这个DWORD数据的时候，是不是按`A,B,G,R`的顺序构造这个颜色呢？
    举个例子，假设我需要的颜色是`R=0x01, G=0x02, B=0x03, A=0x04`，那么这个DWORD就应该是`0x04030201`了。
    后来，为了看起来更加直观，我将上面的结构体修改成了下面这个:

    ```
    struct Light_Vertex
    {
        float x, y, z;
        union
        {
            struct
            {
            byte r;
            byte g;
            byte b;
            byte a;
            };
            DWORD clrDiffuse;
        };
        float tu, tv;
    };
    ```

5.  ***谈一谈VBO***，这周在调试顶点缓冲，所以将一些心得记录一下。
    
    ```
    typedef struct Pure_Vertex
    {
        // 定义顶点包含的数据，除了坐标数据之外，还可包括纹理，法向量等
        float x, y, z;
    }PVertex;

    /*
    一些初始化：如 glewInit()
    */

    /*
    创建顶点缓冲对象
    */
    GLuint vertexbuffer = 0;
    // 创建一个缓冲对象，类似于句柄，返回非0才为有效值
    // glIsBuffer(vertexbuffer)
    glGenBuffers(1, &vertexbuffer);
    // 将该对象与特定类型缓冲绑定，
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // 填入数据
    // 创建时也可不必传进pVertexData，只需传入buffer_size
    // 后面可用glBufferSubData来修改数据
    glBufferData(GL_ARRAY_BUFFER, buffer_size, pVertexData, GL_STATIC_DRAW);
   
    // 解绑定
    glBindBuffer(GL_ARRAY_BUFFER, 0)

    // 对于顶点索引，步骤与上面类似
    GLuint indexbuffer = 0;
    glGenBuffers(1, &indexbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, pIndexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    ```
    上面包括了一些初始化操作，以及创建顶点缓冲操作，在创建顶点缓冲的时候，可以不马上传进数据，可以在任意位置调用`glBufferSubData`来修改数据。当然修改之前，先要绑定缓冲对象。
    当需要绘制的时候，可以参照下面的步骤。

    ```
    /*
    绘制顶点数据
    */
    // 首先绑定相关对象
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);

    //对于顶点数据，使能各个数据部分
    glEnableClientState(GL_VERTEX_ARRAY);
    // 其次还有 GL_NORMAL_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY

    // 在创建顶点缓冲的时候，不需要设置顶点数据格式，只需要在调用绘制函数之前设置格式即可
    // 指定数据的格式，如顶点数据、纹理、法向量的数据类型，偏移量等
    // 顶点数据格式，一个顶点3个数据（齐次坐标为4），GL_FLOAT类型，stride(可以理解为周期)，偏移值
    glVertexPointer(3, GL_FLOAT, sizeof(PVertex), 0);
    // 除此之外还有 glNormalPointer, glColorPointer, glTexCoordPointer

    // 调用绘制指令
    if(bUseIndex)
    {
        // 对于采用了索引缓冲的，调用下面的函数
        // 元素类型，索引数量，索引数据类型，索引数据（对于采用了索引缓冲对象的，此处可以传进NULL）
        glDrawElements(GL_LINES, index_num, GL_UNSIGNED_SHORT, pIndex);
    }
    else
    {
        // 不采用索引
        // 元素类型，起始顶点索引，顶点数量
        glDrawArrays(GL_LINES, 0, vertex_num);
    }
    
    // 绘制完毕，禁用顶点数据各个部分，解绑定
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    ```
    上面的代码展示了一次绘制操作的简要流程，在绘制一次之后，只要不删除缓冲对象，那么就可以继续绘制，当不需要这个缓冲对象时，可以删除对象

    ```
    // 最后可以删除对象
    if(glIsBuffer(vertexbuffer))
        glDeleteBuffers(1, &vertexbuffer);
    if(glIsBuffer(indexbuffer))
        glDeleteBuffers(1, &indexbuffer);
    ```

6.  ***关于OpenGL的调试***，实际上就我个人认为，调试OpenGL还是有点麻烦的。因为OpenGL采用状态机的方式管理渲染流水线，所以有些时候，一个设置不对，就可能不会出现绘制结果，并且也不容易发现到底是哪个地方出了问题。我采用了以下一些步骤来调试。
    
    1.  **首先检查OpenGL函数调用是否出错**。OpenGL函数不像D3D的函数，D3D函数大多有`HRESULT`的返回值，而OpenGL函数一般没有返回值，所以必须调用`glGetError()`来获取函数调用状态，若返回值不是`GL_NO_ERROR`，那么表明函数调用出错。

        ```
        #ifdef _DEBUG

        #define CHECK_ERROR {\
            GLenum error = glGetError();\
            if(error != GL_NO_ERROR)\
                 Log("OpenGL Error > File:%s Line:%d Error:%d",__FILE__, __LINE__, error);\
        }

        #else

        #define CHECK_ERROR {}

        #endif
        ```
        我定义了一个上面的宏，如果一个函数里面执行了一些OpenGL函数调用，那么就在这个函数末尾写上`CHECK_ERROR;`语句来检查前面的OpenGL函数调用是否出错。

        >OpenGL的机制是一旦某一个OpenGL函数调用出错，在内部就会设置相应的错误标识，通过`glGetError()`函数可以获取这个标志，同时，调用glGetError()函数也会清除这个错误标识。否则，这个错误标识会一直存在，后续有其他错误发生，也不会记录新的错误标识。

        OpenGL的错误代码主要描述了错误的类型，比如`GL_INVALID_ENUM`表示发生枚举值错误，`GL_INVALID_VALUE`表示传给OpenGL函数的值错误等等，一般发生了错误之后，主要是根据错误代码，然后分析每一个OpenGL函数调用，来判定错误发生原因。

    2.  **尝试屏蔽次要OpenGL调用，各个击破**。一大堆OpenGL调用聚在一起，是很难调试的，所以要一部分一部分的调试，调试某个部分的时候，先屏蔽掉其他非必要的代码，等这部分调试完毕之后，在调试其它部分。
        比如调试顶点缓冲的时候，可以先屏蔽掉深度测试、光照、裁剪等，尽量减少无关的OpenGL函数调用，这样便更容易判断出那个地方可能会出问题，只用关注那一块儿即可。

    2.  **获取内部状态**。OpenGL提供了一些函数可以用于获取内部的状态变量，比如视图模型矩阵、投影矩阵等的值。
        对于视图模型矩阵、投影矩阵，若是调用`gluLookAt`和`gluPerspective`来设置的话，一般不容易出错，可以使用`glGetFloatv`获取当前试图模型矩阵和投影矩阵。如果手动计算矩阵的话，可以将计算出来的矩阵，和上面两个函数设置的矩阵进行对比，这样就能解决计算上的Bug。
