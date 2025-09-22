#include "DataLoader.h"
#include "LogManager.h"

#include <array>
#include <qdir.h>
#include <zip.h>

static constexpr const char* SDE_URL = "https://eve-static-data-export.s3-eu-west-1.amazonaws.com/tranquility/sde.zip";
static constexpr const char* SDE_ZIP_PATH = "ressources/generated/sde/sde.zip";
static constexpr const char* SDE_EXTRACTED_PATH = "ressources/generated/sde/";

static constexpr std::array< const char*, 11 > currentDataLoadingStep = { "Starting...",
                                                                          "Downloading SDE...",
                                                                          "Extracting SDE...",
                                                                          "Loading item types...",
                                                                          "Loading item groups...",
                                                                          "Loading item categories...",
                                                                          "Loading blueprints...",
                                                                          "Loading attributes...",
                                                                          "Loading dogma attributes...",
                                                                          "Loading dogma effects...",
                                                                          "Finalizing..." };

static inline bool ensureParentDir( const QString& filePath )
{
    const QString parent = QFileInfo( filePath ).absolutePath();
    if ( QDir().mkpath( parent ) )
        return true;
    return false;
}

DataLoader::DataLoader( QObject* parent )
    : QObject( parent )
    , sdeZipFile_( SDE_ZIP_PATH )
    , zipExtractor_( QString( SDE_ZIP_PATH ), QString( SDE_EXTRACTED_PATH ) )
{
}

void DataLoader::StartDataLoading()
{
    LOG_NOTICE( "Starting data loading..." );
    currentDataLoadingStep_ = 0;
    LoadingStepProgressed();

    fileDownloader_ = new FileDownloader( this );
    fileDownloader_->Start( SDE_ZIP_PATH, SDE_URL );
    connect( fileDownloader_, &FileDownloader::DownloadProgress,
             [ this ]( qint64 current, qint64 total ) { emit SubDataLoadingStepChanged( current, total, "Downloading SDE" ); } );
    connect( fileDownloader_, &FileDownloader::DownloadFinished, this, &DataLoader::OnSdeDownloadFinished );
}

void DataLoader::OnSdeDownloadFinished( bool isSuccess )
{
    if ( !isSuccess )
    {
        TriggerError( "Failed to download SDE." );
        return;
    }
    LOG_NOTICE( "SDE downloaded successfully." );
    LoadingStepProgressed();
    ExtractSde();
}

void DataLoader::LoadingStepProgressed()
{
    ++currentDataLoadingStep_;
    emit MainDataLoadingStepChanged( currentDataLoadingStep_, static_cast< int >( currentDataLoadingStep.size() ),
                                     currentDataLoadingStep[ currentDataLoadingStep_ - 1 ] );
}

void DataLoader::ExtractSde()
{
    int zipErr = 0;
    zip_t* za = zip_open( SDE_ZIP_PATH, 0, &zipErr );
    if ( !za )
    {
        DataLoader::TriggerError( QStringLiteral( "zip_open failed (err=%1)" ).arg( zipErr ) );
        return;
    }

    QDir dest( SDE_EXTRACTED_PATH );
    if ( !dest.exists() && !QDir().mkpath( dest.absolutePath() ) )
    {
        DataLoader::TriggerError( QStringLiteral( "Failed to create destination: %1" ).arg( SDE_EXTRACTED_PATH ) );
        zip_close( za );
        return;
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
    constexpr size_t BUF_SIZE = 1 << 16; // 64 KiB

    // Second pass: extract.
    for ( zip_uint64_t i = 0; i < entryCount; ++i )
    {
        zip_stat_t st;
        zip_stat_init( &st );
        if ( zip_stat_index( za, i, 0, &st ) != 0 )
            continue;

        const QString relName = QString::fromUtf8( st.name );

        // Directory entry
        if ( relName.endsWith( '/' ) )
        {
            QDir().mkpath( dest.filePath( relName ) );
            continue;
        }

        // Open file inside zip
        zip_file_t* zf = zip_fopen_index( za, i, 0 );
        if ( !zf )
        {
            DataLoader::TriggerError( QStringLiteral( "Failed to open zip entry: %1" ).arg( relName ) );
            zip_close( za );
            return;
        }

        const QString outPath = QDir::cleanPath( dest.filePath( relName ) );

        if ( !ensureParentDir( outPath ) )
        {
            DataLoader::TriggerError( QStringLiteral( "Failed to create directory for: %1" ).arg( outPath ) );
            zip_fclose( zf );
            zip_close( za );
            return;
        }

        QFile out( outPath );
        if ( !out.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        {
            DataLoader::TriggerError( QStringLiteral( "Cannot write file: %1" ).arg( outPath ) );
            zip_fclose( zf );
            zip_close( za );
            return;
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
                DataLoader::TriggerError( QStringLiteral( "Read error in entry: %1" ).arg( relName ) );
                out.close();
                zip_fclose( zf );
                zip_close( za );
                return;
            }
            if ( out.write( buffer.constData(), n ) != n )
            {
                DataLoader::TriggerError( QStringLiteral( "Write error for: %1" ).arg( outPath ) );
                out.close();
                zip_fclose( zf );
                zip_close( za );
                return;
            }
        }

        out.close();
        zip_fclose( zf );

        ++completed;
        emit SubDataLoadingStepChanged( completed, totalFiles, relName );
    }

    zip_close( za );
}

void DataLoader::TriggerError( const QString& errorMessage )
{
    LOG_WARNING( "{}", errorMessage.toStdString() );
    emit ErrorOccurred( errorMessage );
    return;
}
