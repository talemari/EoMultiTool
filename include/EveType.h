#include "JsonEveInterface.h"

#include <optional>
#include <string>

class QJsonObject;

class EveType : public JsonEveInterface
{
public:
    EveType() = default;
    EveType( const QJsonObject& jsonData );
    ~EveType() = default;

    void FromJsonObject( const QJsonObject& jsonData ) override;
    QJsonObject ToJsonObject() const override;

    std::string GetName() const;
    unsigned int GetTypeId() const;
    unsigned int GetGroupId() const;
    unsigned int GetCategoryId() const;

private:
    unsigned int groupId_ = 0;
    std::optional< unsigned int > categoryId_ = 0;
    std::optional< unsigned int > marketGroupId_ = 0;
    std::optional< unsigned int > iconId_ = 0;
    std::optional< double > basePrice_ = 0.0;
    std::string name_ = "";
    std::optional< std::string > description_ = "";
    std::optional< double > volume_ = 0.0;
};
