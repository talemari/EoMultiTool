#pragma once
#include <QWidget>

class QLabel;
class QProgressBar;

class DataLoadingWidget : public QWidget
{
	Q_OBJECT
public:
	DataLoadingWidget( QWidget* parent = nullptr );
	~DataLoadingWidget() = default;

public slots:
	void OnMainDataLoadingStepChanged( int currentStep, int maxStep, const QString& description );
	void OnSubDataLoadingStepChanged( int currentStep, int maxStep, const QString& description );
	void OnErrorOccurred( const QString& errorMessage );

private:
	QProgressBar* mainProgressBar_;
	QProgressBar* subProgressBar_;
	QLabel* mainProgressLabel_;
	QLabel* subProgressLabel_;
};

