/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2008 Joop Stakenborg <pg4i@amsat.org>

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
 * main.h - program state
 */

/* structure used for program state */
typedef struct
{
	int countries;         /* number of countries loaded */
	int qsos;              /* number of qso's read from the logs */
	bool controlkey;    /* control key is pressed */
	long long rigfrequency; /* frequency read from the rig */
	uint rigmode;          /* mode read from the rig */
	gchar *rigrst;          /* signal strength read from rig */
	uint rigpower;         /* rf power */
	int scounter;          /* counter for s-levels stored in array */
	int hlcounter;         /* counter for hamlib */
	bool tx;            /* transmitting or receiving */
	bool statustimer;   /* 'ready' timer for the statusbar */
	int shmid;             /* id for shared memory */
	int logwindows;        /* number of logwindows */
	gchar *searchstr;       /* array with logs/qsos seached */
	int dupecheck;         /* dupe check this log or all logs */
	bool notdupecheckmode;  /* exclude bands from dupecheck */
	bool notdupecheckband;  /* exclude modes from dupecheck */
	bool utf8error;     /* error in utf-8 conversion when reading the log */
	gchar *importremark;	/* remark added when importing from trlog or cabrillo */
	gchar *px;              /* prefix lookup used for countrymap */
	bool warning_nologopen;	/* No log open while receiving remote data warning dialog */
}
programstatetype;
