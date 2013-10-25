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


#ifndef LOGMACROS_H
#define LOGMACROS_H

#include <QString>
#include <QTime>
#include <QDebug>
#include <QDateTime>
#include "Logger.h"

//! Helper macro for writing log messages. Avoid using directly.
#define LOG_MSG_L(level, msg) if(Buteo::Logger::instance()->enabled())(QDebug((QtMsgType)(level)) << __FILE__ << __LINE__ << ":" << msg)
#define LOG_MSG_L_PLAIN(level, msg) if(Buteo::Logger::instance()->enabled())(QDebug((QtMsgType)(level)) << msg)

//! Macros for writing log messages. Use these.
//! Messages with level below warning are enabled only in debug builds.
#define LOG_FATAL(msg) qFatal(msg)
#define LOG_CRITICAL(msg) qCritical() << msg
#define LOG_WARNING(msg) qWarning() << msg

#if defined(BUTEO_ENABLE_DEBUG)
# define LOG_PROTOCOL(msg) qDebug() << msg
# define LOG_INFO(msg) qDebug() << msg
# define LOG_DEBUG(msg) qDebug() << msg
# define LOG_TRACE(msg) qDebug() << msg
# define LOG_TRACE_PLAIN(msg) qDebug() << msg
 /*!
  * Creates a trace message to log when the function is entered and exited.
  * Logs also to time spent in the function.
  */
# define FUNCTION_CALL_TRACE Buteo::LogTimer timerDebugVariable(QString(__PRETTY_FUNCTION__));
#else
# define LOG_PROTOCOL(msg) if (false) qDebug() << msg
# define LOG_INFO(msg) if (false) qDebug() << msg
# define LOG_DEBUG(msg) if (false) qDebug() << msg
# define LOG_TRACE(msg) if (false) qDebug() << msg
# define LOG_TRACE_PLAIN(msg) if (false) qDebug() << msg
# define FUNCTION_CALL_TRACE
#endif

namespace Buteo {

/*!
 * \brief Helper class for timing function execution time.
 */
class LogTimer
{
public:
    /*!
    * \brief Constructor. Creates an entry message to the log.
    *
    * @param aFunc Name of the function.
    */
    LogTimer(const QString &aFunc) : iFunc(aFunc)
    {
        LOG_TRACE_PLAIN(iFunc << ":Entry");
        iTimer.start();
    }

    /*!
     * \brief Destructor. Creates an exit message to the log, including
     *        function execution time.
     */
    ~LogTimer()
    {
        LOG_TRACE_PLAIN(iFunc << ":Exit, execution time:" << iTimer.elapsed()
            << "ms");
    }

private:
    QTime iTimer;
    QString iFunc;
};

}

#endif // LOGMACROS_H

