#pragma once
#include "FileDownloader.h"

class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QProgressBar;
class QThread;

class RessourcesManager;

enum class eDataLoadingSteps
{
    Waiting,
    DownloadingSde,
    ExtractingSde,
    ValidatingSde,
    LoadingYamlFiles,
    LoadingItemGroups,
    LoadingItemCategories,
    LoadingBlueprints,
    LoadingAttributes,
    LoadingDogmaAttributes,
    LoadingDogmaEffects,
    Finalizing,
    Count
};

class DataLoader : public QObject
{
    Q_OBJECT
public:
    DataLoader( std::shared_ptr< RessourcesManager > ressourcesManager, QObject* parent = nullptr );
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
    void YamlsLoaded();

private slots:
    void OnSdeDownloadFinished( bool isSuccess );
    void ExtractSde();
    void ValidateSde();
    void LoadYamls();

private:
    void DownloadSde();
    void SetLoadingStep( eDataLoadingSteps step );
    void TriggerError( const QString& errorMessage );

private:
    std::shared_ptr< RessourcesManager > ressourcesManager_;
    FileDownloader* fileDownloader_ = nullptr;
    eDataLoadingSteps currentDataLoadingStep_ = eDataLoadingSteps::Waiting;
    const QString sdeExtractedPath_;
    const QString sdeZipPath_;
};
