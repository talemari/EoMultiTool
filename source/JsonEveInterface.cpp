#include "JsonEveInterface.h"
#include "EveType.h"
#include "GlobalRessources.h"

#include <stdexcept>

bool JsonEveInterface::IsValid() const
{
    return isValid_;
}

tTypeId JsonEveInterface::GetTypeId() const
{
    return typeId_;
}

QString JsonEveInterface::GetName() const
{
    if ( !isValid_ )
        throw std::runtime_error( "Cannot Access of invalid JsonEveInterface object." );
    if ( typeId_ == 0 )
        throw std::runtime_error( "Cannot Access of JsonEveInterface object with typeId 0." );
    if ( !GlobalRessources::AreRessourcesReady() )
        throw std::runtime_error( "Cannot Access of JsonEveInterface object because GlobalRessources are not ready." );
    if ( !GlobalRessources::GetTypesMap().contains( typeId_ ) )
        throw std::runtime_error( "Cannot Access of JsonEveInterface object because its typeId does not exist in GlobalRessources." );

    return QString::fromStdString( GlobalRessources::GetTypesMap().at( typeId_ )->GetName() );
}

bool JsonEveInterface::operator==( const JsonEveInterface& other ) const
{
    return other.typeId_ == typeId_;
}

bool JsonEveInterface::operator!=( const JsonEveInterface& other ) const
{
    return other.typeId_ != typeId_;
}

bool JsonEveInterface::operator<( const JsonEveInterface& other ) const
{
    return other.typeId_ < typeId_;
}

bool JsonEveInterface::operator>( const JsonEveInterface& other ) const
{
    return other.typeId_ > typeId_;
}
