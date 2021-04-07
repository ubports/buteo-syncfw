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
#ifndef PROFILETEST_H
#define PROFILETEST_H

#include "Profile.h"
#include <QtTest/QtTest>

namespace Buteo {

class ProfileTest: public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();
    void cleanupTestCase();

    void testConstruction();
    void testProperties();
    void testKeys();
    void testFields();
    void testSubProfiles();
    void testValidate();
    void testMerge();
    void testXmlConversion();

private:

    Profile *loadFromXmlFile(const QString &aName, const QString &aType,
                             const QString &aProfileDir = QString());

    bool saveToXmlFile(const Profile &aProfile, const QString &aName,
                       bool aLocalOnly = true,
                       const QString &aProfileDir = QString());

    QString profileFileToString(const QString &aName, const QString &aType,
                                const QString &aProfileDir = QString());

};

}

#endif // PROFILETEST_H
