#pragma once
#include <qobject.h>

class QFile;
class QJsonObject;

struct EveType
{
    unsigned int typeID = 0;
    unsigned int groupID = 0;
    std::optional< unsigned int > categoryID = 0;
    std::optional< unsigned int > marketGroupID = 0;
    std::optional< unsigned int > iconID = 0;
    std::string name = "";
    std::optional< std::string > description = "";
    bool published = false;
    std::optional< double > volume = 0.0;
};

class RessourcesManager : public QObject
{
    Q_OBJECT
public:
    explicit RessourcesManager( QObject* parent = nullptr );
    ~RessourcesManager() = default;

    bool LoadSdeData( const QString& extractedSdePath );

signals:
    void RessourcesReady();
    void RessourcesLoadingProgress( int current, int total, const QString& progressDescription );
    void RessourcesLoadingMainStepChanged( int step );
    void ErrorOccured( const QString& errorMessage );

private:
    EveType ParseJsonToEveType( const QJsonObject& jsonData );
    bool BuildTypesMap( const QString& typesFilePath );
    bool OpenFile( const QString& filePath, QFile& target );

private:
    std::unordered_map< unsigned int, EveType > types_;
};
