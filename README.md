# NGWorld
[NEWorld](https://github.com/Infinideastudio/NEWorld)激发了我用C++和OpenGL制作一个Minecraft的复刻的兴趣。慢步地走在草地上，和陷在一滩没有边际的沼泽里挣扎，你要选择哪一个？所以我决定新建一个私人的空repo从零开始。

这是一个**实验研究性**项目，作者可能会在数据结构、算法、常数优化、图形渲染上面来来回回地花费很多工夫，做一些benchmark实验，写一写论文什么的。这**不是**实用性项目。

本项目的源代码采用[GPLv3](http://www.gnu.org/licenses/gpl.html)许可协议发布，请遵守和尊重。

## 目录结构

* client: 客户端
* internal: 客户端与服务端共享的代码
* server: 服务端

## 编译

### UNIX-like操作系统

NGWorld根目录下有Makefile，使用`make [options] <target>`命令编译。

| Target名称 | 说明        |
|------------|-------------|
| Client     | 编译客户端  |
| Server     | 编译服务端  |
| TestBench  | 编译测试模块|

选项有

| 选项名称   | 说明        |
|------------|-------------|
| NOWARNING  | 禁止所有警告|
| DEBUG      | 调试模式    |

## 代码风格

NGWorld使用Allman/BSD缩进风格，采用4个空格作为缩进字符。一律采用全小写+下划线的命名方式，例如`void generate_tree_at(v3s32 pos);`。类的私有变量前添加`m_`前缀。

NGWorld**严禁**下列风格的代码进入自主代码库(即3rd Party Code**除外**)：

* 将大括号直接写在前一行的后面
* 采用骆驼峰命名法
* 另外一切具有TanCC语言风格的代码

反面案例:
```C++
void main(int argc, char **argv) {
    int MonthsOfYear = 12;
    return 0;
}
```

