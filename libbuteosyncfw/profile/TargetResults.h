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
#ifndef TARGETRESULTS_H
#define TARGETRESULTS_H

#include <QString>

class QDomDocument;
class QDomElement;

namespace Buteo {

class TargetResultsPrivate;
    
//! \brief Container for number of items added, deleted and modified.
struct ItemCounts {
	//! No. of Items added
    unsigned added;

    //! No. of Items deleted
    unsigned deleted;

    //! No. of Items modified
    unsigned modified;

    //! Default Constructor
    ItemCounts() : added(0), deleted(0), modified(0) { };

    //! Constructor with 3 parameters
    ItemCounts(unsigned aAdded, unsigned aDeleted, unsigned aModified)
        : added(aAdded), deleted(aDeleted), modified(aModified) {}
};

/*! \brief Sync results for one target.
 *
 * TargetResults contains information about how many items were added, deleted
 * and modified in a specific sync target during the sync session.
 */
class TargetResults
{
public:
    /*! \brief Copy constructor.
     *
     * \param aSource Copy source.
     */
    TargetResults(const TargetResults &aSource);

    /*! \brief Constructor.
     *
     * \param aTargetName Name of the target.
     * \param aLocalItems Counts of local items.
     * \param aRemoteItems Counts of remote items.
     */
    TargetResults(const QString &aTargetName, ItemCounts aLocalItems,
                  ItemCounts aRemoteItems);

    /*! \brief Constructs TargetResults from XML.
     *
     * \param aRoot Root element of the XML representation.
     */
    explicit TargetResults(const QDomElement &aRoot);

    /*! \brief Destructor.
     */
    ~TargetResults();

    /*! \brief Assignment operator.
     *
     * \param aRhs Source.
     */
    TargetResults& operator=(const TargetResults &aRhs);

    /*! \brief Exports the target results to XML.
     *
     * \param aDoc Parent document for the created XML elements. The created
     *  elements are not inserted to the document by this function, but the
     *  document is still required for creating the elements.
     * \return Root element of the created XML.
     */
    QDomElement toXml(QDomDocument &aDoc) const;

    /*! \brief Gets the target name.
     *
     * \return Target name.
     */
    QString targetName() const;

    /*! \brief Gets the counts of items added, deleted and modified locally.
     *
     * \return Item counts.
     */
    ItemCounts localItems() const;

    /*! \brief Gets the counts of items added, deleted and modified at remote.
     *
     * \return Item counts.
     */
    ItemCounts remoteItems() const;

private:

    TargetResultsPrivate *d_ptr;
};

}

#endif // TARGETRESULTS_H
