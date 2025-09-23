#include "DataLoader.h"
#include "LogManager.h"
#include "ZipExtractor.h"

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

DataLoader::DataLoader( QObject* parent )
    : QObject( parent )
    , sdeZipFile_( SDE_ZIP_PATH )
{
}

void DataLoader::StartDataLoading()
{
    LOG_NOTICE( "Starting data loading..." );
    currentDataLoadingStep_ = 0;
    connect( this, &DataLoader::SdeDownloaded, this, &DataLoader::ExtractSde );
    connect( this, &DataLoader::SdeExtracted, this, &DataLoader::ValidateSde );

    DownloadSde();
}

void DataLoader::OnSdeDownloadFinished( bool isSuccess )
{
    if ( !isSuccess )
    {
        TriggerError( "Failed to download SDE." );
        return;
    }
    LOG_NOTICE( "SDE downloaded successfully." );
    delete fileDownloader_;
    LoadingStepProgressed();
    emit SdeDownloaded();
}

void DataLoader::DownloadSde()
{
    fileDownloader_ = new FileDownloader( this );
    fileDownloader_->Start( SDE_ZIP_PATH, SDE_URL );
    connect( fileDownloader_, &FileDownloader::DownloadProgress,
             [ this ]( qint64 current, qint64 total ) { emit SubDataLoadingStepChanged( current, total, "Downloading SDE" ); } );
    connect( fileDownloader_, &FileDownloader::DownloadFinished, this, &DataLoader::OnSdeDownloadFinished );
}

void DataLoader::LoadingStepProgressed()
{
    ++currentDataLoadingStep_;
    emit MainDataLoadingStepChanged( currentDataLoadingStep_, static_cast< int >( currentDataLoadingStep.size() ),
                                     currentDataLoadingStep[ currentDataLoadingStep_ - 1 ] );
}

void DataLoader::ExtractSde()
{
    ZipExtractor zipExtractor;
    connect( &zipExtractor, &ZipExtractor::ExtractionProgress, [ this ]( uint current, uint total, const QString& fileName )
             { emit SubDataLoadingStepChanged( current, total, fileName ); } );
    connect( &zipExtractor, &ZipExtractor::ErrorOccurred, this, &DataLoader::TriggerError );
    const bool isSuccess = zipExtractor.ExtractZip( SDE_ZIP_PATH, SDE_EXTRACTED_PATH );
    if ( !isSuccess )
        return;
    LOG_NOTICE( "SDE extracted successfully." );
    LoadingStepProgressed();
    emit SdeExtracted();
}

void DataLoader::ValidateSde()
{
    ZipExtractor zipExtractor;
    connect( &zipExtractor, &ZipExtractor::ValidationProgress, [ this ]( uint current, uint total, const QString& fileName )
             { emit SubDataLoadingStepChanged( current, total, fileName ); } );
    connect( &zipExtractor, &ZipExtractor::ErrorOccurred, this, &DataLoader::TriggerError );
    const bool isSuccess = zipExtractor.ValidateExtractedData( SDE_ZIP_PATH, SDE_EXTRACTED_PATH );
    if ( !isSuccess )
        return;
    LOG_NOTICE( "SDE validated successfully." );
    LoadingStepProgressed();
    emit SdeValidated();
}

void DataLoader::TriggerError( const QString& errorMessage )
{
    LOG_WARNING( "{}", errorMessage.toStdString() );
    emit ErrorOccurred( errorMessage );
    return;
}
