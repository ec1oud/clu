/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2017 Andy Stewart KB1OIQ <kb1oiq@arrl.net>

   This file is part of xlog.

   Xlog is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Xlog is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with xlog.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
 * adif3.h
 */

#ifndef _ADIF3_H
#define _ADIF3_H 1

extern preferencestype preferences;

/*
 * fields to be stored in the adif file
 */

static const gint adif_fields[] = { DATE, GMT, CALL, BAND, MODE, RST, MYRST,
	QSLOUT, QSLIN, REMARKS, POWER, GMTEND, NAME, QTH, LOCATOR, U1, U2 };
static const gint adif_field_nr = 17;

static LOGDB *adif_handle;
static gint (*adif_fn)(LOGDB*, qso_t*, gpointer);
static gpointer adif_arg;
static qso_t q[QSO_FIELDS];
static gint adif_field = -1;

static gint adif_open(LOGDB*);
static void adif_close(LOGDB*);
static gint adif_create(LOGDB*);
static gint adif_qso_append(LOGDB*, const qso_t*);
static gint adif_qso_foreach(LOGDB*, gint (*fn)(LOGDB*, qso_t*, gpointer), gpointer);
static const gchar *xlog2adif_name(gint);

/*
 * mode2enum converts all possible modes and submodes to an enumerated type.  That's fine for the ADIF3
 * <SUBMODE> field, but another function is needed to convert submodes to modes for the
 * <MODE> field.
*/

static gint adif3_mode2enum (gchar * str);

const struct log_ops adif3_ops = {
	.open =		adif_open,
	.close =	adif_close,
	.create =	adif_create,
	.qso_append =	adif_qso_append,   /* used for export */
	.qso_foreach =	adif_qso_foreach,  /* used for import */
	.type =		TYPE_ADIF3,
	.name =		"ADIF3",
	.extension =	".adi",
};

#endif /* _ADIF3_H */
