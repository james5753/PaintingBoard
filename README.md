#  PaintingBoard 画板程序

> 基于 Qt 6 开发的交互式绘图工具，支持多种图形绘制、编辑、变换与图形学算法演示。

---

##  项目简介

**PaintingBoard** 是一个基于 C++ 与 Qt 框架开发的图形绘制与编辑程序，支持直线、圆、多边形、Bezier 曲线、B 样条曲线等多种图形的绘制与变换。是上海大学计算机图形学课程的课程项目。
- 本项目核心特性包括：

- 交互式鼠标绘制
- 多种图形变换：平移 / 缩放 / 旋转
- 经典图形学算法实现
- 图形选择、多选、组合绑定操作
- 支持撤销、填色、选择、绑定、裁剪等复杂功能

---

##  功能特性

###  图形绘制

- **直线**：支持 Bresenham 与中点算法
- **圆**：中点圆绘制算法
- **圆弧**：基于正负法绘制圆弧
- **多边形**
  - 任意多边形绘制
  - 规则多边形（输入边数与半径）
- **Bezier 曲线**：支持任意控制点，使用 De Casteljau 算法
- **B 样条曲线**：支持自定义阶数的 B-Spline 曲线绘制

###  图形操作

- **平移**：支持单图形与多图形拖拽平移
- **缩放**：支持鼠标控制缩放比例，支持自定义旋转中心
- **旋转**：绕指定旋转中心旋转任意角度
- **撤销操作**：支持撤销最近一次绘图或变换
- **图形选择与多选**：支持图形高亮选中与多图形批量操作
- **图形绑定**：支持图形组合变换

###  图形裁剪

- **直线裁剪**：
  - Cohen-Sutherland 算法
  - 中点分割裁剪
- **多边形裁剪**：Sutherland-Hodgman 算法

###  颜色填充功能

- **多边形扫描线填充**
- **FloodFill（泛洪填充）**

---

## ️ 使用界面预览

> 示例界面和绘图效果：
> ![绘图界面](./screenshot1.png)


---

##  快速开始

### 运行环境

|    项目   |     版本要求       |
|-----------|-------------------|
|  操作系统 |    Windows 10/11   |
|  开发工具 | Visual Studio 2022 |
|   编译器  |     MSVC（x64）    |
|   Qt版本  |       Qt 6.5       |
|  C++ 标准 |       C++14        |

### 编译与运行

1. 确保已经安装 Qt 开发环境（推荐使用 Qt 6.5 + MSVC）
2. 使用 Visual Studio 2022 打开 `.sln` 项目文件
3. 切换为 `Release` 模式，构建并运行项目

---

##  操作指南

### 绘图流程

1. 选择图形类型（线段、圆、Bezier、B样条、多边形等）
2. 使用鼠标点击或拖拽绘制图形
3. 支持右键完成某些图形的绘制（如任意多边形、Bezier 曲线）

### 编辑与变换

- **选择图形**：点击或框选图形
- **执行操作**：选择操作类型（如平移、缩放、旋转）后拖动图形
- **绑定图形**：多选后点击右键完成绑定
- **裁剪图形**：选择裁剪类型，先选择目标图形再绘制裁剪区域

### 颜色与填充

- 可选择任意颜色作为线条色或填充色
- 点击图形内部可进行区域填充

---

## 文件结构
- **PaintWidget.cpp**：核心绘图逻辑实现。
- **PaintWidget.h**：`PaintWidget` 类的声明。
- **main.cpp**：程序入口。

---

## 项目待改进之处
- **中文乱码问题** 在某些情况下，程序可能会出现中文显示乱码的情况，后续需要做出调整。
- **按钮排列问题** 界面的按钮排列需要美化，需要重新设计更合理的按钮布局。

## 贡献
- 如果您对该项目有任何建议或改进，欢迎提交 Issue 或 Pull Request！



