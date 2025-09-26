#include "EveType.h"

#include <QJsonObject>

EveType::EveType( const QJsonObject& jsonData )
{
    FromJsonObject( jsonData );
}

void EveType::FromJsonObject( const QJsonObject& jsonData )
{
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

    if ( jsonData.contains( "volume" ) && !jsonData.value( "volume" ).isNull() )
        volume_ = jsonData.value( "volume" ).toDouble();
}
