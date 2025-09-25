#include "RessourcesManager.h"
#include "LogManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

static constexpr const char* TYPES_JSONL = "types.jsonl";
static constexpr const char* BLUEPRINTS_JSONL = "blueprints.jsonl";

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
{
}

bool RessourcesManager::LoadSdeData( const QString& extractedSdePath )
{
    if ( !BuildTypesMap( extractedSdePath + TYPES_JSONL ) )
        return false;
    return true;
}

EveType RessourcesManager::ParseJsonToEveType( const QJsonObject& jsonData )
{
    EveType result;

    result.published = jsonData.value( "published" ).toBool();
    if ( !result.published )
        return result;

    result.typeID = jsonData.value( "_key" ).toInt();
    result.groupID = jsonData.value( "groupID" ).toInt();

    if ( jsonData.contains( "categoryID" ) && !jsonData.value( "categoryID" ).isNull() )
        result.categoryID = jsonData.value( "categoryID" ).toInt();

    if ( jsonData.contains( "marketGroupID" ) && !jsonData.value( "marketGroupID" ).isNull() )
        result.marketGroupID = jsonData.value( "marketGroupID" ).toInt();

    if ( jsonData.contains( "iconID" ) && !jsonData.value( "iconID" ).isNull() )
        result.iconID = jsonData.value( "iconID" ).toInt();

    result.name = jsonData.value( "name" )[ "en" ].toString().toStdString();

    if ( jsonData.contains( "description" ) && !jsonData.value( "description" ).isNull() )
        result.description = jsonData.value( "description" ).toString().toStdString();

    result.published = jsonData.value( "published" ).toBool();

    if ( jsonData.contains( "volume" ) && !jsonData.value( "volume" ).isNull() )
        result.volume = jsonData.value( "volume" ).toDouble();

    return result;
}

bool RessourcesManager::BuildTypesMap( const QString& typesFilePath )
{
    emit RessourcesLoadingMainStepChanged( 0 );
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
        EveType eveType = ParseJsonToEveType( obj );
        if ( eveType.published )
            types_[ eveType.typeID ] = ParseJsonToEveType( obj );

        double percentageProgression = ( static_cast< double >( ++currentLine ) / static_cast< double >( totalLines ) ) * 100.0;
        if ( percentageProgression - lastPercentageProgression > 1.0 )
        {
            emit RessourcesLoadingProgress( currentLine, totalLines,
                                            tr( "Loading types... %1%" ).arg( static_cast< int >( percentageProgression ) ) );
            lastPercentageProgression = percentageProgression;
        }
    }
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
