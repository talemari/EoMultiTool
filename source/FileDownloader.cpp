#include "FileDownloader.h"
#include "LogManager.h"

#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

FileDownloader::FileDownloader( QObject* parent /* = nullptr */)
	: QObject( parent )
{
	connect( this, &FileDownloader::DownloadFinished, [ this ]() { isDownloading_ = false; } );
}

void FileDownloader::Start( const QString& targetPath, const QString& fileUrl )
{
    if ( isDownloading_ )
    {
        LOG_WARNING( "A download is already in progress." );
        return;
    }
    targetPath_ = targetPath;
    fileUrl_ = fileUrl;
    networkManager_ = new QNetworkAccessManager( this );
    isDownloading_ = true;
	DownloadFile();
}

void FileDownloader::OnDownloadFinished()
{
    downloadedFile_.close();
    if ( networkReply_->error() != QNetworkReply::NoError )
    {
        LOG_WARNING( "Reply error : {}", networkReply_->errorString().toStdString() );
        emit DownloadFinished( false );
        networkReply_->deleteLater();
    }
    emit DownloadFinished( true );
    networkManager_->deleteLater();
}

void FileDownloader::DownloadFile()
{
	LOG_NOTICE( "Starting file download from URL: {}", fileUrl_.toStdString() );
    if ( QFile::exists( targetPath_ ) )
    {
		LOG_NOTICE( "File {} already exists, skipping download.", targetPath_.toStdString() );
        emit DownloadFinished( true );
        return;
    }

    QFileInfo fileInfo( targetPath_ );
    QDir dir;
    if ( !dir.mkpath( fileInfo.absolutePath() ) ) {
        LOG_WARNING( "Failed to create directory {}", fileInfo.absolutePath().toStdString() );
        emit DownloadFinished( false );
        return;
    }

	downloadedFile_.setFileName( targetPath_ );
    if ( !downloadedFile_.open( QIODevice::WriteOnly ) ) {
        emit DownloadFinished( false );
		LOG_WARNING( "Failed to open file {} for writing.", targetPath_.toStdString() );
        return;
    }

    QUrl url( fileUrl_ );
    QNetworkRequest request( url );
    networkReply_ = networkManager_->get( request );

    connect( networkReply_, &QNetworkReply::readyRead, [ this ]() { downloadedFile_.write( networkReply_->readAll() ); } );
    connect( networkReply_, &QNetworkReply::finished, this, &FileDownloader::OnDownloadFinished );
    connect( networkReply_, &QNetworkReply::downloadProgress, [ this ]( qint64 a, qint64 b ) { emit DownloadProgress(a, b); } );
    connect( networkReply_, &QNetworkReply::errorOccurred,
        [ this ]() { LOG_WARNING( "Error while downloading : {}", networkReply_->errorString().toStdString() ); } );
    connect( networkReply_, &QNetworkReply::sslErrors,
        [ this ]() { LOG_WARNING( "Error while downloading : {}", networkReply_->errorString().toStdString() ); } );
	LOG_NOTICE( "Downloading file into {}", targetPath_.toStdString() );
}
