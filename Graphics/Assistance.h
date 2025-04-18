#ifndef ASSISTANCE_H
#define ASSISTANCE_H

//Point�࣬�Լ�����ʵ��

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
	// ���캯��������һ��ģʽ�ַ���
	LinePattern() {}
	LinePattern(const string& pattern) : pattern(pattern) {}
	LinePattern(const LinePattern& x) {
		pattern = x.getPattern();
	}
	// ��ȡ����ģʽ
	string getPattern() const {
		return pattern;
	}

protected:
	string pattern;  // ����ģʽ��ʹ���ַ�����ʾ
};

Point computeIntersection(const Point& p1, const Point& p2, const Point& edgeStart, const Point& edgeEnd);
bool inside(const Point& p, const Point& edgeStart, const Point& edgeEnd);
// ����㵽�߶εľ���

struct shape {
	enum Type { Line, Circle, Arc, Polygon, Bezier, Bspline } type;
	// ������״����Ĳ��������磺
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
	bool matchLine(Point x, Point y);//�б��Ƿ�ƥ��ֱ��
	bool matchLines(Point click);
	bool matchArc(Point click);
	bool matchCircle(Point click);
	bool matchPolygon(Point click);//�б��Ƿ�ƥ������
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