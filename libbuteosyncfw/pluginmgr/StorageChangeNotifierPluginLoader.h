/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2021 Jolla Ltd.
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
#ifndef STORAGECHANGENOTIFIERPLUGINLOADER_H
#define STORAGECHANGENOTIFIERPLUGINLOADER_H

#include <QObject>

namespace Buteo {

class StorageChangeNotifierPlugin;

/*! \brief Base class for storage change notifier plugin loaders
 *
 */
class StorageChangeNotifierPluginLoader : public QObject
{
    Q_OBJECT
public:
    virtual StorageChangeNotifierPlugin *createPlugin(const QString &aStorageName) = 0;
};

}

Q_DECLARE_INTERFACE(Buteo::StorageChangeNotifierPluginLoader, "com.buteo.msyncd.StorageChangeNotifierPluginLoader/1.0")

#endif  //  STORAGECHANGENOTIFIERPLUGINLOADER_H
