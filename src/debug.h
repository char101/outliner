#include <QDebug>

#define QDEBUG qDebug() << qUtf8Printable(QString("[Outliner] %1:%2 (%3)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__))

#define DVAL(x) #x " =" << x

#define QP(s) qUtf8Printable(s)
