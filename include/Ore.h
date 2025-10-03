#pragma once
#include "HelperTypes.h"
#include "JsonEveInterface.h"

#include <vector>

class QJsonObject;

class Ore : public JsonEveInterface
{
public:
    Ore() = default;
    ~Ore() = default;
    Ore( const QJsonObject& jsonData );

    void FromJsonObject( const QJsonObject& jsonData );
    QJsonObject ToJsonObject() const override;
    void PostLoadingInitialization() override;

    const std::vector< WithQuantity< tTypeId > >& GetRefinedProducts() const;
    double GetBasePrice() const;

private:
    std::vector< WithQuantity< tTypeId > > refinedProducts_;
};
