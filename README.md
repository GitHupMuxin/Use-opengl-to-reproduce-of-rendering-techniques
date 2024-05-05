介绍
运行依赖的外部库 opengl, assimp, glfw, glad, stb_image.h（即learn opengl 里面使用的外部链接库）。

下面是对整个项目的介绍

前言 这个项目主要是使用OpenGL实现了pcf pcss阴影生成，简单的球斜函数(sh)的使用，屏幕空间的反射(ssr/ssrt)，以及一些基于物理的渲染(pbr)，实现了Cook-Torrance模型和Kulla-Conty模型，使用LUT加速方式，IBL等环境光等等。

本人在写这个项目的时候并不是很熟悉OpenGL，所以项目里面的代码封装考虑的并不是很周到，以及一些复用做的不是很好(一个shader能做完的事情硬生成了n个shader)，还有glsl也是一坨，毕竟主要以实现功能为主

![image](https://github.com/GitHupMuxin/Use-opengl-to-reproduce-of-rendering-techniques/tree/master/resultPicture/p1.png)
