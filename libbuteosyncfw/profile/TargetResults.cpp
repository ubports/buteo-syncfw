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
#include "TargetResults.h"
#include "ProfileEngineDefs.h"
#include <QDomDocument>

namespace Buteo {
    
// Private implementation class for TargetResults
class TargetResultsPrivate
{
public:
    TargetResultsPrivate();

    TargetResultsPrivate(const TargetResultsPrivate &aSource);

    // Target name.
    QString iTargetName;

    ItemCounts iLocalItems;

    ItemCounts iRemoteItems;
};

}

using namespace Buteo;

TargetResultsPrivate::TargetResultsPrivate()
{
}

TargetResultsPrivate::TargetResultsPrivate(const TargetResultsPrivate &aSource)
:   iTargetName(aSource.iTargetName),
    iLocalItems(aSource.iLocalItems),
    iRemoteItems(aSource.iRemoteItems)
{
}

TargetResults::TargetResults(const TargetResults &aSource)
:   d_ptr(new TargetResultsPrivate(*aSource.d_ptr))
{
}

TargetResults::TargetResults(const QString &aTargetName,
    ItemCounts aLocalItems, ItemCounts aRemoteItems)
:   d_ptr(new TargetResultsPrivate())
{
    d_ptr->iTargetName = aTargetName;
    d_ptr->iLocalItems = aLocalItems;
    d_ptr->iRemoteItems = aRemoteItems;
}

TargetResults::TargetResults(const QDomElement &aRoot)
:   d_ptr(new TargetResultsPrivate())
{
    d_ptr->iTargetName = aRoot.attribute(ATTR_NAME);

    QDomElement local = aRoot.firstChildElement(TAG_LOCAL);
    if (!local.isNull())
    {
        d_ptr->iLocalItems.added = local.attribute(ATTR_ADDED).toUInt();
        d_ptr->iLocalItems.deleted = local.attribute(ATTR_DELETED).toUInt();
        d_ptr->iLocalItems.modified = local.attribute(ATTR_MODIFIED).toUInt();
    } // no else

    QDomElement remote = aRoot.firstChildElement(TAG_REMOTE);
    if (!remote.isNull())
    {
        d_ptr->iRemoteItems.added = remote.attribute(ATTR_ADDED).toUInt();
        d_ptr->iRemoteItems.deleted = remote.attribute(ATTR_DELETED).toUInt();
        d_ptr->iRemoteItems.modified = remote.attribute(ATTR_MODIFIED).toUInt();
    } // no else
}

TargetResults::~TargetResults()
{
    delete d_ptr;
    d_ptr = 0;
}

TargetResults& TargetResults::operator=(const TargetResults &aRhs)
{
    if (&aRhs != this)
    {
        delete d_ptr;
        d_ptr = new TargetResultsPrivate(*aRhs.d_ptr);
    }

    return *this;
}

QDomElement TargetResults::toXml(QDomDocument &aDoc) const
{
    QDomElement root = aDoc.createElement(TAG_TARGET_RESULTS);
    root.setAttribute(ATTR_NAME, d_ptr->iTargetName);

    QDomElement local = aDoc.createElement(TAG_LOCAL);
    local.setAttribute(ATTR_ADDED, d_ptr->iLocalItems.added);
    local.setAttribute(ATTR_DELETED, d_ptr->iLocalItems.deleted);
    local.setAttribute(ATTR_MODIFIED, d_ptr->iLocalItems.modified);
    root.appendChild(local);

    QDomElement remote = aDoc.createElement(TAG_REMOTE);
    remote.setAttribute(ATTR_ADDED, d_ptr->iRemoteItems.added);
    remote.setAttribute(ATTR_DELETED, d_ptr->iRemoteItems.deleted);
    remote.setAttribute(ATTR_MODIFIED, d_ptr->iRemoteItems.modified);
    root.appendChild(remote);

    return root;
}

QString TargetResults::targetName() const
{
    return d_ptr->iTargetName;
}

ItemCounts TargetResults::localItems() const
{
    return d_ptr->iLocalItems;
}

ItemCounts TargetResults::remoteItems() const
{
    return d_ptr->iRemoteItems;
}


