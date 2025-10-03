#pragma once
#include "HelperTypes.h"
#include "JsonEveInterface.h"
#include "ManufacturingJob.h"

#include <memory>
#include <vector>

class QJsonObject;

class Blueprint : public JsonEveInterface
{
public:
    Blueprint() = default;
    Blueprint( const QJsonObject& jsonData );
    ~Blueprint() = default;

    void FromJsonObject( const QJsonObject& jsonData ) override;
    QJsonObject ToJsonObject() const override;
    void PostLoadingInitialization() override;

    const std::shared_ptr< ManufacturingJob > GetManufacturingJob() const;

private:
    double matEfficiency_ = 0.0;
    double timeEfficiency_ = 0.0;
    std::shared_ptr< ManufacturingJob > manufacturingJob_ = nullptr;
};
