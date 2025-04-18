#pragma once

#include <QtWidgets/QWidget>
#include "ui_Graphics.h"

class Graphics : public QWidget
{
    Q_OBJECT

public:
    Graphics(QWidget *parent = nullptr);
    ~Graphics();

private:
    Ui::GraphicsClass ui;
};
