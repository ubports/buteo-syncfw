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

#ifndef USBMODEDPROXY_H_1272105195
#define USBMODEDPROXY_H_1272105195

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

namespace Buteo
{
    /*
     * Proxy class for interface com.meego.usb_moded
     */
    class USBModedProxy: public QDBusAbstractInterface
    {
        Q_OBJECT
    public:
        static inline const char *staticInterfaceName()
        { return "com.meego.usb_moded"; }

    public:
        USBModedProxy(QObject *parent = 0);

        ~USBModedProxy();

        /// Returns true if the USB cable is connected in the Ovi Suite mode,
        // false, if it's in any other mode, or if it isn't connected
        bool isUSBConnected();

    public Q_SLOTS: // METHODS

        void slotModeChanged(const QString &mode);
        inline QDBusPendingReply<QString> mode_request()
        {
            QList<QVariant> argumentList;
            return asyncCallWithArgumentList(QLatin1String("mode_request"), argumentList);
        }

    Q_SIGNALS: // SIGNALS

        void sig_usb_state_ind(const QString &mode);

        void usbConnection(bool bConnected);
    };
}

namespace com {
  namespace meego {
    typedef Buteo::USBModedProxy usb_moded;
  }
}
#endif

