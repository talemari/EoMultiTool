#include "ManufacturingJob.h"
#include "EveType.h"
#include "GlobalRessources.h"
#include "LogManager.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

ManufacturingJob::ManufacturingJob( const QJsonObject& jsonData )
{
    FromJsonObject( jsonData );
}

void ManufacturingJob::FromJsonObject( const QJsonObject& jsonData )
{
    if ( jsonData.isEmpty() )
        return;

    if ( jsonData.contains( "time" ) && !jsonData.value( "time" ).isNull() )
        timeInSeconds_ = jsonData.value( "time" ).toInt();
    else
        LOG_WARNING( "Missing time data." );

    if ( jsonData.contains( "materials" ) && !jsonData.value( "materials" ).isNull() )
    {
        QJsonArray materialsArray = jsonData.value( "materials" ).toArray();
        for ( const QJsonValue& matValue : materialsArray )
        {
            if ( !matValue.isObject() )
            {
                LOG_WARNING( "Invalid material entry" );
                continue;
            }
            QJsonObject matObject = matValue.toObject();
            WithQuantity< tTypeId > matReq;
            if ( matObject.contains( "typeID" ) && !matObject.value( "typeID" ).isNull() )
                matReq.item = matObject.value( "typeID" ).toInt();
            else
            {
                LOG_WARNING( "Material missing typeID." );
                continue;
            }
            if ( matObject.contains( "quantity" ) && !matObject.value( "quantity" ).isNull() )
                matReq.quantity = matObject.value( "quantity" ).toInt();
            else
            {
                LOG_WARNING( "Material missing quantity." );
                continue;
            }
            matRequirements_.push_back( matReq );
        }
    }
    else
        LOG_WARNING( "no materials for manufacture job." );

    if ( jsonData.contains( "products" ) && !jsonData.value( "products" ).isNull() )
    {
        QJsonArray productsArray = jsonData.value( "products" ).toArray();
        for ( const QJsonValue& prodValue : productsArray )
        {
            if ( !prodValue.isObject() )
            {
                LOG_WARNING( "Invalid product entry." );
                continue;
            }
            QJsonObject prodObject = prodValue.toObject();
            WithQuantity< tTypeId > product;
            if ( prodObject.contains( "typeID" ) && !prodObject.value( "typeID" ).isNull() )
                product.item = prodObject.value( "typeID" ).toInt();
            else
            {
                LOG_WARNING( "Product missing typeID." );
                continue;
            }
            if ( prodObject.contains( "quantity" ) && !prodObject.value( "quantity" ).isNull() )
                product.quantity = prodObject.value( "quantity" ).toInt();
            else
            {
                LOG_WARNING( "Product missing quantity." );
                continue;
            }
            manufacturedProducts_.push_back( product );
        }
    }
    isValid_ = true;
}

QJsonObject ManufacturingJob::ToJsonObject() const
{
    if ( !isValid_ )
        throw std::runtime_error( "Attempted to serialize an invalid ManufacturingJob." );

    QJsonObject manufacturingObj;

    manufacturingObj[ "time" ] = static_cast< qint64 >( timeInSeconds_ );

    QJsonArray materialsArray;
    for ( const auto& matReq : matRequirements_ )
    {
        QJsonObject matObj;
        matObj[ "typeID" ] = static_cast< qint64 >( matReq.item );
        matObj[ "quantity" ] = static_cast< qint64 >( matReq.quantity );
        materialsArray.append( matObj );
    }
    manufacturingObj[ "materials" ] = materialsArray;

    QJsonArray productsArray;
    for ( const auto& product : manufacturedProducts_ )
    {
        QJsonObject prodObj;
        prodObj[ "typeID" ] = static_cast< qint64 >( product.item );
        prodObj[ "quantity" ] = static_cast< qint64 >( product.quantity );
        productsArray.append( prodObj );
    }
    manufacturingObj[ "products" ] = productsArray;

    return manufacturingObj;
}

const std::vector< WithQuantity< tTypeId > >& ManufacturingJob::GetComponents() const
{
    return components_;
}

const std::vector< WithQuantity< tTypeId > >& ManufacturingJob::GetRawMaterials() const
{
    return rawMaterials_;
}

const std::vector< WithQuantity< tTypeId > >& ManufacturingJob::GetManufacturedProducts() const
{
    return manufacturedProducts_;
}

const std::vector< WithQuantity< tTypeId > >& ManufacturingJob::GetFullMaterialList() const
{
    return matRequirements_;
}

bool ManufacturingJob::IsValid() const
{
    return isValid_;
}

void ManufacturingJob::FilterComponents()
{
    if ( !GlobalRessources::AreRessourcesReady() )
        throw std::runtime_error( "Attempted to filter components when ressources are not ready." );

    for ( const auto& matReq : matRequirements_ )
    {

        auto matType = GlobalRessources::GetTypeById( matReq.item );
        if ( matType->IsManufacturable() )
            components_.emplace_back( matReq.item, matReq.quantity );
        else
            rawMaterials_.emplace_back( matReq.item, matReq.quantity );
    }
    componentsFiltered_ = true;
}
