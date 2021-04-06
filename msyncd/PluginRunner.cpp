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

#include "PluginRunner.h"
#include "LogMacros.h"

using namespace Buteo;

PluginRunner::PluginRunner(PluginType aPluginType, const QString &aPluginName,
                           PluginManager *aPluginMgr, PluginCbInterface *aPluginCbIf, QObject *aParent)
    :   QObject(aParent),
        iInitialized(false),
        iPluginMgr(aPluginMgr),
        iPluginCbIf(aPluginCbIf),
        iType(aPluginType),
        iPluginName(aPluginName)
{
    FUNCTION_CALL_TRACE;

    // register various metatypes used in DBus arguments
    qRegisterMetaType<Sync::SyncStatus>("Sync::SyncStatus");
    qRegisterMetaType<Sync::TransferDatabase>("Sync::TransferDatabase");
    qRegisterMetaType<Sync::TransferType>("Sync::TransferType");
    qRegisterMetaType<Sync::ConnectivityType>("Sync::ConnectivityType");
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
}

PluginRunner::PluginType PluginRunner::pluginType() const
{
    FUNCTION_CALL_TRACE;

    return iType;
}

QString PluginRunner::pluginName() const
{
    FUNCTION_CALL_TRACE;

    return iPluginName;
}



