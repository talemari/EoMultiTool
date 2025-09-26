#include "RessourcesManager.h"
#include "EveType.h"
#include "LogManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

static constexpr const char* TYPES_JSONL = "types.jsonl";
static constexpr const char* BLUEPRINTS_JSONL = "blueprints.jsonl";
static constexpr std::array< const char*, static_cast< int >( eDataLoadingSteps::Count ) > currentDataLoadingStep = {
    "Waiting...",
    "Downloading SDE...",
    "Extracting SDE...",
    "Validating SDE...",
    "Loading Jsonl files...",
    "Loading types...",
    "Loading blueprints...",
    "Building ores table...",
    "Loading attributes...",
    "Loading dogma attributes...",
    "Finalizing..." };

unsigned int GetNumberOfLinesInFile( QFile& file )
{
    unsigned int lineCount = 0;
    while ( !file.atEnd() )
    {
        file.readLine();
        ++lineCount;
    }
    file.seek( 0 );
    return lineCount;
}

RessourcesManager::RessourcesManager( QObject* parent )
    : QObject( parent )
    , dataLoader_( std::make_unique< DataLoader >() )
{
}

void RessourcesManager::LoadRessources()
{
    connect( dataLoader_.get(), &DataLoader::MainDataLoadingStepChanged, this, &RessourcesManager::SetLoadingStep );
    connect( dataLoader_.get(), &DataLoader::SubDataLoadingStepChanged, this, &RessourcesManager::RessourcesLoadingSubStepChanged );
    connect( dataLoader_.get(), &DataLoader::ErrorOccurred, this, &RessourcesManager::ErrorOccured );
    connect( dataLoader_.get(), &DataLoader::SdeValidated, this, &RessourcesManager::LoadSdeData );

    dataLoader_->StartDataLoading();
}

void RessourcesManager::SetLoadingStep( eDataLoadingSteps step )
{
    emit RessourcesLoadingMainStepChanged(
        static_cast< int >( step ), static_cast< int >( eDataLoadingSteps::Count ), currentDataLoadingStep[ static_cast< int >( step ) ] );
}

void RessourcesManager::LoadSdeData( const QString& extractedSdePath )
{
    dataLoader_->deleteLater();
    if ( !BuildTypesMap( extractedSdePath + TYPES_JSONL ) )
        return;
    if ( !BuildBlueprintsMap( extractedSdePath + BLUEPRINTS_JSONL ) )
        return;
    isRessourcesReady_ = true;
}

bool RessourcesManager::BuildTypesMap( const QString& typesFilePath )
{
    SetLoadingStep( eDataLoadingSteps::LoadingTypes );
    QFile typesFile;
    if ( !OpenFile( typesFilePath, typesFile ) )
        return false;

    unsigned int totalLines = GetNumberOfLinesInFile( typesFile );
    unsigned int currentLine = 0;
    double lastPercentageProgression = 0.0;
    while ( !typesFile.atEnd() )
    {
        QByteArray line = typesFile.readLine();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson( line, &parseError );
        if ( parseError.error != QJsonParseError::NoError )
        {
            emit ErrorOccured( tr( "Failed to parse JSON in %1: %2" ).arg( typesFile.fileName(), parseError.errorString() ) );
            return false;
        }
        if ( !doc.isObject() )
        {
            emit ErrorOccured( tr( "Expected JSON object in %1" ).arg( typesFile.fileName() ) );
            return false;
        }
        QJsonObject obj = doc.object();
        if ( obj.value( "published" ).toBool() == true )
            types_[ obj.value( "_key" ).toInt() ] = std::make_shared< EveType >( EveType( obj ) );

        double percentageProgression = ( static_cast< double >( ++currentLine ) / static_cast< double >( totalLines ) ) * 100.0;
        if ( percentageProgression - lastPercentageProgression > 1.0 )
        {
            emit RessourcesLoadingSubStepChanged(
                currentLine, totalLines, tr( "Loading types... %1%" ).arg( static_cast< int >( percentageProgression ) ) );
            lastPercentageProgression = percentageProgression;
        }
    }
    return true;
}

bool RessourcesManager::BuildBlueprintsMap( const QString& blueprintsFilePath )
{
    SetLoadingStep( eDataLoadingSteps::LoadingBlueprints );
    QFile blueprintsFile;
    if ( !OpenFile( blueprintsFilePath, blueprintsFile ) )
        return false;
    unsigned int totalLines = GetNumberOfLinesInFile( blueprintsFile );
    unsigned int currentLine = 0;
    double lastPercentageProgression = 0.0;
    while ( !blueprintsFile.atEnd() )
    {
        QByteArray line = blueprintsFile.readLine();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson( line, &parseError );

        if ( parseError.error != QJsonParseError::NoError )
        {
            emit ErrorOccured( tr( "Failed to parse JSON in %1: %2" ).arg( blueprintsFile.fileName(), parseError.errorString() ) );
            return false;
        }
        if ( !doc.isObject() )
        {
            emit ErrorOccured( tr( "Expected JSON object in %1" ).arg( blueprintsFile.fileName() ) );
            return false;
        }

        QJsonObject obj = doc.object();
        tTypeId blueprintTypeId = obj.value( "_key" ).toInt();
        if ( types_.contains( blueprintTypeId ) )
            blueprints_[ blueprintTypeId ] = std::make_shared< Blueprint >( Blueprint( obj ) );

        double percentageProgression = ( static_cast< double >( ++currentLine ) / static_cast< double >( totalLines ) ) * 100.0;
        if ( percentageProgression - lastPercentageProgression > 1.0 || currentLine >= totalLines )
        {
            QString msg = tr( "Loading blueprints... %1%" ).arg( static_cast< int >( percentageProgression ) );
            emit RessourcesLoadingSubStepChanged( currentLine, totalLines, msg );
            lastPercentageProgression = percentageProgression;
        }
    }
    return true;
}

bool RessourcesManager::OpenFile( const QString& filePath, QFile& outFile )
{
    outFile.setFileName( QDir::cleanPath( filePath ) );
    QFileInfo info( outFile );

    if ( !outFile.exists() )
    {
        emit ErrorOccured( tr( "Could not find file %1" ).arg( info.absoluteFilePath() ) );
        return false;
    }
    if ( !outFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        emit ErrorOccured( tr( "Failed to open %1" ).arg( info.absoluteFilePath() ) );
        return false;
    }
    return true;
}
