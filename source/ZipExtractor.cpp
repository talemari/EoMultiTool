#include "ZipExtractor.h"
#include <qstring.h>

ZipExtractor::ZipExractor( const QString& zipPath, const QString& zipDest, QObject* parent )
    : QObject( parent )
    , zipPath_( zipPath )
    , zipDest_( zipDest )
{
}
