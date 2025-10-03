#include "Ore.h"
#include "LogManager.h"

#include <qjsonarray.h>
#include <qjsonobject.h>

Ore::Ore( const QJsonObject& jsonData )
{
    FromJsonObject( jsonData );
}

void Ore::FromJsonObject( const QJsonObject& jsonData )
{
    if ( !jsonData.contains( "_key" ) || !jsonData.contains( "materials" ) )
    {
        LOG_WARNING( "Ore JSON data missing required fields." );
        return;
    }
    typeId_ = jsonData.value( "_key" ).toInt();
    QJsonArray materialsArray = jsonData.value( "materials" ).toArray();
    for ( const QJsonValue& matValue : materialsArray )
    {
        if ( !matValue.isObject() )
        {
            LOG_WARNING( "Invalid material entry in ore id {}.", typeId_ );
            continue;
        }
        QJsonObject matObject = matValue.toObject();
        WithQuantity< tTypeId > refinedProduct;
        if ( matObject.contains( "materialTypeID" ) && !matObject.value( "materialTypeID" ).isNull() )
            refinedProduct.item = matObject.value( "materialTypeID" ).toInt();
        else
        {
            LOG_WARNING( "Material in ore id {} missing typeID.", typeId_ );
            continue;
        }
        if ( matObject.contains( "quantity" ) && !matObject.value( "quantity" ).isNull() )
            refinedProduct.quantity = matObject.value( "quantity" ).toInt();
        else
        {
            LOG_WARNING( "Material in ore id {} missing quantity.", typeId_ );
            continue;
        }
        refinedProducts_.push_back( refinedProduct );
    }

    isValid_ = true;
}

QJsonObject Ore::ToJsonObject() const
{
    QJsonObject obj;

    obj[ "_key" ] = QString::number( typeId_ );

    QJsonArray materialsArray;
    for ( const auto& refinedProduct : refinedProducts_ )
    {
        QJsonObject matObj;
        matObj[ "materialTypeID" ] = QString::number( refinedProduct.item );
        matObj[ "quantity" ] = QString::number( refinedProduct.quantity );
        materialsArray.append( matObj );
    }

    obj[ "materials" ] = materialsArray;

    return obj;
}

void Ore::PostLoadingInitialization()
{
}

const std::vector< WithQuantity< tTypeId > >& Ore::GetRefinedProducts() const
{
    return refinedProducts_;
}

double Ore::GetBasePrice() const
{
    return 1.0;
}
