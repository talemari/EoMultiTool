#pragma once

typedef unsigned int tTypeId;

template < typename T >
struct WithQuantity
{
    T item;
    unsigned int quantity = 0;
};

struct Ore
{
    tTypeId typeId;
    double volumePerUnit = 0.0;
    int basePricePerUnit = 0.0;
    std::vector< WithQuantity< tTypeId > > refinedProducts;
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
    LoadingAttributes,
    LoadingDogmaAttributes,
    LoadingDogmaEffects,
    Finalizing,
    Count
};