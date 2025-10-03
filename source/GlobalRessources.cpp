#include "GlobalRessources.h"

#include "Blueprint.h"
#include "EveType.h"
#include "Ore.h"

#include <stdexcept>

GlobalRessources& GlobalRessources::Get()
{
    static GlobalRessources instance;
    return instance;
}

const TypeIdMap< EveType >& GlobalRessources::GetTypesMap()
{
    if ( !Get().areRessourcesReady_ )
    {
        throw std::runtime_error( "Ressources are not ready yet." );
    }
    return Get().types_;
}

const TypeIdMap< Blueprint >& GlobalRessources::GetBlueprintsMap()
{
    if ( !Get().areRessourcesReady_ )
    {
        throw std::runtime_error( "Ressources are not ready yet." );
    }
    return Get().blueprints_;
}

const TypeIdMap< Ore >& GlobalRessources::GetOresMap()
{
    if ( !Get().areRessourcesReady_ )
    {
        throw std::runtime_error( "Ressources are not ready yet." );
    }
    return Get().ores_;
}

const std::shared_ptr< EveType > GlobalRessources::GetTypeById( tTypeId typeId )
{
    if ( !Get().GetTypesMap().contains( typeId ) )
        return nullptr;

    return Get().GetTypesMap().at( typeId );
}

const std::shared_ptr< Blueprint > GlobalRessources::GetBlueprintById( tTypeId typeId )
{
    if ( !Get().GetBlueprintsMap().contains( typeId ) )
        return nullptr;

    return Get().GetBlueprintsMap().at( typeId );
}

const std::shared_ptr< Blueprint > GlobalRessources::GetBlueprintByProductId( tTypeId productId )
{
    for ( const auto& [ _, blueprint ] : Get().blueprints_ )
    {
        const auto& products = blueprint->GetManufacturingJob()->GetManufacturedProducts();
        for ( const auto& [ typeId, _ ] : products )
        {
            if ( typeId == productId )
                return blueprint;
        }
    }
    return nullptr;
}

void GlobalRessources::ISetRessources( TypeIdMap< EveType >&& types, TypeIdMap< Blueprint >&& blueprints, TypeIdMap< Ore >&& ores )
{
    if ( areRessourcesReady_ )
        return;
    types_ = std::move( types );
    blueprints_ = std::move( blueprints );
    ores_ = std::move( ores );
    areRessourcesReady_ = true;

    for ( auto& type : types_ )
        type.second->PostLoadingInitialization();
    for ( auto& blueprint : blueprints_ )
        blueprint.second->PostLoadingInitialization();
    for ( auto& ore : ores_ )
        ore.second->PostLoadingInitialization();
}

const std::shared_ptr< EveType > GlobalRessources::IGetTypesById( tTypeId typeId ) const
{
    return GetTypesMap().at( typeId );
}

bool GlobalRessources::IIsBlueprint( tTypeId typeId ) const
{
    return GetBlueprintsMap().contains( typeId );
}
