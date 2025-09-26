#include "DataLoader.h"
#include "LogManager.h"
#include "RessourcesManager.h"
#include "ZipExtractor.h"

#include <qcoreapplication.h>

#include <array>
#include <qdir.h>
#include <zip.h>

static constexpr const char* SDE_URL = "https://developers.eveonline.com/static-data/tranquility/eve-online-static-data-3031812-jsonl.zip";

DataLoader::DataLoader( QObject* parent )
    : QObject( parent )
    , sdeExtractedPath_( QCoreApplication::applicationDirPath() + "/ressources/generated/sde/" )
    , sdeZipPath_( QCoreApplication::applicationDirPath() + "/ressources/generated/sde.zip" )
{
}

void DataLoader::StartDataLoading()
{
    LOG_NOTICE( "Starting data loading..." );
    currentDataLoadingStep_ = eDataLoadingSteps::Waiting;
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
    fileDownloader_->deleteLater();
    emit SdeDownloaded();
}

void DataLoader::DownloadSde()
{
    fileDownloader_ = new FileDownloader( this );
    connect( fileDownloader_,
             &FileDownloader::DownloadProgress,
             [ this ]( qint64 current, qint64 total ) { emit SubDataLoadingStepChanged( current, total, "Downloading SDE" ); } );
    connect( fileDownloader_, &FileDownloader::DownloadFinished, this, &DataLoader::OnSdeDownloadFinished );
    SetLoadingStep( eDataLoadingSteps::DownloadingSde );
    fileDownloader_->Start( sdeZipPath_, SDE_URL );
}

void DataLoader::SetLoadingStep( eDataLoadingSteps step )
{
    if ( step == currentDataLoadingStep_ )
        return;
    currentDataLoadingStep_ = step;
    emit MainDataLoadingStepChanged( currentDataLoadingStep_ );
}

void DataLoader::ExtractSde()
{
    SetLoadingStep( eDataLoadingSteps::ExtractingSde );

    ZipExtractor zipExtractor;
    connect( &zipExtractor,
             &ZipExtractor::ExtractionProgress,
             [ this ]( uint current, uint total, const QString& fileName )
             { emit SubDataLoadingStepChanged( current, total, fileName ); } );
    connect( &zipExtractor, &ZipExtractor::ErrorOccurred, this, &DataLoader::TriggerError );
    const bool isSuccess = zipExtractor.ExtractZip( sdeZipPath_.toStdString().c_str(), sdeExtractedPath_.toStdString().c_str() );
    if ( !isSuccess )
        return;
    LOG_NOTICE( "SDE extracted successfully." );
    emit SdeExtracted();
}

void DataLoader::ValidateSde()
{
    SetLoadingStep( eDataLoadingSteps::ValidatingSde );
    ZipExtractor zipExtractor;
    connect( &zipExtractor,
             &ZipExtractor::ValidationProgress,
             [ this ]( uint current, uint total, const QString& fileName )
             { emit SubDataLoadingStepChanged( current, total, fileName ); } );
    connect( &zipExtractor, &ZipExtractor::ErrorOccurred, this, &DataLoader::TriggerError );
    const bool isSuccess = zipExtractor.ValidateExtractedData( sdeZipPath_.toStdString().c_str(), sdeExtractedPath_.toStdString().c_str() );
    if ( !isSuccess )
        return;
    LOG_NOTICE( "SDE validated successfully." );
    emit SdeValidated( sdeExtractedPath_ );
}

void DataLoader::TriggerError( const QString& errorMessage )
{
    LOG_WARNING( "{}", errorMessage.toStdString() );
    emit ErrorOccurred( errorMessage );
    return;
}
