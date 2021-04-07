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

#ifndef SYNCSIGHANDLER_H
#define SYNCSIGHANDLER_H

#include <QSocketNotifier>
/*! \brief   About this class.
 *     We can't call Qt functions from Unix signal handlers.We can only call async-signal-safe functions from signal handlers.
 *          So this provides a way to use Unix signal handlers with Qt. The strategy is to have our Unix signal handler will eventually
 *          cause a Qt signal to be emitted, and then we simply return from our Unix signal handler.
 *          Back in our Qt program, that Qt signal gets emitted and then received by our Qt slot function, where we are safely doing Qt stuff
 *          which weren't allowed to do in the Unix signal handler.
 *    One simple way to make this happen is declares a socket pair in our class for each Unix signal we want to handle.
 *          The socket pairs are declared as static data members.We also created a QSocketNotifier to monitor the read end of each socket pair,
 *          declare your Unix signal handlers to be static class methods, and declare a slot function corresponding to each of our
 *          Unix signal handlers. In this class, we intend to handle both the SIGHUP and SIGTERM signals.
 */
class SyncSigHandler : public QObject
{
    Q_OBJECT

public:
    /*! \brief Constructor
     *
     * @param aParent object
     * @param aName const char
     */
    SyncSigHandler(QObject *aParent = 0, const char *aName = 0);

    /*! \brief Destructor
     *
     */
    ~SyncSigHandler();

    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);

public slots:
    /*! \brief QT signal handler to handle SIG_HUP
     *
     * @return None
     */
    void handleSigHup();

    /*! \brief QT signal handler to handle SIG_TERM
     *
     * @return None
     */
    void handleSigTerm();

private:
    //socket pair for each Unix signal to handle
    static int iSigHupFd[2];
    static int iSigTermFd[2];

    //QSocketNotifier to monitor the read end of each socket pair,
    // declare your Unix signal handlers to be static class methods
    QSocketNotifier *iSigHup;
    QSocketNotifier *iSigTerm;

#ifdef SYNCFW_UNIT_TESTS
    friend class SyncSigHandlerTest;
#endif
};


#endif // SYNCSIGHANDLER_H
