/*
 *  Hamlib Interface - Rotator API header
 *  Copyright (c) 2000-2005 by Stephane Fillod
 *
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#ifndef _ROTATOR_H
#define _ROTATOR_H 1

/**
 * \addtogroup rotator
 * @{
 */

/**
 * \file rotator.h
 * \brief Hamlib rotator data structures.
 *
 * This file contains the data structures and declarations for the Hamlib
 * rotator Application Programming Interface (API).
 *
 * See the rotator.c file for more details on the rotator API functions.
 */

enum rig_errcode_e {
    RIG_OK = 0,     /*!< 0 No error, operation completed successfully */
    RIG_EINVAL,     /*!< 1 invalid parameter */
    RIG_ECONF,      /*!< 2 invalid configuration (serial,..) */
    RIG_ENOMEM,     /*!< 3 memory shortage */
    RIG_ENIMPL,     /*!< 4 function not implemented, but will be */
    RIG_ETIMEOUT,   /*!< 5 communication timed out */
    RIG_EIO,        /*!< 6 IO error, including open failed */
    RIG_EINTERNAL,  /*!< 7 Internal Hamlib error, huh! */
    RIG_EPROTO,     /*!< 8 Protocol error */
    RIG_ERJCTED,    /*!< 9 Command rejected by the rig */
    RIG_ETRUNC,     /*!< 10 Command performed, but arg truncated */
    RIG_ENAVAIL,    /*!< 11 Function not available */
    RIG_ENTARGET,   /*!< 12 VFO not targetable */
    RIG_BUSERROR,   /*!< 13 Error talking on the bus */
    RIG_BUSBUSY,    /*!< 14 Collision on the bus */
    RIG_EARG,       /*!< 15 NULL RIG handle or any invalid pointer parameter in get arg */
    RIG_EVFO,       /*!< 16 Invalid VFO */
    RIG_EDOM,       /*!< 17 Argument out of domain of func */
    RIG_EDEPRECATED,/*!< 18 Function deprecated */
    RIG_ESECURITY,  /*!< 19 Security error */
    RIG_EPOWER,     /*!< 20 Rig not powered on */
    RIG_ELIMIT,     /*!< 21 Limit exceeded */
    RIG_EACCESS,    /*!< 22 Access denied -- e.g. port already in use */
    RIG_EEND        // MUST BE LAST ITEM IN LAST
};

int longlat2locator(double longitude,
                               double latitude,
                               char *locator_res,
                               int pair_count);

int locator2longlat(double *longitude,
                               double *latitude,
                               const char *locator);

int qrb (double lon1,
                   double lat1,
                   double lon2,
                   double lat2,
                   double *distance,
                   double *azimuth);

double distance_long_path(double distance);

double azimuth_long_path(double azimuth);

double dms2dec(int degrees,
                       int minutes,
                       double seconds,
                       int sw);

int dec2dms(double dec,
                       int *degrees,
                       int *minutes,
                       double *seconds,
                       int *sw);

int dec2dmmm(double dec,
                        int *degrees,
                        double *minutes,
                        int *sw);

double dmmm2dec(int degrees,
                        double minutes,
                        double seconds,
                        int sw);

//! @endcond

/**
 * \def rot_debug
 * \brief Convenience macro for generating debugging messages.
    Purposely unimplemented so that debug output disappears;
    could be implemented with printf if we need it.
 */
#define rot_debug(level, format, args...) ((void)0)

#endif /* _ROTATOR_H */
