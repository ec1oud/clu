/*
   clu - callsign lookup program
   adapted from xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2008 Joop Stakenborg <pg4i@amsat.org>
   Copyright (C) 2020 Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2025 Shawn Rutledge <s@ecloud.org>

   clu is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   clu is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with clu.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <locale.h>
#include <string.h>

// --- cruft to remove ---
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "gui_b4window.h"
#include "gui_scorewindow.h"
#include "gui_mainwindow.h"
#include "gui_setupdialog.h"
#include "gui_warningdialog.h"
#include "gui_utils.h"
#include "support.h"
#include "callbacks_mainwindow_menu.h"
#include "callbacks_mainwindow_qsoframe.h"
#include "callbacks_mainwindow.h"
#include "cfg.h"
#include "log.h"
#include "utils.h"
#include "dxcc.h"
#include "remote.h"
#include "history.h"
#include "main.h"
#include "hamlib-utils.h"

GtkWidget *mainwindow, *mainnotebook;
gchar *xlogdir;
gint clocktimer = -1, savetimer = -1;
gint sockettimer_2333 = -1;
gint remotetimer_7311 = -1, sockettimer_7311 = -1;
gchar **qso = NULL;
programstatetype programstate;
glong msgid_7311;
GList *logwindowlist = NULL;
GdkColormap *colormap;
GIOChannel *channel_2333;
GIOFlags flags_2333;
GIOChannel *channel_7311;

extern GtkWidget *b4window, *scorewindow;
extern preferencestype preferences;
extern remotetype remote;
extern gint server_sockfd_7311;
extern gint server_sockfd_2333;
extern GtkUIManager *ui_manager;
extern GtkPrintSettings *print_settings;
extern GtkPageSetup *print_page_setup;
// --- end of cruft to remove ---

#include "dxcc.h"

/* command line options */
static void
parsecommandline (int argc, char *argv[])
{
	int p;

	while ((p = getopt (argc, argv, "hv")) != -1)
	{
		switch (p)
		{
			case ':':
			case '?':
			case 'h':
				printf ("Usage: %s [option] callsign\n",
					PACKAGE);
				printf ("	-h	Display this help and exit\n");
				printf ("	-v	Output version information and exit\n");
				exit (0);
		}
	}
}

/* the fun starts here */
int
main (int argc, char *argv[])
{
	parsecommandline(argc, argv);
}
