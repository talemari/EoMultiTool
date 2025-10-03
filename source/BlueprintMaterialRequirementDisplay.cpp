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
    auto job = blueprint->GetManufacturingJob();
    const auto rawMaterials = job->GetRawMaterials();
    for ( const auto& matReq : rawMaterials )
    {
        const EveType& matType = *GlobalRessources::GetTypeById( matReq.item );

        QString matName = QString::fromStdString( matType.GetName() );
        int basePrice = matType.GetBasePrice();
        auto* newChild = new QTreeWidgetItem( { matName, QString::number( matReq.quantity ), QString::number( basePrice ) } );
        parent->addChild( newChild );
    }
    const auto components = job->GetComponents();
    for ( auto& component : components )
    {
        const EveType& compType = *GlobalRessources::GetTypeById( component.item );
        const std::shared_ptr< const Blueprint > componentBlueprint = GlobalRessources::GetBlueprintById( compType.GetSourceBlueprintId() );
        QTreeWidgetItem* compItem =
            new QTreeWidgetItem( parent, { QString::fromStdString( compType.GetName() ), QString::number( component.quantity ), "0" } );
        AddMaterialsToTree( componentBlueprint, compItem );
        LOG_NOTICE( "Added component {} to list", componentBlueprint->GetName().toStdString() );
    }
}

void BlueprintMaterialRequirementDisplay::SetBlueprint( const std::shared_ptr< const Blueprint > blueprint )
{
    materialsTree_->clear();
    QTreeWidgetItem* root = new QTreeWidgetItem( materialsTree_, { blueprint->GetName(), "1", "0" } );
    QTreeWidgetItem* totalRawMats = new QTreeWidgetItem( root, { tr( "Total raw materials" ), "", "" } );
    QTreeWidgetItem* details = new QTreeWidgetItem( root, { tr( "Details" ), "", "" } );
    AddMaterialsToTree( blueprint, details );

    for ( const auto& [ matTypeId, quantity ] : blueprint->GetManufacturingJob()->GetRecursedRawMaterialList() )
    {
        const auto matType = GlobalRessources::GetTypeById( matTypeId );
        QString matName = QString::fromStdString( matType->GetName() );
        int averagePrice = matType->GetMarketPrice().averagePrice;
        auto* newChild = new QTreeWidgetItem( { matName, QString::number( quantity ), QString::number( averagePrice ) } );
        totalRawMats->addChild( newChild );
    }

    materialsTree_->update();
}
