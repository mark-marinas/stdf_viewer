#include "stdf_viewer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    STDF_viewer w;
    w.show();
    return a.exec();
}
