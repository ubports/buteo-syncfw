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


#ifndef LOGGER_H
#define LOGGER_H

#include <QBitArray>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QMutex>
#include <QString>
#include <QTextStream>


namespace Buteo {
    
/*!
 * \brief A logger singleton class.
 *
 * Using the logger is thread safe, but creating the instance is not.
 * Make sure that the instance is created before any threads that use the
 * logger are started. This can be done by calling createInstance explicitly,
 * or by simply logging some message, which will automatically create the
 * logger instance with default parameters.
 */
class Logger
{
public:
    
    static const int NUM_LEVELS = 4;
    
    //! Default indent size.
    static const int DEFAULT_INDENT_SIZE;

    /*!
     * \brief Returns the logger instance.
     *
     * If the instance is not yet created, creates it with default parameters.
     * @return The instance.
     */
    static Logger *instance();

    //! Destructor.
    ~Logger();

    /*!
     * \brief Creates a logger instance.
     *
     * If an instance already exists, deletes the old instance first.
     * This function should be called in the beginning of the program
     * before any threads using the logger are created, because creating
     * the log instance is not thread safe.
     * @param aLogFileName Name of the file where log messages are written.
     *          If this is empty, messages are not written to a file.
     * @param aUseStdOut Should messages be written to standard output.
     * @param aIndentSize Number of spaces that each indent level inserts.
     */
    static void createInstance(const QString &aLogFileName = "",
                               bool aUseStdOut = false,
                               int aIndentSize = DEFAULT_INDENT_SIZE);

    //! Deletes the logger instance. Closes the log file in a controlled way.
    static void deleteInstance();

    /*!
     * \brief Enables given log levels.
     * @param aLevels Log levels to enable. Default enables all levels.
     */
    void enable(const QBitArray &aLevels = QBitArray(NUM_LEVELS, true));

    /*!
     * \brief Disables given log levels.
     * @param aLevels Log levels to disable. Default disables all levels.
     */
    void disable(const QBitArray &aLevels = QBitArray(NUM_LEVELS, true));

    /*!
     * \brief Adds one indent level.
     */
    void push();

    /*!
     * \brief Removes one indent level.
     */
    void pop();

    /*!
     * \brief Writes a message to the log.
     *
     * @param aLevel Message level.
     * @param aMsg Message.
     */
    void write(int aLevel, const char *aMsg);

    /*!
     * \brief Sets logging level.
     *
     * Messages with the given level and levels more severe than it will be
     * enabled. Qt built-in log levels will also be enabled.
     * @param aLevel Logging level.
     */
    bool setLogLevel(int aLevel);

    /*!
     * \brief Gets logging level BitArray
     *
     * Use this API to count the levels the BitArray has been set to true
     */
    QBitArray getLogLevelArray();

    bool enabled(){return iEnabled;}

private:
    Logger(const QString &aLogFileName, bool aUseStdOut, int aIndentSize);

    static Logger *sInstance;

    QBitArray   iEnabledLevels;

    int         iIndentLevel;

    int         iIndentSize;

    QFile       iFile;

    QTextStream *iFileStream;

    QTextStream *iStdOutStream;

    QTextStream *iStdErrStream;

    QMutex      iMutex;

    bool        iEnabled;
};

}

#endif // LOGGER_H

