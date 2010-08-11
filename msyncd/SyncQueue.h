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
#ifndef SYNCQUEUE_H
#define SYNCQUEUE_H

#include <QQueue>

namespace Buteo {
    
class SyncSession;

/*! \brief Class for queuing sync sessions.
 *
 * The queue is sorted every time when new items are added to it, so that
 * the sync sessions with highest priority will be at the front of the queue.
 */
class SyncQueue
{
public:
    /*! \brief Adds a new profile to the queue. Queue is sorted automatically.
     *
     * \param aSession Session to add to queue
     */
    void enqueue(SyncSession *aSession);

    /*! \brief Removes the first item from the queue and returns it.
     *
     * \return The removed item. NULL if the queue was empty.
     */
    SyncSession *dequeue();

    /*! \brief Returns the first item in the queue but does not remove it.
     *
     * \return First item of the queue. NULL if the queue is empty.
     */
    SyncSession *head();

    /*! \brief Checks if the queue is empty.
     *
     * \return Is the queue empty.
     */
    bool isEmpty() const;

    /*! \brief Checks if a profile with the given name is in the queue.
     *
     * \return Is the profile in the queue.
     */
    bool contains(const QString &aProfileName) const;

private:

    void sort();

    QQueue<SyncSession*> iItems;
};

}

#endif // SYNCQUEUE_H
