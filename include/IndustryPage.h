#pragma once
#include <QWidget>

class RessourcesManager;
class BlueprintMaterialRequirementDisplay;

class QComboBox;

class IndustryPage : public QWidget
{
    Q_OBJECT

public:
    explicit IndustryPage( QWidget* parent = nullptr );
    ~IndustryPage() override = default;

private:
    QComboBox* BuildBlueprintsComboBox();

private:
    BlueprintMaterialRequirementDisplay* blueprintMaterialRequirementDisplay_;
};
