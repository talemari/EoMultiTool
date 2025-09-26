#pragma once

class QJsonObject;

class EveTypeBase
{
public:
    EveTypeBase() = default;
    virtual ~EveTypeBase() = default;

    virtual bool LoadFromJsonObject( const QJsonObject& jsonData ) = 0;
};