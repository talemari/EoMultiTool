#pragma once
#include "JsonEveInterface.h"

#include <optional>
#include <string>

class QJsonObject;

struct MarketPrice
{
    double averagePrice = 0.0;
    double adjustedPrice = 0.0;
};

class EveType : public JsonEveInterface
{
public:
    EveType() = default;
    EveType( const QJsonObject& jsonData );
    ~EveType() = default;

    void FromJsonObject( const QJsonObject& jsonData ) override;
    QJsonObject ToJsonObject() const override;
    void PostLoadingInitialization() override;

    std::string GetName() const;
    unsigned int GetTypeId() const;
    unsigned int GetGroupId() const;
    unsigned int GetCategoryId() const;
    double GetBasePrice() const;
    bool IsManufacturable() const;
    tTypeId GetSourceBlueprintId() const;
    bool IsReprocessedFromOre() const;
    bool IsPublished() const;

private:
    void SetIsManufacturable( bool isManufacturable );
    void SetIsReprocessedFromOre( bool isReprocessedFromOre );
    void SetSourceBlueprintId( tTypeId blueprintId );
    void SetMarketPrice( double averagePrice, double adjustedPrice );

    friend class RessourcesManager;

private:
    unsigned int groupId_ = 0;
    bool isPublished_ = false;
    std::optional< unsigned int > categoryId_ = 0;
    std::optional< unsigned int > marketGroupId_ = 0;
    std::optional< unsigned int > iconId_ = 0;
    std::optional< double > basePrice_ = 0.0;
    std::string name_ = "";
    std::optional< std::string > description_ = "";
    std::optional< double > volume_ = 0.0;

    MarketPrice marketPrice_;
    bool isManufacturable_ = false;
    bool isReprocessedFromOre_ = false;
    tTypeId sourceBlueprintId_ = 0;
};
