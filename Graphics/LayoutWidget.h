#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include "PaintWidget.h"
#include "Assistance.h"
#include <QCheckBox>
#pragma execution_character_set("utf-8")

//布局类
class LayoutWidget : public QWidget {
	Q_OBJECT

public:
	LayoutWidget(QWidget* parent = nullptr) : QWidget(parent) {
		// 创建绘图小部件
		PaintWidget* paintWidget = new PaintWidget();

		// 创建信息显示标签
		QLabel* infoLabel = new QLabel(QString::fromLocal8Bit("暂无绘制信息"));
		infoLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
		infoLabel->setFixedHeight(30);  // 设置高度

		// 创建形状选择下拉框
		QComboBox* shapeComboBox = new QComboBox();
		shapeComboBox->addItem("Line");
		shapeComboBox->addItem("Circle");
		shapeComboBox->addItem("Arc");
		shapeComboBox->addItem("polygon");
		shapeComboBox->addItem("anyPolygon");
		shapeComboBox->addItem("ClipLine");
		shapeComboBox->addItem("ClipPoly");
		shapeComboBox->addItem("fillColor");
		shapeComboBox->addItem("Bind");
		shapeComboBox->addItem("Bezier");
		shapeComboBox->addItem("Translation");
		shapeComboBox->addItem("Zoom");
		shapeComboBox->addItem("Spin");
		shapeComboBox->addItem("Bspline");
		shapeComboBox->addItem("SelectSpinCenter");

		// 创建算法选择下拉框
		QComboBox* algorithmComboBox = new QComboBox();
		algorithmComboBox->addItem("Bresenham");
		algorithmComboBox->addItem("Midpoint");

		QComboBox* ClipalgorithmComboBox = new QComboBox();
		ClipalgorithmComboBox->addItem("cohenSutherlandClip");
		ClipalgorithmComboBox->addItem("midpointClip");
		// 创建线宽选择滑块
		QSlider* lineWidthSlider = new QSlider(Qt::Horizontal);
		lineWidthSlider->setRange(1, 10);
		lineWidthSlider->setValue(3);
		lineWidthSlider->setFixedWidth(150);

		// 创建线型选择下拉框
		QComboBox* penStyleComboBox = new QComboBox();
		penStyleComboBox->addItem("Solid");
		penStyleComboBox->addItem("Dash");
		penStyleComboBox->addItem("Dot");
		penStyleComboBox->addItem("DashDot");

		// 创建布局
		QVBoxLayout* mainLayout = new QVBoxLayout();
		QHBoxLayout* controlLayout = new QHBoxLayout();

		// 设置下拉框宽度
		ClipalgorithmComboBox->setFixedWidth(100);
		shapeComboBox->setFixedWidth(100);
		algorithmComboBox->setFixedWidth(100);
		penStyleComboBox->setFixedWidth(100);

		// 创建撤销按钮
		QPushButton* undoButton = new QPushButton("Undo");
		QPushButton* clearButton = new QPushButton("Clear");
		//选择填充颜色
		QCheckBox* fillPolygonCheckBox = new QCheckBox("FillPolygon");
		QCheckBox* multiSelectModeBox = new QCheckBox("multiSelectMode");
		QCheckBox* BeizerAdjustModeBox = new QCheckBox("BeizerAdjustMode");
		// 创建填充按钮
		QCheckBox* useSpinCenterCheckBox = new QCheckBox("UseSpinCenter");
		QPushButton* fillColorButton = new QPushButton("SelectColor");

		// 使用单独的布局来控制每个标签和下拉框的距离
		QHBoxLayout* shapeLayout = new QHBoxLayout();
		QLabel* shapeLabel = new QLabel("Shape:");
		shapeLayout->addWidget(shapeLabel);
		shapeLayout->addWidget(shapeComboBox);
		shapeLayout->addItem(new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)); // 设置5像素的水平间距

		QHBoxLayout* algorithmLayout = new QHBoxLayout();
		QLabel* algorithmLabel = new QLabel("Algorithm:");
		algorithmLayout->addWidget(algorithmLabel);
		algorithmLayout->addWidget(algorithmComboBox);
		algorithmLayout->addItem(new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)); // 设置5像素的水平间距

		QHBoxLayout* clipalgorithmLayout = new QHBoxLayout();
		QLabel* clipalgorithmLabel = new QLabel("ClipAlgorithm:");
		clipalgorithmLayout->addWidget(clipalgorithmLabel);
		clipalgorithmLayout->addWidget(ClipalgorithmComboBox);
		clipalgorithmLayout->addItem(new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)); // 设置5像素的水平间距

		QHBoxLayout* lineWidthLayout = new QHBoxLayout();
		QLabel* lineWidthLabel = new QLabel("LineWidth:");
		lineWidthLayout->addWidget(lineWidthLabel);
		lineWidthLayout->addWidget(lineWidthSlider);
		lineWidthLayout->addItem(new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)); // 设置5像素的水平间距

		QHBoxLayout* penStyleLayout = new QHBoxLayout();
		QLabel* penStyleLabel = new QLabel("Pen Style:");
		penStyleLayout->addWidget(penStyleLabel);
		penStyleLayout->addWidget(penStyleComboBox);
		penStyleLayout->addItem(new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)); // 设置5像素的水平间距

		// 将布局添加到主布局中
		controlLayout->addLayout(shapeLayout);
		controlLayout->addLayout(algorithmLayout);
		controlLayout->addLayout(clipalgorithmLayout);
		controlLayout->addWidget(new QLabel("LineWidth:"));
		controlLayout->addWidget(lineWidthSlider);
		controlLayout->addLayout(penStyleLayout);
		controlLayout->addWidget(undoButton);  // 添加撤销按钮
		controlLayout->addWidget(clearButton);
		controlLayout->addWidget(fillPolygonCheckBox);
		controlLayout->addWidget(multiSelectModeBox);
		controlLayout->addWidget(BeizerAdjustModeBox);
		controlLayout->addWidget(useSpinCenterCheckBox);
		controlLayout->addWidget(fillColorButton);

		mainLayout->addLayout(controlLayout);
		mainLayout->addWidget(paintWidget);
		mainLayout->addWidget(infoLabel);  // 添加信息显示区域

		setLayout(mainLayout);

		// 信号与槽连接
		QObject::connect(shapeComboBox, &QComboBox::currentTextChanged, paintWidget, &PaintWidget::setShape);
		QObject::connect(lineWidthSlider, &QSlider::valueChanged, paintWidget, &PaintWidget::setLineWidth);
		QObject::connect(algorithmComboBox, &QComboBox::currentTextChanged, paintWidget, &PaintWidget::setAlgorithm);
		QObject::connect(ClipalgorithmComboBox, &QComboBox::currentTextChanged, paintWidget, &PaintWidget::setClipAlgorithm);
		QObject::connect(penStyleComboBox, &QComboBox::currentTextChanged, paintWidget, &PaintWidget::setLinePattern);
		QObject::connect(undoButton, &QPushButton::clicked, paintWidget, &PaintWidget::undo);  // 连接撤销按钮
		QObject::connect(clearButton, &QPushButton::clicked, paintWidget, &PaintWidget::clear);  // 连接清空按钮
		QObject::connect(fillColorButton, &QPushButton::clicked, paintWidget, &PaintWidget::selectColor);  // 连接选颜色按钮
		QObject::connect(fillPolygonCheckBox, &QCheckBox::toggled, paintWidget, &PaintWidget::filling);
		QObject::connect(multiSelectModeBox, &QCheckBox::toggled, paintWidget, &PaintWidget::multiSelect);
		QObject::connect(BeizerAdjustModeBox, &QCheckBox::toggled, paintWidget, &PaintWidget::BeizerAdjust);

		QObject::connect(useSpinCenterCheckBox, &QCheckBox::toggled, paintWidget, &PaintWidget::spincenter);
		// 可选：更新信息标签
		QObject::connect(paintWidget, &PaintWidget::UpdateInfo, infoLabel, &QLabel::setText);
	}
};