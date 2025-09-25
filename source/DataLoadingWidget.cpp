#include "DataLoadingWidget.h"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

DataLoadingWidget::DataLoadingWidget( QWidget* parent )
    : QGroupBox( parent )
    , mainProgressBar_( new QProgressBar )
    , subProgressBar_( new QProgressBar )
    , mainProgressLabel_( new QLabel( "Main Progress" ) )
    , subProgressLabel_( new QLabel( "Sub Progress" ) )
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget( mainProgressLabel_, Qt::AlignHCenter | Qt::AlignBottom );
    mainLayout->addWidget( mainProgressBar_ );
    mainLayout->addWidget( subProgressBar_ );
    mainLayout->addWidget( subProgressLabel_, Qt::AlignHCenter | Qt::AlignTop );
    setLayout( mainLayout );
}

void DataLoadingWidget::OnMainDataLoadingStepChanged( int currentStep, int maxStep, const QString& description )
{
    mainProgressBar_->setMaximum( maxStep );
    mainProgressBar_->setValue( currentStep );
    mainProgressLabel_->setText( QString( "%1 : %2/%3" ).arg( description ).arg( currentStep ).arg( maxStep ) );
}

void DataLoadingWidget::OnSubDataLoadingStepChanged( int currentStep, int maxStep, const QString& description )
{
    subProgressBar_->setMaximum( maxStep );
    subProgressBar_->setValue( currentStep );
    subProgressLabel_->setText( QString( "%1 : %2/%3" ).arg( description ).arg( currentStep ).arg( maxStep ) );
}

void DataLoadingWidget::OnErrorOccurred( const QString& errorMessage )
{
    mainProgressLabel_->setText( errorMessage );
    mainProgressBar_->hide();
    subProgressBar_->hide();
    subProgressLabel_->hide();
}
