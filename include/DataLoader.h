#pragma once
#include "FileDownloader.h"

#include <QFile>
#include <QGroupBox>

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

private slots:
	void OnSdeDownloadFinished( bool isSuccess );

private:
	void LoadingStepProgressed();

private:
	FileDownloader* fileDownloader_ = nullptr;
	QFile sdeZipFile_;
	unsigned int currentDataLoadingStep_ = 0;
};

