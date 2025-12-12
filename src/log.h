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
 * log.h
 */

#include "logfile/logfile.h"

/* struct for every log window */
typedef struct logtype
{
	GtkWidget *scrolledwindow;
	GtkWidget *treeview;
	GtkWidget *label;
	char *logname;
	char *filename;
	bool logchanged;
	bool readonly;
	int qsos;
	int columns;
	int logfields[QSO_FIELDS + 1];
}
logtype;

void backuplog (char *filename, char *backupfilename);
char *logname (char * filename);
int fillin_list (LOGDB * handle, qso_t q[], gpointer arg);
logtype *openlogwindow (LOGDB * lp, char * name, int page);
void savelog (gpointer arg, char * logfile, int type, int first, int last);
GPtrArray *getxlogs (char *path, char *patt);

