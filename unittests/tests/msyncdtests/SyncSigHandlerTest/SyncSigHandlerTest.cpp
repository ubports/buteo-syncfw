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



#include "SyncSigHandlerTest.h"

#include <QtTest/QtTest>
#include <QSignalSpy>

using namespace Buteo;

void SyncSigHandlerTest :: init()
{
    FUNCTION_CALL_TRACE;
    // initiate sighandler
    iSigHandler = new SyncSigHandler();
}

void SyncSigHandlerTest :: cleanup()
{
    FUNCTION_CALL_TRACE;
    delete iSigHandler ;
    iSigHandler = nullptr;
}


void SyncSigHandlerTest :: testSigTerm()
{
    FUNCTION_CALL_TRACE;
    //kill("TERM", 'pidof msyncd');
    LOG_DEBUG("Check there should not be any core dump in /home/user/Mydocs/core-dumps");
}

QTEST_MAIN(Buteo::SyncSigHandlerTest)
