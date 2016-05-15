/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "SpatialTemporalException.h"
#include<QDebug>

SpatialTemporalException::SpatialTemporalException(QString message)
{
    this->message = message;
}

SpatialTemporalException::SpatialTemporalException(const SpatialTemporalException &e)
{
    this->message = e.message;
}

QString SpatialTemporalException::getMessage()
{
    return this->message;
}
