---
layout: post
title: 应用Docker搭建SVN服务器
categories: Dev
description: 利用Docker容器技术，快速搭建SVN服务器
codelang: shell
---

## 前言

不知道你们有没有碰到这样的情境

*   经常需要用Linux服务器来搭建各种各样的服务，比如SVN服务器，Gitlab，Kanboard，Django开发环境等，安装倒不是一件难事，但是在安装的时候，会附带安装一堆依赖库，有些情况下这些依赖库还会冲突，解决这样的问题总是很痛苦的。
*   同一个程序的不同版本也会有兼容性问题，比如Python，Node.js，Ruby等，为解决这个问题，人们开发出了对应的“虚拟机”，如virtualEnv，nvm，rvm，来解决版本切换问题。
*   有很多时候，你只是想尝尝鲜，体验一下，所以你可能希望能够快速在沙盒环境下测试一下，用完之后，能简单的清理掉。
*   还有一点是对于一些服务性的程序，比如SVN服务器，Gitlab等，你可能希望能够将数据与程序进行明显分离，以便后续迁移或备份。

利用Docker容器技术的话，能够很方便的解决上述问题。

*   使用Docker为每一个程序单独构建一个沙盒环境，这样就能解决依赖问题，这个沙盒作为一个容器，也能很方便的清理掉。
*   利用Docker建立一个沙盒环境是很快速的，而且成本也很低，这也是容器技术与传统虚拟机技术相比的一个重大优势。
*   利用Docker的**卷**概念，可以打通容器与主机的数据通道，实现容器内跑程序，数据存主机。


## 概念术语

*   **镜像**：可以认为是一组程序的集合，比如一个Django镜像里面可能包括了一个Linux系统，Python环境，Django等等一组相关程序。可以很方便的从镜像市场上下载一个别人制作好的镜像，也可以在企业内部自己搭建镜像市场，分享企业内部的镜像。
*   **容器**：一个沙盒环境，可以看作是一个简易的Linux环境，容器之间都是互相隔离的。
*   **仓库**：一个镜像的不同tag组合在一起称为该镜像的仓库，如一个Ubuntu仓库里面包含12.04的镜像，14.04的镜像。
*   **宿主机**：运行Docker引擎的机器。

## 安装Docker

现在Docker的安装都很方便了，也不用翻墙，访问速度还可以。
而且除了Linux系统，还支持Mac、Windows。
可以按照下面的链接来选择安装。
[https://store.docker.com/search?type=edition&offering=community](https://store.docker.com/search?type=edition&offering=community 'docker')

## 搭建SVN服务器

SVN服务器镜像的地址  
[https://store.docker.com/community/images/marvambass/subversion](https://store.docker.com/community/images/marvambass/subversion 'subversion')  
上面这个地址已经给出了运行方法，但是有一点小错误，所以下面重新整理一下运行步骤。

1.  首先执行`docker pull marvambass/subversion`将这个镜像拉取到本地。
2.  然后需要创建三个目录：
    *   `mkdir ~/svn`：存放svn仓库
    *   `mkdir ~/svn_backup`：存放svn仓库备份
    *   `mkdir ~/svn_conf`：存放SVN权限控制文件`dav_svn.authz`和账号密码文件`dav_svn.passwd`
3.  创建SVN权限控制文件`touch ~/svn_conf/dav_svn.authz`，按照如下示例修改文件内容
    ```
    [groups]
    admin = user1, user2, testuser
    devgroup = user5, user6

    [project1:/]
    @admin = rw
    @devgroup = r

    # devgroup members are able to read and write on project2
    [project2:/]
    @devgroup = rw

    # admins have control over every project - and can list all projects on the root point
    [/]
    @admin = rw
    ```
    *   `groups`：代表权限组
    *   `project1`：表示svn仓库，其路径为`~/svn/project1`
    *   `/`：代表所有仓库
4.  创建账号密码文件`touch ~/svn_conf/dav_svn.passwd`，**使用`htdigest`命令创建账号（上面网站说是使用`htpasswd`，但是从项目对应github上看配置文件可知，验证方式不是Basic，而是Digest）**  
    输入命令`htdigest ~/svn_conf/dav_svn.passwd Subversion testuser`，按照提示输入密码，创建成功的账号密码会自动写入到`~/svn_conf/dav_svn.passwd`这个文件里。
5.  使用如下命令创建启动容器
    ```
    docker run \
    -d -p 9200:80 -p 9201:443 \
    -v /home/username/svn:/var/local/svn \
    -v /home/username/svn_backup:/var/svn-backup \
    -v /home/username/svn_conf/:/etc/apache2/dav_svn/ \
    --name svn marvambass/subversion \
    ```
    *   `-p 9200:80`是将容器的80端口映射到宿主机的9200端口，这样外部可以通过访问宿主机的9200端口来访问容器的80端口。
    *   `-v /home/username/svn:/var/local/svn`是将宿主机的`/home/username/svn`目录映射到容器的`/var/local/svn`目录，这样容器内程序对`/var/local/svn`目录的读写，实际上是对宿主机`/home/username/svn`目录的读写。
    *   `--name svn`是将容器命名为`svn`，后续停止、重启等操作都可以直接用名字来操作，如`docker stop svn`、`docker restart svn`。
6.  在`~/svn`目录下新建一个目录即相当于新建了一个仓库。**新建目录、修改访问权限、账号密码都是即时生效的，不用重启容器。**
7.  在局域网内，可以通过`http://ipaddress:9200/svn/project1`来访问该仓库，也可以用SVN客户端Checkout该地址。其中`ipaddress`为宿主机的局域网IP。

## 参考

[镜像对应github项目地址](https://github.com/MarvAmBass/docker-subversion)