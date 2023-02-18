#include "KUSBLFTOASC.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KUSBLFTOASC w;
    w.show();
    a.setWindowIcon(QIcon(":/qrc/logo.ico"));
    return a.exec();
}
