#pragma once
#include "EveDataTypes.h"

#include <optional>

#include <yaml-cpp/yaml.h>

template < typename T > T Required( const YAML::Node& node, const char* key )
{
    const YAML::Node value = node[ key ]; // undefined node if missing
    if ( !value || value.IsNull() )
    {
        throw std::runtime_error( std::string( "Missing required key: " ) + key );
    }
    return value.as< T >();
}

template < typename T > std::optional< T > Optional( const YAML::Node& node, const char* key )
{
    auto value = node[ key ];
    if ( !value )
        return std::nullopt;
    return value.as< T >();
}

namespace YAML
{

template <> struct convert< EveType >
{
    static bool decode( const Node& node, EveType& out )
    {
        if ( !node.IsMap() )
            return false;
        out.typeID = Required< unsigned int >( node, "typeID" );
        out.groupID = Required< unsigned int >( node, "groupID" );
        out.categoryID = Optional< unsigned int >( node, "categoryID" );
        out.marketGroupID = Optional< unsigned int >( node, "marketGroupID" );
        out.iconID = Optional< unsigned int >( node, "iconID" );
        out.name = Required< std::string >( node, "name" );
        out.description = Optional< std::string >( node, "description" );
        out.published = Required< bool >( node, "published" );
        out.volume = Optional< double >( node, "volume" );
        return true;
    }
};

} // namespace YAML