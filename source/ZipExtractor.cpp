#include "ZipExtractor.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qstring.h>

#include <zip.h>
#include <zlib.h>

static inline bool EnsureParentDir( const QString& filePath )
{
    const QString parent = QFileInfo( filePath ).absolutePath();
    if ( QDir().mkpath( parent ) )
        return true;
    return false;
}

static QString GetZipErrorString( int zipErrorCode )
{
    zip_error_t error;
    zip_error_init_with_code( &error, zipErrorCode );

    const char* errMsg = zip_error_strerror( &error );
    QString result = QString::fromUtf8( errMsg );

    zip_error_fini( &error );
    return result;
}

static uLong Crc32OfFile( const QString& path, qint64* outSize = nullptr )
{
    QFile file( path );
    if ( !file.open( QIODevice::ReadOnly ) )
        return 0;                     // caller handles error
    constexpr int BUF_SIZE = 1 << 16; // 64 KiB
    QByteArray buf;
    buf.resize( BUF_SIZE );
    uLong crc = crc32( 0L, Z_NULL, 0 );
    qint64 total = 0;
    while ( true )
    {
        const qint64 n = file.read( buf.data(), buf.size() );
        if ( n == 0 )
            break;
        if ( n < 0 )
            return 0;
        crc = crc32( crc, reinterpret_cast< const Bytef* >( buf.constData() ), static_cast< uInt >( n ) );
        total += n;
    }
    if ( outSize )
        *outSize = total;
    return crc;
}

ZipExtractor::ZipExtractor( QObject* parent )
    : QObject( parent )
{
}

bool ZipExtractor::ExtractZip( const QString& zipPath, const QString& destPath )
{
    if ( zipPath.isEmpty() || destPath.isEmpty() )
    {
        ErrorOccurred( QStringLiteral( "Zip path or destination path is empty." ) );
        return false;
    }

    int zipErr = 0;

    zip_t* za = zip_open( zipPath.toUtf8().constData(), 0, &zipErr );
    if ( !za )
    {
        ErrorOccurred( QStringLiteral( "failed to open zip \"%1\" failed (err=%2) : %3" )
                           .arg( zipPath.toUtf8().constData() )
                           .arg( zipErr )
                           .arg( GetZipErrorString( zipErr ) ) );
        return false;
    }

    QDir dest( destPath );
    if ( !dest.exists() && !QDir().mkpath( dest.absolutePath() ) )
    {
        ErrorOccurred( QStringLiteral( "Failed to create destination: %1" ).arg( destPath ) );
        zip_close( za );
        return false;
    }

    const zip_int64_t entryCount = zip_get_num_entries( za, 0 );

    int totalFiles = 0;
    for ( zip_uint64_t i = 0; i < entryCount; ++i )
    {
        zip_stat_t st;
        zip_stat_init( &st );
        if ( zip_stat_index( za, i, 0, &st ) != 0 )
            continue;
        const QString name = QString::fromUtf8( st.name );
        if ( !name.endsWith( '/' ) )
            ++totalFiles;
    }

    int completed = 0;
    constexpr size_t BUF_SIZE = 1 << 16;

    for ( zip_uint64_t i = 0; i < entryCount; ++i )
    {
        zip_stat_t st;
        zip_stat_init( &st );
        if ( zip_stat_index( za, i, 0, &st ) != 0 )
            continue;

        const QString relName = QString::fromUtf8( st.name );

        if ( relName.endsWith( '/' ) )
        {
            QDir().mkpath( dest.filePath( relName ) );
            continue;
        }

        zip_file_t* zf = zip_fopen_index( za, i, 0 );
        if ( !zf )
        {
            ErrorOccurred( QStringLiteral( "Failed to open zip entry: %1" ).arg( relName ) );
            zip_close( za );
            return false;
        }

        const QString outPath = QDir::cleanPath( dest.filePath( relName ) );

        if ( !EnsureParentDir( outPath ) )
        {
            ErrorOccurred( QStringLiteral( "Failed to create directory for: %1" ).arg( outPath ) );
            zip_fclose( zf );
            zip_close( za );
            return false;
        }

        QFile out( outPath );
        if ( !out.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        {
            ErrorOccurred( QStringLiteral( "Cannot write file: %1" ).arg( outPath ) );
            zip_fclose( zf );
            zip_close( za );
            return false;
        }

        QByteArray buffer;
        buffer.resize( int( BUF_SIZE ) );

        while ( true )
        {
            const zip_int64_t n = zip_fread( zf, buffer.data(), buffer.size() );
            if ( n == 0 )
                break; // EOF
            if ( n < 0 )
            {
                ErrorOccurred( QStringLiteral( "Read error in entry: %1" ).arg( relName ) );
                out.close();
                zip_fclose( zf );
                zip_close( za );
                return false;
            }
            if ( out.write( buffer.constData(), n ) != n )
            {
                ErrorOccurred( QStringLiteral( "Write error for: %1" ).arg( outPath ) );
                out.close();
                zip_fclose( zf );
                zip_close( za );
                return false;
            }
        }

        out.close();
        zip_fclose( zf );

        ++completed;
        emit ExtractionProgress( completed, totalFiles, relName );
    }

    zip_close( za );
    return true;
}

bool ZipExtractor::ValidateExtractedData( const QString& zipPath, const QString& destPath )
{
    int err = 0;
    const QByteArray zipPathBytes = QFile::encodeName( zipPath );
    zip_t* za = zip_open( zipPathBytes.constData(), ZIP_CHECKCONS, &err );
    if ( !za )
    {
        ErrorOccurred( QStringLiteral( "zip_open failed (err=%1)" ).arg( err ) );
        return false;
    }

    QDir dest( destPath );
    if ( !dest.exists() )
    {
        zip_close( za );
        ErrorOccurred( QStringLiteral( "Destination directory does not exist: %1" ).arg( destPath ) );
        return false;
    }

    const zip_int64_t entryCount = zip_get_num_entries( za, 0 );

    int totalFiles = 0;
    for ( zip_uint64_t i = 0; i < static_cast< zip_uint64_t >( entryCount ); ++i )
    {
        zip_stat_t st;
        zip_stat_init( &st );
        if ( zip_stat_index( za, i, 0, &st ) != 0 )
            continue;
        const QString name = QString::fromUtf8( st.name );
        if ( !name.endsWith( '/' ) )
            ++totalFiles;
    }

    int completed = 0;

    for ( zip_uint64_t i = 0; i < static_cast< zip_uint64_t >( entryCount ); ++i )
    {
        zip_stat_t st;
        zip_stat_init( &st );
        if ( zip_stat_index( za, i, 0, &st ) != 0 )
            continue;

        const QString relName = QString::fromUtf8( st.name );

        if ( relName.endsWith( '/' ) )
            continue;

        const QString onDiskPath = QDir::cleanPath( dest.filePath( relName ) );

        QFileInfo fileInfo( onDiskPath );
        if ( !fileInfo.exists() || !fileInfo.isFile() )
        {
            zip_close( za );
            ErrorOccurred( QStringLiteral( "Missing extracted file: %1" ).arg( onDiskPath ) );
            return false;
        }

        qint64 actualSize = -1;
        const uLong actualCrc = Crc32OfFile( onDiskPath, &actualSize );
        if ( actualSize < 0 )
        {
            zip_close( za );
            ErrorOccurred( QStringLiteral( "Read error while checking: %1" ).arg( onDiskPath ) );
            return false;
        }

        bool sizeOk = true, crcOk = true;
        if ( st.valid & ZIP_STAT_SIZE )
            sizeOk = ( static_cast< quint64 >( actualSize ) == st.size );
        if ( st.valid & ZIP_STAT_CRC )
            crcOk = ( actualCrc == st.crc );

        if ( !sizeOk || !crcOk )
        {
            zip_close( za );
            ErrorOccurred(
                QStringLiteral(
                    "Integrity check failed for %1 (size ok: %2, crc ok: %3; read=%4, expected size=%5, expected crc=%6, got crc=%7)" )
                    .arg( relName )
                    .arg( sizeOk )
                    .arg( crcOk )
                    .arg( actualSize )
                    .arg( ( st.valid & ZIP_STAT_SIZE ) ? QString::number( st.size ) : QStringLiteral( "n/a" ) )
                    .arg( ( st.valid & ZIP_STAT_CRC ) ? QString::number( st.crc, 16 ) : QStringLiteral( "n/a" ) )
                    .arg( QString::number( actualCrc, 16 ) ) );
            return false;
        }

        ++completed;
        if ( completed % 100 == 0 || completed == totalFiles )
            Q_EMIT ValidationProgress( completed, totalFiles, relName );
    }

    zip_close( za );
    return true;
}
