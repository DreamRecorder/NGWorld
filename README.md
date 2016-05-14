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

### Microsoft Windows操作系统

由于开发者并不使用Microsoft Windows，即使编写的代码理论上可以在Microsoft Windows操作系统上通过编译，开发者并不能事实上确定在Microsoft Windows上NGWorld可以正常运行。同时，也存在一定程度上，Microsoft在过去某一段时间内对某些人物、团体、功能支持的偏见、傲慢、怠慢和态度令开发者个人在情绪上的愤怒。所以，

**Microsoft Windows暂时不被支持**

## 代码风格

NGWorld使用Allman/BSD缩进风格，采用4个空格作为缩进字符。类名使用骆驼峰命名法，函数名、变量名一律采用全小写+下划线的命名方式，类的私有变量前添加`m_`前缀。以下代码是合格的。

```C++
// 类
class WorldGenerator
{
private:
	u64 m_seed;
public:
	void generate_tree_at(v3s32 pos);
};

// 函数声明
void init_crc32_table();
```

NGWorld**严禁**下列风格的代码进入自主代码库(即3rd Party Code**除外**)：

* 将大括号直接写在前一行的后面
* 变量名或函数名采用骆驼峰命名法
* 另外一切具有TanCC语言风格的代码

反面案例:

```C++
// 违规使用TanCC语法和违规的大括号放置风格
void main(int argc, char **argv) {
	// 变量名违规地采用了骆驼峰命名法
	int MonthsOfYear = 12;
	return 0;
}
```
