#include "Assistance.h"

// 定义并初始化全局变量
LinePattern DotLine("100100");
LinePattern DashDotLine("1111111111100100");
LinePattern DashLine("1111111111100");
LinePattern SolidLine("11111111");

int tolerance = 10;

double LinePointDistance(Point A, Point B, Point P) {
	double normalLength = std::sqrt(std::pow(B.x - A.x, 2) + std::pow(B.y - A.y, 2));
	if (normalLength == 0.0) return std::sqrt(std::pow(P.x - A.x, 2) + std::pow(P.y - A.y, 2)); // A和B重叠的情况
	double r = ((P.x - A.x) * (B.x - A.x) + (P.y - A.y) * (B.y - A.y)) / (normalLength * normalLength);
	if (r < 0.0) return std::sqrt(std::pow(P.x - A.x, 2) + std::pow(P.y - A.y, 2)); // P在A外部
	if (r > 1.0) return std::sqrt(std::pow(P.x - B.x, 2) + std::pow(P.y - B.y, 2)); // P在B外部

	Point projection = { static_cast<int>(A.x + r * (B.x - A.x)), static_cast<int>(A.y + r * (B.y - A.y)) };
	return std::sqrt(std::pow(P.x - projection.x, 2) + std::pow(P.y - projection.y, 2)); // 返回投影的距离
}

// 选择形状
bool shape::matchFigure(Point click) {
	switch (type) {
	case Line:
		return matchLines(click);
	case Circle:
		return matchCircle(click);
	case Arc:
		return matchArc(click);
	case Polygon:
		return matchPolygon(click);
	default:
		return false;
	}
}

Point computeIntersection(const Point& p1, const Point& p2, const Point& edgeStart, const Point& edgeEnd) {
	float a1 = p2.y - p1.y;
	float b1 = p1.x - p2.x;
	float c1 = a1 * p1.x + b1 * p1.y;

	float a2 = edgeEnd.y - edgeStart.y;
	float b2 = edgeStart.x - edgeEnd.x;
	float c2 = a2 * edgeStart.x + b2 * edgeStart.y;

	float determinant = a1 * b2 - a2 * b1;

	if (determinant == 0) {
		// Lines are parallel, return a default point or handle as needed
		return Point(0, 0);
	}
	else {
		float x = (b2 * c1 - b1 * c2) / determinant;
		float y = (a1 * c2 - a2 * c1) / determinant;
		return Point(x, y);
	}
}

bool inside(const Point& p, const Point& edgeStart, const Point& edgeEnd) {
	return (edgeEnd.x - edgeStart.x) * (p.y - edgeStart.y) > (edgeEnd.y - edgeStart.y) * (p.x - edgeStart.x);
}

// 判断点是否在线段上
bool shape::matchLines(Point click) {
	double distance = LinePointDistance(start, end, click);
	return distance <= lineWidth / 2.0 + tolerance; // 考虑线宽
}

// 判断点是否在圆上
bool shape::matchCircle(Point click) {
	double distToCenter = std::sqrt(std::pow(click.x - start.x, 2) + std::pow(click.y - start.y, 2));
	return std::abs(distToCenter - radius) <= lineWidth / 2.0 + tolerance; // 考虑线宽
}

// 判断点是否在多边形的某条边上
bool shape::matchPolygon(Point click) {
	for (size_t i = 0; i < polygonPoints.size(); ++i) {
		Point start = polygonPoints[i];
		Point end = polygonPoints[(i + 1) % polygonPoints.size()]; // 下一个点，形成边
		if (LinePointDistance(start, end, click) <= tolerance) { // 允许一定的容差
			return true; // 判断点是否在边上
		}
	}
	return false;
}

bool shape::matchArc(Point click) {
	// 计算弧的中心点（水平和垂直是计算简化）
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float d = std::sqrt(dx * dx + dy * dy);
	if (d == 0) return false;  // 防止线段长度为零的情况

	// 根据给定的起点和终点，计算圆心
	int a = (end.x + start.x) / 2.0;
	int b = (end.y + start.y) / 2.0;
	float h = std::sqrt(radius * radius - (d / 2.0) * (d / 2.0));
	Point center;
	center.x = static_cast<int>(a + h * (dy) / d);
	center.y = static_cast<int>(b - h * (dx) / d);

	// 计算点击点到圆心的距离
	float distToCenter = std::sqrt(std::pow(click.x - center.x, 2) + std::pow(click.y - center.y, 2));

	// 判断点击点的距离是否与半径相近
	if (std::abs(distToCenter - radius) > lineWidth / 2.0 + tolerance) {
		return false; // 超出允许的容差，不在弧上
	}

	// 计算起始角度和结束角度
	float startAngle = std::atan2(start.y - center.y, start.x - center.x);
	float endAngle = std::atan2(end.y - center.y, end.x - center.x);
	float clickAngle = std::atan2(click.y - center.y, click.x - center.x);

	// 确保角度在0到2*PI之间
	if (startAngle < 0) startAngle += 2 * 3.1415946;
	if (endAngle < 0) endAngle += 2 * 3.1415946;
	if (clickAngle < 0) clickAngle += 2 * 3.1415946;

	// 判断点击的角度是否在起始角度和结束角度之间
	if (startAngle < endAngle) {
		return clickAngle >= startAngle && clickAngle <= endAngle;
	}
	else {
		// 处理角度包围0的情况
		return clickAngle >= startAngle || clickAngle <= endAngle;
	}
}