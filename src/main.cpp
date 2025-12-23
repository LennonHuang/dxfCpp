#include "AutoDxfCpp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    AutoDxfCpp window;
    window.show();
    return app.exec();
}
