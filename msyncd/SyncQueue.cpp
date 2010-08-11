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
#include "SyncQueue.h"
#include "SyncSession.h"
#include "SyncProfile.h"
#include "LogMacros.h"

using namespace Buteo;

void SyncQueue::enqueue(SyncSession *aSession)
{
    FUNCTION_CALL_TRACE;

    iItems.enqueue(aSession);
    sort();
}

SyncSession *SyncQueue::dequeue()
{
    FUNCTION_CALL_TRACE;

    SyncSession *p = NULL;

    if (!iItems.isEmpty())
    {
        p = iItems.dequeue();
    } // no else

    return p;
}

SyncSession *SyncQueue::head()
{
    FUNCTION_CALL_TRACE;

    SyncSession *p = NULL;
    if (!iItems.isEmpty())
    {
        p = iItems.head();
    } // no else

    return p;
}

bool SyncQueue::isEmpty() const
{
    FUNCTION_CALL_TRACE;

    return iItems.isEmpty();
}

bool SyncQueue::contains(const QString &aProfileName) const
{
    FUNCTION_CALL_TRACE;

    QQueue<SyncSession*>::const_iterator i;
    for (i = iItems.begin(); i != iItems.end(); ++i)
    {
        if ((*i)->profileName() == aProfileName)
            return true;
    }

    return false;
}

bool syncSessionPointerLessThan(SyncSession *&aLhs, SyncSession *&aRhs)
{
    if (aLhs && aRhs) {
        // Manual sync has higher priority than scheduled sync.
        if (aLhs->isScheduled() != aRhs->isScheduled())
            return !aLhs->isScheduled();

        SyncProfile *lhsProfile = aLhs->profile();
        SyncProfile *rhsProfile = aRhs->profile();
        if (lhsProfile == 0 || rhsProfile == 0)
            return false;

        // Device sync has higher priority than online sync.
        SyncProfile::DestinationType lhsDestType = lhsProfile->destinationType();
        SyncProfile::DestinationType rhsDestType = rhsProfile->destinationType();
        if (lhsDestType != rhsDestType)
            return (lhsDestType == SyncProfile::DESTINATION_TYPE_DEVICE);
    }

    return false;
}

void SyncQueue::sort()
{
    FUNCTION_CALL_TRACE;

    // @todo: Sort queued profiles using some criteria.
}
