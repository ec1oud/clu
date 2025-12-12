/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2013 Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>

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
 * cfg.h
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "clu_enum.h"

/* preferences saved to ~/.xlog/xlog.cfg */
typedef struct
{
	int x;
	int y;
	int width;
	int height;
	gdouble latitude;
	int NS;
	gdouble longitude;
	int EW;
	int units;           /* kilometers or nautical miles */
	char *modes;
	char *bands;
	int bandseditbox;    /* optionmenu or entry for bands? */
	int modeseditbox;    /* optionmenu or entry modes? */
	int bandoptionmenu;  /* position of optionmenu */
	int modeoptionmenu;  /* position of optionmenu */
	int hamlib;  /* hamlib enabled? 0=no, 1=yes, 2=display freq on statusbar */
	int rigid;      /* what type of radio do you use? */
	char *device;   /* which serial port does it use? */
	char *rigconf;  /* string for rig_set_conf */
	int round;      /* how many digits to round to */
	int polltime;   /* poll every 'polltime' milliseconds, 0 = no polling */
	int clock;      /* clock on the statusbar? */
	char *logfont;
	int autosave;
	char *savedir;
	char *backupdir;
	int saving;    /* 1= autosave, 2=save with click on toolbar */
	int backup;    /* 1= standard, 2=safe */
	char *logstoload;
	int logorder;  /* 0=by last modification data, 1=alphabetically */
	char *locator;
	char *freefield1;
	char *freefield2;
	char *callsign;
	char *defaultmhz;
	char *defaultmode;
	char *defaulttxrst;
	char *defaultrxrst;
	char *defaultawards;
	char *defaultpower;
	char *defaultfreefield1;
	char *defaultfreefield2;
	char *defaultremarks;
	int *b4columns2;
	int *logcwidths2;
	int typeaheadfind;  /* do we want type ahead find */
	int remoteadding;   /* add remote data to log or qso frame */
	int viewtoolbar;    /* do we want to see the toolbar button */
	int viewb4;         /* do we want to see the worked before dialog */
	int b4x;
	int b4y;
	int b4width;
	int b4height;
	int saveasadif;
	int saveascabrillo;
	int *saveastsv2;
	int tsvcalc;
	int tsvsortbydxcc;
	int tsvgroupbycallsign;
	int handlebarpos;
	char *cwf1;
	char *cwf2;
	char *cwf3;
	char *cwf4;
	char *cwf5;
	char *cwf6;
	char *cwf7;
	char *cwf8;
	char *cwf9;
	char *cwf10;
	char *cwf11;
	char *cwf12;
	char *cwcq;
	char *cwsp;
	int cwspeed;
	int fcc;
	int viewscoring;
	int scorex;
	int scorey;
	int scorewidth;
	int scoreheight;
	int *scoringbands;
	int distqrb;
	int awardswac;  /* WAC visible in the scoring window */
	int awardswas;  /* WAS visible in the scoring window */
	int awardswaz;  /* WAZ visible in the scoring window */
	int awardsiota; /* IOTA visible in the scoring window */
	int awardsloc;  /* Locator visible in the scoring window */
	char *openurl;
	char *initlastmsg;
	int areyousure;      /* Display "Are You Sure" dialog on exit */
	int adif3_ok;        /* modes and bands in xlog.cfg updated to ADIF3 */
}
preferencestype;

void loadpreferences (void);
void savepreferences (void);

#endif	/* PREFERENCES_H */
