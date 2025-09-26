#pragma once
#include "FileDownloader.h"
#include "HelperTypes.h"

class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QProgressBar;
class QThread;

class RessourcesManager;

class DataLoader : public QObject
{
    Q_OBJECT
public:
    DataLoader( QObject* parent = nullptr );
    ~DataLoader() override = default;

    void StartDataLoading();

signals:
    void MainDataLoadingStepChanged( eDataLoadingSteps step );
    void SubDataLoadingStepChanged( int currentStep, int maxStep, const QString& description );
    void ErrorOccurred( const QString& errorMessage );
    void DataLoadingFinished();

    void SdeDownloaded();
    void SdeExtracted();
    void SdeValidated( const QString& sdePath );

private slots:
    void OnSdeDownloadFinished( bool isSuccess );
    void ExtractSde();
    void ValidateSde();

private:
    void DownloadSde();
    void SetLoadingStep( eDataLoadingSteps step );
    void TriggerError( const QString& errorMessage );

private:
    FileDownloader* fileDownloader_ = nullptr;
    eDataLoadingSteps currentDataLoadingStep_ = eDataLoadingSteps::Waiting;
    const QString sdeExtractedPath_;
    const QString sdeZipPath_;
};
