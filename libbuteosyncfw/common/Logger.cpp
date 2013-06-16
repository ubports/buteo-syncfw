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


#include "Logger.h"
#include <QMutexLocker>
#include <iostream>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <syslog.h>

using namespace Buteo;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void logMessageHandler(QtMsgType aType, const QMessageLogContext&, const QString& aMsg)
#else
void logMessageHandler(QtMsgType aType, const char *aMsg)
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Logger::instance()->write(aType, qPrintable(aMsg));
#else
  Logger::instance()->write(aType, aMsg);
#endif
}

const int Logger::DEFAULT_INDENT_SIZE = 4;

Logger *Logger::sInstance = NULL;

Logger *Logger::instance()
{
    if (NULL == sInstance)
    {
        createInstance();
    }
    return sInstance;
}

void Logger::createInstance(const QString &aLogFileName, bool aUseStdOut,
                            int aIndentSize)
{
    deleteInstance();
    sInstance = new Logger(aLogFileName, aUseStdOut, aIndentSize);
}

void Logger::deleteInstance()
{
    delete sInstance;
    sInstance = NULL;
}

Logger::Logger(const QString &aLogFileName, bool aUseStdOut, int aIndentSize)
:   iEnabledLevels(NUM_LEVELS),
    iIndentLevel(0),
    iIndentSize(aIndentSize),
    iFileStream(0),
    iStdOutStream(0),
    iStdErrStream(0),
    iEnabled(false)
{
    if (aUseStdOut)
    {
        // Create a stream for writing log messages to standard output.
        iStdOutStream = new QTextStream(stdout);
    } // no else

    iStdErrStream = new QTextStream(stderr);

    if (!aLogFileName.isEmpty())
    {
        // Create stream for writing log messages to a file.
        iFile.setFileName(aLogFileName);
        QFileInfo fileInfo(iFile);
        QDir dir;
        dir.mkpath(fileInfo.absolutePath());
        if (iFile.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            iFileStream = new QTextStream(&iFile);
        } // no else
    } // no else

    // Install our own message handler.
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    qInstallMessageHandler(logMessageHandler);
#else
    qInstallMsgHandler(logMessageHandler);
#endif
}

Logger::~Logger()
{
    // Restore default message handler.
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    qInstallMessageHandler(0);
#else
    qInstallMsgHandler(0);
#endif

    delete iFileStream;
    iFileStream = 0;

    delete iStdOutStream;
    iStdOutStream = 0;

    delete iStdErrStream;
    iStdErrStream = 0;
}


bool  Logger::setLogLevel(int aLevel)
{
    bool retVal = false;
    if ((aLevel > 0)  && (aLevel <= NUM_LEVELS))
    {
        disable(iEnabledLevels);
        QBitArray iLevels(NUM_LEVELS, false);
        for(int i = aLevel; i > 0; i--)
        {
            iLevels.setBit(NUM_LEVELS - i);
        }
        enable(iLevels);
        retVal = true;
    } // no else
    return retVal;
}

QBitArray Logger::getLogLevelArray()
{
        return iEnabledLevels;
}

void Logger::enable(const QBitArray &aLevels)
{
    QMutexLocker lock(&iMutex);
    iEnabledLevels |= aLevels;
    iEnabled = true;
}

void Logger::disable(const QBitArray &aLevels)
{
    QMutexLocker lock(&iMutex);
    iEnabledLevels &= ~(aLevels);
    iEnabled = false;
}

void Logger::push()
{
    QMutexLocker lock(&iMutex);
    iIndentLevel += iIndentSize;
}

void Logger::pop()
{
    QMutexLocker lock(&iMutex);
    iIndentLevel -= iIndentSize;
    if (iIndentLevel < 0)
        iIndentLevel = 0;
}

void Logger::write(int aLevel, const char *aMsg)
{
    static const char* levelTexts[NUM_LEVELS] = {
        "Debug: ",
        "Warning: ",
        "Critical: ",
        "Fatal: ",
    };

    int syslogLevel[NUM_LEVELS] = {
            LOG_DEBUG,
            LOG_WARNING,
            LOG_CRIT,
            LOG_CRIT,
    } ;

    QMutexLocker lock(&iMutex);

    // Verify that the log message can and should be written.
    if (aLevel < QtDebugMsg || aLevel > QtFatalMsg )
    {
        return;
    }

    // We don't make use of format specifiers in our logs, but syslog (vprintf) might interpret any % as part of
    // a format specifier and might fail (crash) when it doesn't find values corresponding to the format, prevent that.
    QString sysLogMsg = QString::fromLocal8Bit(aMsg);
    sysLogMsg = sysLogMsg.remove("%");

    if(!iEnabledLevels.count(true))
    {
        if(aLevel >= QtCriticalMsg)
        {
            syslog(LOG_CRIT, sysLogMsg.toLocal8Bit().data());
        }
        return;
    }

    // Verify that the log message can and should be written.
    if (!iEnabledLevels.testBit(aLevel))
    {
        return;
    }

    syslog(syslogLevel[aLevel], sysLogMsg.toLocal8Bit().data());

    if (iFileStream != 0)
    {
        *iFileStream << QString(iIndentLevel, ' ') << levelTexts[aLevel] <<
           aMsg << "\n";
        iFileStream->flush();
    }

    if (aLevel != QtDebugMsg)
    {
        if (iStdErrStream != 0)
        {
            *iStdErrStream << QString(iIndentLevel, ' ') << levelTexts[aLevel] <<
                   aMsg << "\n";
            iStdErrStream->flush();
        }
    }
    else if (iStdOutStream != 0)
    {
        *iStdOutStream << QString(iIndentLevel, ' ') << levelTexts[aLevel] <<
           aMsg << "\n";
        iStdOutStream->flush();
    }

    if (QtFatalMsg == aLevel)
    {
        // Exit on fatal message.
        abort();
    } // no else
}
