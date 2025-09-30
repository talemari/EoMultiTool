#include "HelperFunctions.h"

#include <QFile>

unsigned int GetNumberOfLinesInFile( QFile& file )
{
    unsigned int lineCount = 0;
    while ( !file.atEnd() )
    {
        file.readLine();
        ++lineCount;
    }
    file.seek( 0 );
    return lineCount;
}