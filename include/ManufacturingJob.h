#pragma once
#include "HelperTypes.h"

#include <map>

class QJsonObject;

class ManufacturingJob
{
public:
    ManufacturingJob() = default;
    ManufacturingJob( const QJsonObject& jsonData );
    ~ManufacturingJob() = default;

    void FromJsonObject( const QJsonObject& jsonData );
    QJsonObject ToJsonObject() const;

    const std::vector< WithQuantity< tTypeId > >& GetComponents() const;
    const std::vector< WithQuantity< tTypeId > >& GetRawMaterials() const;
    const std::vector< WithQuantity< tTypeId > >& GetManufacturedProducts() const;
    const std::vector< WithQuantity< tTypeId > >& GetFullMaterialList() const;

    std::map< tTypeId, unsigned int > GetRecursedRawMaterialList() const;

    bool IsValid() const;

    void FilterComponents();

private:
    bool isValid_ = false;
    bool componentsFiltered_ = false;
    unsigned int timeInSeconds_ = 0;
    std::vector< WithQuantity< tTypeId > > manufacturedProducts_;
    std::vector< WithQuantity< tTypeId > > matRequirements_;
    std::vector< WithQuantity< tTypeId > > components_;
    std::vector< WithQuantity< tTypeId > > rawMaterials_;
};
