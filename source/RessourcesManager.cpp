#include "RessourcesManager.h"
#include "EveType.h"
#include "GlobalRessources.h"
#include "HelperFunctions.h"
#include "LogManager.h"
#include "Ore.h"

#include <QBinaryJson>
#include <QCoreapplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSettings>

static constexpr const char* TYPES_JSONL = "types.jsonl";
static constexpr const char* BLUEPRINTS_JSONL = "blueprints.jsonl";
static constexpr const char* TYPEMATERIALS_JSONL = "typeMaterials.jsonl";
static constexpr const char* GROUPS_JSONL = "groups.jsonl";
static constexpr unsigned int ORES_CATEGORY_ID = 25;

static constexpr std::array< const char*, static_cast< int >( eDataLoadingSteps::Count ) > currentDataLoadingStep = {
    "Waiting...",
    "Downloading SDE...",
    "Extracting SDE...",
    "Validating SDE...",
    "Loading Jsonl files...",
    "Loading types...",
    "Loading blueprints...",
    "Loading ores...",
    "Filtering irrelevant data...",
    "Saving filtered json...",
    "Finalizing..." };

RessourcesManager::RessourcesManager( QSettings& settings, QObject* parent )
    : QObject( parent )
    , settings_( settings )
    , dataLoader_( std::make_unique< DataLoader >() )
    , BINARY_DATA_DIRECTORY_PATH_( QCoreApplication::applicationDirPath() + "/ressources/generated/data/" )
    , BINARY_TYPES_FILEPATH_( BINARY_DATA_DIRECTORY_PATH_ + "types.bin" )
    , BINARY_BLUEPRINTS_FILEPATH_( BINARY_DATA_DIRECTORY_PATH_ + "blueprints.bin" )
    , BINARY_ORES_FILEPATH_( BINARY_DATA_DIRECTORY_PATH_ + "ores.bin" )
{
}

void RessourcesManager::LoadRessources()
{
    if ( QFile::exists( BINARY_TYPES_FILEPATH_ ) && QFile::exists( BINARY_BLUEPRINTS_FILEPATH_ ) && QFile::exists( BINARY_ORES_FILEPATH_ ) )
    {
        if ( !LoadMapsFromBinaryFiles() )
            LOG_WARNING( "Failed to load ressources from binary files, falling back to JSON." );
        else
        {
            LOG_NOTICE( "Loaded ressources from binary files." );
            OnRessourcesReady();
            return;
        }
    }

    connect( dataLoader_.get(), &DataLoader::MainDataLoadingStepChanged, this, &RessourcesManager::SetLoadingStep );
    connect( dataLoader_.get(), &DataLoader::SubDataLoadingStepChanged, this, &RessourcesManager::RessourcesLoadingSubStepChanged );
    connect( dataLoader_.get(), &DataLoader::ErrorOccurred, this, &RessourcesManager::ErrorOccured );
    connect( dataLoader_.get(), &DataLoader::MarketPricesReady, this, &RessourcesManager::LoadSdeData );

    dataLoader_->StartDataLoading();
}

void RessourcesManager::SetLoadingStep( eDataLoadingSteps step )
{
    emit RessourcesLoadingMainStepChanged(
        static_cast< int >( step ), static_cast< int >( eDataLoadingSteps::Count ), currentDataLoadingStep[ static_cast< int >( step ) ] );
}

void RessourcesManager::LoadSdeData()
{
    QString extractedSdePath = dataLoader_->GetSdeExtractedPath();
    QJsonObject marketPricesJson = dataLoader_->GetMarketPricesJson();
    dataLoader_->deleteLater();

    if ( !BuildMapFromJsonlFile< EveType >( extractedSdePath + TYPES_JSONL, types_, eDataLoadingSteps::LoadingTypes ) )
        return;
    if ( !BuildMapFromJsonlFile< Blueprint >( extractedSdePath + BLUEPRINTS_JSONL, blueprints_, eDataLoadingSteps::LoadingBlueprints ) )
        return;
    if ( !BuildMapFromJsonlFile< Ore >( extractedSdePath + TYPEMATERIALS_JSONL, ores_, eDataLoadingSteps::LoadingOres ) )
        return;
    SetManufacturableTypes();
    FilterIrrelevantTypes( extractedSdePath + GROUPS_JSONL );
    AddMarketPricesToTypes( marketPricesJson );
    AddReprocessedFromOreDataToTypes();
    if ( !SaveToBinaryFile() )
        return;

    OnRessourcesReady();
}

bool RessourcesManager::OpenFile( const QString& filePath, QFile& outFile, bool isBinary )
{
    outFile.setFileName( QDir::cleanPath( filePath ) );
    QFileInfo info( outFile );

    if ( !outFile.exists() )
    {
        emit ErrorOccured( tr( "Could not find file %1" ).arg( info.absoluteFilePath() ) );
        return false;
    }
    QIODevice::OpenMode mode = isBinary ? QIODevice::ReadOnly : ( QIODevice::ReadOnly | QIODevice::Text );
    if ( !outFile.open( mode ) )
    {
        emit ErrorOccured( tr( "Failed to open %1" ).arg( info.absoluteFilePath() ) );
        return false;
    }
    return true;
}

void RessourcesManager::RemoveNonOreMaterials( const QString& groupFilePath )
{
    QFile jsonFile;
    QFileInfo info( jsonFile );
    if ( !OpenFile( groupFilePath, jsonFile, false ) )
    {
        emit ErrorOccured( tr( "Could not open %1 to filter ores." ).arg( info.absolutePath() ) );
        return;
    }
    std::unordered_set< tTypeId > validOreGroupTypeIds;
    while ( !jsonFile.atEnd() )
    {
        QByteArray line = jsonFile.readLine();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson( line, &parseError );
        if ( parseError.error != QJsonParseError::NoError )
        {
            emit ErrorOccured( tr( "Failed to parse JSON in %1: %2" ).arg( jsonFile.fileName(), parseError.errorString() ) );
            return;
        }
        if ( !doc.isObject() )
        {
            emit ErrorOccured( tr( "Expected JSON object in %1" ).arg( jsonFile.fileName() ) );
            return;
        }
        QJsonObject obj = doc.object();
        tTypeId groupId = obj.value( "_key" ).toInt();
        tTypeId categoryId = obj.value( "categoryID" ).toInt();
        if ( categoryId == ORES_CATEGORY_ID )
            validOreGroupTypeIds.insert( groupId );
    }
    unsigned int removedCount = 0;
    for ( auto it = ores_.begin(); it != ores_.end(); )
    {
        tTypeId typeId = it->second->GetTypeId();
        if ( !types_.at( typeId ) )
        {
            LOG_WARNING( "Ore with typeId {} has no corresponding EveType, removing from ores list.", typeId );
            it = ores_.erase( it );
            continue;
        }
        unsigned int groupId = types_.at( typeId )->GetGroupId();
        if ( validOreGroupTypeIds.find( groupId ) == validOreGroupTypeIds.end() )
        {
            it = ores_.erase( it );
            removedCount++;
        }
        else
            ++it;
    }
    LOG_NOTICE( "Removed {} non-ore materials from ores list, leaving {} ores", removedCount, ores_.size() );
}

void RessourcesManager::FilterIrrelevantTypes( const QString& groupFilepath )
{
    SetLoadingStep( eDataLoadingSteps::FilteringIrrelevantData );
    static constexpr unsigned int PROGRESS_TOTAL_STEPS = 3;
    std::unordered_map< tTypeId, std::shared_ptr< EveType > > types;
    std::unordered_set< tTypeId > relevantTypeIds;

    emit RessourcesLoadingSubStepChanged( 0, PROGRESS_TOTAL_STEPS, "Identifying relevant blueprints and materials..." );
    unsigned int removedBlueprints = 0;
    for ( auto it = blueprints_.begin(); it != blueprints_.end(); )
    {
        const Blueprint& blueprint = *( it->second );
        const EveType& type = *types_.at( blueprint.GetTypeId() );
        if ( !type.IsPublished() )
        {
            it = blueprints_.erase( it );
            removedBlueprints++;
            continue;
        }

        relevantTypeIds.insert( blueprint.GetTypeId() );
        const auto job = blueprint.GetManufacturingJob();

        for ( const auto& product : job->GetManufacturedProducts() )
            relevantTypeIds.insert( product.item );
        for ( const auto& matReq : job->GetFullMaterialList() )
            relevantTypeIds.insert( matReq.item );
        it++;
    }

    for ( auto it = ores_.begin(); it != ores_.end(); )
    {
        tTypeId typeId = it->second->GetTypeId();
        if ( !types_.contains( typeId ) || !types_.at( typeId )->IsPublished() )
        {
            LOG_NOTICE( "Ore with typeId {} has no corresponding EveType, removing from ores list.", typeId );
            it = ores_.erase( it );
            continue;
        }
        ++it;
    }
    emit RessourcesLoadingSubStepChanged( 1, PROGRESS_TOTAL_STEPS, "Filtering Non ore materials..." );
    RemoveNonOreMaterials( groupFilepath );
    emit RessourcesLoadingSubStepChanged( 2, PROGRESS_TOTAL_STEPS, "Building filtered types list..." );
    for ( const auto& typeId : relevantTypeIds )
    {
        if ( types_.find( typeId ) != types_.end() )
            types.insert( { typeId, types_.at( typeId ) } );
    }
    types_.clear();
    types_ = std::move( types );
}

void RessourcesManager::SetManufacturableTypes()
{
    for ( auto it = blueprints_.begin(); it != blueprints_.end(); )
    {
        const auto& typeId = it->first;
        const auto& blueprint = it->second;
        const auto& job = blueprint->GetManufacturingJob();

        if ( job->GetManufacturedProducts().empty() )
        {
            LOG_WARNING( "Blueprint with typeId {} has no manufactured products, removing from blueprints list.", typeId );
            it = blueprints_.erase( it );
            continue;
        }

        bool remove = false;
        for ( const auto& product : job->GetManufacturedProducts() )
        {
            if ( types_.find( product.item ) == types_.end() )
            {
                LOG_WARNING( "Blueprint with typeId {} produces item with typeId {} which is not in the types list, removing blueprint.",
                             typeId,
                             product.item );
                remove = true;
                break;
            }
            else
            {
                types_.at( product.item )->SetIsManufacturable( true );
                types_.at( product.item )->SetSourceBlueprintId( typeId );
            }
        }

        if ( remove )
            it = blueprints_.erase( it );
        else
            ++it;
    }
}

bool RessourcesManager::SaveToBinaryFile()
{
    SetLoadingStep( eDataLoadingSteps::SavingFilteredJson );
    static constexpr unsigned int PROGRESS_TOTAL_STEPS = 3;
    emit RessourcesLoadingSubStepChanged( 0, PROGRESS_TOTAL_STEPS, "Saving types..." );

    QDir dir;
    QFileInfo fileInfo( BINARY_DATA_DIRECTORY_PATH_ );
    if ( !dir.mkpath( fileInfo.absolutePath() ) )
    {
        emit ErrorOccured( tr( "Could not create directory %1 to save filtered data." ).arg( fileInfo.absolutePath() ) );
        return false;
    }

    QJsonObject typesJson = GetJsonFromMap( types_ );
    if ( !SaveJsonObjectToBinaryFile( typesJson, BINARY_TYPES_FILEPATH_ ) )
        return false;

    emit RessourcesLoadingSubStepChanged( 1, PROGRESS_TOTAL_STEPS, "Saving blueprints..." );
    QJsonObject blueprintsJson = GetJsonFromMap( blueprints_ );
    if ( !SaveJsonObjectToBinaryFile( blueprintsJson, BINARY_BLUEPRINTS_FILEPATH_ ) )
        return false;

    emit RessourcesLoadingSubStepChanged( 2, PROGRESS_TOTAL_STEPS, "Saving ores..." );
    QJsonObject oresJson = GetJsonFromMap( ores_ );
    if ( !SaveJsonObjectToBinaryFile( oresJson, BINARY_ORES_FILEPATH_ ) )
        return false;

    return true;
}

bool RessourcesManager::SaveJsonObjectToBinaryFile( const QJsonObject& jsonObject, const QString& binaryFilepath )
{
    LOG_NOTICE( "Saving {} elements to {}", jsonObject.size(), binaryFilepath.toStdString() );
    QFile file( binaryFilepath );
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        emit ErrorOccured( tr( "Could not save map to %1" ).arg( binaryFilepath ) );
        return false;
    }

    QJsonDocument doc( jsonObject );
    QByteArray data = QBinaryJson::toBinaryData( doc );

    qint64 bytesWritten = file.write( data );
    if ( bytesWritten != data.size() )
    {
        emit ErrorOccured( tr( "Failed to write all data to file %1" ).arg( binaryFilepath ) );
        return false;
    }

    file.close();
    settings_.setValue( "StaticData/" + binaryFilepath + "TotalElements", jsonObject.size() );
#ifndef NDEBUG
    QFile jsonVersion( binaryFilepath + ".json" );
    if ( jsonVersion.open( QIODevice::WriteOnly ) )
    {
        jsonVersion.write( doc.toJson( QJsonDocument::Indented ) );
        jsonVersion.close();
    }
#endif
    return true;
}

bool RessourcesManager::LoadMapsFromBinaryFiles()
{
    SetLoadingStep( eDataLoadingSteps::Finalizing );
    static constexpr unsigned int PROGRESS_TOTAL_STEPS = 3;
    emit RessourcesLoadingSubStepChanged( 0, PROGRESS_TOTAL_STEPS, "Loading types from binary files..." );
    if ( !BuildMapFromBinaryFile< EveType >( BINARY_TYPES_FILEPATH_, types_ ) )
        return false;
    emit RessourcesLoadingSubStepChanged( 1, PROGRESS_TOTAL_STEPS, "Loading blueprints from binary files..." );
    if ( !BuildMapFromBinaryFile< Blueprint >( BINARY_BLUEPRINTS_FILEPATH_, blueprints_ ) )
        return false;
    emit RessourcesLoadingSubStepChanged( 2, PROGRESS_TOTAL_STEPS, "Loading ores from binary files..." );
    if ( !BuildMapFromBinaryFile< Ore >( BINARY_ORES_FILEPATH_, ores_ ) )
        return false;
    emit RessourcesLoadingSubStepChanged( PROGRESS_TOTAL_STEPS, PROGRESS_TOTAL_STEPS, "Done." );
    return true;
}

unsigned int RessourcesManager::GetNumberOfLinesInFile( QFile& file )
{
    unsigned int lineCount = 0;
    if ( settings_.contains( "StaticData/" + file.fileName() + "TotalLines" ) )
    {
        LOG_NOTICE( "Using cached line count for file {}", file.fileName().toStdString() );
        return settings_.value( "StaticData/" + file.fileName() + "TotalLines" ).toUInt();
    }

    while ( !file.atEnd() )
    {
        file.readLine();
        ++lineCount;
    }
    file.seek( 0 );
    settings_.setValue( "StaticData/" + file.fileName() + "TotalLines", lineCount );
    return lineCount;
}

void RessourcesManager::AddMarketPricesToTypes( const QJsonObject& marketPricesJson )
{
    for ( auto& [ typeId, eveType ] : types_ )
    {
        if ( marketPricesJson.contains( QString::number( typeId ) ) )
        {
            QJsonObject priceObj = marketPricesJson.value( QString::number( typeId ) ).toObject();
            eveType->SetMarketPrice( priceObj.value( "average_price" ).toDouble(), priceObj.value( "adjusted_price" ).toDouble() );
        }
    }
}

void RessourcesManager::AddReprocessedFromOreDataToTypes()
{
    for ( const auto& [ oreTypeId, ore ] : ores_ )
    {
        for ( const auto& refinedProducts : ore->GetRefinedProducts() )
        {
            tTypeId materialTypeId = refinedProducts.item;
            if ( types_.find( materialTypeId ) != types_.end() )
                types_.at( materialTypeId )->SetIsReprocessedFromOre( true );
        }
    }
}

bool RessourcesManager::IsBlueprintValid( const Blueprint& blueprint ) const
{
    const auto job = blueprint.GetManufacturingJob();
    if ( !job->IsValid() || !types_.contains( blueprint.GetTypeId() ) )
    {
        return false;
    }
    return true;
}

void RessourcesManager::OnRessourcesReady()
{
    isRessourcesReady_ = true;
    GlobalRessources::SetRessources( std::move( types_ ), std::move( blueprints_ ), std::move( ores_ ) );
    emit RessourcesReady();
}

template < JsonEveChild T >
QJsonObject RessourcesManager::GetJsonFromMap( const TypeIdMap< T >& map ) const
{
    QJsonObject result;
    for ( const auto& [ typeId, element ] : map )
    {
        QJsonObject elementObj = element->ToJsonObject();
        if ( elementObj.contains( "_key" ) )
            result[ QString::number( typeId ) ] = element->ToJsonObject();
    }
    return result;
}

template < JsonEveChild T >
bool RessourcesManager::BuildMapFromBinaryFile( const QString& filePath, TypeIdMap< T >& targetMap )
{
    QFile binFile;
    if ( !OpenFile( filePath, binFile, true ) )
        return false;
    QJsonDocument jsonDoc = QBinaryJson::fromBinaryData( binFile.readAll() );
    if ( jsonDoc.isNull() || !jsonDoc.isObject() )
    {
        emit ErrorOccured( tr( "Failed to parse binary json from %1" ).arg( binFile.fileName() ) );
        return false;
    }
    QJsonObject jsonobject = jsonDoc.object();
    LOG_NOTICE( "Loading {} elements from {}", jsonobject.size(), binFile.fileName().toStdString() );
    unsigned int currentElement = 0;
    double lastPercentageProgression = 0.0;
    for ( const QString& key : jsonobject.keys() )
    {
        QJsonObject elementObj = jsonobject.value( key ).toObject();
        tTypeId elementTypeId = key.toInt();
        elementObj.insert( "_key", static_cast< int >( elementTypeId ) );
        std::shared_ptr< T > element = std::make_shared< T >( elementObj );
        if ( element->IsValid() )
        {
            targetMap[ elementTypeId ] = element;
            double percentageProgression =
                ( static_cast< double >( ++currentElement ) / static_cast< double >( jsonobject.size() ) ) * 100.0;
            if ( percentageProgression - lastPercentageProgression > 1.0 || currentElement >= jsonobject.size() )
            {
                QString msg = tr( "Loading %1... %2%" ).arg( filePath ).arg( static_cast< int >( percentageProgression ) );
                emit RessourcesLoadingSubStepChanged( currentElement, jsonobject.size(), msg );
                lastPercentageProgression = percentageProgression;
            }
        }
        else
            LOG_WARNING( "Element with typeId {} in file {} is not valid, skipping.", elementTypeId, binFile.fileName().toStdString() );
    }
    return true;
}

template < JsonEveChild T >
inline bool RessourcesManager::BuildMapFromJsonlFile( const QString& filePath, TypeIdMap< T >& targetMap, eDataLoadingSteps step )
{
    SetLoadingStep( step );
    QFile jsonFile;
    if ( !OpenFile( filePath, jsonFile, false ) )
        return false;
    unsigned int totalLines = GetNumberOfLinesInFile( jsonFile );
    unsigned int currentLine = 0;
    double lastPercentageProgression = 0.0;
    while ( !jsonFile.atEnd() )
    {
        QByteArray line = jsonFile.readLine();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson( line, &parseError );

        if ( parseError.error != QJsonParseError::NoError )
        {
            emit ErrorOccured( tr( "Failed to parse JSON in %1: %2" ).arg( jsonFile.fileName(), parseError.errorString() ) );
            return false;
        }
        if ( !doc.isObject() )
        {
            emit ErrorOccured( tr( "Expected JSON object in %1" ).arg( jsonFile.fileName() ) );
            return false;
        }

        QJsonObject obj = doc.object();
        tTypeId elementTypeId = obj.value( "_key" ).toInt();
        std::shared_ptr< T > element = std::make_shared< T >( obj );
        if ( element->IsValid() )
            targetMap[ elementTypeId ] = element;

        double percentageProgression = ( static_cast< double >( ++currentLine ) / static_cast< double >( totalLines ) ) * 100.0;
        if ( percentageProgression - lastPercentageProgression > 1.0 || currentLine >= totalLines )
        {
            QString msg = tr( "Loading %1... %2%" ).arg( jsonFile.fileName() ).arg( static_cast< int >( percentageProgression ) );
            emit RessourcesLoadingSubStepChanged( currentLine, totalLines, msg );
            lastPercentageProgression = percentageProgression;
        }
    }
    return true;
}

template bool RessourcesManager::BuildMapFromJsonlFile< EveType >( const QString&, TypeIdMap< EveType >&, eDataLoadingSteps );
template bool RessourcesManager::BuildMapFromJsonlFile< Blueprint >( const QString&, TypeIdMap< Blueprint >&, eDataLoadingSteps );
template bool RessourcesManager::BuildMapFromJsonlFile< Ore >( const QString&, TypeIdMap< Ore >&, eDataLoadingSteps );

template QJsonObject RessourcesManager::GetJsonFromMap< EveType >( const TypeIdMap< EveType >& ) const;
template QJsonObject RessourcesManager::GetJsonFromMap< Blueprint >( const TypeIdMap< Blueprint >& ) const;
template QJsonObject RessourcesManager::GetJsonFromMap< Ore >( const TypeIdMap< Ore >& ) const;

template bool RessourcesManager::BuildMapFromBinaryFile< EveType >( const QString& filePath, TypeIdMap< EveType >& targetMap );
template bool RessourcesManager::BuildMapFromBinaryFile< Blueprint >( const QString& filePath, TypeIdMap< Blueprint >& targetMap );
template bool RessourcesManager::BuildMapFromBinaryFile< Ore >( const QString& filePath, TypeIdMap< Ore >& targetMap );