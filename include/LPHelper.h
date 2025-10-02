#pragma once
#include "HelperTypes.h"

#include <map>

#include <highs/Highs.h>

class Ore;
struct Blueprint;

class LPHelper
{
public:
    LPHelper( const TypeIdMap< const Ore >& ores );
    ~LPHelper() = default;

    bool SolveForBlueprint( const Blueprint& blueprint );
    const std::map< tTypeId, unsigned int >& GetResult() const;
    const std::map< tTypeId, unsigned int >& GetLeftover() const;

private:
    HighsLp BuildHighsLp( const Blueprint& blueprint ) const;
    std::map< tTypeId, double > ComputeTotalProduced() const;
    void ComputeLeftover( std::map< tTypeId, double >& produced, const Blueprint& blueprint );

private:
    const TypeIdMap< const Ore >& ores_;
    std::map< tTypeId, unsigned int > lpResult_;
    std::map< tTypeId, unsigned int > leftover_;

    std::vector< double > lowerBounds_;
    std::vector< double > upperBounds_;
    std::vector< double > objectiveCoefficients_;
    std::vector< HighsVarType > variableTypes_;

    std::vector< int > constraintRowStarts_;
    std::vector< int > constraintColumnIndices_;
    std::vector< double > constraintValues_;
    std::vector< double > constraintLowerBounds_;
    std::vector< double > constraintUpperBounds_;
};
