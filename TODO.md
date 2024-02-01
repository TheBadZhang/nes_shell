
https://github.com/Robert-van-Engelen/tinylisp
尝试添加一个 lisp repl 到项目当中

https://www.taterli.com/3199/
https://twitter.com/shuichi_takano/status/1477702448907419649
https://github.com/pondahai
nes 模拟器

https://wokwi.com/projects/376479299848582145 （电池充电）
https://wokwi.com/projects/328271658006610514 （旋转立方体）
https://wokwi.com/projects/376931330900285441 （指南针）
https://wokwi.com/projects/385987811439126529 （水平指南针）

手写数字识别

摄像头

滚动数字时钟

上位机播放视频

APP 进入退出动画可以考虑重新设计一个更富有动画效果的
文字超过范围滚动显示
列表动画、滑动条、复选框（开关）（方形或者圆形样式）、滑动条、进度条、按钮
多级菜单，列表

1602模拟器

感觉现在性能的瓶颈在读写flash上面，外部flash的速度实在是太慢，无法支撑这么高的主频
写一个内存池管理，用于管理整个DTCMRAM的内容，把一些内容提前预读到内存池当中，这样可以提高读写速度
freertos内部就实现了一个内存池管理，尺寸可以在外面设置，不知道这个heap size最终是如何作用的
静态的图片（flash中）甚至比动态的图片（ram中）还要更吃处理
静态图片（48fps）动态图片（52fps）
APP选择器底部的弧形甚至会产生10fps的波动
试着把pic数组移动到dtcm里面，看看能否提升一定的速度（失败）

使用xmake构建dsp库生成指定版本的lib文件（优化编译速度）

但是项目书里面提到了使用到了psram和nand flash，要用的话引脚绝对不够用了
但是目前感觉ram还是比较够用的

实现 ramfunc

https://github.com/miloyip/misc/tree/master
井字棋游戏
https://github.com/miloyip/light2d/tree/master
https://github.com/miloyip/line/tree/master

⚀⚁⚂⚃⚄⚅

libjpeg dma2d

APP 调试的问题


如果使用ospi的flash大概率没有办法直接使用openocd烧录程序了
要么在bootloader里面实现一个简易的，也许是可以通过usb进行烧录的程序
要么就是利用串口，通过bootloader烧写外部的hyper flash，这个倒是还得额外定制一个软件才行
