#include "DataLoader.h"
#include "LogManager.h"

#include <array>

#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressBar>
#include <QThread>
#include <QVBoxLayout>

static constexpr const char* SDE_URL = "https://eve-static-data-export.s3-eu-west-1.amazonaws.com/tranquility/sde.zip";
static constexpr const char* SDE_ZIP_PATH = "Ressources/sde/sde.zip";
static constexpr const char* SDE_EXTRACTED_PATH = "Ressources/sde/";

static constexpr std::array< const char*, 11 > currentDataLoadingStep = {
	"Starting...",
    "Downloading SDE...",
    "Extracting SDE...",
    "Loading item types...",
    "Loading item groups...",
    "Loading item categories...",
    "Loading blueprints...",
    "Loading attributes...",
    "Loading dogma attributes...",
    "Loading dogma effects...",
    "Finalizing..."
};

DataLoader::DataLoader( QObject* parent )
	: QObject( parent )
	, sdeZipFile_( SDE_ZIP_PATH )
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
        LOG_WARNING( "SDE download failed." );
		emit ErrorOccurred( "Failed to download SDE." );
        return;
    }
	LOG_NOTICE( "SDE downloaded successfully." );
	LoadingStepProgressed();
}

void DataLoader::LoadingStepProgressed()
{
    ++currentDataLoadingStep_;
	emit MainDataLoadingStepChanged( currentDataLoadingStep_, static_cast< int >( currentDataLoadingStep.size() ),
        currentDataLoadingStep[ currentDataLoadingStep_ - 1 ] );
}