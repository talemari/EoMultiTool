#pragma once

typedef unsigned int tTypeId;

template < typename T >
struct WithQuantity
{
    T item;
    unsigned int quantity = 0;
};

enum class eDataLoadingSteps
{
    Waiting,
    DownloadingSde,
    ExtractingSde,
    ValidatingSde,
    LoadingJsonlFiles,
    LoadingTypes,
    LoadingBlueprints,
    LoadingOres,
    FilteringIrrelevantData,
    SavingFilteredJson,
    Finalizing,
    Count
};