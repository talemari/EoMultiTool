#include "MainWindow.h"
#include "DataLoadingWidget.h"
#include "IndustryPage.h"
#include "LogManager.h"
#include "RessourcesManager.h"
#include "SideMenu.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QProgressBar>
#include <QStackedWidget>
#include <QThread>

MainWindow::MainWindow( const QString& appName, const QString& version, QWidget* parent )
    : QMainWindow( parent )
    , appName_( appName )
    , version_( version )
    , isFullScreen_( false )
    , app_( nullptr )
    , mainLayout_( new QVBoxLayout )
    , centralWidget_( new QWidget )
    , pages_( new QStackedWidget )
    , sideMenu_( new SideMenu )
    , settings_( QDir( QCoreApplication::applicationDirPath() ).filePath( "settings.ini" ), QSettings::IniFormat )
    , ressourcesManager_( std::make_unique< RessourcesManager >( settings_ ) )
    , industryPage_( nullptr )
{
    app_ = dynamic_cast< QApplication* >( QApplication::instance() );
    installEventFilter( this );

    setWindowTitle( appName_ + " version : " + version_ );
    setFixedSize( 1280, 720 );

    BuildBaseInterface();
    AddStylesheet( ":/Ressources/Space.qss" );
    ReloadStylesheets();

    AddKeyboardEvent( Qt::Key_F, [ this ]() { SetFullscreenMode( !GetIsFullScreen() ); } );
    AddKeyboardEvent( Qt::Key_Escape, [ this ]() { emit close(); } );
    AddKeyboardEvent( Qt::Key_S, [ this ]() { ReloadStylesheets(); } );

    sideMenu_->AddItem( tr( "Home" ) );
    sideMenu_->AddItem( tr( "Industry" ) );

    LOG_NOTICE( "Main window initialized" );

    StartDataLoading();
}

MainWindow::~MainWindow()
{
    dataLoadingThread_->quit();
    dataLoadingThread_->wait();
}

bool MainWindow::eventFilter( QObject* target, QEvent* event )
{
    QKeyEvent* keyEvent = static_cast< QKeyEvent* >( event );

    if ( event->type() == QEvent::KeyPress )
    {
        if ( keyFunctionsMap_.count( keyEvent->key() ) > 0 )
        {
            for ( const auto& action : keyFunctionsMap_[ keyEvent->key() ] )
                action();
        }
    }
    if ( event->type() == QEvent::MouseButtonPress )
        emit ClickedOnWindow();

    return QObject::eventFilter( target, event );
}

void MainWindow::resizeEvent( QResizeEvent* event )
{
    QMainWindow::resizeEvent( event );
    sideMenu_->setMaximumWidth( static_cast< int >( width() * 0.15 ) );
    pages_->setFixedHeight( height() );
    pages_->setMaximumWidth( width() - ( sideMenu_->isVisible() ? sideMenu_->width() : 0 ) );
    LOG_NOTICE( "Resize event . Final size {} x {}", width(), height() );
}

SideMenu* MainWindow::GetSideMenu() const
{
    return sideMenu_;
}

void MainWindow::AddKeyboardEvent( Qt::Key key, std::function< void( void ) > action )
{
    int keyCode = static_cast< int >( key );

    keyFunctionsMap_[ keyCode ].push_back( action );
}

void MainWindow::AddStylesheet( const QString& sheet )
{
    if ( !activeStylesheets_.contains( sheet ) && !sheet.isEmpty() )
        activeStylesheets_.push_back( sheet );

    LOG_NOTICE( "Added stylesheet {}", sheet.toStdString() );
}

QString MainWindow::LoadStyleSheet( const QString& path )
{
    QFile styleSheetFile( path );
    if ( !styleSheetFile.open( QIODevice::ReadOnly ) )
    {
        LOG_WARNING( "Failed to open stylesheet {}", QFileInfo( styleSheetFile ).absoluteFilePath().toStdString() );
        return "";
    }
    QString result = styleSheetFile.readAll();
    styleSheetFile.close();
    LOG_NOTICE( "Loaded stylesheet {}", path.toStdString() );
    return result;
}

void MainWindow::ReloadStylesheets()
{
    QString newStyle;
    for ( const auto& stylesheet : activeStylesheets_ )
        newStyle += LoadStyleSheet( stylesheet );
    app_->setStyleSheet( newStyle );

    for ( QWidget* widget : QApplication::allWidgets() )
        widget->update();
}

void MainWindow::SetFullscreenMode( bool fullscreen )
{
    if ( fullscreen )
        setWindowState( Qt::WindowFullScreen );
    else
        setWindowState( Qt::WindowMaximized );

    isFullScreen_ = fullscreen;
}

void MainWindow::AddPage( QWidget* page )
{
    pages_->addWidget( page );
}

void MainWindow::GoToPage( int index )
{
    assert( index < pages_->count() );
    pages_->setCurrentIndex( index );
}

bool MainWindow::GetIsFullScreen() const
{
    return isFullScreen_;
}

void MainWindow::BuildBaseInterface()
{
    QHBoxLayout* menuAndPagesLayout = new QHBoxLayout();

    menuAndPagesLayout->addWidget( sideMenu_ );
    menuAndPagesLayout->addWidget( pages_ );
    mainLayout_->addLayout( menuAndPagesLayout );
    mainLayout_->setContentsMargins( 0, 0, 0, 0 );

    centralWidget_->setLayout( mainLayout_ );

    setCentralWidget( centralWidget_ );
    sideMenu_->hide();
}

void MainWindow::OnDataLoadingFinished()
{
    LOG_NOTICE( "Quitting data loading thread" );
    dataLoadingThread_->quit();
    dataLoadingThread_->wait();
    LOG_NOTICE( "Thread closed" );
    dataLoadingThread_->deleteLater();
    ressourcesManager_->deleteLater();

    industryPage_ = new IndustryPage();
    AddPage( industryPage_ );
    sideMenu_->show();
    GoToPage( 1 );
    LOG_NOTICE( "Data loading finished" );
}

void MainWindow::StartDataLoading()
{
    DataLoadingWidget* dataLoadingWidget = new DataLoadingWidget;
    AddPage( dataLoadingWidget );
    GoToPage( pages_->count() - 1 );

    dataLoadingThread_ = new QThread( this );
    ressourcesManager_->moveToThread( dataLoadingThread_ );

    connect( ressourcesManager_.get(),
             &RessourcesManager::RessourcesLoadingMainStepChanged,
             dataLoadingWidget,
             &DataLoadingWidget::OnMainDataLoadingStepChanged,
             Qt::QueuedConnection );
    connect( ressourcesManager_.get(),
             &RessourcesManager::RessourcesLoadingSubStepChanged,
             dataLoadingWidget,
             &DataLoadingWidget::OnSubDataLoadingStepChanged,
             Qt::QueuedConnection );
    connect( ressourcesManager_.get(),
             &RessourcesManager::ErrorOccured,
             dataLoadingWidget,
             &DataLoadingWidget::OnErrorOccurred,
             Qt::QueuedConnection );
    connect(
        ressourcesManager_.get(), &RessourcesManager::RessourcesReady, this, &MainWindow::OnDataLoadingFinished, Qt::QueuedConnection );

    connect( dataLoadingThread_, &QThread::started, ressourcesManager_.get(), &RessourcesManager::LoadRessources );
    dataLoadingThread_->start();
}
