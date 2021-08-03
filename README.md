# 嵌入式软件模块
这个仓库存放的是一些嵌入式常用的软件模块，每个模块中都包含了一个例子，以帮助你理解模块的使用方法。

这些模块是笔者经过实际验证的，只需要付出少量的劳动就能方便的应用。

模块化编程的重要性不言而喻，不论这些模块对他人是否有用，我们在编写这些模块化程序的同时就已经达到目的了。如果你对这些程序模块有任何改进意见，或者有其他的模块化程序推荐，欢迎你向这个仓库推送。

## 索引
 - [gpio模拟串口TX](#gpio模拟串口TX)
 - [软件定时器](#软件定时器)
 - [通用队列](#通用队列)
 - [用python对信号进行仿真分析](#用pyhton对信号进行仿真分析)

## gpio模拟串口TX
该模块用MCU的一个普通gpio口实现了模拟串口TX的功能。该功能的用处在于在一些MCU缺乏硬件串口的情况下，用一个gpio来模拟串口TX，以输出调试信息或者把一些数据发送到对端MCU的串口接收端。

[gpio模拟串口TX](https://github.com/liuhao1946/embedded-software-module/tree/master/gpio%E6%A8%A1%E6%8B%9F%E4%B8%B2%E5%8F%A3TX)

## 软件定时器
该模块用于构建软件定时器。可以通过模块提供的API方便且灵活的构建、停止、启动定时任务。也可以轻易的应用于大多数MCU中。

[软件定时器](https://github.com/liuhao1946/embedded-software-module/tree/master/%E8%BD%AF%E4%BB%B6%E5%AE%9A%E6%97%B6%E5%99%A8)

## 通用队列
该模块实现了2种队列，一种有更广泛的使用场景（包括入队、出队、队列内容检索、队列内容删除等），另一种能在特殊时候相比前一种有更高的效率。两种队列都有良好的可移植性，可以轻易的应用在多数MCU中。

[通用队列](https://github.com/liuhao1946/embedded-software-module/tree/master/%E9%80%9A%E7%94%A8%E9%98%9F%E5%88%97)

## 用pyhton对信号进行仿真分析
该部分放置的是几篇关于怎么用python做信号仿真与分析的方法，包括滤波器设计、陷波器设计、FFT、希尔伯特变换与希尔伯特-黄变换、小波变换。

[用pyhton对信号进行仿真分析](https://github.com/liuhao1946/embedded-software-module/tree/master/%E7%94%A8python%E5%AF%B9%E4%BF%A1%E5%8F%B7%E8%BF%9B%E8%A1%8C%E4%BB%BF%E7%9C%9F%E5%88%86%E6%9E%90)






