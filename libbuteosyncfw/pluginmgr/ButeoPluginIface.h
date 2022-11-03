/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -v -c ButeoPluginIface -p ButeoPluginIface.h:ButeoPluginIface.cpp com.buteo.msyncd.baseplugin.xml
 *
 * qdbusxml2cpp is Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef BUTEOPLUGINIFACE_H_1391581887
#define BUTEOPLUGINIFACE_H_1391581887

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include <SyncCommonDefs.h>

/*
 * Proxy class for interface com.buteo.msyncd.baseplugin
 */
class ButeoPluginIface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.buteo.msyncd.baseplugin"; }

public:
    ButeoPluginIface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~ButeoPluginIface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> abortSync(uchar aStatus)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(aStatus);
        return asyncCallWithArgumentList(QLatin1String("abortSync"), argumentList);
    }

    inline QDBusPendingReply<bool> cleanUp()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("cleanUp"), argumentList);
    }

    inline QDBusPendingReply<> connectivityStateChanged(int aType, bool aState)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(aType) << QVariant::fromValue(aState);
        return asyncCallWithArgumentList(QLatin1String("connectivityStateChanged"), argumentList);
    }

    inline QDBusPendingReply<QString> getSyncResults()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("getSyncResults"), argumentList);
    }

    inline QDBusPendingReply<bool> init()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("init"), argumentList);
    }

    inline QDBusPendingReply<> resume()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("resume"), argumentList);
    }

    inline QDBusPendingReply<bool> startListen()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("startListen"), argumentList);
    }

    inline QDBusPendingReply<bool> startSync()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("startSync"), argumentList);
    }

    inline QDBusPendingReply<> stopListen()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("stopListen"), argumentList);
    }

    inline QDBusPendingReply<> suspend()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("suspend"), argumentList);
    }

    inline QDBusPendingReply<bool> uninit()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("uninit"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void accquiredStorage(const QString &aMimeType);
    void error(const QString &aProfileName, const QString &aMessage, int aErrorCode);
    void newSession(const QString &aDestination);
    void success(const QString &aProfileName, const QString &aMessage);
    void syncProgressDetail(const QString &aProfileName, int aProgressDetail);
    void transferProgress(const QString &aProfileName, Sync::TransferDatabase aDatabase, Sync::TransferType aType, const QString &aMimeType, int aCommittedItems);
};

namespace com {
  namespace buteo {
    namespace msyncd {
      typedef ::ButeoPluginIface baseplugin;
    }
  }
}
#endif
