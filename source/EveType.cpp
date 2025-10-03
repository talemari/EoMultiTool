#include "EveType.h"

#include <QJsonObject>

EveType::EveType( const QJsonObject& jsonData )
{
    FromJsonObject( jsonData );
}

void EveType::FromJsonObject( const QJsonObject& jsonData )
{
    if ( jsonData.isEmpty() )
        return;
    isPublished_ = jsonData.value( "published" ).toBool( false );

    typeId_ = jsonData.value( "_key" ).toInt();
    groupId_ = jsonData.value( "groupID" ).toInt();

    if ( jsonData.contains( "categoryID" ) && !jsonData.value( "categoryID" ).isNull() )
        categoryId_ = jsonData.value( "categoryID" ).toInt();

    if ( jsonData.contains( "marketGroupID" ) && !jsonData.value( "marketGroupID" ).isNull() )
        marketGroupId_ = jsonData.value( "marketGroupID" ).toInt();

    if ( jsonData.contains( "iconID" ) && !jsonData.value( "iconID" ).isNull() )
        iconId_ = jsonData.value( "iconID" ).toInt();

    name_ = jsonData.value( "name" )[ "en" ].toString().toStdString();

    if ( jsonData.contains( "description" ) && !jsonData.value( "description" ).isNull() )
        description_ = jsonData.value( "description" ).toString().toStdString();

    if ( jsonData.contains( "basePrice" ) && !jsonData.value( "basePrice" ).isNull() )
        basePrice_ = jsonData.value( "basePrice" ).toDouble();

    if ( jsonData.contains( "volume" ) && !jsonData.value( "volume" ).isNull() )
        volume_ = jsonData.value( "volume" ).toDouble();

    if ( jsonData.contains( "isManufacturable" ) && !jsonData.value( "isManufacturable" ).isNull() )
        isManufacturable_ = jsonData.value( "isManufacturable" ).toBool();

    if ( jsonData.contains( "sourceBlueprintId" ) && !jsonData.value( "sourceBlueprintId" ).isNull() )
        sourceBlueprintId_ = jsonData.value( "sourceBlueprintId" ).toInt();

    if ( jsonData.contains( "marketPrice" ) && !jsonData.value( "marketPrice" ).isNull() )
    {
        QJsonObject marketPriceObj = jsonData.value( "marketPrice" ).toObject();
        marketPrice_.averagePrice = marketPriceObj.value( "averagePrice" ).toDouble();
        marketPrice_.adjustedPrice = marketPriceObj.value( "adjustedPrice" ).toDouble();
    }
    if ( jsonData.contains( "reprocessedFromOre" ) && !jsonData.value( "reprocessedFromOre" ).isNull() )
        isReprocessedFromOre_ = jsonData.value( "reprocessedFromOre" ).toBool();
    isValid_ = true;
}

QJsonObject EveType::ToJsonObject() const
{
    QJsonObject obj;

    obj[ "_key" ] = QString::number( typeId_ );
    obj[ "groupID" ] = QString::number( groupId_ );

    if ( categoryId_.has_value() )
        obj[ "categoryID" ] = QString::number( categoryId_.value() );

    if ( marketGroupId_.has_value() )
        obj[ "marketGroupID" ] = QString::number( marketGroupId_.value() );

    if ( iconId_.has_value() )
        obj[ "iconID" ] = QString::number( iconId_.value() );

    if ( !name_.empty() )
    {
        QJsonObject nameObj;
        nameObj[ "en" ] = QString::fromStdString( name_ );
        obj[ "name" ] = nameObj;
    }

    if ( description_.has_value() )
        obj[ "description" ] = QString::fromStdString( description_.value() );

    if ( basePrice_.has_value() )
        obj[ "basePrice" ] = basePrice_.value();

    if ( volume_.has_value() )
        obj[ "volume" ] = volume_.value();

    obj[ "isManufacturable" ] = isManufacturable_;
    obj[ "sourceBlueprintId" ] = static_cast< qint64 >( sourceBlueprintId_ );

    QJsonObject marketPriceObj;
    marketPriceObj[ "averagePrice" ] = marketPrice_.averagePrice;
    marketPriceObj[ "adjustedPrice" ] = marketPrice_.adjustedPrice;
    obj[ "marketPrice" ] = marketPriceObj;
    obj[ "reprocessedFromOre" ] = isReprocessedFromOre_;
    return obj;
}

void EveType::PostLoadingInitialization()
{
}

std::string EveType::GetName() const
{
    return name_;
}

unsigned int EveType::GetTypeId() const
{
    return typeId_;
}

unsigned int EveType::GetGroupId() const
{
    return groupId_;
}

unsigned int EveType::GetCategoryId() const
{
    return categoryId_.has_value() ? categoryId_.value() : 0;
}

double EveType::GetBasePrice() const
{
    return basePrice_.has_value() ? basePrice_.value() : 0.0;
}

bool EveType::IsManufacturable() const
{
    return isManufacturable_;
}

tTypeId EveType::GetSourceBlueprintId() const
{
    return sourceBlueprintId_;
}

bool EveType::IsReprocessedFromOre() const
{
    return isReprocessedFromOre_;
}

bool EveType::IsPublished() const
{
    return isPublished_;
}

void EveType::SetIsManufacturable( bool isManufacturable )
{
    isManufacturable_ = isManufacturable;
}

void EveType::SetIsReprocessedFromOre( bool isReprocessedFromOre )
{
    isReprocessedFromOre_ = isReprocessedFromOre;
}

void EveType::SetSourceBlueprintId( tTypeId blueprintId )
{
    sourceBlueprintId_ = blueprintId;
}

void EveType::SetMarketPrice( double averagePrice, double adjustedPrice )
{
    marketPrice_.averagePrice = averagePrice;
    marketPrice_.adjustedPrice = adjustedPrice;
}
