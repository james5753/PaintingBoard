/********************************************************************************
** Form generated from reading UI file 'Graphics.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GRAPHICS_H
#define UI_GRAPHICS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GraphicsClass
{
public:

    void setupUi(QWidget *GraphicsClass)
    {
        if (GraphicsClass->objectName().isEmpty())
            GraphicsClass->setObjectName("GraphicsClass");
        GraphicsClass->resize(600, 400);

        retranslateUi(GraphicsClass);

        QMetaObject::connectSlotsByName(GraphicsClass);
    } // setupUi

    void retranslateUi(QWidget *GraphicsClass)
    {
        GraphicsClass->setWindowTitle(QCoreApplication::translate("GraphicsClass", "Graphics", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GraphicsClass: public Ui_GraphicsClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GRAPHICS_H
