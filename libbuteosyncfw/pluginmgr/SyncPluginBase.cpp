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
#include "SyncPluginBase.h"
#include "PluginCbInterface.h"

using namespace Buteo;

SyncPluginBase::SyncPluginBase( const QString &aPluginName,
                                const QString &aProfileName,
                                PluginCbInterface *aCbInterface )
:   iCbInterface(aCbInterface),
    iPluginName(aPluginName),
    iProfileName(aProfileName)
{
    qRegisterMetaType<Sync::ConnectivityType>("Sync::ConnectivityType");
    qRegisterMetaType<Sync::TransferDatabase>("Sync::TransferDatabase");
    qRegisterMetaType<Sync::TransferType>("Sync::TransferType");
}

QString SyncPluginBase::getPluginName() const
{
    return iPluginName;
}

QString SyncPluginBase::getProfileName() const
{
    return iProfileName;
}

SyncResults SyncPluginBase::getSyncResults() const
{
    return SyncResults();
}
