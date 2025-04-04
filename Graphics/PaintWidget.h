#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPointF>
#include <QMessagebox>
#include <QColorDialog>
#include <cmath>
#include "Assistance.h"
#include <QInputDialog> // 引入输入对话框
#include <stack>
#include <string>
#include <sstream>
// 绘图类声明

class PaintWidget : public QFrame {
	Q_OBJECT

signals:
	void UpdateInfo(QString info);
public:
	explicit PaintWidget(QWidget* parent = nullptr);

	// 设置绘图形状
	void setShape(const QString& shape);

	void message(string mes);
	// 设置线宽
	void setLineWidth(int width);

	// 设置算法
	void setAlgorithm(const QString& algorithm);

	void setClipAlgorithm(const QString& algorithm);

	// 设置线型
	void setLinePattern(const QString& pattern);  // 修改为使用LinePattern类

	void undo();

	void clear();

	void filling(bool checked);

	void multiSelect(bool checked);

	void BeizerAdjust(bool checked);

	void spincenter(bool checked);

	void selectColor();

	void Initialize();

	void setStatus(OperationType type);//管理更新shapes，pixmapStack，shapesStack

	void saveOriginalState(shape* item);

	void updateTransitionOriginalState(Point startPos, Point endPos, shape* item);

	void updateZoomOriginalState(Point startPos, Point endPos, shape* item);

	void updateSpinOriginalState(Point startPos, Point endPos, shape* item);

	void repaint();

	shape* locateFigure(Point click);

	int locateBezierPoint(Point click);

protected:
	// 绘制事件
	void paintEvent(QPaintEvent* event) override;

	// 鼠标按下事件
	void mousePressEvent(QMouseEvent* event) override;

	// 鼠标移动事件
	void mouseMoveEvent(QMouseEvent* event) override;

	// 鼠标释放事件
	void mouseReleaseEvent(QMouseEvent* event) override;

	//撤销绘制

private:

	void drawBSplineCurve(const std::vector<QPoint>& controlPoints, QPainter& painter, int k);

	QPoint calculateBSplinePoint(const vector<QPoint>& controlPoints, double t, const QVector<double>& knots, int k);

	double bsplineBasis(int i, int k, double t, const QVector<double>& knots);

	QVector<double> generateKnots(int n, int k);

	std::vector<Point> clipPolygon(const std::vector<Point>& subjectPolygon, const std::vector<Point>& clipPolygon);
	// 使用正负法绘制圆弧
	void drawArcBySignMethod(const Point& start, const Point& end, int radius, QPainter& painter);

	void drawInscribedPolygon(const Point& center, double radius, int sides, QPainter& painter);

	// Bresenham 直线算法
	void bresenhamLine(const Point& start, const Point& end, QPainter& painter);

	// 修复后的中点直线算法
	void midpointLine(const Point& start, const Point& end, QPainter& painter);

	// 中点圆算法
	void midPointCircle(const Point& center, int r, QPainter& painter);

	Point rotatePoint(Point& p, const Point& center, double angle);

	void rotateShape(shape& shape, double angle);

	void cohenSutherlandClip(const QRectF& clipRect);

	void filler(const std::vector<Point>& vertices, QPainter& painter);

	void midpointClip(const QRectF& clipRect);

	void floodFill(int x, int y, const QColor& borderColor, const QColor& newColor);

	void drawBezierCurve(const vector<QPoint>& controlPoints, QPainter& painter);

	QPoint bezierPoint(const std::vector<QPoint>& points, qreal t, int degree);

	// 成员变量

	vector<shape> shapes;//储存当前所有图形信息
	stack<QPixmap> pixmapStack;//储存当前画板状态（用于减少重绘次数以减少开销）
	stack<std::vector<shape>> shapesStack;//在涉及复杂操作（清屏，截取操作）用于撤销的锚点。
	stack<OperationType> OperateStack;//记录操作是否为清屏/裁剪
	vector<vector<shape*>> jointShapes;
	Point startPos;  // 起点
	Point endPos;    // 终点
	Point arcStartPos;  // 圆弧起点
	Point arcEndPos;    // 圆弧中点
	Point polygonCenter;
	Point spinCenter;
	Point arcCenterPos; // 圆心
	int clickCount = 0;  // 点击次数
	int selectedIndex = -1;
	QPixmap pixmap;      // 保存绘图内容
	QPixmap tempPixmap;  // 临时map用于绘制实时预览
	QString currentShape; // 当前形状
	QString currentAlgorithm; // 当前算法
	QString currentClipAlgorithm; // 当前算法
	QColor filledColor; // 填充的颜色
	QColor currentPenColor; // 当前笔的颜色
	int lineWidth;       // 线宽
	LinePattern currentLinePattern; // 当前线型模式
	shape* selectedItem = NULL;
	vector<shape*> selectedItems;
	bool drawingClipPoly = false;
	bool isEditing = false;
	bool drawingClipRect = false;
	bool drawing = false; // 是否正在绘制
	bool usespincenter = false;
	bool startClip = false;
	bool multiSelectMode = false;
	bool BeizerAdjustMode = false;
	bool isfilling = false;
	bool isDrawingBspline = false;
	bool isDrawingPolygon = false; //是否正在绘制任意多边形
	bool polygonCompleted = false; //任意多边形是否完成
	bool colorSelected = false;	// 是否填充颜色中
	vector<Point>clipPoints;
	vector<Point> polygonPoints; //存储任意多边形的顶点
	vector<QPoint> controlPoints;
	bool isDrawingBezier = false;
};

#endif // PAINTWIDGET_H