#pragma once
#include "HelperTypes.h"

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

    bool IsValid() const
    {
        return isValid_;
    }
    tTypeId GetTypeId() const
    {
        return typeId_;
    }

protected:
    bool isValid_ = false;
    tTypeId typeId_ = 0;
};
