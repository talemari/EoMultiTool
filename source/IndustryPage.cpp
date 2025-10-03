#include "IndustryPage.h"
#include "Blueprint.h"
#include "BlueprintMaterialRequirementDisplay.h"
#include "CompressedOreWidget.h"
#include "EveType.h"
#include "GlobalRessources.h"
#include "RessourcesManager.h"

#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>

IndustryPage::IndustryPage( QWidget* parent )
    : QWidget( parent )
    , blueprintMaterialRequirementDisplay_( new BlueprintMaterialRequirementDisplay( this ) )
    , compressedOreWidget_( new CompressedOreWidget( this ) )
{
    QGridLayout* mainLayout = new QGridLayout( this );
    QComboBox* blueprintSelectionCombobox = BuildBlueprintsComboBox();
    mainLayout->addWidget( blueprintSelectionCombobox, 0, 0, 1, 2 );
    mainLayout->addWidget( blueprintMaterialRequirementDisplay_, 1, 0, 1, 1 );
    mainLayout->addWidget( compressedOreWidget_, 1, 2, 1, 1 );
}

QComboBox* IndustryPage::BuildBlueprintsComboBox()
{
    QComboBox* result = new QComboBox();
    auto& blueprintMap = GlobalRessources::GetBlueprintsMap();
    result->setEditable( true );

    for ( const auto& [ typeId, blueprint ] : blueprintMap )
    {
        QString blueprintName = blueprint->GetName();
        result->addItem( blueprintName, QVariant::fromValue( typeId ) );
    }
    connect( result,
             QOverload< int >::of( &QComboBox::currentIndexChanged ),
             this,
             [ this, result ]( int index )
             {
                 if ( index < 0 )
                     return;
                 tTypeId typeId = result->currentData().toUInt();
                 const std::shared_ptr< Blueprint > blueprint = GlobalRessources::GetBlueprintById( typeId );
                 if ( blueprint != nullptr )
                 {
                     blueprintMaterialRequirementDisplay_->SetBlueprint( blueprint );
                     compressedOreWidget_->SetBlueprint( blueprint );
                 }
             } );

    return result;
}
