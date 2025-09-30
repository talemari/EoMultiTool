#include "Blueprint.h"
#include "LogManager.h"

#include <QJsonObject>
#include <QJsonarray>

Blueprint::Blueprint( const QJsonObject& jsonData )
{
    FromJsonObject( jsonData );
}

void Blueprint::FromJsonObject( const QJsonObject& jsonData )
{
    typeId_ = jsonData.value( "_key" ).toInt();
    if ( jsonData.contains( "activities" ) && !jsonData.value( "activities" ).isNull() )
    {
        QJsonObject activitiesObj = jsonData.value( "activities" ).toObject();
        if ( activitiesObj.contains( "manufacturing" ) && !activitiesObj.value( "manufacturing" ).isNull() )
        {
            ParseManufacturingJob( activitiesObj.value( "manufacturing" ).toObject() );
        }
        else
        {
            LOG_WARNING( "Blueprint id {} does not contain manufacturing job data.", typeId_ );
            return;
        }
    }
    if ( !manufacturingJob_.isValid )
    {
        LOG_WARNING( "Blueprint id {}'s manufacturing job is invalid", typeId_ );
        return;
    }
    isValid_ = true;
}

QJsonObject Blueprint::ToJsonObject() const
{
    QJsonObject obj;

    if ( !manufacturingJob_.isValid )
        return QJsonObject();

    obj[ "_key" ] = QString::number( typeId_ );
    QJsonObject activitiesObj;
    QJsonObject manufacturingObj;

    manufacturingObj[ "time" ] = QString::number( manufacturingJob_.timeInSeconds );

    QJsonArray materialsArray;
    for ( const auto& matReq : manufacturingJob_.matRequirements )
    {
        QJsonObject matObj;
        matObj[ "typeID" ] = QString::number( matReq.item );
        matObj[ "quantity" ] = QString::number( matReq.quantity );
        materialsArray.append( matObj );
    }
    manufacturingObj[ "materials" ] = materialsArray;

    QJsonArray productsArray;
    for ( const auto& product : manufacturingJob_.manufacturedProducts )
    {
        QJsonObject prodObj;
        prodObj[ "typeID" ] = QString::number( product.item );
        prodObj[ "quantity" ] = QString::number( product.quantity );
        productsArray.append( prodObj );
    }
    manufacturingObj[ "products" ] = productsArray;

    activitiesObj[ "manufacturing" ] = manufacturingObj;
    obj[ "activities" ] = activitiesObj;

    return obj;
}

ManufacturingJob Blueprint::GetManufacturingJob() const
{
    return manufacturingJob_;
}

void Blueprint::ParseManufacturingJob( const QJsonObject& jsonData )
{
    if ( jsonData.isEmpty() )
        return;

    if ( jsonData.contains( "time" ) && !jsonData.value( "time" ).isNull() )
        manufacturingJob_.timeInSeconds = jsonData.value( "time" ).toInt();
    else
        LOG_WARNING( "Blueprint id {} does not contain time duration for manufacture job.", typeId_ );

    if ( jsonData.contains( "materials" ) && !jsonData.value( "materials" ).isNull() )
    {
        QJsonArray materialsArray = jsonData.value( "materials" ).toArray();
        for ( const QJsonValue& matValue : materialsArray )
        {
            if ( !matValue.isObject() )
            {
                LOG_WARNING( "Invalid material entry in blueprint id {}.", typeId_ );
                continue;
            }
            QJsonObject matObject = matValue.toObject();
            WithQuantity< tTypeId > matReq;
            if ( matObject.contains( "typeID" ) && !matObject.value( "typeID" ).isNull() )
                matReq.item = matObject.value( "typeID" ).toInt();
            else
            {
                LOG_WARNING( "Material in blueprint id {} missing typeID.", typeId_ );
                continue;
            }
            if ( matObject.contains( "quantity" ) && !matObject.value( "quantity" ).isNull() )
                matReq.quantity = matObject.value( "quantity" ).toInt();
            else
            {
                LOG_WARNING( "Material in blueprint id {} missing quantity.", typeId_ );
                continue;
            }
            manufacturingJob_.matRequirements.push_back( matReq );
        }
    }
    else
        LOG_WARNING( "Blueprint id {} does not contain materials for manufacture job.", typeId_ );

    if ( jsonData.contains( "products" ) && !jsonData.value( "products" ).isNull() )
    {
        QJsonArray productsArray = jsonData.value( "products" ).toArray();
        for ( const QJsonValue& prodValue : productsArray )
        {
            if ( !prodValue.isObject() )
            {
                LOG_WARNING( "Invalid product entry in blueprint id {}.", typeId_ );
                continue;
            }
            QJsonObject prodObject = prodValue.toObject();
            WithQuantity< tTypeId > product;
            if ( prodObject.contains( "typeID" ) && !prodObject.value( "typeID" ).isNull() )
                product.item = prodObject.value( "typeID" ).toInt();
            else
            {
                LOG_WARNING( "Product in blueprint id {} missing typeID.", typeId_ );
                continue;
            }
            if ( prodObject.contains( "quantity" ) && !prodObject.value( "quantity" ).isNull() )
                product.quantity = prodObject.value( "quantity" ).toInt();
            else
            {
                LOG_WARNING( "Product in blueprint id {} missing quantity.", typeId_ );
                continue;
            }
            manufacturingJob_.manufacturedProducts.push_back( product );
        }
    }
    manufacturingJob_.isValid = true;
}
