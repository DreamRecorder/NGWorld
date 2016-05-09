# NGWorld
[NEWorld](https://github.com/Infinideastudio/NEWorld)激发了我用C++和OpenGL制作一个Minecraft的复刻的兴趣。慢步地走在草地上，和陷在一滩没有边际的沼泽里挣扎，你要选择哪一个？所以我决定新建一个私人的空repo从零开始。

这是一个**实验研究性**项目，作者可能会在数据结构、算法、常数优化、图形渲染上面来来回回地花费很多工夫，做一些benchmark实验，写一写论文什么的。这**不是**实用性项目。

本项目的源代码采用[GPLv3](http://www.gnu.org/licenses/gpl.html)许可协议发布，请遵守和尊重。

## IDE与编译

NGWorld在Git库中加入了Xcode的项目文件，也按照Xcode的文件目录组织结构进行了重组。Xcode是我近一段时间开始尝试使用的IDE，在试用了Eclipse-CDT、Code::Blocks、CodeLite、NetBeans之后，我发现在Mac OS X平台上最好用的C/C++ IDE开发工具仍然是Xcode。

Xcode相对于其它跨平台IDE的优点有：

* 与OS X系统相对集成，操作界面相对原生。
* 反应相对迅速，Eclipse在滚动代码的时候存在严重的丢帧卡顿问题。
* 相对于Eclipse和Netbeans而言基本没有发热，非常省电。

但是也有缺点：

* 内存占用极大，4GB内存的Macbook Air直接被吞掉1GB。
* 不能自定义缩进，自动补全的代码块的缩进风格让我非常不爽。

## 代码风格

NGWorld使用Allman/BSD缩进风格，采用4个空格作为缩进字符。一律采用全小写+下划线的命名方式，例如`void generate_tree_at(v3s32 pos);`。类的私有变量前添加`m_`前缀。

NGWorld**严禁**下列风格的代码进入自主代码库(即3rd Party Code**除外**，例如QuickLZ、LZ4等)：

* 将大括号直接写在前一行的后面
* 采用骆驼峰命名法
* 另外一切具有TanCC语言风格的代码

```C++
void main(int argc, char **argv) {
    int MonthsOfYear = 12;
    return 0;
}
```

