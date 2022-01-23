#ifndef COMMON_H
#define COMMON_H
#include <QByteArray>
#include <QString>

QByteArray intToByte(int);
int bytesToInt(QByteArray);
QByteArray paddingQByteArray(quint64,quint8);

#endif // COMMON_H
