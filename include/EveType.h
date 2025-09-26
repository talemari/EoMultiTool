#include <optional>
#include <string>

class QJsonObject;

class EveType
{
public:
    EveType() = default;
    EveType( const QJsonObject& jsonData );
    ~EveType() = default;

    void FromJsonObject( const QJsonObject& jsonData );

private:
    unsigned int typeId_ = 0;
    unsigned int groupId_ = 0;
    std::optional< unsigned int > categoryId_ = 0;
    std::optional< unsigned int > marketGroupId_ = 0;
    std::optional< unsigned int > iconId_ = 0;
    std::string name_ = "";
    std::optional< std::string > description_ = "";
    std::optional< double > volume_ = 0.0;
};
