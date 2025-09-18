// SideMenu.h  (unchanged API; small tweaks inside)
#pragma once
#include <functional>
#include <QFrame>
#include <vector>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QTimer;
class QVBoxLayout;

class SideMenu : public QFrame
{
    Q_OBJECT
        struct ItemAction {
        QListWidgetItem* item = nullptr;
        std::function<void( void )> action;
    };

public:
    explicit SideMenu( QWidget* parent = nullptr );
    ~SideMenu() override = default;

    QListWidgetItem* AddItem( const QString& itemName );
    QListWidgetItem* AddItemWithAction( const QString& itemName, std::function<void( void )> action );
    QListWidgetItem* AddItemWithAction( QListWidgetItem* item, std::function<void( void )> action );

    void RemoveItem( const QString& itemName );
    void RemoveItem( QListWidgetItem* item );

    void SetConnectedUser( const QString& name, const QString& surname, const QString& poste );
    void SetUsersWidgetVisible( bool isVisible );

protected:
    void resizeEvent( QResizeEvent* event ) override;

private:
    void ShowWidget( QWidget* menu, QListWidgetItem* item );

public slots:
    void ClearSelection();

private slots:
    void LocalItemActivated( QListWidgetItem* item );

private:
    QListWidget* menu_ = nullptr;
    QVBoxLayout* layout_ = nullptr;
    QLabel* userIconLabel_ = nullptr;
    QLabel* nameLabel_ = nullptr;
    QLabel* posteLabel_ = nullptr;  // currently unused
    QLabel* footerLabel_ = nullptr;
    QTimer* timer_ = nullptr;
    QWidget* currentMenu_ = nullptr;
    std::vector<ItemAction> itemsActions_;
    QPixmap userIcon_;        // value, not pointer
    unsigned int currentIcon_ = 0;
};
