#include "PaintWidget.h"
#include "LayoutWidget.h"
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <cmath>
#include<iostream>

using namespace std;
using namespace Qt;

// 主程序入口
int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	LayoutWidget window;
	window.setWindowTitle("Drawer");
	window.resize(1200, 900);
	window.show();
	return app.exec();
}