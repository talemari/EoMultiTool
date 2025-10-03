#pragma once
#include "HelperTypes.h"

#include <QString>

class QJsonObject;

class JsonEveInterface
{
public:
    JsonEveInterface() = default;
    virtual ~JsonEveInterface() = default;

    JsonEveInterface( const QJsonObject& jsonData )
    {
        FromJsonObject( jsonData );
    }

    virtual void FromJsonObject( const QJsonObject& obj ) = 0;
    virtual QJsonObject ToJsonObject() const = 0;
    virtual void PostLoadingInitialization() = 0; // Actions to perform after all json loading is done.

    bool IsValid() const;
    tTypeId GetTypeId() const;
    QString GetName() const;

    bool operator==( const JsonEveInterface& other ) const;
    bool operator!=( const JsonEveInterface& other ) const;
    bool operator<( const JsonEveInterface& other ) const;
    bool operator>( const JsonEveInterface& other ) const;

protected:
    bool isValid_ = false;
    tTypeId typeId_ = 0;
};
