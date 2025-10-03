#include "DataLoader.h"
#include "LogManager.h"
#include "RessourcesManager.h"
#include "ZipExtractor.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <qcoreapplication.h>

#include <array>
#include <qdir.h>
#include <zip.h>

static constexpr const char* SDE_URL = "https://developers.eveonline.com/static-data/tranquility/eve-online-static-data-3031812-jsonl.zip";
static constexpr const char* MARKET_PRICES_URL = "https://esi.evetech.net/latest/markets/prices/?datasource=tranquility";

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
    connect( this, &DataLoader::SdeValidated, this, &DataLoader::DownloadMarketPrices );

    DownloadSde();
}

QString DataLoader::GetSdeExtractedPath() const
{
    return sdeExtractedPath_;
}

QJsonObject&& DataLoader::GetMarketPricesJson()
{
    return std::move( marketPricesJson_ );
}

void DataLoader::OnSdeDownloadFinished( bool isSuccess )
{
    if ( !isSuccess )
    {
        TriggerError( "Failed to download SDE." );
        return;
    }
    LOG_NOTICE( "SDE downloaded successfully." );
    sdeDownloader_->deleteLater();
    emit SdeDownloaded();
}

void DataLoader::MarketPricesDownloaded( QByteArray data )
{
    marketPricesDownloader_->deleteLater();
    if ( data.isEmpty() )
    {
        TriggerError( "Failed to download market prices." );
        return;
    }
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson( data, &parseError );
#ifndef NDEBUG
    QFile jsonVersion( sdeExtractedPath_ + "/market_prices.json" );
    if ( jsonVersion.open( QIODevice::WriteOnly ) )
    {
        jsonVersion.write( doc.toJson( QJsonDocument::Indented ) );
        jsonVersion.close();
    }
#endif // !NDEBUG

    if ( parseError.error != QJsonParseError::NoError )
    {
        TriggerError( tr( "Failed to parse market prices JSON: %1" ).arg( parseError.errorString() ) );
        return;
    }
    if ( !doc.isArray() )
    {
        TriggerError( "Expected JSON array for market prices." );
        return;
    }
    QJsonArray jsonArray = doc.array();
    for ( const QJsonValue& value : jsonArray )
    {
        if ( !value.isObject() )
            continue;
        QJsonObject obj = value.toObject();
        if ( !obj.contains( "type_id" ) || !obj.contains( "average_price" ) || !obj.contains( "adjusted_price" ) )
            continue;
        int typeId = obj.value( "type_id" ).toInt();
        double averagePrice = obj.value( "average_price" ).toDouble();
        double adjustedPrice = obj.value( "adjusted_price" ).toDouble();
        QJsonObject priceObj;
        priceObj[ "average_price" ] = averagePrice;
        priceObj[ "adjusted_price" ] = adjustedPrice;
        marketPricesJson_[ QString::number( typeId ) ] = priceObj;
    }
    LOG_NOTICE( "Market prices downloaded and parsed successfully." );
    emit MarketPricesReady();
}

void DataLoader::DownloadSde()
{
    sdeDownloader_ = new FileDownloader( this );
    connect( sdeDownloader_,
             &FileDownloader::DownloadProgress,
             [ this ]( qint64 current, qint64 total ) { emit SubDataLoadingStepChanged( current, total, "Downloading SDE" ); } );
    connect( sdeDownloader_, &FileDownloader::DownloadFinished, this, &DataLoader::OnSdeDownloadFinished );
    SetLoadingStep( eDataLoadingSteps::DownloadingSde );
    sdeDownloader_->Start( sdeZipPath_, SDE_URL );
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
    emit SdeValidated();
}

void DataLoader::DownloadMarketPrices()
{
    marketPricesDownloader_ = new FileDownloader( this );
    connect( marketPricesDownloader_,
             &FileDownloader::DownloadProgress,
             [ this ]( qint64 current, qint64 total ) { emit SubDataLoadingStepChanged( current, total, "Downloading Market prices" ); } );
    connect( marketPricesDownloader_, &FileDownloader::DownloadFinishedWithData, this, &DataLoader::MarketPricesDownloaded );
    SetLoadingStep( eDataLoadingSteps::FetchingMarketPrices );
    marketPricesDownloader_->Start( MARKET_PRICES_URL );
}

void DataLoader::TriggerError( const QString& errorMessage )
{
    LOG_WARNING( "{}", errorMessage.toStdString() );
    emit ErrorOccurred( errorMessage );
    return;
}
