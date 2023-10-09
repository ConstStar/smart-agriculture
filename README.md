## 智慧农业系统



### 简介

这个项目也是一个自定义的效果，我觉得写东西就不应该写死，主动权提供给用户，让用户能够做到更高级的使用方法，也能通过网络远程实时控制，远程查看数据。

简单的说，这个项目也只是获取到传感器数据，通过数据可以判断是否达到某个触发器，如果达到触发器设置的值则将开启某个继电器开关，达到完全自定义的效果。

通过这个项目就可以实现土干自动浇水、日落开灯功能、二氧化碳过浓透气功能、日照过强卷帘功能等等等。



开发者也可以在这个系统下简单的更换一套新的传感器，就能实现：工厂环境、养殖环境、智能家具环境等等。



###  功能&&待实现

- [x] QT界面
- [x] MQTT远程管理
- [x] 环境温度、环境湿度、光照强度、二氧化碳温度、土壤湿度
- [x] uCOS-III
- [ ] W25Q128存储配置信息
- [ ] 更换ENS160空气质量传感器（不适合农业下的二氧化碳监测）
- [ ] 多重触发器（不再通过用户串联、并联继电器实现与、或操作，通过软件直接实现触发器的嵌套）
- [ ] 智能配网
- [ ] 网络状态指示



### 开发环境

MCU: stm32f103c8t6

IDE: Keil MDK 5.26.2.0

QT: 5.15.2



### 目录结构

```
├───qt			// QT项目
├───stm32		// Keil项目
└───document    // 文档
    └───image	// 文档图片
```



### 展示

![stm32-main](document\image\stm32-main.jpg)

![ui-state](document\image\ui-state.png)

![ui-setting](document\image\ui-setting.png)

![ui-relay](document\image\ui-relay.png)

![ui-log](document\image\ui-log.png)



### 连线

型号不一样可能会略有不同。

- OLED
  - VCC - 3.3v
  - SCL - PA2
  - SDA - PA1
  - END - END
- ESP8266（需刷MQTT固件，[⑦、MQTT透传AT固件（固件号：1471）](https://docs.ai-thinker.com/%E5%9B%BA%E4%BB%B6%E6%B1%87%E6%80%BB)）
  - VCC - 3.3v
  - RX - PB10
  - TX - PB11
  - END -END
  - 其他悬空
- AHT20
  - VCC - 3.3v
  - SCL - PA2
  - SDA - PA1
  - END - END
- ENS160
  - VCC - 3.3v
  - SCL - PA2
  - SDA - PA1
  - END - END
  - 其他悬空
- 土壤湿度传感器（模拟量）
  - VCC - 3.3v
  - AO - PA0
  - END - END
- GY-30
  - VCC - 3.3v
  - SCL - PA2
  - SDA - PA1
  - 其他悬空
- 八路继电器（5V）
  - DC+ 5v
  - DC- END
  - IN1 - PB5
  - IN2 - PB6
  - IN3 - PB7
  - IN4 - PB8
  - IN5 - PB9
  - IN6 - PB12
  - IN7 - PB13
  - IN8 - PB14



### 许可证

[![license](https://img.shields.io/github/license/ConstStar/smart-agriculture.svg?style=flat-square)](LICENSE)

使用 Apache 2.0 协议开源，请遵守开源协议。