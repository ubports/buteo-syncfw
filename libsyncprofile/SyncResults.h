/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */


#ifndef SYNCRESULTS_H_2
#define SYNCRESULTS_H_2

#include <QDateTime>
#include <QList>
#include "TargetResults.h"

class QDomDocument;
class QDomElement;

namespace Buteo {

class SyncResultsPrivate;
    
/*! \brief Contains information about a completed synchronization session.
 *
 * SyncResults consists of the sync time/date, result code and one
 * TargetResults object for each sync target/storage (calendar, bookmarks
 * etc.).
 */
class SyncResults
{
public:

    enum SyncResultCode
    {
        SYNC_RESULT_SUCCESS,
        SYNC_RESULT_FAILED
    };

    /*! \brief Constructs an empty sync results object.
     *
     * Sync time is set to current time, result code should be set later by
     * calling setResultCode.
     */
    SyncResults();

    /*! \brief Copy constructor.
     *
     * \param aSource Copy source.
     */
    SyncResults(const SyncResults &aSource);

    /*! \brief Constructs sync results, sets sync time and result code.
     *
     * \param aTime Sync time for the results.
     * \param aResultCode Sync result code.
     */
    SyncResults(QDateTime aTime, int aResultCode);

    /*! \brief Constructs sync results from XML.
     *
     * \param aRoot Root element of the XML representation.
     */
    explicit SyncResults(const QDomElement &aRoot);

    /*! \brief Destructor.
     */
    ~SyncResults();

    /*! \brief Assignment operator.
     *
     * \param aRhs Source.
     */
    SyncResults& operator=(const SyncResults &aRhs);

    /*! \brief Exports the sync results to XML.
     *
     * \param aDoc Parent document for the created XML elements. The created
     *  elements are not inserted to the document by this function, but the
     *  document is still required for creating the elements.
     * \return Root element of the created XML.
     */
    QDomElement toXml(QDomDocument &aDoc) const;

    /*! \brief Gets the results of all targets.
     *
     * \return List of target results.
     */
    QList<TargetResults> targetResults() const;

    /*! \brief Adds target results to this object.
     *
     * \param aResults The target results to add.
     */
    void addTargetResults(const TargetResults &aResults);

    /*! \brief Gets the sync time.
     *
     * \return Sync time.
     */
    QDateTime syncTime() const;

    /*! \brief Gets the result code.
     *
     * \return Result code.
     */
    int resultCode() const;

    /*! \brief Sets the result code.
     *
     * \param aResultCode The new result code.
     */
    void setResultCode(int aResultCode);
    
    /*! \brief Sets the remote target Id.
     *
     * \param aTargetId The remote device Id.
     */
    void setTargetId(const QString& aTargetId) ;
    
    /*! \brief Gets the remote target Id.
     *
     * \param TargetId The remote device Id.
     */
    QString getTargetId() const;

    /*! \brief Compares two results objects by sync time.
     *
     * The object with earlier sync time is smaller.
     * \param aOther Point of comparison.
     */
    bool operator<(const SyncResults &aOther) const;

    /*! \brief Sets if the results are from a scheduled sync.
     *
     * \param aScheduled True if this is a scheduled sync.
     */
    void setScheduled(bool aScheduled);

    /*! \brief Checks if the results are from a scheduled sync.
     *
     * \return True if scheduled.
     */
    bool isScheduled() const;

private:

    SyncResultsPrivate *d_ptr;
    
    friend class ClientThreadTest;

};

}
#endif // SYNCRESULTS_H_2
