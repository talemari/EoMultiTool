#pragma once
#include <qobject.h>

class ZipExtractor : public QObject
{
    Q_OBJECT

public:
    ZipExtractor( QObject* parent = nullptr );
    ~ZipExtractor() = default;

    bool ExtractZip( const QString& zipPath, const QString& destPath );
    bool ValidateExtractedData( const QString& zipPath, const QString& destPath );

signals:
    void ExtractionProgress( uint extractedFiles, uint totalFiles, const QString& fileName );
    void ValidationProgress( uint validatedFiles, uint totalFiles, const QString& fileName );
    void ErrorOccurred( const QString& errorMessage );
};
