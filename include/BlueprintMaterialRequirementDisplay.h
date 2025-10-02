#pragma once
#include "EveType.h"
#include "HelperTypes.h"

#include <QGroupBox>

#include <map>

class QTreeWidget;
class QTreeWidgetItem;

class Blueprint;
class RessourcesManager;

class BlueprintMaterialRequirementDisplay : public QGroupBox
{
    Q_OBJECT
public:
    explicit BlueprintMaterialRequirementDisplay( QWidget* parent = nullptr );
    ~BlueprintMaterialRequirementDisplay() override = default;

public slots:
    void SetBlueprint( const std::shared_ptr< const Blueprint > blueprint );

private:
    void AddMaterialsToTree( const std::shared_ptr< const Blueprint > blueprint, QTreeWidgetItem* parent );

private:
    QTreeWidget* materialsTree_;
    std::map< EveType, unsigned int > totalRawMaterials_;
};
