#include "IndustryPage.h"
#include "Blueprint.h"
#include "EveType.h"
#include "RessourcesManager.h"

#include <QComboBox>
#include <QVBoxLayout>

IndustryPage::IndustryPage( std::shared_ptr< RessourcesManager > ressourcesManager, QWidget* parent )
    : QWidget( parent )
    , ressourcesManager_( ressourcesManager )
{
    QVBoxLayout* mainLayout = new QVBoxLayout( this );
    QComboBox* blueprintSelectionCombobox = BuildBlueprintsComboBox();
    mainLayout->addWidget( blueprintSelectionCombobox );
}

QComboBox* IndustryPage::BuildBlueprintsComboBox()
{
    QComboBox* result = new QComboBox();
    auto& blueprintMap = ressourcesManager_->GetBlueprintsMap();

    for ( const auto& [ typeId, blueprint ] : blueprintMap )
    {
        QString blueprintName = QString::fromStdString( ressourcesManager_->GetTypeById( typeId )->GetName() );
        result->addItem( blueprintName, QVariant::fromValue( typeId ) );
    }

    return result;
}
