#pragma once
#include "HelperTypes.h"

class EveType;
class Blueprint;
class Ore;

class QSettings;

class GlobalRessources
{
public:
    GlobalRessources( const GlobalRessources& ) = delete;
    ~GlobalRessources() = default;
    static GlobalRessources& Get();

    static void SetRessources( TypeIdMap< EveType >&& types, TypeIdMap< Blueprint >&& blueprints, TypeIdMap< Ore >&& ores )
    {
        Get().ISetRessources( std::move( types ), std::move( blueprints ), std::move( ores ) );
    }

    static bool AreRessourcesReady()
    {
        return Get().areRessourcesReady_;
    }

    static const TypeIdMap< EveType >& GetTypesMap();
    static const TypeIdMap< Blueprint >& GetBlueprintsMap();
    static const TypeIdMap< Ore >& GetOresMap();

    static const std::shared_ptr< EveType > GetTypeById( tTypeId typeId );
    static const std::shared_ptr< Blueprint > GetBlueprintById( tTypeId typeId );

    static bool IsBlueprint( tTypeId typeId )
    {
        return Get().IIsBlueprint( typeId );
    }

private:
    GlobalRessources() = default;

    void ISetRessources( TypeIdMap< EveType >&& types, TypeIdMap< Blueprint >&& blueprints, TypeIdMap< Ore >&& ores );
    const std::shared_ptr< EveType > IGetTypesById( tTypeId typeId ) const;
    bool IIsBlueprint( tTypeId typeId ) const;

private:
    bool areRessourcesReady_ = false;
    TypeIdMap< EveType > types_;
    TypeIdMap< Blueprint > blueprints_;
    TypeIdMap< Ore > ores_;
};
