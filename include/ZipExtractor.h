#pragma once
#include <qobject.h>

class ZipExtractor : public QObject
{
    Q_OBJECT

public:
    ZipExractor( const QString& zipPath, const QString& zipDest, QObject* parent = nullptr );

private:
    QString zipPath_;
  QString zipDest_;
};
