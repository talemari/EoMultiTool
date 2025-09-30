#pragma once
#include <QWidget>

class RessourcesManager;
class QComboBox;

class IndustryPage : public QWidget
{
    Q_OBJECT

public:
    explicit IndustryPage( std::shared_ptr< RessourcesManager > ressourcesManager, QWidget* parent = nullptr );
    ~IndustryPage() override = default;

private:
    QComboBox* BuildBlueprintsComboBox();

private:
    std::shared_ptr< RessourcesManager > ressourcesManager_;
};
