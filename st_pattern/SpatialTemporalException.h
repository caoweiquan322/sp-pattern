/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#ifndef SPATIALTEMPORALEXCEPTION_H
#define SPATIALTEMPORALEXCEPTION_H

#include<QException>
#include <QString>

/**
 * @brief The SpatialTemporalException class defines an exception used by this package. It contains a member describing cause of
 * this exception.
 */
class SpatialTemporalException : public QException
{
public:
    /**
     * @brief SpatialTemporalException is the default constructor.
     * @param message describes cause of this exception.
     */
    SpatialTemporalException(QString message);

    /**
     * @brief SpatialTemporalException is the copy constructor.
     * @param e is the SpatioTemporalException instance to copy.
     */
    SpatialTemporalException(const SpatialTemporalException &e);

    /**
      * @brief the deconstructor.
      */
    ~SpatialTemporalException() throw() {}

    /**
     * @brief getMessage retrieves the cause of this exception.
     * @return cause of this exception.
     */
    QString getMessage();

    /**
     * @brief raise just inherits from QException to complement the definition.
     */
    void raise() const { throw *this; }

    /**
     * @brief clone just inherits from QException to complement the definition.
     * @return the copied instance of this object.
     */
    SpatialTemporalException *clone() const { return new SpatialTemporalException(*this); }

protected:
    /**
     * @brief message stores cause of this exception. It would be shown to users to understand what happened before
     * the app crashed.
     */
    QString message;
};

#endif // SPATIALTEMPORALEXCEPTION_H
