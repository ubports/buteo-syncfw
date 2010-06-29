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

using namespace Buteo;

void logMessageHandler(QtMsgType aType, const char *aMsg)
{
     Logger::instance()->write(aType, aMsg);
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
:   iEnabledLevels(NUM_LEVELS, true),
    iIndentLevel(0),
    iIndentSize(aIndentSize),
    iFileStream(0),
    iStdOutStream(0),
    iStdErrStream(0)
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
    qInstallMsgHandler(logMessageHandler);

}

Logger::~Logger()
{
    // Restore default message handler.
    qInstallMsgHandler(0);

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
    if ((aLevel >= FIRST_CUSTOM_LEVEL)  && (aLevel < NUM_LEVELS))
    {
		disable(iEnabledLevels);
        enable(QBitArray(aLevel + 1, true));
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
}

void Logger::disable(const QBitArray &aLevels)
{
    QMutexLocker lock(&iMutex);
    iEnabledLevels &= ~(aLevels);
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
        "[FATAL] ",
        "[CRITICAL] ",
        "[WARNING] ",
        "[PROTOCOL] " ,
        "[INFO] ",
        "[DEBUG] ",
        "[TRACE] "
    };

    QMutexLocker lock(&iMutex);

    // Verify that the log message can and should be written.
    if (aLevel < 0 || aLevel >= NUM_LEVELS || !iEnabledLevels.testBit(aLevel))
        return;

    if (iFileStream != 0)
    {
        *iFileStream << QString(iIndentLevel, ' ') << levelTexts[aLevel] <<
           aMsg << "\n";
        iFileStream->flush();
    }

    if (aLevel >= LEVEL_FATAL && aLevel <= LEVEL_WARNING)
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

    if (LEVEL_FATAL == aLevel || QtFatalMsg == aLevel)
    {
        // Exit on fatal message.
        abort();
    } // no else
}

