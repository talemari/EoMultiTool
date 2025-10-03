#include "CompressedOreWidget.h"
#include "EveType.h"
#include "GlobalRessources.h"

#include <QTableWidget>
#include <QVBoxLayout>

CompressedOreWidget::CompressedOreWidget( QWidget* parent )
    : QGroupBox( parent )
    , compressedOreTable_( new QTableWidget( this ) )
    , leftoverTable_( new QTableWidget( this ) )
    , blueprintRequirementSolver_( LPHelper( GlobalRessources::GetOresMap() ) )
{
    QVBoxLayout* mainLayout = new QVBoxLayout( this );

    compressedOreTable_->setColumnCount( 3 );
    compressedOreTable_->setHorizontalHeaderLabels( { tr( "Compressed ore" ), tr( "Quantity" ), tr( "Total price" ) } );
    leftoverTable_->setColumnCount( 3 );
    leftoverTable_->setHorizontalHeaderLabels( { tr( "Leftover material" ), tr( "Quantity" ), tr( "Total price" ) } );

    mainLayout->addWidget( compressedOreTable_ );
    mainLayout->addWidget( leftoverTable_ );
}

void CompressedOreWidget::SetBlueprint( const std::shared_ptr< Blueprint > blueprint )
{
    compressedOreTable_->clear();
    leftoverTable_->clear();

    if ( !blueprintRequirementSolver_.SolveForBlueprint( *blueprint ) )
    {
        QTableWidgetItem* errorIrem = new QTableWidgetItem( "Failed to solve the blueprint" );
        compressedOreTable_->setItem( 0, 0, errorIrem );
        return;
    }

    const auto& compressedOres = blueprintRequirementSolver_.GetResult();
    compressedOreTable_->setRowCount( static_cast< int >( compressedOres.size() ) );
    int row = 0;
    for ( const auto& [ oreTypeId, quantity ] : compressedOres )
    {
        const auto oreType = GlobalRessources::GetTypeById( oreTypeId );
        if ( !oreType )
            continue;
        QTableWidgetItem* oreNameItem = new QTableWidgetItem( QString::fromStdString( oreType->GetName() ) );
        QTableWidgetItem* quantityItem = new QTableWidgetItem( QString::number( quantity ) );
        QTableWidgetItem* totalPriceItem = new QTableWidgetItem( QString::number( quantity * oreType->GetMarketPrice().averagePrice ) );
        compressedOreTable_->setItem( row, 0, oreNameItem );
        compressedOreTable_->setItem( row, 1, quantityItem );
        compressedOreTable_->setItem( row, 2, totalPriceItem );
        row++;
    }

    const auto& leftovers = blueprintRequirementSolver_.GetLeftover();
    leftoverTable_->setRowCount( static_cast< int >( leftovers.size() ) );
    row = 0;
    for ( const auto& [ mineralTypeId, quantity ] : leftovers )
    {
        const auto mineralType = GlobalRessources::GetTypeById( mineralTypeId );
        if ( !mineralType )
            continue;
        QTableWidgetItem* mineralNameItem = new QTableWidgetItem( QString::fromStdString( mineralType->GetName() ) );
        QTableWidgetItem* quantityItem = new QTableWidgetItem( QString::number( quantity ) );
        QTableWidgetItem* totalPriceItem = new QTableWidgetItem( QString::number( quantity * mineralType->GetMarketPrice().averagePrice ) );
        leftoverTable_->setItem( row, 0, mineralNameItem );
        leftoverTable_->setItem( row, 1, quantityItem );
        leftoverTable_->setItem( row, 2, totalPriceItem );
        row++;
    }
}
