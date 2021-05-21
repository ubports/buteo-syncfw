/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2014 Jolla Ltd.
 *
 * Contact: Valerio Valerio <valerio.valerio@jolla.com>
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

#ifndef UNITTEST_H
#define UNITTEST_H

#include <QtGlobal>

/*!
 * \brief A dirty hack to allow modified behavior during unit test execution.
 *
 * If you are writing a unit test, \c include(msyncd/unittest.pri) in your
 * project file.
 */
#define SYNCFW_UNIT_TESTS_RUNTIME Q_UNLIKELY(__SYNCFW_UNIT_TESTS_RUNTIME)

/*! \cond __false */
extern bool __SYNCFW_UNIT_TESTS_RUNTIME;
/*! \endcond __false */

#endif // UNITTEST_H
