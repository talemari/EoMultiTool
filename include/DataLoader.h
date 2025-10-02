#pragma once
#include "FileDownloader.h"
#include "HelperTypes.h"

#include <QJsonObject>

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

    QString GetSdeExtractedPath() const;
    QJsonObject&& GetMarketPricesJson();

signals:
    void MainDataLoadingStepChanged( eDataLoadingSteps step );
    void SubDataLoadingStepChanged( int currentStep, int maxStep, const QString& description );
    void ErrorOccurred( const QString& errorMessage );
    void DataLoadingFinished();

    void SdeDownloaded();
    void SdeExtracted();
    void SdeValidated();
    void MarketPricesReady();

private slots:
    void OnSdeDownloadFinished( bool isSuccess );
    void ExtractSde();
    void ValidateSde();
    void DownloadMarketPrices();

private:
    void MarketPricesDownloaded( QByteArray data );
    void DownloadSde();
    void SetLoadingStep( eDataLoadingSteps step );
    void TriggerError( const QString& errorMessage );

private:
    FileDownloader* sdeDownloader_ = nullptr;
    FileDownloader* marketPricesDownloader_ = nullptr;
    eDataLoadingSteps currentDataLoadingStep_ = eDataLoadingSteps::Waiting;
    QJsonObject marketPricesJson_;

    const QString sdeExtractedPath_;
    const QString sdeZipPath_;
};
