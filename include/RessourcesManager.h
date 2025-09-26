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
class EveType;

class RessourcesManager : public QObject
{
    Q_OBJECT

public:
    RessourcesManager( QObject* parent = nullptr );
    ~RessourcesManager() override = default;

    RessourcesManager( const RessourcesManager& ) = delete;
    RessourcesManager& operator=( const RessourcesManager& ) = delete;

public slots:
    void LoadRessources();

signals:
    void RessourcesReady();
    void RessourcesLoadingMainStepChanged( int current, int total, const QString& progressDescription );
    void RessourcesLoadingSubStepChanged( int current, int total, const QString& progressDescription );
    void ErrorOccured( const QString& errorMessage );

private:
    void SetLoadingStep( eDataLoadingSteps step );
    bool BuildTypesMap( const QString& typesFilePath );
    bool BuildBlueprintsMap( const QString& blueprintsFilePath );
    bool OpenFile( const QString& filePath, QFile& target );

private slots:
    void LoadSdeData( const QString& extractedSdePath );

private:
    std::unordered_map< unsigned int, std::shared_ptr< EveType > > types_;
    std::unordered_map< unsigned int, std::shared_ptr< Blueprint > > blueprints_;
    std::unique_ptr< DataLoader > dataLoader_ = nullptr;
    bool isRessourcesReady_ = false;
};
