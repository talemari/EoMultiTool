#pragma once
#include "Blueprint.h"
#include "DataLoader.h"
#include "HelperTypes.h"

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

public slots:
    void LoadRessources();

signals:
    void RessourcesReady();
    void RessourcesLoadingMainStepChanged( int current, int total, const QString& progressDescription );
    void RessourcesLoadingSubStepChanged( int current, int total, const QString& progressDescription );
    void MarketPricesUpdated();
    void ErrorOccured( const QString& errorMessage );

private:
    void SetLoadingStep( eDataLoadingSteps step );
    bool OpenFile( const QString& filePath, QFile& target, bool isBinary );

    template < JsonEveChild T >
    bool BuildMapFromJsonlFile( const QString& filePath, TypeIdMap< T >& targetMap, eDataLoadingSteps step );
    void RemoveNonOreMaterials( const QString& groupFilepath );
    void FilterIrrelevantData( const QString& groupFilepath );
    void SetManufacturableTypes();
    bool SaveToBinaryFile();

    template < JsonEveChild T >
    QJsonObject GetJsonFromMap( const TypeIdMap< T >& ) const;
    bool SaveJsonObjectToBinaryFile( const QJsonObject& jsonObject, const QString& binaryFilepath );
    bool LoadMapsFromBinaryFiles();

    template < JsonEveChild T >
    bool BuildMapFromBinaryFile( const QString& filePath, TypeIdMap< T >& targetMap );
    unsigned int GetNumberOfLinesInFile( QFile& file );

    void AddMarketPricesToTypes( const QJsonObject& marketPricesJson );

private slots:
    void LoadSdeData();
    void OnRessourcesReady();

private:
    QSettings& settings_;
    TypeIdMap< EveType > types_;
    TypeIdMap< Blueprint > blueprints_;
    TypeIdMap< Ore > ores_;

    std::unique_ptr< DataLoader > dataLoader_ = nullptr;
    std::unique_ptr< FileDownloader > fileDownloader_ = nullptr;
    bool isRessourcesReady_ = false;

    const QString BINARY_DATA_DIRECTORY_PATH_;
    const QString BINARY_TYPES_FILEPATH_;
    const QString BINARY_BLUEPRINTS_FILEPATH_;
    const QString BINARY_ORES_FILEPATH_;
    const QString MARKET_PRICES_URL_ = "https://esi.evetech.net/latest/markets/prices/?datasource=tranquility";
};
