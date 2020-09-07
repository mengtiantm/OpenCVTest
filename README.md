图像处理

基于opencv实现图像处理功能（包括平滑、锐化、减影、伪彩以及融合）

一、概述

环境：
系统：windows7
开发环境：visual studio 2017
第三方库：OpenCV340

二、具体说明

1.配置OpenCV环境，笔者用的是340版本，官网下载opencv包，解压配置环境变量OPENCV_DIR为D:\Program Files\opencv\build（此路径根据自己解压存储路径设置）

2.opencv340.props为属性配置文件，直接在VS的“属性管理器”中添加“现有属性表”即可

3.编译运行出效果

4.各种功能效果图

4.1图像平滑
高斯低通滤波
![image](https://github.com/mengtiantm/OpenCVTest/blob/master/ResultImage/Gaussian3x3.png)

4.2图像锐化
拉普拉斯高通滤波
3x3:
![image](https://github.com/mengtiantm/OpenCVTest/blob/master/ResultImage/Laplacian3x3.png)
5x5:
![image](https://github.com/mengtiantm/OpenCVTest/blob/master/ResultImage/Laplacian5x5.png)

锐化滤波
3x3:
![image](https://github.com/mengtiantm/OpenCVTest/blob/master/ResultImage/Sharpening3x3.png)
5x5:
![image](https://github.com/mengtiantm/OpenCVTest/blob/master/ResultImage/Sharpening5x5.png)

4.3图像减影
![image](https://github.com/mengtiantm/OpenCVTest/blob/master/ResultImage/Subtract.png)

4.4图像伪彩
![image](https://github.com/mengtiantm/OpenCVTest/blob/master/ResultImage/PseudoColor.png)

4.5图像融合
![image](https://github.com/mengtiantm/OpenCVTest/blob/master/ResultImage/Fusion.png)
