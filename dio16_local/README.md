## 项目名称
A1型温湿度仪



## 运行条件
> 列出运行该项目所必须的条件和相关依赖  
* 硬件：  ESP32 , SHT30 , SSD1306 , TTL to 485 
* 软件：  esp-idf , esp-aliyun ,esp-iot-solution，环境变量ESP_ALIYUN_PATH, ESP_IOT_SOLUTION_PATH
* 云端资源： 设备激活码



## 运行说明
> 说明如何运行和使用你的项目，建议给出具体的步骤说明
* 操作一
* 操作二
* 操作三  


## 测试说明
> 如果有测试相关内容需要说明，请填写在这里  
UI： 开机显示LOGO画面，1秒钟。
    main让显示未配网工作画面。
    main中，实现功能键在特定画面的回调函数。
    如果没有配网，显示未配网工作画面，底下栏目提示， ：   请按功能键配网，滚动。 按键进入配网QR，同时使能。配网成功，进入主画面，
    如果已经配网，显示主画面：  上面栏目显示wifi和modbus状态。按功能键，显示参数画面
    参数画面，按键下一个，
    画面有 timeout value. 0 disable.  timeout frame 0 disalbe.
    next. 按键就显示next. 0 disable

## 技术架构
# (Automatically converted from project Makefile by convert_to_cmake.py.)

# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set (EXTRA_COMPONENT_DIRS $ENV{ESP_ALIYUN_PATH})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "${EXTRA_COMPONENT_DIRS} $ENV{ESP_IOT_SOLUTION_PATH}/components/display/screen $ENV{ESP_IOT_SOLUTION_PATH}/components/bus")

项目组件 SSD1306



设计计划：
1. 实现board.c board.h ,   提供init接口， 获取 lcd、sht30、button、485串口 句柄的板资源get函数。
2. 增加lcd_font组件，字库放进去
3. 实现主控逻辑。主控逻辑从板子的资源管理函数获取串口句柄，去初始化modbus slave.


## 协作者
> 高效的协作会激发无尽的创造力，将他们的名字记录在这里吧
