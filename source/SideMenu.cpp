// SideMenu.cpp
#include "LogManager.h"
#include "SideMenu.h"

#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QVBoxLayout>

static void setCircularPixmap( QLabel* label, const QPixmap& pixmap, int size )
{
    if ( !label || pixmap.isNull() || size <= 0 ) return;

    QPixmap scaled = pixmap.scaled( size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
    QPixmap circular( size, size );
    circular.fill( Qt::transparent );

    QPainter p( &circular );
    p.setRenderHint( QPainter::Antialiasing, true );
    p.setRenderHint( QPainter::SmoothPixmapTransform, true );

    QPainterPath path;
    path.addEllipse( 0, 0, size, size );
    p.setClipPath( path );
    p.drawPixmap( 0, 0, scaled );

    label->setPixmap( circular );
}

SideMenu::SideMenu( QWidget* parent )
    : QFrame( parent )
    , menu_( new QListWidget( this ) )
    , layout_( new QVBoxLayout( this ) )
    , userIconLabel_( new QLabel( this ) )
    , nameLabel_( new QLabel( tr( "Nobody connected" ), this ) )
    , footerLabel_( new QLabel( this ) )
    , timer_( new QTimer( this ) )
{
    setObjectName( "SideMenu" );
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );

    menu_->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    menu_->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    menu_->setSelectionMode( QAbstractItemView::SingleSelection );
    menu_->setFocusPolicy( Qt::NoFocus );

    layout_->setContentsMargins( 12, 12, 8, 12 );
    layout_->setSpacing( 12 );

    layout_->addWidget( userIconLabel_, 0, Qt::AlignHCenter | Qt::AlignTop );
    layout_->addWidget( nameLabel_, 0, Qt::AlignHCenter | Qt::AlignTop );
    layout_->addSpacing( 24 );
    layout_->addWidget( menu_, 1 );
    layout_->addWidget( footerLabel_, 0, Qt::AlignHCenter | Qt::AlignBottom );
    setLayout( layout_ );

    userIcon_ = QPixmap( ":/Ressources/images/emptyPortrait.jpeg" );

    connect( timer_, &QTimer::timeout, this, [ this ]() {
        footerLabel_->setText( QDateTime::currentDateTime().toString( "ddd, dd/MM/yy / hh:mm:ss" ) );
        } );
    timer_->start( 1000 );

    connect( menu_, &QListWidget::itemActivated, this, &SideMenu::LocalItemActivated );
}

QListWidgetItem* SideMenu::AddItem( const QString& itemName )
{
    menu_->addItem( itemName );
    return menu_->item( menu_->count() - 1 );
}

QListWidgetItem* SideMenu::AddItemWithAction( const QString& itemName, std::function<void( void )> action )
{
    auto* item = new QListWidgetItem( itemName );
    return AddItemWithAction( item, std::move( action ) );
}

QListWidgetItem* SideMenu::AddItemWithAction( QListWidgetItem* item, std::function<void( void )> action )
{
    itemsActions_.push_back( { item, std::move( action ) } );
    menu_->addItem( item );
    return item;
}

void SideMenu::RemoveItem( const QString& itemName )
{
    for ( auto it = itemsActions_.begin(); it != itemsActions_.end(); ++it ) {
        if ( it->item && it->item->text() == itemName ) {
            RemoveItem( it->item );
            return;
        }
    }
}

void SideMenu::RemoveItem( QListWidgetItem* item )
{
    itemsActions_.erase( std::remove_if( itemsActions_.begin(), itemsActions_.end(),
        [ item ]( const ItemAction& a ) { return a.item == item; } ),
        itemsActions_.end() );

    int row = menu_->row( item );
    if ( row >= 0 ) {
        LOG_NOTICE( "Removing menu item : \" {} \"", item->text().toStdString() );
        delete menu_->takeItem( row );
    }
}

void SideMenu::SetConnectedUser( const QString& name, const QString& surname, const QString& poste )
{
    nameLabel_->setText( name + " " + surname + "\n" + poste );
    currentIcon_ = ( poste == "Bureaux" ) ? 3u : 1u;
    setCircularPixmap( userIconLabel_, userIcon_, width() / 2 );
}

void SideMenu::SetUsersWidgetVisible( bool isVisible )
{
    userIconLabel_->setVisible( isVisible );
    nameLabel_->setVisible( isVisible );
}

void SideMenu::resizeEvent( QResizeEvent* event )
{
    QFrame::resizeEvent( event );
    setCircularPixmap( userIconLabel_, userIcon_, width() / 2 );
}

void SideMenu::ShowWidget( QWidget* menu, QListWidgetItem* item )
{
    if ( currentMenu_ && currentMenu_ == menu ) {
        ClearSelection();
        return;
    }
    if ( !menu || !item ) return;

    if ( currentMenu_ ) {
        currentMenu_->hide();
        currentMenu_ = nullptr;
    }

    currentMenu_ = menu;

    const QRect itemRect = menu_->visualItemRect( item );
    QPoint p = menu_->viewport()->mapTo( parentWidget(), itemRect.topRight() );

    p.setX( geometry().right() );
    currentMenu_->move( p );

    const int maxY = parentWidget()->height() - currentMenu_->height();
    if ( currentMenu_->y() > maxY ) currentMenu_->move( currentMenu_->x(), maxY );

    currentMenu_->show();
    currentMenu_->raise();
}

void SideMenu::ClearSelection()
{
    if ( currentMenu_ ) {
        currentMenu_->hide();
        currentMenu_ = nullptr;
    }
    menu_->clearSelection();
}

void SideMenu::LocalItemActivated( QListWidgetItem* item )
{
    if ( !item ) return;
    for ( auto& a : itemsActions_ ) {
        if ( a.item == item && a.action ) {
            a.action();
            break;
        }
    }
}
