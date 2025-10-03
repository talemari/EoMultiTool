#include "Blueprint.h"
#include "LogManager.h"

#include <QJsonObject>
#include <QJsonarray>

Blueprint::Blueprint( const QJsonObject& jsonData )
{
    FromJsonObject( jsonData );
}

void Blueprint::FromJsonObject( const QJsonObject& jsonData )
{
    typeId_ = jsonData.value( "_key" ).toInt();
    if ( typeId_ == 0 )
    {
        LOG_WARNING( "Blueprint does not contain a valid typeId." );
        return;
    }

    if ( jsonData.contains( "activities" ) && !jsonData.value( "activities" ).isNull() )
    {
        QJsonObject activitiesObj = jsonData.value( "activities" ).toObject();
        if ( activitiesObj.contains( "manufacturing" ) && !activitiesObj.value( "manufacturing" ).isNull() )
        {
            manufacturingJob_ = std::make_shared< ManufacturingJob >( activitiesObj.value( "manufacturing" ).toObject() );
        }
        else
        {
            LOG_NOTICE( "Blueprint id {} does not contain manufacturing job data.", typeId_ );
            return;
        }
    }
    if ( !manufacturingJob_->IsValid() )
    {
        LOG_WARNING( "Blueprint id {}'s manufacturing job is invalid", typeId_ );
        return;
    }
    isValid_ = true;
}

QJsonObject Blueprint::ToJsonObject() const
{
    QJsonObject obj;

    if ( manufacturingJob_ == nullptr || !manufacturingJob_->IsValid() )
        return QJsonObject();

    obj[ "_key" ] = QString::number( typeId_ );
    QJsonObject activitiesObj;
    activitiesObj[ "manufacturing" ] = manufacturingJob_->ToJsonObject();
    obj[ "activities" ] = activitiesObj;

    return obj;
}

void Blueprint::PostLoadingInitialization()
{
    manufacturingJob_->FilterComponents();
}

const std::shared_ptr< ManufacturingJob > Blueprint::GetManufacturingJob() const
{
    return manufacturingJob_;
}
