#include "LuckyDog.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/pic/icon.ico"));
    LuckyDog w;
    w.show();
    return a.exec();
}
