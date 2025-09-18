#pragma once
#include <functional>
#include <unordered_map>

#include <QMainWindow>
#include <QTimer>

class QLabel;
class QScrollArea;
class QStackedWidget;
class QVBoxLayout;

class SideMenu;
class DataLoader;
class DataLoadingWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( const QString& appName, const QString& version, QWidget* parent = nullptr );
    ~MainWindow();

protected:
    bool eventFilter( QObject* target, QEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;

    void AddKeyboardEvent( Qt::Key key, std::function< void( void ) > action );
	void AddStylesheet( const QString& sheet );
    QString LoadStyleSheet( const QString& path );
	void ReloadStylesheets();

    void SetFullscreenMode( bool fullscreen );
    void AddPage( QWidget* page );
    void GoToPage( int index );
    SideMenu* GetSideMenu() const;

    bool GetIsFullScreen() const;

private:
    void BuildBaseInterface();
	void StartDataLoading();

signals:
    void ClickedOnWindow();

private:
    QString appName_;
    QString version_;
    QVBoxLayout* mainLayout_;
    QWidget* centralWidget_;
    QTimer inactivityTimer_;
    bool isFullScreen_;
    std::unordered_map< int, std::vector< std::function< void( void ) > > > keyFunctionsMap_;
    QApplication* app_;
    QStackedWidget* pages_ = nullptr;
    SideMenu* sideMenu_ = nullptr;
    QStringList activeStylesheets_;
	QThread* dataLoadingThread_;
};
