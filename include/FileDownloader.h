#pragma once
#include <QFile>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader( QObject* parent = nullptr );
    ~FileDownloader() = default;

public slots:
    void Start( const QString& targetPath, const QString& fileUrl );

signals:
	void DownloadProgress( qint64 downloadedBytes, qint64 totalBytes );
    void DownloadFinished( bool isSuccess );

private slots:
    void OnDownloadFinished();

private:
    void DownloadFile();

private:
	bool isDownloading_ = false;
	QString targetPath_;
	QString fileUrl_;
    QFile downloadedFile_;
    QNetworkAccessManager* networkManager_ = nullptr;
    QNetworkReply* networkReply_ = nullptr;
};

