# FFT

今天说说用Python进行快速傅里叶变换，把时域信号变换为频域信号，以从另一个角度观察信号，这在信号处理当中十分重要。或许你学过fft，但不知道fft到底怎么去用它，可能还认为fft对与普通的单片机来说是可望不可及的。但是，fft并非只有用在单片机中才叫有用，我们也可以利用它帮助我们分析一些信号，以评估信号质量，进而指导我们的程序甚至硬件设计。这一篇文章就尝试让你看看如何用Python进行傅里叶变换以及傅里叶变换可以怎么用。

我先说说如何使用Python进行傅里叶变换，先看代码。

```python
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt

#生成1000个时间点，时间总长为1s,采样率1000Hz
fs = 1000.0
t = np.arange(fs) / fs

#生成一个持续时间为1s，有1000个数据点的离散信号
s = np.sin(2*np.pi*50.0*t) + np.sin(2*np.pi*100.0*t)

N = len(s)
T = 0.001  #fs = 1000Hz
#np.fft.fftfreq(N,d=T)用于计算N点FFT时对应的时域信号频率f,f=k*fs/N，k=0、1、2.......
#np.fft.fftshift()是把零频率分量移动到频谱中心，见后面的频谱图
f = np.fft.fftshift(np.fft.fftfreq(N, d=T))

#np.fft.fft()原型参见[1]
#np.fft.fft(s)是对信号s做fft，该函数返回值的模是每一个频率对应的幅值(频率的个数等于s的点数)
#由于0频率分量移动到了频谱中心，所以，对信号做完fft后，还需要移动其结果以对应上频率
s_fft = np.fft.fftshift(np.fft.fft(s))

plt.text(50,-20,'50',color='b')
plt.text(100,-20,'100',color='b')
plt.ylim(0,600)
plt.plot(f,abs(s_fft))
plt.show()
```

运行程序，出现下图。可以看到频谱图中50Hz、100Hz出现了幅度值，对应于时域信号的由50Hz、100Hz正弦信号合成的信号。另外，可以看到，时域信号对应的频谱图是关于0频率分量对应的幅度是对称的。

![50+100Hz fft](https://github.com/liuhao1946/embedded-software-module/blob/master/%E7%94%A8python%E5%AF%B9%E4%BF%A1%E5%8F%B7%E8%BF%9B%E8%A1%8C%E4%BB%BF%E7%9C%9F%E5%88%86%E6%9E%90/png/fft/50%2B100Hz%20fft.png)

接下来，我们看看FFT的一个应用，这是我在做一个计步算法的时候用到的。

一般而言，计步算法用到的传感器是加速度计，而加速度传感器可配置的寄存器众多，而这其中有两个参数很重要，一个是滤波带宽，另一个就是信号的输出率（实际上就是内部采样率）。在加速度计中，信号输出率一般是滤波带宽的两倍。选择不同的滤波带宽有不同的信号输出率，那问题是我选择了一个滤波带宽，我怎么知道是好还是不好的呢，这里就需要用到fft了。

思路如下，我们首先获得传感器3个轴的加速度值，求3个轴加速度矢量和的模：

$|a|$ = $\sqrt{a_x^2+a_y^2+a_z^2}$

由此得到一段时间内的加速度值。然后把这段时间内所获得的加速度值进行fft，得到这个时域信号对应的频谱。通过频谱图，我们就能知道这个时域信号的频谱分布，哪些是不应该存在的，哪些是应该存在的，也就知道了在这个滤波带宽下输出信号是否能满足我们的应用要求。这里要注意，为了获得可靠的信号源，还需要确保产生加速度的测试条件应该是稳定的，比如把加速度计放在摇步器上，从而减少一些未知情况的干扰，让获得的数据更为可靠。

由于人走步时，频率一般不超过5Hz。基于此，我选择了两个滤波带宽，一个是15Hz，一个是1000Hz。采集了两者数据保存在excel表中[3]，然后分别对两种情况下的数据做fft得到两者的频谱图，结果如下图所示。

```python
# -*- coding: utf-8 -*-

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

#读取excel中的加速度数据（内部保存了加速度矢量和的模，详见附件）
a = pd.read_excel("accel data.xlsx")
N = len(a['15Hz'])

a15_fft = np.fft.fftshift(np.fft.fft(a['15Hz']))
a1000_fft = np.fft.fftshift(np.fft.fft(a['1000Hz']))

#我的加速度采样率是20Hz，因此T=0.05
T = 0.05
freq = np.fft.fftshift(np.fft.fftfreq(N,d=T))

plt.subplot(2,1,1)
plt.ylim(0,3000)
plt.plot(freq,abs(a15_fft))

plt.subplot(2,1,2)
plt.ylim(0,3000)
plt.plot(freq,abs(a1000_fft))
plt.show()
```

运行上述代码，得到下面的频谱图（横轴是频率，纵轴是对应的幅度值）。

![bw=15Hz bw=1000Hz](https://github.com/liuhao1946/embedded-software-module/blob/master/%E7%94%A8python%E5%AF%B9%E4%BF%A1%E5%8F%B7%E8%BF%9B%E8%A1%8C%E4%BB%BF%E7%9C%9F%E5%88%86%E6%9E%90/png/fft/15Hz%20and%201000Hz%20fft.png)

从频谱图中发现，由于我的加速度传感器放在摇步器上，而摇步器的运行基本上又是稳定的，按理说频谱图中只应该很少量的频率才对，但从上面的频谱图可以看出，15Hz的频谱图中具有大能量的频率明显多于1000Hz的频谱图，而且15Hz频谱图中，低幅值的频点的杂乱程度以及幅度的大小都明显高于1000Hz的频谱图。可见，对比二者，1000Hz的带宽设置对于我的应用而言是更合适的。从实际结果来看，也的确如此。由此，单片机中或许用不了fft算法，但是我们依然可以借助一些仿真工具在电脑上对采集的信号做fft，从而分析信号质量，指导我们设计软件。

这里，顺便说一下，由于加速度计的数据输出率是在1000Hz带宽的情况下为2000Hz，而我的应用采样率只有20Hz，这实际是做了一个减采样，只要构成时域信号的频率最大值小于10Hz（需要满足奎奈斯特定理，我们前面说过，一般人正常的走步是不超过5Hz的），那么就不会发生混频，进而也不会影响我们的采样信号（因为时域与频域的等价关系，出现频谱变形时，如果我们把所采集到的离散信号点用直线连接起来，你可能会发现信号就像有噪音一样，变形严重，进而导致后面的算法设计更为困难）。

fft除了在信号评估上的一个应用外，另外一个应用就是为硬件的低通（或者高通之类的）滤波器提供设计支持。一些输出模拟信号的传感器一般需要在信号输出脚加一个硬件低通或者高通之类的滤波器，但是在设计之前，你需要知道信号的频率范围，然后才能合理的设计滤波器。而要知道模拟信号的频率范围，我们可以使用高的采样率（满足奎奈斯特定理）采样模拟信号，然后采样的信号做fft获得其频谱，由此，我们就知道了时域信号的频率范围，从而确定硬件滤波器的滤波参数。

[[1]:fft()](https://numpy.org/devdocs/reference/generated/numpy.fft.fft.html#numpy.fft.fft)

[[2]:fftfreq()](https://numpy.org/devdocs/reference/generated/numpy.fft.fftfreq.html#numpy.fft.fftfreq)

[[3]:accel data.xlsx](https://github.com/liuhao1946/embedded-software-module/blob/master/%E7%94%A8python%E5%AF%B9%E4%BF%A1%E5%8F%B7%E8%BF%9B%E8%A1%8C%E4%BB%BF%E7%9C%9F%E5%88%86%E6%9E%90/png/fft/accel%20data.xlsx)

