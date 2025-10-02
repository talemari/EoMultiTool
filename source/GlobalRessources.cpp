#include "GlobalRessources.h"

#include <stdexcept>

GlobalRessources& GlobalRessources::Get()
{
    static GlobalRessources instance;
    return instance;
}

const TypeIdMap< const EveType >& GlobalRessources::GetTypesMap()
{
    if ( !Get().areRessourcesReady_ )
    {
        throw std::runtime_error( "Ressources are not ready yet." );
    }
    return Get().types_;
}

const TypeIdMap< const Blueprint >& GlobalRessources::GetBlueprintsMap()
{
    if ( !Get().areRessourcesReady_ )
    {
        throw std::runtime_error( "Ressources are not ready yet." );
    }
    return Get().blueprints_;
}

const TypeIdMap< const Ore >& GlobalRessources::GetOresMap()
{
    if ( !Get().areRessourcesReady_ )
    {
        throw std::runtime_error( "Ressources are not ready yet." );
    }
    return Get().ores_;
}

const std::shared_ptr< const EveType > GlobalRessources::GetTypeById( tTypeId typeId )
{
    return Get().GetTypesMap().at( typeId );
}

const std::shared_ptr< const Blueprint > GlobalRessources::GetBlueprintById( tTypeId typeId )
{
    return Get().GetBlueprintsMap().at( typeId );
}

void GlobalRessources::ISetRessources( TypeIdMap< EveType >&& types, TypeIdMap< Blueprint >&& blueprints, TypeIdMap< Ore >&& ores )
{
    if ( areRessourcesReady_ )
        return;
    types_ = std::move( types );
    blueprints_ = std::move( blueprints );
    ores_ = std::move( ores );
    areRessourcesReady_ = true;
}

const std::shared_ptr< const EveType > GlobalRessources::IGetTypesById( tTypeId typeId ) const
{
    return GetTypesMap().at( typeId );
}

bool GlobalRessources::IIsBlueprint( tTypeId typeId ) const
{
    return GetBlueprintsMap().contains( typeId );
}
