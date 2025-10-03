#pragma once
#include "LPHelper.h"

#include <QGroupBox>

class Blueprint;

class QTableWidget;

class CompressedOreWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit CompressedOreWidget( QWidget* parent = nullptr );
    ~CompressedOreWidget() override = default;

public slots:
    void SetBlueprint( const std::shared_ptr< Blueprint > blueprint );

private:
    const std::shared_ptr< Blueprint > blueprint_;
    LPHelper blueprintRequirementSolver_;
    QTableWidget* compressedOreTable_;
    QTableWidget* leftoverTable_;
};
