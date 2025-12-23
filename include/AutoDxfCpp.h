#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AutoDxfCpp.h"

class AutoDxfCpp : public QMainWindow
{
    Q_OBJECT

public:
    AutoDxfCpp(QWidget *parent = nullptr);
    ~AutoDxfCpp();

private:
    Ui::AutoDxfCppClass ui;
};

