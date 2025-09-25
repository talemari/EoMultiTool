#pragma once
#include <vector>

class QJsonObject;

struct MatRequirement
{
    unsigned int typeID = 0;
    unsigned int quantity = 0;
};

class Blueprint
{
public:
    Blueprint() = default;
    ~Blueprint() = default;

    void FromJsonObject( const QJsonObject& jsonData );

private:
    unsigned int typeID = 0;
    unsigned int productTypeID = 0;
    std::vector< MatRequirement > materials_;
};
