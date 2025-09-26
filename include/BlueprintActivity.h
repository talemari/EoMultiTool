#pragma once

class QJsonObject;

enum class eBlueprintActivityType
{
    Manufacturing,
    Copying,
    Invention,
    ResearchTime,
    ResearchMaterial,
};

class BlueprintActivity
{
public:
    BlueprintActivity() = default;
    virtual ~BlueprintActivity() = default;
    virtual eBlueprintActivityType GetActivityType();

    void FromJsonObject( const QJsonObject& jsonData );

private:
};
