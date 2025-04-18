#include "PaintWidget.h"
#include <set>

using namespace std;
//PaintWidget绘图类实现

// 构造函数
PaintWidget::PaintWidget(QWidget* parent)
	: QFrame(parent), currentShape("Line"), lineWidth(3), currentAlgorithm("Bresenham"), currentLinePattern(SolidLine) {
	setMouseTracking(true);
	setFrameStyle(QFrame::Sunken | QFrame::Panel);  // 添加边框
	pixmap = QPixmap(2400, 1600);  // 初始化 QPixmap
	pixmap.fill(Qt::white);      // 填充背景为白色
	tempPixmap.fill(Qt::white);  // 填充预览背景为白色
	selectedItem = NULL;
	drawingClipRect = false;
	startClip = false;
}

void PaintWidget::undo() {
	if (!pixmapStack.empty()) {
		pixmap = pixmapStack.top();  // 恢复上一个状态
		pixmapStack.pop();
		OperationType type = OperateStack.top();
		OperateStack.pop();
		if (type != OperationType::Draw)         // 弹出栈顶状态
		{
			shapes = shapesStack.top();
			shapesStack.pop();
		}
		else {
			if (!shapes.empty()) { // 检查 vector 是否为空
				shapes.pop_back(); // 移除最后一个元素
			}
		}
		update();                  // 更新绘图
		if (shapes.empty() != true)
			emit UpdateInfo(shapes.back().shapeInfo);
		else
			message("暂无绘制信息");
	}
}

void PaintWidget::filling(bool checked) {
	isfilling = checked;
}

void PaintWidget::spincenter(bool checked) {
	usespincenter = checked;
}

void PaintWidget::multiSelect(bool checked) {
	multiSelectMode = checked;
}

void PaintWidget::BeizerAdjust(bool checked) {
	BeizerAdjustMode = checked;
	if (currentShape == "Bezier" && isDrawingBezier) {
		QPainter painter(&pixmap);
		if (checked) {
			for (int i = 0; i < controlPoints.size(); i++)
				painter.drawEllipse(controlPoints[i], 4, 4);
			update();
		}
		else {
			pixmap = pixmapStack.top();
			drawBezierCurve(controlPoints, painter);
			update();
		}
	}
}

void PaintWidget::setStatus(OperationType type) {
	pixmapStack.push(pixmap);
	OperateStack.push(type);
	if (type != OperationType::Draw)
	{
		shapesStack.push(shapes);
		if (type == OperationType::Clear)
			shapes.clear();
	}
}

void PaintWidget::repaint() {
	// 创建一个QPainter对象，绘制到pixmap中
	QPainter painter(&pixmap);
	pixmap.fill(Qt::white);  // 重新填充背景为白色

	// 遍历所有形状并重新绘制
	for (const auto& shape : shapes) {
		QPen pen;
		pen.setWidth(shape.lineWidth);
		pen.setColor(shape.lineColor);
		painter.setPen(pen);
		currentLinePattern = shape.linePattern;
		switch (shape.type) {
		case shape::Type::Line:
			if (shape.algorithm == "Bresenham") {
				bresenhamLine(shape.start, shape.end, painter);
			}
			else if (shape.algorithm == "Midpoint") {
				midpointLine(shape.start, shape.end, painter);
			}
			break;
		case shape::Type::Circle:
			midPointCircle(shape.center, shape.radius, painter);
			break;
		case shape::Type::Arc:
			drawArcBySignMethod(shape.start, shape.end, shape.radius, painter);
			break;
		case shape::Type::Polygon:
			for (size_t i = 0; i < shape.polygonPoints.size(); ++i) {
				bresenhamLine(shape.polygonPoints[i], shape.polygonPoints[(i + 1) % shape.polygonPoints.size()], painter);
			}
			if (shape.filling) {
				filler(shape.polygonPoints, painter);
			}
			break;
		case shape::Type::Bezier:
			drawBezierCurve(shape.controlPoints, painter);
			break;
		default:
			break;
		}
	}

	// 更新显示
	update();
}

void PaintWidget::clear() {
	setStatus(OperationType::Clear);
	pixmap = QPixmap(2400, 1600);  // 初始化 QPixmap
	pixmap.fill(Qt::white);      // 填充背景为白色
	update();
	message("暂无绘制信息");
}

void PaintWidget::selectColor() {
	// 弹出颜色选择对话框
	QColor selectedColor = QColorDialog::getColor(Qt::white, this, tr("Select Fill Color"));

	if (selectedColor.isValid()) {
		filledColor = selectedColor; // 更新填充颜色
		currentPenColor = selectedColor;
		colorSelected = true;
		QString colorName = selectedColor.name(); // 获取颜色的名称
		string color = "当前颜色：";
		color.append(colorName.toStdString());
		message(color); // 显示颜色名称
	}
	else {
		message("没有选择颜色");
	}
}

shape* PaintWidget::locateFigure(Point click)
{
	for (int i = 0; i < shapes.size(); i++) {
		if (shapes[i].matchFigure(click))
		{
			shape* res = &shapes[i];
			drawingClipRect = true;
			emit UpdateInfo(QString::fromLocal8Bit("已选中: ") + res->shapeInfo);
			drawing = true;
			return res;
		}
	}
	return NULL;
}

int PaintWidget::locateBezierPoint(Point click)
{
	for (int i = 0; i < controlPoints.size(); i++)
		if (QLineF(QPoint(click.x, click.y), controlPoints[i]).length() < 5.0)
			return i;
	return -1;
}

void PaintWidget::Initialize() {
	clickCount = 0;  // 点击次数
	selectedItem = NULL;
	drawingClipRect = false;
	drawing = false; // 是否正在绘制
	startClip = false;
	isDrawingPolygon = false; //是否正在绘制任意多边形
	isEditing = false;
	polygonCompleted = false; //任意多边形是否完成
	colorSelected = false;	// 是否填充颜色中
	isDrawingBezier = false;
	selectedItems.clear();
	polygonPoints.clear(); //存储任意多边形的顶点
	controlPoints.clear();
}

void PaintWidget::message(const string mes) {
	QString info = QString::fromLocal8Bit(mes);
	emit UpdateInfo(info);
}

// 设置绘图形状
void PaintWidget::setShape(const QString& shape) {
	currentShape = shape;
	Initialize();
	if (shape == "fillColor") {
		if (!colorSelected) {
			message("请选择填充颜色");
		}
		//else {
		//	message("请选择要填充区域内的点");
		//}
	}
	else if (shape == "ClipLine") {
		message("Choose Line");
	}
	else if (shape == "Choose") {
		message("Choose Figure");
	}
	else if (shape == "SelectSpinCenter") {
		message("Choose SpinCenter");
	}
}

void PaintWidget::saveOriginalState(shape* item) {
	if (item) {
		ostringstream infoStream;  // 使用 std 命名空间
		if (item->type == shape::Line) {
			item->center.x = (item->start.x + item->end.x) / 2;
			item->center.y = (item->start.y + item->end.y) / 2;
			item->originalcenter = item->center;
			item->originalstart = item->start;
			item->originalend = item->end;
			infoStream << "直线段 (" << item->start.x << ", " << item->start.y
				<< ")->(" << item->end.x << ", " << item->end.y
				<< ")";
			item->shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
		}
		else if (item->type == shape::Arc) {
			item->originalcenter = item->center;
			item->originalradius = item->radius;
			item->originalstart = item->start;
			item->originalend = item->end;
			infoStream << "圆弧，起点：(" << item->start.x << ", " << item->start.y
				<< "), 终点：(" << item->end.x << ", " << item->end.y
				<< "), 半径：" << item->radius;
			item->shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
		}
		else if (item->type == shape::Circle) {
			item->originalcenter = item->center;
			item->originalradius = item->radius;
			infoStream << "圆，圆心：(" << item->center.x << ", " << item->center.y
				<< "), 半径：" << item->radius;
			item->shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
		}
		else if (item->type == shape::Polygon) {
			item->center.x = 0;
			item->center.y = 0;
			for (int i = 0; i < item->polygonPoints.size(); i++) {
				item->center.x += item->polygonPoints[i].x;
				item->center.y += item->polygonPoints[i].y;
			}
			item->center.x /= item->polygonPoints.size();
			item->center.y /= item->polygonPoints.size();
			item->originalcenter = item->center;
			item->originalpolygonPoints = item->polygonPoints;
			infoStream << item->polygonPoints.size() << "边形，顶点：";
			for (int i = 0; i < item->polygonPoints.size(); i++)
				infoStream << "(" << item->polygonPoints[i].x << "," << item->polygonPoints[i].y << ") ";
			item->shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
		}
	}
}

void PaintWidget::updateTransitionOriginalState(Point startPos, Point endPos, shape* selectedItem) {
	int dx = endPos.x - startPos.x;
	int dy = endPos.y - startPos.y;

	// 将形状移动 dx 和 dy
	if (selectedItem) {
		if (selectedItem->type == shape::Polygon) {
			for (auto& point : selectedItem->polygonPoints) {
				point.x += dx;
				point.y += dy;
			}
			selectedItem->center.x += dx;
			selectedItem->center.y += dy;
		}
		else if (selectedItem->type == shape::Line) {
			selectedItem->start.x += dx;
			selectedItem->start.y += dy;
			selectedItem->end.x += dx;
			selectedItem->end.y += dy;
			selectedItem->center.x += dx;
			selectedItem->center.y += dy;
		}
		else if (selectedItem->type == shape::Circle) {
			selectedItem->center.x += dx;
			selectedItem->center.y += dy;
			selectedItem->start.x += dx;
			selectedItem->start.y += dy;
		}
		else if (selectedItem->type == shape::Arc) {
			selectedItem->start.x += dx;
			selectedItem->start.y += dy;
			selectedItem->end.x += dx;
			selectedItem->end.y += dy;
			selectedItem->center.x += dx;
			selectedItem->center.y += dy;
		}
	}
}

void PaintWidget::updateZoomOriginalState(Point startPos, Point endPos, shape* selectedItem) {
	Point center = usespincenter ? spinCenter : selectedItem->originalcenter;
	double startDistance = std::sqrt(pow(startPos.x - center.x, 2) + pow(startPos.y - center.y, 2));
	double endDistance = std::sqrt(pow(endPos.x - center.x, 2) + pow(endPos.y - center.y, 2));
	double scaleFactor = (endDistance / startDistance);  // 根据鼠标移动的距离变化计算比例

	// 基于原始数据进行变换
	if (selectedItem->type == shape::Polygon) {
		for (int i = 0; i < selectedItem->polygonPoints.size(); i++) {
			selectedItem->polygonPoints[i].x = center.x +
				(selectedItem->originalpolygonPoints[i].x - center.x) * scaleFactor;
			selectedItem->polygonPoints[i].y = center.y +
				(selectedItem->originalpolygonPoints[i].y - center.y) * scaleFactor;
		}
	}
	else if (selectedItem->type == shape::Line) {
		selectedItem->start.x = center.x +
			(selectedItem->originalstart.x - center.x) * scaleFactor;
		selectedItem->start.y = center.y +
			(selectedItem->originalstart.y - center.y) * scaleFactor;
		selectedItem->end.x = center.x +
			(selectedItem->originalend.x - center.x) * scaleFactor;
		selectedItem->end.y = center.y +
			(selectedItem->originalend.y - center.y) * scaleFactor;
	}
	else if (selectedItem->type == shape::Circle) {
		selectedItem->radius = selectedItem->originalradius * scaleFactor;
	}
	else if (selectedItem->type == shape::Arc) {
		selectedItem->radius = selectedItem->originalradius * scaleFactor;
		selectedItem->start.x = center.x +
			(selectedItem->originalstart.x - center.x) * scaleFactor;
		selectedItem->start.y = center.y +
			(selectedItem->originalstart.y - center.y) * scaleFactor;
		selectedItem->end.x = center.x +
			(selectedItem->originalend.x - center.x) * scaleFactor;
		selectedItem->end.y = center.y +
			(selectedItem->originalend.y - center.y) * scaleFactor;
	}
}

void PaintWidget::updateSpinOriginalState(Point startPos, Point endPos, shape* selectedItem) {
	Point center = usespincenter ? spinCenter : selectedItem->center;
	double startAngle = std::atan2(startPos.y - center.y, startPos.x - center.x);
	double endAngle = std::atan2(endPos.y - center.y, endPos.x - center.x);
	double angle = endAngle - startAngle;
	rotateShape(*selectedItem, angle); // 旋转选中的图形
}
// 设置线宽
void PaintWidget::setLineWidth(int width) {
	lineWidth = width;
}

// 设置算法
void PaintWidget::setAlgorithm(const QString& algorithm) {
	currentAlgorithm = algorithm;
}

void PaintWidget::setClipAlgorithm(const QString& algorithm) {
	currentClipAlgorithm = algorithm;
}

// 设置线型模式
void PaintWidget::setLinePattern(const QString& pattern) {
	if (pattern == "Dash")
		currentLinePattern = DashLine;
	else if (pattern == "Dot")
		currentLinePattern = DotLine;
	else if (pattern == "Solid")
		currentLinePattern = SolidLine;
	else if (pattern == "DashDot")
		currentLinePattern = DashDotLine;
	update();  // 更新绘图
}

// 绘制事件
void PaintWidget::paintEvent(QPaintEvent* event) {
	QPainter painter(this);

	if (!tempPixmap.isNull()) {
		painter.drawPixmap(0, 0, tempPixmap);  // 绘制临时预览
	}

	painter.drawPixmap(0, 0, pixmap);  // 先绘制之前的内容

	if (drawing) {
		// 设置线宽和线型
		QPen pen(currentPenColor);
		pen.setWidth(lineWidth);
		//pen.setStyle(Qt::CustomDashLine);  // 使用自定义线型

		painter.setPen(pen);  // 应用笔
		if (currentShape == "Line") {
			if (currentAlgorithm == "Bresenham") {
				bresenhamLine(startPos, endPos, painter);
			}
			else if (currentAlgorithm == "Midpoint") {
				midpointLine(startPos, endPos, painter);
			}
		}
		else if (currentShape == "Circle") {
			int radius = std::sqrt(pow(endPos.x - startPos.x, 2) + pow(endPos.y - startPos.y, 2));
			midPointCircle(startPos, radius, painter);
		}
		else if (currentShape == "ClipLine" && startClip) {
			painter.setPen(QPen(Qt::DashLine));
			qreal x = std::min(startPos.x, endPos.x);
			qreal y = std::min(startPos.y, endPos.y);
			qreal width = std::abs(endPos.x - startPos.x);
			qreal height = std::abs(endPos.y - startPos.y);
			QRectF clipRect(x, y, width, height);
			painter.drawRect(clipRect);
		}
		else if (currentShape == "Bezier" && selectedIndex != -1 && BeizerAdjustMode) {
			pixmap = pixmapStack.top();
			vector<QPoint> temp = controlPoints;
			temp[selectedIndex] = QPoint(endPos.x, endPos.y);
			drawBezierCurve(temp, painter);
		}
	}
}

void PaintWidget::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		if (currentShape == "Translation" || currentShape == "Zoom" || currentShape == "Spin") {
			if (!multiSelectMode) {
				startPos = Point(event->pos().x(), event->pos().y());
				selectedItem = locateFigure(startPos);
				if (selectedItem != NULL) {
					drawing = true;
					setStatus(OperationType::Move);
					if (selectedItem->jointShape == NULL)
						saveOriginalState(selectedItem);
					else {
						for (int i = 0; i < selectedItem->jointShape->size(); i++)
							saveOriginalState((*(selectedItem->jointShape))[i]);
					}
				}
			}
			else {
				startPos = Point(event->pos().x(), event->pos().y());
				if (!isEditing) {
					selectedItem = locateFigure(startPos);
					if (selectedItem != NULL) {
						auto it = std::find(selectedItems.begin(), selectedItems.end(), selectedItem);
						if (it == selectedItems.end()) {
							selectedItems.push_back(selectedItem);
							saveOriginalState(selectedItem);
						}
						else
							selectedItems.erase(it);
						QString shapesInfo = "Right Click to Edit,Already Chosen:";
						for (const auto& item : selectedItems)
							shapesInfo += item->shapeInfo + " ";
						emit UpdateInfo(shapesInfo);
					}
				}
			}
		}
		else if (currentShape == "Bezier" && BeizerAdjustMode) {
			selectedIndex = locateBezierPoint(Point(event->pos().x(), event->pos().y()));
			QPainter painter(&pixmap);
			/*if (selectedIndex != -1) {
				painter.save(); painter.setPen(Qt::white); painter.setBrush(Qt::white); painter.drawEllipse(controlPoints[selectedIndex], 2, 2); painter.restore();
			}*/
		}
		else if (currentShape == "Bind") {
			startPos = Point(event->pos().x(), event->pos().y());
			selectedItem = locateFigure(startPos);
			if (selectedItem != NULL) {
				auto it = std::find(selectedItems.begin(), selectedItems.end(), selectedItem);
				if (it == selectedItems.end()) {
					selectedItems.push_back(selectedItem);
				}
				else
					selectedItems.erase(it);
				QString shapesInfo = "Right Click to Bind,Already Chosen:";
				for (const auto& item : selectedItems)
					shapesInfo += item->shapeInfo + " ";
				emit UpdateInfo(shapesInfo);
			}
		}
		else if (currentShape != "Arc" && currentShape != "polygon" && currentShape != "ClipLine" && currentShape != "ClipPoly") {
			startPos = Point(event->pos().x(), event->pos().y());
			drawing = true;
		}
		else if (currentShape == "ClipLine") {
			startPos = Point(event->pos().x(), event->pos().y());
			if (!drawingClipRect) {
				selectedItem = locateFigure(startPos);
				if (selectedItem != NULL) {
					if (selectedItem->type == shape::Type::Line) {
						drawingClipRect = true;
						message("Choose ClipRect");
					}
				}
				else
					message("Choose Line to Clip");
			}
			else {
				drawing = true;
				startClip = true;
			}
		}
		else if (currentShape == "ClipPoly") {
			Point Pos = Point(event->pos().x(), event->pos().y());
			if (selectedItem) {
				drawingClipPoly = true;
			}
			if (!drawingClipPoly) {
				selectedItem = locateFigure(Pos);
				if (selectedItem != NULL && selectedItem->type == shape::Type::Polygon)
					message("paint clip polygon");
			}
		}
	}
}

// 鼠标移动事件
void PaintWidget::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton && drawing) {
		endPos = Point(event->pos().x(), event->pos().y());

		// 创建一个临时的QPainter对象用于绘制实时预览
		QPainter painter(&tempPixmap);
		QPen pen(currentPenColor);
		pen.setWidth(lineWidth);
		painter.setPen(pen);
		if ((currentShape == "Translation" || currentShape == "Zoom" || currentShape == "Spin")) {
			if (currentShape == "Translation") {
				// 计算位移
				if (!multiSelectMode) {
					if (selectedItem != NULL) {
						if (selectedItem->jointShape == NULL)
							updateTransitionOriginalState(startPos, endPos, selectedItem);
						else {
							for (int i = 0; i < selectedItem->jointShape->size(); i++)
								updateTransitionOriginalState(startPos, endPos, (*(selectedItem->jointShape))[i]);
						}
					}
				}
				else if (isEditing) {
					for (int i = 0; i < selectedItems.size(); i++)
						updateTransitionOriginalState(startPos, endPos, selectedItems[i]);
				}
				startPos = endPos;
			}
			if (currentShape == "Zoom") {
				if (!multiSelectMode) {
					if (selectedItem != NULL) {
						if (selectedItem->jointShape == NULL)
							updateZoomOriginalState(startPos, endPos, selectedItem);
						else {
							for (int i = 0; i < selectedItem->jointShape->size(); i++)
								updateZoomOriginalState(startPos, endPos, (*(selectedItem->jointShape))[i]);
						}
					}
				}
				else if (isEditing) {
					for (int i = 0; i < selectedItems.size(); i++)
						updateZoomOriginalState(startPos, endPos, selectedItems[i]);
				}
			}
			if (currentShape == "Spin") {
				if (!multiSelectMode) {
					if (selectedItem != NULL) {
						if (selectedItem->jointShape == NULL)
							updateSpinOriginalState(startPos, endPos, selectedItem);
						else {
							for (int i = 0; i < selectedItem->jointShape->size(); i++)
								updateSpinOriginalState(startPos, endPos, (*(selectedItem->jointShape))[i]);
						}
					}
				}
				else if (isEditing) {
					for (int i = 0; i < selectedItems.size(); i++)
						updateSpinOriginalState(startPos, endPos, selectedItems[i]);
				}
			}
			repaint();
		}
		if (currentShape == "Line") {
			if (currentAlgorithm == "Bresenham") {
				bresenhamLine(startPos, endPos, painter);
			}
			else if (currentAlgorithm == "Midpoint") {
				midpointLine(startPos, endPos, painter);
			}
		}
		else if (currentShape == "Circle") {
			int radius = std::sqrt(pow(endPos.x - startPos.x, 2) + pow(endPos.y - startPos.y, 2));
			midPointCircle(startPos, radius, painter);
		}
		else if (currentShape == "ClipLine" && drawingClipRect) {
			painter.setPen(QPen(Qt::DashLine));
			qreal x = std::min(startPos.x, endPos.x);
			qreal y = std::min(startPos.y, endPos.y);
			qreal width = std::abs(endPos.x - startPos.x);
			qreal height = std::abs(endPos.y - startPos.y);
			QRectF clipRect(x, y, width, height);
			painter.drawRect(clipRect);
		}
		else if (currentShape == "Bezier" && selectedIndex != -1 && BeizerAdjustMode) {
			tempPixmap = pixmapStack.top();
			vector<QPoint> temp = controlPoints;
			temp[selectedIndex] = QPoint(endPos.x, endPos.y);
			drawBezierCurve(temp, painter);
		}

		// 更新显示
		update();
	}
}

// 鼠标释放事件
void PaintWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton && currentShape != "anyPolygon" && currentShape != "Bezier" && currentShape != "Bspline" && currentShape != "ClipPoly") {
		if (currentShape == "Translation" || currentShape == "Zoom" || currentShape == "Spin") {
			// 在操作结束时，保存最终状态到原始数据
			setStatus(OperationType::Move);
			if (!multiSelectMode) {
				if (selectedItem != NULL) {
					if (selectedItem->jointShape == NULL)
						saveOriginalState(selectedItem);
					else {
						for (int i = 0; i < selectedItem->jointShape->size(); i++)
							saveOriginalState((*(selectedItem->jointShape))[i]);
					}
				}
			}
			else if (isEditing) {
				for (int i = 0; i < selectedItems.size(); i++)
					saveOriginalState(selectedItems[i]);
			}
		}
		else if (currentShape == "Arc") {
			// 处理圆弧的点击逻辑
			if (clickCount == 0) {
				arcStartPos = Point(event->pos().x(), event->pos().y());  // 第一次点击，获取圆弧起点
				clickCount++;
			}
			else if (clickCount == 1) {
				setStatus(OperationType::Draw);
				arcEndPos = Point(event->pos().x(), event->pos().y());  // 第二次点击，获取圆心
				clickCount++;

				// 弹出输入框，获取圆弧半径
				bool ok;
				double min = sqrt(pow(arcStartPos.x - arcEndPos.x, 2) + pow(arcStartPos.y - arcEndPos.y, 2)) / 2 + 1;
				double radius = QInputDialog::getDouble(this, tr("Radius Input"), tr("Radius:"), min, min, 1000, 1, &ok);
				if (!ok) {
					clickCount = 0;
					return;  // 用户取消输入
				}

				// 绘制圆弧，使用第一和第二次点击的点
				QPainter painter(&pixmap);  // 绘制到 pixmap 中保存
				QPen pen(currentPenColor);
				pen.setWidth(lineWidth);  // 设置线宽
				painter.setPen(pen);  // 确保绘图使用了设置的pen
				float dx = arcEndPos.x - arcStartPos.x;
				float dy = arcEndPos.y - arcStartPos.y;
				float d = std::sqrt(dx * dx + dy * dy);
				if (d == 0) return;  // Prevent division by zero
				Point center;
				int a = (arcEndPos.x + arcStartPos.x) / 2.0;
				int b = (arcEndPos.y + arcStartPos.y) / 2.0;
				float h = std::sqrt(radius * radius - (d / 2) * (d / 2));
				center.x = (int)(a + h * (dy) / d);
				center.y = (int)(b - h * (dx) / d);
				shape arc;
				arc.type = shape::Type::Arc;
				arc.start = arcStartPos;
				arc.end = arcEndPos;
				arc.center = center;
				arc.radius = radius;
				arc.lineColor = currentPenColor;
				arc.linePattern = currentLinePattern;
				arc.lineWidth = lineWidth;
				ostringstream infoStream;  // 使用 std 命名空间
				infoStream << "圆弧，起点：(" << arcStartPos.x << ", " << arcStartPos.y
					<< "), 终点：(" << arcEndPos.x << ", " << arcEndPos.y
					<< "), 半径：" << radius;
				arc.shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
				shapes.push_back(arc);
				drawArcBySignMethod(arcStartPos, arcEndPos, radius, painter);

				clickCount = 0;  // 重置点击计数
				update();  // 重新绘制
			}
		}
		else if (currentShape == "polygon") {
			if (clickCount == 0) {
				shape polygon;
				polygon.type = shape::Type::Polygon;
				polygon.polygonPoints = polygonPoints;
				polygon.lineWidth = lineWidth;
				polygon.linePattern = currentLinePattern;
				polygonCenter = Point(event->pos().x(), event->pos().y());
				clickCount++;
			}
			else {
				setStatus(OperationType::Draw);
				clickCount = 0;
				double radius = std::sqrt(pow(event->pos().x() - polygonCenter.x, 2) + pow(event->pos().y() - polygonCenter.y, 2));
				bool ok;
				int sides = QInputDialog::getInt(this, tr("Input Sides"), tr("Number of sides:"), 3, 3, 100, 1, &ok);
				QPainter painter(&pixmap);
				QPen pen(currentPenColor);
				pen.setWidth(lineWidth);
				painter.setPen(pen);
				drawInscribedPolygon(polygonCenter, radius, sides, painter);
			}
		}
		else if (currentShape == "ClipLine" && startClip) {
			setStatus(OperationType::Clip);
			qreal  x = startPos.x;
			qreal  y = startPos.y;
			qreal  width = endPos.x - startPos.x;
			qreal  height = endPos.y - startPos.y;

			QRectF clipRect(x, y, width, height);
			// 裁剪函数调用
			if (currentClipAlgorithm == "cohenSutherlandClip")
				cohenSutherlandClip(clipRect);
			else
				midpointClip(clipRect);
			ostringstream infoStream;
			infoStream << "直线段 (" << selectedItem->start.x << ", " << selectedItem->start.y
				<< ")->(" << selectedItem->end.x << ", " << selectedItem->end.y
				<< ")";
			selectedItem->shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
			repaint();
			drawingClipRect = false;
			startClip = false;
			drawing = false;
			selectedItem = NULL;
			update();  // 更新绘图
			message("Choose Line to Clip");
		}
		else if (currentShape == "fillColor") {
			drawing = true;
			QPoint point = event->pos();
			QColor borderColor = pixmap.toImage().pixel(point);
			QColor newColor = filledColor; // 使用之前选择的颜色
			setStatus(OperationType::Draw);
			if (borderColor != newColor)
				floodFill(point.x(), point.y(), borderColor, newColor);
		}
		else if (currentShape == "SelectSpinCenter") {
			spinCenter = Point(event->pos().x(), event->pos().y());
			ostringstream infoStream;  // 使用 std 命名空间
			infoStream << "旋转点:(" << spinCenter.x << ", " << spinCenter.y << ")";
			message(infoStream.str());
		}
		else {
			endPos = Point(event->pos().x(), event->pos().y());
			drawing = false;
			QPainter painter(&pixmap);
			QPen pen(currentPenColor);
			pen.setWidth(lineWidth);
			painter.setPen(pen);
			if (currentShape == "Line") {
				setStatus(OperationType::Draw);
				shape line;
				line.start = startPos;
				line.end = endPos;
				line.center = Point((startPos.x + endPos.x) / 2, (startPos.y + endPos.y) / 2);
				line.algorithm = currentAlgorithm;
				line.type = shape::Type::Line;
				line.lineWidth = lineWidth;
				line.linePattern = currentLinePattern;
				line.lineColor = currentPenColor;
				ostringstream infoStream;  // 使用 std 命名空间
				infoStream << "直线段 (" << startPos.x << ", " << startPos.y
					<< ")->(" << endPos.x << ", " << endPos.y
					<< ")";
				line.shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
				shapes.push_back(line);
				if (currentAlgorithm == "Bresenham") {
					bresenhamLine(startPos, endPos, painter);
				}
				else if (currentAlgorithm == "Midpoint") {
					midpointLine(startPos, endPos, painter);
				}
			}
			else if (currentShape == "Circle") {
				setStatus(OperationType::Draw);
				int radius = std::sqrt(pow(endPos.x - startPos.x, 2) + pow(endPos.y - startPos.y, 2));
				ostringstream infoStream;  // 使用 std 命名空间
				shape circle;
				circle.type = shape::Type::Circle;
				circle.start = startPos;
				circle.center = startPos;
				circle.lineWidth = lineWidth;
				circle.linePattern = currentLinePattern;
				circle.radius = radius;
				circle.lineColor = currentPenColor;
				infoStream << "圆，圆心：(" << startPos.x << ", " << startPos.y
					<< "), 半径：" << radius;
				circle.shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
				shapes.push_back(circle);
				midPointCircle(startPos, radius, painter);
			}
			tempPixmap = QPixmap();  // 清空临时预览
			update();
		}
	}
	else if (currentShape == "anyPolygon") {
		if (event->button() == Qt::LeftButton) {
			Point clickedPoint(event->pos().x(), event->pos().y());

			if (!isDrawingPolygon) {
				// 第一次点击，开始绘制多边形
				setStatus(OperationType::Draw);
				isDrawingPolygon = true;
				polygonPoints.clear();  // 清除之前的点
				polygonPoints.push_back(clickedPoint);  // 存储起始点
			}
			else {
				// 每次点击新的点，绘制新线段并更新连接关系
				QPainter painter(&pixmap);
				QPen pen(currentPenColor);
				pen.setWidth(lineWidth);
				painter.setPen(pen);

				polygonPoints.push_back(clickedPoint);  // 更新点集

				for (int i = 1; i < polygonPoints.size(); i++) {
					bresenhamLine(polygonPoints[i - 1], polygonPoints[i], painter);
				}
				update();
			}
		}
		else if (event->button() == Qt::RightButton) {
			// 右键点击，结束多边形绘制
			if (isDrawingPolygon && polygonPoints.size() > 2) {
				QPainter painter(&pixmap);
				QPen pen(currentPenColor);
				pen.setWidth(lineWidth);
				painter.setPen(pen);
				Point center;
				center.x = 0;
				center.y = 0;
				for (int i = 0; i < polygonPoints.size(); i++) {
					center.x += polygonPoints[i].x;
					center.y += polygonPoints[i].y;
				}
				center.x /= polygonPoints.size();
				center.y /= polygonPoints.size();
				shape polygon;
				polygon.type = shape::Type::Polygon;
				polygon.polygonPoints = polygonPoints;
				polygon.center = center;
				polygon.filling = isfilling;
				polygon.lineWidth = lineWidth;
				polygon.lineColor = currentPenColor;
				polygon.linePattern = currentLinePattern;
				polygon.filling = isfilling;
				ostringstream infoStream;  // 使用 std 命名空间
				infoStream << polygonPoints.size() << "边形，顶点：";
				for (int i = 0; i < polygonPoints.size(); i++)
					infoStream << "(" << polygonPoints[i].x << "," << polygonPoints[i].y << ") ";
				polygon.shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
				shapes.push_back(polygon);

				// 连接最后一个点和起始点，完成多边形
				bresenhamLine(polygonPoints.back(), polygonPoints.front(), painter);
				if (isfilling) {
					filler(polygonPoints, painter);
				}
				isDrawingPolygon = false;
				polygonCompleted = true;  // 多边形绘制完成
				update();
			}
		}
	}

	else if (currentShape == "Bezier") {
		QPoint clickedPoint(event->pos().x(), event->pos().y());
		if (event->button() == Qt::LeftButton) {
			if (!isDrawingBezier) {
				// 第一次点击，开始绘制
				setStatus(OperationType::Draw);
				isDrawingBezier = true;
				controlPoints.clear();  // 清除之前的点
				controlPoints.push_back(clickedPoint);  // 存储起始点
				if (pixmapStack.empty())
					pixmapStack.push(pixmap);
			}
			else if (isDrawingBezier && controlPoints.size() > 30) {
				drawing = false;
				//isDrawingBezier = false;
				update();
			}
			else {
				if (!BeizerAdjustMode) {
					pixmap = pixmapStack.top();
					QPainter painter(&pixmap);
					QPen pen(currentPenColor);
					pen.setWidth(lineWidth);
					painter.setPen(pen);
					controlPoints.push_back(clickedPoint);
					drawing = true;
					drawBezierCurve(controlPoints, painter);
					update();
				}
				else if (controlPoints.size() > 1) {
					if (selectedIndex != -1) {
						pixmap = pixmapStack.top();
						QPainter painter(&pixmap);
						QPen pen(currentPenColor);
						pen.setWidth(lineWidth);
						controlPoints[selectedIndex] = clickedPoint;
						for (int i = 0; i < controlPoints.size(); i++)
							painter.drawEllipse(controlPoints[i], 4, 4);
						drawBezierCurve(controlPoints, painter);
						update();
					}
				}
			}
		}
		else if (event->button() == Qt::RightButton && !controlPoints.size() == 0) {
			// 确保至少有一个控制点
			drawing = false;
			isDrawingBezier = false;
			pixmap = pixmapStack.top();
			// 开始绘制贝塞尔曲线
			QPainter painter(&pixmap);
			QPen pen(currentPenColor);
			pen.setWidth(lineWidth);
			painter.setPen(pen);

			// 创建一个形状对象来存储贝塞尔曲线的信息
			shape bezierShape;
			bezierShape.type = shape::Type::Bezier; // 假设你有一个贝塞尔曲线的枚举类型
			bezierShape.controlPoints = controlPoints; // 存储控制点
			bezierShape.lineWidth = lineWidth;
			bezierShape.linePattern = currentLinePattern;

			// 绘制贝塞尔曲线
			QPainterPath path;
			path.moveTo(controlPoints[0]);
			const int steps = 20; // 曲线细分的步数
			const qreal stepSize = 1.0 / steps;
			for (int i = 1; i <= steps; ++i) {
				qreal t = stepSize * i;
				QPoint point = bezierPoint(controlPoints, t, controlPoints.size() - 1);
				path.lineTo(point);
			}
			painter.drawPath(path);

			ostringstream infoStream;
			infoStream << controlPoints.size() << " 个控制点的贝塞尔曲线: ";
			for (size_t i = 0; i < min(static_cast<int>(controlPoints.size()), 15); ++i) {
				infoStream << "(" << controlPoints[i].x() << ", " << controlPoints[i].y() << ") ";
			}
			if (static_cast<int>(controlPoints.size()) > 15) infoStream << "...";
			// 将构建的信息字符串设置为形状的信息
			bezierShape.shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
			shapes.push_back(bezierShape);
			update();
		}
	}
	else if (currentShape == "Bspline") {
		QPoint clickedPoint(event->pos().x(), event->pos().y());
		if (event->button() == Qt::LeftButton) {
			if (!isDrawingBspline) {
				// 第一次点击，开始绘制
				setStatus(OperationType::Draw);
				isDrawingBspline = true;
				controlPoints.clear();  // 清除之前的点
				controlPoints.push_back(clickedPoint);  // 存储起始点
				if (pixmapStack.empty())
					pixmapStack.push(pixmap);
			}
			else if (isDrawingBspline && controlPoints.size() > 30) {
				drawing = false;
				update();
			}
			else {
				pixmap = pixmapStack.top();
				QPainter painter(&pixmap);
				QPen pen(currentPenColor);
				pen.setWidth(lineWidth);
				painter.setPen(pen);
				controlPoints.push_back(clickedPoint);
				drawing = true;
				drawBSplineCurve(controlPoints, painter, 3);
				update();
			}
		}
		else if (event->button() == Qt::RightButton && !controlPoints.empty()) {
			// 结束绘制B样条曲线
			drawing = false;
			isDrawingBspline = false;

			// 弹出对话框要求用户输入degree（1~5）
			bool ok;
			int degree = QInputDialog::getInt(this, tr("Degree Input"),
				tr("Degree :"), 3, 1, controlPoints.size() - 1, 1, &ok);
			if (!ok) {
				QMessageBox::warning(this, tr("Input cancelled"), tr("Invalid degree."));
				return;
			}
			pixmap = pixmapStack.top();

			QPainter painter(&pixmap);
			QPen pen(currentPenColor);
			pen.setWidth(lineWidth);
			painter.setPen(pen);

			shape bsplineShape;
			bsplineShape.type = shape::Type::Bspline;
			bsplineShape.controlPoints = controlPoints;
			bsplineShape.lineWidth = lineWidth;
			bsplineShape.linePattern = currentLinePattern;

			drawBSplineCurve(controlPoints, painter, degree);

			update();

			ostringstream infoStream;
			infoStream << controlPoints.size() << " 个控制点的B样条曲线 (degree " << degree << "): ";
			for (size_t i = 0; i < min(static_cast<int>(controlPoints.size()), 15); ++i) {
				infoStream << "(" << controlPoints[i].x() << ", " << controlPoints[i].y() << ") ";
			}
			if (static_cast<int>(controlPoints.size()) > 15) infoStream << "...";
			bsplineShape.shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
			shapes.push_back(bsplineShape);
			update();
			controlPoints.clear();  // 清除之前的点
		}
	}
	else if (event->button() == Qt::RightButton && !selectedItems.size() == 0)
	{
		if (currentShape == "Translation" || currentShape == "Zoom" || currentShape == "Spin") {
			if (!isEditing)
				message("Start Editing(Right Click to Quit)");
			else
				message("Choose Figures to Edit");
			isEditing = !isEditing;
		}
		else if (currentShape == "Bind") {
			jointShapes.push_back(selectedItems);
			for (int i = 0; i < selectedItems.size(); i++) {
				selectedItems[i]->jointShape = &jointShapes.back();
			}
			QString shapesInfo = "Already Bind:";
			for (const auto& item : selectedItems)
				shapesInfo += item->shapeInfo + " ";
			emit UpdateInfo(shapesInfo);
			selectedItems.clear();
			selectedItem = NULL;
		}
	}
	else if (currentShape == "ClipPoly" && drawingClipPoly) {
		QPainter painter(&pixmap);
		QPen pen(currentPenColor);
		pen.setWidth(lineWidth);
		painter.setPen(QPen(Qt::DashLine));
		if (event->button() == Qt::LeftButton) {
			Point clickedPoint(event->pos().x(), event->pos().y());
			clipPoints.push_back(clickedPoint);
			if (clipPoints.size() >= 2) {
				bresenhamLine(clipPoints[clipPoints.size() - 2], clipPoints[clipPoints.size() - 1], painter);
				update();
			}
		}
		else if (event->button() == Qt::RightButton && clipPoints.size() > 2) {
			drawingClipPoly = false;
			vector<Point> result;
			result = clipPolygon(selectedItem->polygonPoints, clipPoints);
			selectedItem->polygonPoints = result;
			selectedItem->originalpolygonPoints = result;
			Point center = Point(0, 0);
			for (int i = 0; i < result.size(); i++) {
				center.x += result[i].x;
				center.y += result[i].y;
			}
			center.x /= result.size();
			center.y /= result.size();
			selectedItem->center = center;
			selectedItem->originalcenter = center;
			ostringstream infoStream;  // 使用 std 命名空间
			infoStream << result.size() << "边形，顶点：";
			for (int i = 0; i < result.size(); i++)
				infoStream << "(" << result[i].x << "," << result[i].y << ") ";
			selectedItem->shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
			clipPoints.clear();
			selectedItem = NULL;
		}
	}
	if (shapes.empty() != true) {
		if (currentShape == "Line" || currentShape == "Circle" || currentShape == "Arc" || currentShape == "polygon" || currentShape == "anyPolygon" || currentShape == "Bezier") {
			emit UpdateInfo(shapes.back().shapeInfo);
		}
	}
	if (currentShape == "ClipPoly" && event->button() == Qt::RightButton) {
		repaint();
	}
}

Point PaintWidget::rotatePoint(Point& p, const Point& center, double angle) {
	// 将角度转换为弧度
	double cosTheta = std::cos(angle);
	double sinTheta = std::sin(angle);

	// 计算相对于中心点的坐标
	int x = p.x - center.x;
	int y = p.y - center.y;

	// 旋转公式
	int newX = center.x + (x * cosTheta - y * sinTheta);
	int newY = center.y + (x * sinTheta + y * cosTheta);

	return Point(newX, newY);
}
void PaintWidget::rotateShape(shape& shape, double angle) {
	Point center = usespincenter ? spinCenter : shape.center;
	// 针对不同形状类型进行旋转
	if (shape.type == shape::Polygon) {
		for (size_t i = 0; i < shape.originalpolygonPoints.size(); ++i) {
			shape.polygonPoints[i] = rotatePoint(shape.originalpolygonPoints[i], center, angle);
		}
	}
	else if (shape.type == shape::Line) {
		shape.start = rotatePoint(shape.originalstart, center, angle);
		shape.end = rotatePoint(shape.originalend, center, angle);
	}
	else if (shape.type == shape::Arc) {
		shape.start = rotatePoint(shape.originalstart, center, angle);
		shape.end = rotatePoint(shape.originalend, center, angle);
	}
	else if (shape.type == shape::Circle) {
		shape.center = rotatePoint(shape.originalcenter, center, angle);
		shape.start = rotatePoint(shape.originalstart, center, angle);
	}
}

// Bresenham 直线算法
void PaintWidget::bresenhamLine(const Point& start, const Point& end, QPainter& painter) {
	int x1 = start.x, y1 = start.y, x2 = end.x, y2 = end.y;
	int dx = abs(x2 - x1), dy = abs(y2 - y1);
	int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
	int err = dx - dy;
	int patternLength = currentLinePattern.getPattern().length();
	int patternIndex = 0;
	int count = 0;
	string pattern = currentLinePattern.getPattern();  // 获取线型模式

	while (true) {
		if (pattern[floor(patternIndex % patternLength)] == '1')
		{
			painter.drawPoint(x1, y1);  // 根据模式绘制点
			patternIndex++;
		}
		else {
			if (count == lineWidth - 1)
			{
				patternIndex++;
				count = 0;
			}
			else
				count++;
		}
		if (x1 == x2 && y1 == y2) break;
		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; x1 += sx; }
		if (e2 < dx) { err += dx; y1 += sy; }
	}
}

// 修复后的中点直线算法
void PaintWidget::midpointLine(const Point& start, const Point& end, QPainter& painter) {
	int x1 = start.x, y1 = start.y, x2 = end.x, y2 = end.y;
	int dx = abs(x2 - x1), dy = abs(y2 - y1);
	int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
	bool steep = dy > dx;
	int patternLength = currentLinePattern.getPattern().length();
	int patternIndex = 0;
	int count = 0;
	string pattern = currentLinePattern.getPattern();  // 获取线型模式

	if (steep) {
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);
		std::swap(sx, sy);
	}

	int d = 2 * dy - dx, y = y1;
	for (int x = x1; x != x2 + sx; x += sx) {
		if (pattern[floor(patternIndex % patternLength)] == '1')
		{
			if (steep) {
				painter.drawPoint(y, x);
			}
			else {
				painter.drawPoint(x, y);
			}
			patternIndex++;
		}
		else {
			if (count == lineWidth - 1)
			{
				patternIndex++;
				count = 0;
			}
			else
				count++;
		}
		if (d > 0) { y += sy; d -= 2 * dx; }
		d += 2 * dy;
	}
}

void PaintWidget::drawInscribedPolygon(const Point& center, double radius, int sides, QPainter& painter) {
	if (sides < 3) return;  // 至少需要三条边

	double angleStep = 2 * M_PI / sides;  // 每个角度的弧度
	std::vector<Point> vertices;

	// 计算多边形的顶点
	for (int i = 0; i < sides; ++i) {
		double angle = i * angleStep;
		int x = center.x + static_cast<int>(radius * cos(angle));
		int y = center.y + static_cast<int>(radius * sin(angle));
		vertices.push_back(Point(x, y));
	}

	// 使用bressenham算法绘制多边形的边
	for (int i = 0; i < sides; ++i) {
		bresenhamLine(vertices[i], vertices[(i + 1) % sides], painter);
	}
	if (isfilling) {
		filler(vertices, painter);
	}
	shape polygon;
	polygon.filling = isfilling;
	polygon.type = shape::Type::Polygon;
	polygon.polygonPoints = vertices;
	polygon.lineWidth = lineWidth;
	polygon.center = center;
	polygon.lineColor = currentPenColor;
	polygon.linePattern = currentLinePattern;
	ostringstream infoStream;  // 使用 std 命名空间
	infoStream << vertices.size() << "边形，顶点：";
	for (int i = 0; i < vertices.size(); i++)
		infoStream << "(" << vertices[i].x << "," << vertices[i].y << ") ";
	polygon.shapeInfo = QString::fromLocal8Bit(infoStream.str().c_str());
	shapes.push_back(polygon);
}

void PaintWidget::filler(const std::vector<Point>& vertices, QPainter& painter) {
	int minY = INT_MAX, maxY = INT_MIN;
	// 寻找最小和最大 y 值
	for (const auto& point : vertices) {
		minY = std::min(minY, point.y);
		maxY = std::max(maxY, point.y);
	}

	QVector<QVector<int>> edgeTable(maxY - minY + 1);

	// 构建边表
	for (int i = 0; i < vertices.size(); ++i) {
		QPoint p1 = QPoint(vertices[i].x, vertices[i].y);
		QPoint p2 = QPoint(vertices[(i + 1) % vertices.size()].x, vertices[(i + 1) % vertices.size()].y);

		if (p1.y() > p2.y()) std::swap(p1, p2); // 确保 p1 是下端点

		if (p1.y() != p2.y()) {
			double slope = static_cast<double>(p2.x() - p1.x()) / (p2.y() - p1.y());
			for (int y = p1.y(); y < p2.y(); ++y) {
				int x = static_cast<int>(p1.x() + slope * (y - p1.y()));
				edgeTable[y - minY].push_back(x);
			}
		}
	}

	// 对每一条扫描线进行填充
	for (int y = minY; y <= maxY; ++y) {
		if (!edgeTable[y - minY].isEmpty()) {
			std::sort(edgeTable[y - minY].begin(), edgeTable[y - minY].end());
			for (int j = 0; j < edgeTable[y - minY].size(); j += 2) {
				int xStart = edgeTable[y - minY][j];
				int xEnd = edgeTable[y - minY][j + 1];
				painter.drawLine(xStart, y, xEnd, y); // 填充
			}
		}
	}
}

// 中点圆算法
void PaintWidget::midPointCircle(const Point& center, int r, QPainter& painter) {
	int x = 0, y = r;
	int d = 1 - r;
	int xc = center.x, yc = center.y;
	int patternLength = currentLinePattern.getPattern().length();
	int patternIndex = 0;
	int count = 0;
	string pattern = currentLinePattern.getPattern();  // 获取线型模式

	painter.drawPoint(xc + x, yc + y);
	painter.drawPoint(xc - x, yc + y);
	painter.drawPoint(xc + x, yc - y);
	painter.drawPoint(xc - x, yc - y);
	painter.drawPoint(xc + y, yc + x);
	painter.drawPoint(xc - y, yc + x);
	painter.drawPoint(xc + y, yc - x);
	painter.drawPoint(xc - y, yc - x);

	while (x < y) {
		if (d < 0) {
			d = d + 2 * x + 3;
		}
		else {
			d = d + 2 * (x - y) + 5;
			y--;
		}
		x++;
		if (pattern[floor(patternIndex % patternLength)] == '1')
		{
			painter.drawPoint(xc + x, yc + y);
			painter.drawPoint(xc - x, yc + y);
			painter.drawPoint(xc + x, yc - y);
			painter.drawPoint(xc - x, yc - y);
			painter.drawPoint(xc + y, yc + x);
			painter.drawPoint(xc - y, yc + x);
			painter.drawPoint(xc + y, yc - x);
			painter.drawPoint(xc - y, yc - x);
			patternIndex++;
		}
		else {
			if (count == (lineWidth - 1))
			{
				patternIndex++;
				count = 0;
			}
			else
				count++;
		}
	}
}

// 判断点序列是否按顺时针排列
bool isClockwise(const std::vector<Point>& polygon) {
	double sum = 0;
	for (size_t i = 0; i < polygon.size(); i++) {
		const Point& p1 = polygon[i];
		const Point& p2 = polygon[(i + 1) % polygon.size()];
		sum += (p2.x - p1.x) * (p2.y + p1.y);
	}
	return sum < 0; // 如果为负则是顺时针
}

// 反转点集的顺序
std::vector<Point> reversePolygon(const std::vector<Point>& polygon) {
	std::vector<Point> reversed = polygon;
	std::reverse(reversed.begin(), reversed.end());
	return reversed;
}

std::vector<Point> PaintWidget::clipPolygon(const std::vector<Point>& subjectPolygon, const std::vector<Point>& clipPolygon) {
	// 检查 subjectPolygon 和 clipPolygon 的顺序
	std::vector<Point> localSubjectPolygon = subjectPolygon;
	std::vector<Point> localClipPolygon = clipPolygon;

	if (!isClockwise(subjectPolygon)) {
		localSubjectPolygon = reversePolygon(subjectPolygon);
	}

	if (!isClockwise(clipPolygon)) {
		localClipPolygon = reversePolygon(clipPolygon);
	}

	std::vector<Point> outputList = localSubjectPolygon;

	for (size_t i = 0; i < localClipPolygon.size(); ++i) {
		const Point& edgeStart = localClipPolygon[i];
		const Point& edgeEnd = localClipPolygon[(i + 1) % localClipPolygon.size()];
		std::vector<Point> inputList = outputList;
		outputList.clear();

		Point S = inputList.back();

		for (const auto& E : inputList) {
			if (inside(E, edgeStart, edgeEnd)) {
				if (!inside(S, edgeStart, edgeEnd)) {
					outputList.push_back(computeIntersection(S, E, edgeStart, edgeEnd));
				}
				outputList.push_back(E);
			}
			else if (inside(S, edgeStart, edgeEnd)) {
				outputList.push_back(computeIntersection(S, E, edgeStart, edgeEnd));
			}
			S = E;
		}
	}

	return outputList;
}

// 使用正负法绘制圆弧
void PaintWidget::drawArcBySignMethod(const Point& start, const Point& end, int radius, QPainter& painter) {
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float d = std::sqrt(dx * dx + dy * dy);
	if (d == 0) return;  // Prevent division by zero
	Point center;
	int a = (end.x + start.x) / 2.0;
	int b = (end.y + start.y) / 2.0;
	float h = std::sqrt(radius * radius - (d / 2) * (d / 2));
	center.x = (int)(a + h * (dy) / d);
	center.y = (int)(b - h * (dx) / d);
	int x = start.x;
	int y = start.y;
	int count = 0;
	double delta = 0;
	//abs(x - end.x) > 3 || abs(y - end.y) > 3

	while (abs(x - end.x) > 3 || abs(y - end.y) > 3) {
		delta = pow(x - center.x, 2) + pow(y - center.y, 2) - pow(radius, 2);
		if (x == center.x && y < center.y)
		{
			x++;
			y = center.y - radius;
		}
		else if (x == center.x && y > center.y)
		{
			x--;
			y = center.y + radius;
		}
		else if (y == center.y && x > center.x)
		{
			y++;
			x = center.x + radius;
		}
		else if (y == center.y && x < center.x)
		{
			y--;
			x = center.x - radius;
		}
		else if (delta >= 0 && x > center.x && y < center.y)  // 在圆上或圆外, 且在1/4圆上 (x >= center.x, y > center.y)
			y++;
		else if (delta < 0 && x > center.x && y < center.y)  // 在圆内, 且在1/4圆上 (x > center.x, y >= center.y)
			x++;
		else if (delta >= 0 && x > center.x && y > center.y)  // 在圆上或圆外, 且在2/4圆上 (x > center.x, y <= center.y)
			x--;
		else if (delta < 0 && x > center.x && y > center.y)  // 在圆内, 且在2/4圆上 (x > center.x, y <= center.y)
			y++;
		else if (delta >= 0 && x < center.x && y > center.y)  // 在圆上或圆外, 且在3/4圆上 (x <= center.x, y < center.y)
			y--;
		else if (delta < 0 && x < center.x && y > center.y)  // 在圆内, 且在3/4圆上 (x <= center.x, y <= center.y)
			x--;
		else if (delta >= 0 && x < center.x && y < center.y)  // 在圆上或圆外, 且在4/4圆上 (x <= center.x, y >= center.y)
			x++;
		else if (delta < 0 && x < center.x && y < center.y)  // 在圆内, 且在4/4圆上 (x <= center.x, y > center.y)
			y--;
		if (x >= painter.device()->width() || y >= painter.device()->height() || x < 0 || y < 0)
			continue;
		painter.drawPoint(x, y);
	}
}

int computeOutCode(double x, double y, const QRectF& rect) {
	int code = 0;
	if (x < rect.left()) code |= 1;   // 左
	else if (x > rect.right()) code |= 2;   // 右
	if (y < rect.top()) code |= 4;   // 上
	else if (y > rect.bottom()) code |= 8;   // 下
	return code;
}

// Cohen-Sutherland 直线裁剪算法的实现
void PaintWidget::cohenSutherlandClip(const QRectF& clipRect) {
	double x1 = selectedItem->start.x, y1 = selectedItem->start.y;
	double x2 = selectedItem->end.x, y2 = selectedItem->end.y;

	int outcode1 = computeOutCode(x1, y1, clipRect);
	int outcode2 = computeOutCode(x2, y2, clipRect);

	bool accept = false;

	while (true) {
		if (!(outcode1 | outcode2)) {
			accept = true;
			break;
		}
		else if (outcode1 & outcode2) {
			break;
		}
		else {
			double x, y;
			int outcodeOut = outcode1 ? outcode1 : outcode2;
			if (outcodeOut & 8) {
				x = x1 + (x2 - x1) * (clipRect.bottom() - y1) / (y2 - y1);
				y = clipRect.bottom();
			}
			else if (outcodeOut & 4) {
				x = x1 + (x2 - x1) * (clipRect.top() - y1) / (y2 - y1);
				y = clipRect.top();
			}
			else if (outcodeOut & 2) {
				y = y1 + (y2 - y1) * (clipRect.right() - x1) / (x2 - x1);
				x = clipRect.right();
			}
			else if (outcodeOut & 1) {
				y = y1 + (y2 - y1) * (clipRect.left() - x1) / (x2 - x1);
				x = clipRect.left();
			}

			if (outcodeOut == outcode1) {
				x1 = x;
				y1 = y;
				outcode1 = computeOutCode(x1, y1, clipRect);
			}
			else {
				x2 = x;
				y2 = y;
				outcode2 = computeOutCode(x2, y2, clipRect);
			}
		}
	}

	if (accept) {
		selectedItem->start.x = static_cast<int>(x1);
		selectedItem->start.y = static_cast<int>(y1);
		selectedItem->end.x = static_cast<int>(x2);
		selectedItem->end.y = static_cast<int>(y2);
		selectedItem->center = Point((selectedItem->start.x + selectedItem->end.x) / 2,
			(selectedItem->start.y + selectedItem->end.y) / 2);
	}
}

Point clipEndpoint(double x0, double y0, double x1, double y1, const QRectF& clipRect, bool& flag) {
	if (computeOutCode(x0, y0, clipRect) == 0) {
		flag = true;
		return Point(x0, y0);
	}
	int outcode0 = computeOutCode(x0, y0, clipRect);
	int outcode1 = computeOutCode(x1, y1, clipRect);
	if (outcode0 & outcode1) {
		// 线段完全在矩形外
		flag = false;
		return Point(x0, y0);
	}
	while (true) {
		double xm = (x0 + x1) / 2;
		double ym = (y0 + y1) / 2;
		int outcode0 = computeOutCode(x0, y0, clipRect);
		if (sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) < 2) {
			flag = true;
			return Point(xm, ym);
		}

		int outcodeM = computeOutCode(xm, ym, clipRect);

		if (!(outcode0 & outcodeM)) {
			// 最近可见点在 x0 和 xm 之间
			x1 = xm;
			y1 = ym;
		}
		else {
			// 最近可见点在 xm 和 x1 之间
			x0 = xm;
			y0 = ym;
		}
	}
};

void PaintWidget::midpointClip(const QRectF& clipRect) {
	double x0 = selectedItem->start.x, y0 = selectedItem->start.y;
	double x1 = selectedItem->end.x, y1 = selectedItem->end.y;

	bool flag1 = false, flag2 = false;
	// 裁剪两端
	Point visibleStart = clipEndpoint(x0, y0, x1, y1, clipRect, flag1);
	Point visibleEnd = clipEndpoint(x1, y1, x0, y0, clipRect, flag2);

	if (flag1 && flag2) {
		// 更新线段为裁剪后线段
		selectedItem->start = visibleStart;
		selectedItem->end = visibleEnd;
		selectedItem->center = Point((selectedItem->start.x + selectedItem->end.x) / 2,
			(selectedItem->start.y + selectedItem->end.y) / 2);
	}
}

void PaintWidget::floodFill(int x, int y, const QColor& borderColor, const QColor& newColor) {
	QImage image = pixmap.toImage();
	int width = image.width();
	int height = image.height();

	if (x < 0 || y < 0 || x >= width || y >= height || image.pixel(x, y) != borderColor.rgb() || borderColor == newColor)
		return;

	// 使用堆栈进行非递归填充
	std::stack<std::pair<int, int>> stack;
	stack.push({ x, y });

	while (!stack.empty()) {
		auto [currentX, currentY] = stack.top();
		stack.pop();

		if (currentX < 0 || currentY < 0 || currentX >= width || currentY >= height || image.pixel(currentX, currentY) != borderColor.rgb())
			continue;

		image.setPixel(currentX, currentY, newColor.rgb());

		stack.push({ currentX + 1, currentY });
		stack.push({ currentX - 1, currentY });
		stack.push({ currentX, currentY + 1 });
		stack.push({ currentX, currentY - 1 });
	}

	pixmap = QPixmap::fromImage(image);
	update();
}

// 计算阶乘
qreal factorial(qreal n) {
	qreal result = 1;
	for (qreal i = 2; i <= n; ++i) {
		result *= i;
	}
	return result;
}

// 计算组合数（二项式系数）
qreal combination(qreal n, qreal k) {
	return factorial(n) / (factorial(k) * factorial(n - k));
}

QPoint PaintWidget::bezierPoint(const std::vector<QPoint>& points, qreal t, int degree) {
	QPoint p(0, 0);
	for (int i = 0; i <= degree; ++i) {
		qreal binomialCoeff = combination(degree, i);
		qreal tPower = std::pow(t, i);
		qreal bPower = std::pow(1 - t, degree - i);
		p += points[i] * binomialCoeff * tPower * bPower;
	}
	return p;
}

void PaintWidget::drawBSplineCurve(const vector<QPoint>& controlPoints, QPainter& painter, int k) {
	if (controlPoints.size() < 4) return;  // 至少需要4个控制点

	QPen pen(currentPenColor);
	pen.setWidth(lineWidth);
	painter.setPen(pen);

	QPainterPath path;

	const int steps = 20;  // 细分的步数
	const double stepSize = 1.0 / steps;

	// B样条的阶数为3次（可以根据需求调整）
	//int k = 1;
	QVector<double> knots = generateKnots(controlPoints.size() - 1, k);  // 生成结点矢量

	// 计算第一个点，避免默认从(0,0)连接
	double t = knots[k];  // 选择t的初始值
	QPoint firstPoint = calculateBSplinePoint(controlPoints, t, knots, k);
	path.moveTo(firstPoint);  // 从第一个有效的曲线点开始

	for (int i = 1; i < steps; ++i) {
		t = knots[k] + i * stepSize * (knots[knots.size() - k - 1] - knots[k]);
		QPoint pt = calculateBSplinePoint(controlPoints, t, knots, k);
		path.lineTo(pt);
	}

	painter.drawPath(path);
}

QPoint PaintWidget::calculateBSplinePoint(const vector<QPoint>& controlPoints, double t, const QVector<double>& knots, int k) {
	QPoint result(0, 0);
	int n = controlPoints.size() - 1;

	for (int i = 0; i <= n; ++i) {
		double basis = bsplineBasis(i, k, t, knots);  // 计算B样条基函数值
		result += basis * controlPoints[i];
	}

	return result;
}

// B样条基函数计算
double PaintWidget::bsplineBasis(int i, int k, double t, const QVector<double>& knots) {
	if (k == 0) {
		return (knots[i] <= t && t < knots[i + 1]) ? 1.0 : 0.0;
	}
	else {
		double denom1 = knots[i + k] - knots[i];
		double denom2 = knots[i + k + 1] - knots[i + 1];
		double term1 = (denom1 != 0) ? (t - knots[i]) / denom1 * bsplineBasis(i, k - 1, t, knots) : 0.0;
		double term2 = (denom2 != 0) ? (knots[i + k + 1] - t) / denom2 * bsplineBasis(i + 1, k - 1, t, knots) : 0.0;
		return term1 + term2;
	}
}

// 生成B样条的结点矢量
QVector<double> PaintWidget::generateKnots(int n, int k) {
	QVector<double> knots(n + k + 2);
	for (int i = 0; i <= n + k + 1; ++i) {
		knots[i] = (i < k) ? 0 : (i > n) ? 1 : (double)(i - k + 1) / (n - k + 2);
	}
	return knots;
}

// de Casteljau 算法实现，用于计算贝塞尔曲线上的一个点
QPoint deCasteljau(const std::vector<QPoint>& controlPoints, qreal t) {
	std::vector<QPoint> points = controlPoints;  // 拷贝一份控制点的副本
	while (points.size() > 1) {
		std::vector<QPoint> nextLevel;
		for (size_t i = 0; i < points.size() - 1; ++i) {
			// 线性插值计算下一层的点
			QPoint interpolated = (1 - t) * points[i] + t * points[i + 1];
			nextLevel.push_back(interpolated);
		}
		points = nextLevel;
	}
	return points[0];  // 最终收敛到一个点，即为贝塞尔曲线上 t 位置的点
}void PaintWidget::drawBezierCurve(const vector<QPoint>& controlPoints, QPainter& painter) {
	QPen pen(currentPenColor);
	pen.setWidth(lineWidth);
	painter.setPen(pen);

	const int steps = 20; // 曲线细分的步数
	const qreal stepSize = 1.0 / steps;

	QPainterPath path;
	path.moveTo(controlPoints[0]);

	for (int i = 1; i <= steps; ++i) {
		qreal t = stepSize * i;
		QPoint point = deCasteljau(controlPoints, t);  // 使用 de Casteljau 算法计算曲线上点
		path.lineTo(point);
	}
	painter.drawPath(path);
}