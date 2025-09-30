#pragma once
#include "HelperTypes.h"
#include "JsonEveInterface.h"

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

class Blueprint : public JsonEveInterface
{
public:
    Blueprint() = default;
    Blueprint( const QJsonObject& jsonData );
    ~Blueprint() = default;

    void FromJsonObject( const QJsonObject& jsonData ) override;
    QJsonObject ToJsonObject() const override;

    ManufacturingJob GetManufacturingJob() const;

private:
    void ParseManufacturingJob( const QJsonObject& jsonData );

private:
    double matEfficiency_ = 0.0;
    double timeEfficiency_ = 0.0;
    ManufacturingJob manufacturingJob_;
};
