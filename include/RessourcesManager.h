#pragma once
#include "Blueprint.h"
#include "DataLoader.h"

#include <memory>
#include <optional>
#include <qobject.h>
#include <string>
#include <unordered_map>

class QFile;
class QJsonObject;
class QSettings;
class EveType;
class Ore;

template < typename T >
concept JsonEveChild = std::is_base_of_v< JsonEveInterface, T >;

class RessourcesManager : public QObject
{
    Q_OBJECT

public:
    RessourcesManager( QSettings& settings, QObject* parent = nullptr );
    ~RessourcesManager() override = default;

    RessourcesManager( const RessourcesManager& ) = delete;
    RessourcesManager& operator=( const RessourcesManager& ) = delete;

    const std::unordered_map< tTypeId, std::shared_ptr< EveType > >& GetTypesMap() const;
    const std::unordered_map< tTypeId, std::shared_ptr< Blueprint > >& GetBlueprintsMap() const;
    const std::unordered_map< tTypeId, std::shared_ptr< Ore > >& GetOresMap() const;

    const std::shared_ptr< EveType > GetTypeById( tTypeId typeId ) const;

public slots:
    void LoadRessources();

signals:
    void RessourcesReady();
    void RessourcesLoadingMainStepChanged( int current, int total, const QString& progressDescription );
    void RessourcesLoadingSubStepChanged( int current, int total, const QString& progressDescription );
    void ErrorOccured( const QString& errorMessage );

private:
    void SetLoadingStep( eDataLoadingSteps step );
    bool OpenFile( const QString& filePath, QFile& target, bool isBinary );

    template < JsonEveChild T >
    bool BuildMapFromJsonlFile( const QString& filePath,
                                std::unordered_map< tTypeId, std::shared_ptr< T > >& targetMap,
                                eDataLoadingSteps step );
    void RemoveNonOreMaterials( const QString& groupFilepath );
    void FilterIrrelevantData( const QString& groupFilepath );
    bool SaveToBinaryFile();

    template < JsonEveChild T >
    QJsonObject GetJsonFromMap( const std::unordered_map< tTypeId, std::shared_ptr< T > >& ) const;
    bool SaveJsonObjectToBinaryFile( const QJsonObject& jsonObject, const QString& binaryFilepath );
    bool LoadMapsFromBinaryFiles();

    template < JsonEveChild T >
    bool BuildMapFromBinaryFile( const QString& filePath, std::unordered_map< tTypeId, std::shared_ptr< T > >& targetMap );
    unsigned int GetNumberOfLinesInFile( QFile& file );

private slots:
    void LoadSdeData( const QString& extractedSdePath );

private:
    QSettings& settings_;
    std::unordered_map< tTypeId, std::shared_ptr< EveType > > types_;
    std::unordered_map< tTypeId, std::shared_ptr< Blueprint > > blueprints_;
    std::unordered_map< tTypeId, std::shared_ptr< Ore > > ores_;
    std::unique_ptr< DataLoader > dataLoader_ = nullptr;
    bool isRessourcesReady_ = false;

    const QString BINARY_DATA_DIRECTORY_PATH_;
    const QString BINARY_TYPES_FILEPATH_;
    const QString BINARY_BLUEPRINTS_FILEPATH_;
    const QString BINARY_ORES_FILEPATH_;
};
