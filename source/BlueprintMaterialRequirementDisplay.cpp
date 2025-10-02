#include "BlueprintMaterialRequirementDisplay.h"
#include "Blueprint.h"
#include "GlobalRessources.h"
#include "LogManager.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

BlueprintMaterialRequirementDisplay::BlueprintMaterialRequirementDisplay( QWidget* parent )
    : QGroupBox( parent )
    , materialsTree_( new QTreeWidget( this ) )
{
    QVBoxLayout* mainLayout = new QVBoxLayout( this );
    materialsTree_->setColumnCount( 3 );
    materialsTree_->setHeaderLabels( { tr( "Material" ), tr( "Quantity" ), tr( "Base price" ) } );

    mainLayout->addWidget( materialsTree_ );
}

void BlueprintMaterialRequirementDisplay::AddMaterialsToTree( const std::shared_ptr< const Blueprint > blueprint, QTreeWidgetItem* parent )
{
    const auto& materials = blueprint->GetManufacturingJob().matRequirements;
    std::vector< WithQuantity< EveType > > components;
    for ( const auto& matReq : materials )
    {
        const EveType& matType = *GlobalRessources::GetTypeById( matReq.item );
        if ( matType.IsManufacturable() )
        {
            components.emplace_back( matType, matReq.quantity );
            continue;
        }

        if ( totalRawMaterials_.contains( matType ) )
        {
            totalRawMaterials_.at( matType ) += matReq.quantity;
            continue;
        }
        else
            totalRawMaterials_[ matType ] = matReq.quantity;

        QString matName = QString::fromStdString( matType.GetName() );
        int basePrice = matType.GetBasePrice();
        auto* newChild = new QTreeWidgetItem( { matName, QString::number( matReq.quantity ), QString::number( basePrice ) } );
        parent->addChild( newChild );
    }
    for ( auto& component : components )
    {
        const std::shared_ptr< const Blueprint > componentBlueprint =
            GlobalRessources::GetBlueprintById( component.item.GetSourceBlueprintId() );
        QTreeWidgetItem* compItem = new QTreeWidgetItem(
            parent, { QString::fromStdString( component.item.GetName() ), QString::number( component.quantity ), "0" } );
        AddMaterialsToTree( componentBlueprint, compItem );
        LOG_NOTICE( "Added component {} to list", componentBlueprint->GetName().toStdString() );
    }
}

void BlueprintMaterialRequirementDisplay::SetBlueprint( const std::shared_ptr< const Blueprint > blueprint )
{
    materialsTree_->clear();
    totalRawMaterials_.clear();
    QTreeWidgetItem* root = new QTreeWidgetItem( materialsTree_, { blueprint->GetName(), "1", "0" } );
    QTreeWidgetItem* totalRawMats = new QTreeWidgetItem( root, { tr( "Total raw materials" ), "", "" } );
    QTreeWidgetItem* details = new QTreeWidgetItem( root, { tr( "Details" ), "", "" } );
    AddMaterialsToTree( blueprint, details );

    for ( const auto& [ matType, quantity ] : totalRawMaterials_ )
    {
        QString matName = QString::fromStdString( matType.GetName() );
        int basePrice = matType.GetBasePrice();
        auto* newChild = new QTreeWidgetItem( { matName, QString::number( quantity ), QString::number( basePrice ) } );
        totalRawMats->addChild( newChild );
    }

    materialsTree_->update();
}
