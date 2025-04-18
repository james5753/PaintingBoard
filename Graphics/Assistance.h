#ifndef ASSISTANCE_H
#define ASSISTANCE_H

//Point类，以及线型实现

#include<iostream>
#include<string>
#include<QString>
#include <QPointF>
#include<QColor>
using namespace std;

struct Point {
	int x, y, z;
	Point(int X, int Y, int Z = 0) :x(X), y(Y), z(Z) {}
	Point() :x(0), y(0), z(0) {}
};

class LinePattern {
public:
	// 构造函数，传入一个模式字符串
	LinePattern() {}
	LinePattern(const string& pattern) : pattern(pattern) {}
	LinePattern(const LinePattern& x) {
		pattern = x.getPattern();
	}
	// 获取线型模式
	string getPattern() const {
		return pattern;
	}

protected:
	string pattern;  // 线型模式，使用字符串表示
};

Point computeIntersection(const Point& p1, const Point& p2, const Point& edgeStart, const Point& edgeEnd);
bool inside(const Point& p, const Point& edgeStart, const Point& edgeEnd);
// 计算点到线段的距离

struct shape {
	enum Type { Line, Circle, Arc, Polygon, Bezier, Bspline } type;
	// 定义形状所需的参数，例如：
	Point start, end, center;
	Point originalstart, originalend, originalcenter;
	int radius;
	int originalradius;
	int lineWidth;
	QColor lineColor;
	QString algorithm;
	int degree;
	LinePattern linePattern;
	QString shapeInfo;
	vector<Point> polygonPoints;
	vector<Point> originalpolygonPoints;
	vector<QPoint> controlPoints;
	vector<shape*>* jointShape = NULL;
	bool filling;
	bool matchLine(Point x, Point y);//判别是否匹配直线
	bool matchLines(Point click);
	bool matchArc(Point click);
	bool matchCircle(Point click);
	bool matchPolygon(Point click);//判别是否匹配多边形
	bool matchFigure(Point x);
};

enum class OperationType {
	Draw,
	Clear,
	Clip,
	Move
};

extern LinePattern DotLine, DashLine, SolidLine, DashDotLine;
#endif 