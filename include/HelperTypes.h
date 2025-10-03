#pragma once
#include <memory>
#include <unordered_map>

typedef unsigned int tTypeId;

template < typename T >
using TypeIdMap = std::unordered_map< tTypeId, std::shared_ptr< T > >;

template < typename T >
struct WithQuantity
{
    T item;
    unsigned int quantity = 0;

    bool operator<( const WithQuantity& other ) const
    {
        return item < other.item;
    }
};

enum class eDataLoadingSteps
{
    Waiting,
    DownloadingSde,
    ExtractingSde,
    ValidatingSde,
    FetchingMarketPrices,
    LoadingJsonlFiles,
    LoadingTypes,
    LoadingBlueprints,
    LoadingOres,
    FilteringIrrelevantData,
    SavingFilteredJson,
    Finalizing,
    Count
};
