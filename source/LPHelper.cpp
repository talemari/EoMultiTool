#include "LPHelper.h"
#include "Blueprint.h"
#include "LogManager.h"
#include "Ore.h"

LPHelper::LPHelper( const TypeIdMap< const Ore >& ores )
    : ores_( ores )
{
    for ( const auto& [ oreId, orePtr ] : ores_ )
    {
        lowerBounds_.push_back( 0.0 );
        upperBounds_.push_back( kHighsInf );
        objectiveCoefficients_.push_back( orePtr->GetBasePrice() );
        variableTypes_.push_back( HighsVarType::kInteger );
    }
}

bool LPHelper::SolveForBlueprint( const Blueprint& blueprint )
{
    lpResult_.clear();
    leftover_.clear();
    const auto& manufacturingJob = blueprint.GetManufacturingJob();
    constraintRowStarts_.clear();
    constraintColumnIndices_.clear();
    constraintValues_.clear();
    constraintLowerBounds_.clear();
    constraintUpperBounds_.clear();

    for ( const auto& requirement : manufacturingJob.matRequirements )
    {
        int rowStart = static_cast< int >( constraintColumnIndices_.size() );
        constraintRowStarts_.push_back( rowStart );

        int oreIndex = 0;
        for ( auto& [ oreId, orePtr ] : ores_ )
        {
            double yield = 0.0;
            for ( const auto& product : orePtr->GetRefinedProducts() )
            {
                if ( product.item == requirement.item )
                {
                    yield = static_cast< double >( product.quantity ) / 100.0;
                    break;
                }
            }
            if ( yield > 0.0 )
            {
                constraintColumnIndices_.push_back( oreIndex );
                constraintValues_.push_back( yield );
            }
            ++oreIndex;
        }
        constraintLowerBounds_.push_back( static_cast< double >( requirement.quantity ) );
        constraintUpperBounds_.push_back( kHighsInf );
    }
    constraintRowStarts_.push_back( static_cast< int >( constraintColumnIndices_.size() ) );
    HighsLp lp = BuildHighsLp( blueprint );
    Highs highs;
    highs.passModel( lp );

    for ( int i = 0; i < static_cast< int >( variableTypes_.size() ); i++ )
        highs.changeColIntegrality( i, variableTypes_[ i ] );

    HighsStatus status = highs.run();
    if ( status != HighsStatus::kOk )
    {
        LOG_WARNING( "Failed to solve LP" );
        return false;
    }

    HighsModelStatus modelStatus = highs.getModelStatus();
    if ( modelStatus == HighsModelStatus::kInfeasible )
    {
        LOG_WARNING( "Failed to solve LP : Judged infeasible" );
        return false;
    }

    const HighsSolution& sol = highs.getSolution();
    int oreIndex = 0;
    for ( const auto& [ oreId, orePtr ] : ores_ )
    {
        double quantity = sol.col_value[ oreIndex ];
        if ( quantity > 0.0 )
            lpResult_[ oreId ] = static_cast< unsigned int >( std::ceil( quantity ) );
        ++oreIndex;
    }

    auto produced = ComputeTotalProduced();
    ComputeLeftover( produced, blueprint );

    return true;
}

const std::map< tTypeId, unsigned int >& LPHelper::GetResult() const
{
    return lpResult_;
}

const std::map< tTypeId, unsigned int >& LPHelper::GetLeftover() const
{
    return leftover_;
}

HighsLp LPHelper::BuildHighsLp( const Blueprint& blueprint ) const
{
    HighsLp lp;
    lp.num_col_ = static_cast< int >( ores_.size() );
    lp.num_row_ = static_cast< int >( blueprint.GetManufacturingJob().matRequirements.size() );

    lp.col_cost_ = objectiveCoefficients_;
    lp.col_lower_ = lowerBounds_;
    lp.col_upper_ = upperBounds_;
    lp.row_lower_ = constraintLowerBounds_;
    lp.row_upper_ = constraintUpperBounds_;

    lp.a_matrix_.format_ = MatrixFormat::kRowwise;
    lp.a_matrix_.start_ = constraintRowStarts_;
    lp.a_matrix_.index_ = constraintColumnIndices_;
    lp.a_matrix_.value_ = constraintValues_;

    return lp;
}

std::map< tTypeId, double > LPHelper::ComputeTotalProduced() const
{
    std::map< tTypeId, double > produced;
    int oreIndex = 0;
    for ( auto& kv : ores_ )
    {
        tTypeId oreId = kv.first;
        const std::shared_ptr< const Ore >& ore = kv.second;

        unsigned int oreCount = lpResult_.count( oreId ) ? lpResult_.at( oreId ) : 0;
        if ( oreCount == 0 )
        {
            oreIndex++;
            continue;
        }

        for ( const auto& product : ore->GetRefinedProducts() )
            produced[ product.item ] += oreCount * product.quantity;

        oreIndex++;
    }
    return produced;
}

void LPHelper::ComputeLeftover( std::map< tTypeId, double >& produced, const Blueprint& blueprint )
{
    for ( auto& kv : produced )
    {
        tTypeId typeId = kv.first;
        double totalProduced = kv.second;

        double required = 0.0;
        const auto& job = blueprint.GetManufacturingJob();
        for ( const auto& req : job.matRequirements )
        {
            if ( req.item == typeId )
            {
                required = req.quantity;
                break;
            }
        }

        double extra = totalProduced - required;

        if ( extra > 0.5 )
            leftover_[ typeId ] = static_cast< unsigned int >( std::round( extra ) );
    }
}
