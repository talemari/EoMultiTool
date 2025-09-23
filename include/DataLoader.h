#pragma once
#include "FileDownloader.h"

class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QProgressBar;
class QThread;

class DataLoader : public QObject
{
    Q_OBJECT
public:
    DataLoader( QObject* parent = nullptr );
    ~DataLoader() override = default;

    void StartDataLoading();

signals:
    void MainDataLoadingStepChanged( int currentStep, int maxStep, const QString& description );
    void SubDataLoadingStepChanged( int currentStep, int maxStep, const QString& description );
    void ErrorOccurred( const QString& errorMessage );
    void DataLoadingFinished();

    void SdeDownloaded();
    void SdeExtracted();
    void SdeValidated();

private slots:
    void OnSdeDownloadFinished( bool isSuccess );
    void ExtractSde();
    void ValidateSde();

private:
    void DownloadSde();
    void LoadingStepProgressed();
    void TriggerError( const QString& errorMessage );

private:
    FileDownloader* fileDownloader_ = nullptr;
    QFile sdeZipFile_;
    unsigned int currentDataLoadingStep_ = 0;
};
