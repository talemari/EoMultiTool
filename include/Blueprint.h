#pragma once
#include "HelperTypes.h"

#include <memory>
#include <vector>

class QJsonObject;

struct ManufacturingJob
{
    bool isValid = false;
    unsigned int timeInSeconds = 0;
    std::vector< WithQuantity< tTypeId > > manufacturedProducts;
    std::vector< WithQuantity< tTypeId > > matRequirements;
};

class Blueprint
{
public:
    Blueprint() = default;
    Blueprint( const QJsonObject& jsonData );
    ~Blueprint() = default;

    void FromJsonObject( const QJsonObject& jsonData );

private:
    void ParseManufacturingJob( const QJsonObject& jsonData );

private:
    unsigned int typeId_ = 0;
    double matEfficiency_ = 0.0;
    double timeEfficiency_ = 0.0;
    ManufacturingJob manufacturingJob_;
};
