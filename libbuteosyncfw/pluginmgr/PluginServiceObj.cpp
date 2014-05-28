/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 Jolla Ltd. and/or its subsidiary(-ies).
*
* Author: Sateesh Kavuri <sateesh.kavuri@gmail.com>
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
*/
#include "PluginServiceObj.h"
#include <SyncResults.h>
#include <SyncProfile.h>

using namespace Buteo;

PluginServiceObj::PluginServiceObj(const QString aProfile,
                                   const QString aPluginName,
                                   QObject *parent) :
    iProfile(aProfile), iPluginName(aPluginName), QObject(parent)
{
}

PluginServiceObj::~PluginServiceObj()
{
}

void PluginServiceObj::abortSync(uchar aStatus)
{
}

void PluginServiceObj::cleanUp()
{
}

void PluginServiceObj::connectivityStateChanged(int aType, int aState)
{
}

QString PluginServiceObj::getPluginName()
{
}

QString PluginServiceObj::getProfileName()
{
}

QString PluginServiceObj::getSyncResults()
{
}

void PluginServiceObj::init()
{
}

QString PluginServiceObj::profile()
{
}

void PluginServiceObj::resume()
{
}

bool PluginServiceObj::startListen()
{
}

void PluginServiceObj::startSync()
{
}

void PluginServiceObj::stopListen()
{
}

void PluginServiceObj::suspend()
{
}

void PluginServiceObj::uninit()
{
}
