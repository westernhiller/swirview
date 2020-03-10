#include "swirdialog.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
//    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
//    QTextCodec:::setCodecForTr(codec);

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
//    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

    QApplication a(argc, argv);
    SwirDialog w;
//    w.showFullScreen();
    w.show();

    return a.exec();
}
