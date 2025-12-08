/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2021 Andy Stewart <kb1oiq@arrl.net>
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
 * Specifications from https://adif.org/309/ADIF_309.htm
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <time.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <locale.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>

#include "logfile.h"
#include "../cfg.h"
#include "../utils.h"
#include "../clu_enum.h"

#ifndef HAVE_STRPTIME
#include "../strptime.h"
#define strptime(s,f,t) mystrptime(s,f,t)
#endif

#include "adif3.h"

static gint adif_qso_foreach
(LOGDB *handle,  gint (*fn)(LOGDB*, qso_t*, gpointer arg), gpointer arg)
{
  gchar line [1024], sdate[16], *buffer, *p, *tmp;
  gchar **adifline, **adifitem, **adifid;
  gint ret, items, i=0;
  struct tm timestruct;
  FILE *fp = (FILE *) handle->priv;
  adif_handle = handle;
  adif_fn = fn;
  adif_arg = arg;

  memset (q, 0, sizeof (q));
  buffer = g_strdup ("");
  while (!feof (fp))
  {
    if (!fgets (line, 1023, fp)) break;

	if (line[0] != '\n') /* ignore empty lines */
	  buffer = g_strconcat (buffer, line, NULL);
	else continue;

	if (g_strrstr (line, "<EOH>") || g_strrstr (line, "<eoh>")) /* read past header */
	  buffer = g_strdup ("");

	if (g_strrstr (line, "<EOR>") || g_strrstr (line, "<eor>")) /* read past one record */
	{
	  /* let's do this simple, check if we have multiple '<' */
	  buffer = my_strreplace (buffer, "\n", "");
	  adifline = g_strsplit (buffer, "<", 0);
	  for (items = 0;; items++)
	    if (!adifline || adifline[items] == NULL) break;

	  if (items > 2) /* we have a valid adif line*/
	  {
		for (i = 0; i < items; i++)
		  if (g_strrstr (adifline[i], ">")
			  && g_strrstr (adifline[i], ":"))
			/* valid adif item */
			{
			  adifitem = g_strsplit (adifline[i], ">", 2);
			  /* adifitem [0] contains 'id' and string length */
			  adifid = g_strsplit (adifitem[0], ":", -1);
			  /* that's all we need, now fill in the fields */
			  if (!strcasecmp (adifid[0], "CALL"))
		      {
				adif_field = CALL;
				q[adif_field] =
				  g_strndup (adifitem[1], atoi(adifid[1]));
		      }
			  else if (!strcasecmp (adifid[0], "QSO_DATE"))
		      {
				sscanf(adifitem[1], "%4d%2d%2d", &timestruct.tm_year,
					   &timestruct.tm_mon,	&timestruct.tm_mday);
				timestruct.tm_year -= 1900;
				timestruct.tm_mon--;
				strftime (sdate, 16, "%d %b %Y", &timestruct);
				adif_field = DATE;
				q[adif_field] = g_strdup (sdate);
		      }
			  else if (!strcasecmp (adifid[0], "TIME_ON"))
				{
				  adif_field = GMT;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "TIME_OFF"))
				{
				  adif_field = GMTEND;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "BAND"))
				{
				  /* prefer FREQ over BAND */
				  if (!q[BAND])
					{
					  adif_field = BAND;
					  guint enumband = meters2enum (adifitem[1]);
					  q[adif_field] = band_enum2char (enumband);
					}
				}
			  else if (!strcasecmp (adifid[0], "FREQ"))
				{
				  /* prefer FREQ over BAND */
				  if (q[BAND])
					{
					  g_free(q[BAND]);
					  q[BAND] = NULL;
					}
				  adif_field = BAND;
				  if ((p = g_strrstr(adifitem[1], ",")))
					*p = '.';
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}

			  /* MODE and SUBMODE might both be specified as of ADIF 3.0.4.
				 Assume(!) that they are consistent in the imported ADIF3 file.
				 Give precedence to the SUBMODE if both occur.
			  */

			  else if (!strcasecmp (adifid[0], "MODE"))
				{
				  adif_field = MODE;
				  if (!q[adif_field]) {
					q[adif_field] = g_strndup (adifitem[1], atoi(adifid[1]));
				  }
				}
			  else if (!strcasecmp (adifid[0], "SUBMODE"))
				{
				  adif_field = MODE;
				  if (q[adif_field]) {
					g_free(q[adif_field]);
					q[adif_field] = NULL;
				  }
				  q[adif_field] = g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "TX_PWR"))
				{
				  adif_field = POWER;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "RST_SENT"))
				{
				  adif_field = RST;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "RST_RCVD"))
				{
				  adif_field = MYRST;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "QSL_SENT"))
				{
				  adif_field = QSLOUT;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "QSL_RCVD"))
				{
				  adif_field = QSLIN;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "QSL_VIA"))
				{
				  if (!strcasecmp (preferences.freefield1, "QSL_VIA"))
					{
					  adif_field = U1;
					  q[adif_field] =
						g_strndup (adifitem[1], atoi(adifid[1]));
					}
				  else if (!strcasecmp (preferences.freefield2, "QSL_VIA"))
					{
					  adif_field = U2;
					  q[adif_field] =
						g_strndup (adifitem[1], atoi(adifid[1]));
					}
				}
			  else if (!strcasecmp (adifid[0], "NAME"))
				{
				  adif_field = NAME;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "QTH"))
				{
				  adif_field = QTH;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "GRIDSQUARE"))
				{
				  adif_field = LOCATOR;
				  q[adif_field] =
					g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "COMMENT"))
				{
				  adif_field = REMARKS;
				  /* append comment to remarks */
				  if (q[REMARKS])
					{
					  tmp = g_strndup (adifitem[1], atoi(adifid[1]));
					  q[adif_field] =
						g_strconcat (q[adif_field], ", ", tmp, NULL);
					  g_free (tmp);
					}
				  else
					q[adif_field] =
					  g_strndup (adifitem[1], atoi(adifid[1]));
				}
			  else if (!strcasecmp (adifid[0], "NOTES"))
				{
				  adif_field = REMARKS;
				  /* append notes to remarks */
				  if (q[REMARKS])
					{
					  tmp = g_strndup (adifitem[1], atoi(adifid[1]));
					  q[adif_field] =
						g_strconcat (q[adif_field], ", ", tmp, NULL);
					  g_free (tmp);
					}
				  else
					q[adif_field] =
					  g_strndup (adifitem[1], atoi(adifid[1]));

				}
			  g_strfreev (adifid);
			  g_strfreev (adifitem);
			}
	  }
	  g_strfreev (adifline);
	  buffer = g_strdup ("");
	}
	if (adif_field != -1)
	  {
		/* fill in empty fields */
		for (i = 0; i < adif_field_nr; i++)
		  if ( !q[adif_fields[i]] ) q[adif_fields[i]] = g_strdup ("");
		ret = (*adif_fn)(adif_handle, q, adif_arg);
		adif_field = -1;
		memset(q, 0, sizeof(q));
		if (ret) return ret;
	  }
  }
  g_free (buffer);
  return 0;
}

/*
 * open for read
 */
gint adif_open(LOGDB *handle)
{
	FILE *fp;
	const gint xlog_fields [] = {DATE, GMT, GMTEND, CALL, BAND, MODE, RST, MYRST,
		QSLOUT, QSLIN, POWER, NAME, QTH, LOCATOR, U1, U2, REMARKS};

	fp = g_fopen(handle->path, "r");
	if (!fp) return -1;
	handle->priv = (gpointer)fp;

	handle->column_nr = adif_field_nr;
	memcpy (handle->column_fields, xlog_fields, sizeof (xlog_fields));
	/* TODO: set and use handle->column_widths */

	return 0;
}

/*
 * open for write
 */
gint adif_create(LOGDB *handle)
{
	FILE *fp;
	time_t timet;
	gchar sdate[32];

	fp = g_fopen(handle->path, "w");
	if (!fp) return -1;
	handle->priv = (gpointer)fp;

	/* use C locale for date */
	setlocale (LC_TIME, "C");
	time(&timet);
	strftime (sdate, 32, "%d %b %Y %T", localtime(&timet));
	setlocale (LC_TIME, "");

	/* write header */
	fprintf(fp, "ADIF Export from " PACKAGE " Version " VERSION "\n"
				"Copyright (C) 2012-2020 Andy Stewart <kb1oiq@arrl.net>\n"
				"Copyright (C) 2001-2010 Joop Stakenborg <pg4i@amsat.org>\n"
				"Internet:  http://savannah.nongnu.org/projects/Xlog\n\n"
				"Date of export: %s\n",
				sdate);


	if (preferences.callsign[0] != '\0') {
		fprintf(fp, "Callsign: %s\n", preferences.callsign);
	}
	if (preferences.locator[0] != '\0') {
		fprintf(fp, "Locator: %s\n", preferences.locator);
	}

	fprintf(fp, "\n<ADIF_VER:5>3.1.1\n" "<EOH>\n");
	return 0;
}

void adif_close(LOGDB *handle)
{
	FILE *fp = (FILE*)handle->priv;
	fclose(fp);
}

/*
 * 'field' is a constant defined by our scanner
 */
static const gchar *xlog2adif_name(gint fld)
{
	switch (fld) {
		case DATE: return "QSO_DATE";
		case GMT: return "TIME_ON";
		case GMTEND: return "TIME_OFF";
		case CALL: return "CALL";
		case BAND:
		{
			if (preferences.saveasadif == 0)
				return "FREQ";
			else
				return "BAND";
		}
		case MODE: return "MODE";
		case POWER: return "TX_PWR";
		case RST: return "RST_SENT";
		case MYRST: return "RST_RCVD";
		case QSLOUT: return "QSL_SENT";
		case QSLIN: return "QSL_RCVD";
		case REMARKS: return "COMMENT";
		case NAME: return "NAME";
		case QTH: return "QTH";
		case LOCATOR: return "GRIDSQUARE";
		case U1:
		  if (!strcmp (preferences.freefield1, "Distance"))
			return "DISTANCE";
		  else
			return preferences.freefield1;
		case U2:
		  if (!strcmp (preferences.freefield2, "Azimuth"))
			return "ANT_AZ";
		  else
			return preferences.freefield2;
			  
		default: return "UNKNOWN";
	}
}

gint adif_qso_append(LOGDB *handle, const qso_t *q)
{
	FILE *fp = (FILE *)handle->priv;
	int i, mode;
	struct tm tm_a;
	gchar *result;
	gchar *distance;
	gchar *distance_unit;
        int distance_int;
	
	mode = adif3_mode2enum (q[MODE]);
	if (mode == -1) {
	  printf("ERROR: Illegal ADIF mode in adif_qso_append() with %s, mode %0d, callsign %s\n", q[MODE], mode, q[CALL]);
	  return 1;
	}

	for (i = 0; i < handle->column_nr; i++)
	{
		gint qfield_len, number;
		gint fld = handle->column_fields[i];
		gchar *qfield = NULL, *serial = NULL, *endptr = NULL, *space_ptr = NULL;

		if (!q[fld])
			continue;
		qfield_len = strlen(q[fld]);
		if (qfield_len == 0)
			continue;

		/* reshape date, convert from locale notation to yyyymmdd */
		if (fld == DATE)
		{
			gchar date[32];

			result = strptime (q[DATE], "%d %b %Y", &tm_a);
			if (result)
			{
				strftime (date, sizeof(date), "%Y%m%d", &tm_a);
				qfield = date;
				qfield_len = 8;
			}
			else
			{
				qfield = q[fld];
				qfield_len = strlen(qfield);
			}
		}
		else if (fld == BAND)
		{
			if (preferences.saveasadif == 0)
				qfield = q[fld];
			else
			{
				gint bandenum = freq2enum (q[fld]);
				qfield = band_enum2bandchar (bandenum);
				qfield_len = strlen(qfield);
			}
		}

		/* export all QSL-info's as 'Y' for 'y', 'Y' or 'x', 
		 * drop entry for 'n' or 'N' and 
		 * as 'I' (invalid) for all other values of nonzero length
		 */
		else if (fld == QSLOUT || fld == QSLIN)
		{
			qfield = q[fld];	
			if (qfield_len == 1 && (qfield[0] == 'Y' || qfield[0] == 'y' || qfield[0] == 'X' || qfield[0] == 'x'))
				qfield[0] = 'Y';
			else if (qfield_len == 1 && (qfield[0] == 'N' || qfield[0] == 'n'))
				continue;
			else
			{
				qfield[0] = 'I';
				qfield[1] = '\0';
				qfield_len = 1;
			}
		}

		else if (fld == RST || fld == MYRST)
		{
			if (mode == MODE_SSB || mode == MODE_AM || mode == MODE_FM)
			{
				serial = g_strdup (g_strstrip(q[fld] + 2));
				q[fld][2] = '\0';
				qfield = q[fld];
				qfield_len = 2;
			}
			else
			{
				serial = g_strdup (g_strstrip(q[fld] + 3));
				q[fld][3] = '\0';
				qfield = q[fld];
				qfield_len = 3;
			}
		}

		else if (fld == MODE)
		{
		  /* First, print the SUBMODE field in the ADIF3 output */
		  /* Be careful, as not all modes are submodes.  Check first. */
		  gint sub_mode_enum = mode2enum (q[fld]);
		  gint mode_enum = adif3_mode2enum (q[fld]);

		  if (sub_mode_enum != mode_enum) {
		    qfield = mode_enum2char (sub_mode_enum);
		    qfield_len = strlen(qfield);
		    fprintf(fp, "<SUBMODE:%d>%s ", qfield_len, qfield);
		  }

		  /* Last, setup to print the MODE field in the ADIF3 output */
		  qfield = mode_enum2char (mode_enum);
		  qfield_len = strlen(qfield);
		}

		else if ((fld == U2) && (!strcmp (xlog2adif_name(fld), "ANT_AZ"))) {
		  // Azimuth field is one of the following:
		  //     null
		  //     <number> deg
		  //
		  // The code looks for the space, and saves the part of the string prior to the space.
                  // That value is the desired ADIF information for <ANT_AZ>
		  // If there is no space, the field is null.
		  
		  space_ptr = q[fld];
		  space_ptr = strpbrk (space_ptr, " ");
		  qfield_len = space_ptr - q[fld];         // length of string from start to space
		  
		  if (qfield_len == 0) {                   // if no space, then the field is either null or somehow malformed
			qfield = q[fld];
		  } else {
		    gchar tmp_str[10];
		    strncpy (tmp_str, q[fld], qfield_len);  // Save the string from start to space, not including the space.
		    qfield = tmp_str;
		    qfield[qfield_len] = '\0';             // Null terminate the desired string
		  }
		}

		else if ((fld == U1) && (!strcmp (xlog2adif_name(fld), "DISTANCE"))) {
		  // Distance field is one of the following:
		  //     null
		  //     <number> m (must be converted to km)
		  //     <number> km
		  //
		  // The code looks for the space, and saves the part of the string prior to the space.
                  // That value is the desired ADIF information for <DISTANCE>
		  // If there is no space, the field is null.
		  
		  space_ptr = q[fld];
		  space_ptr = strpbrk (space_ptr, " ");
		  qfield_len = space_ptr - q[fld];         // length of string from start to space

		  if (qfield_len == 0) {                   // if no space, then the field is null
			qfield = q[fld];
		  } else {
			// Convert distance to km
			if (fld == U1)
			{
			  const char s[2] = " ";               // separate the numerical distance from the unit
			  distance = strtok(q[fld], s);
			  distance_unit = strtok(NULL, s);

			  if (!strcmp(distance_unit, "m"))     // if the unit is miles, convert it to km per ADIF specification
			  {
			    distance_int = atoi(distance) * 1.6093;
			  } else {
			    distance_int = atoi(distance);
			  }
			  gchar tmp_str[10];
			  sprintf(tmp_str, "%0d", distance_int);
			  qfield_len = strlen(tmp_str);
			  qfield = tmp_str;
			  qfield[qfield_len] = '\0';
			}
		  }
		}

		else
		{
			qfield = q[fld];
		}

		fprintf(fp, "<%s:%d>%s ", xlog2adif_name(fld), qfield_len, qfield);

/* export exchange fields when present...
 *
 * NOTE: only digits will be converted to the designated SRX/STX fields,
 * if there is an ascii character present, we use SRX_STRING/STX_STRING
 * TODO: use endptr to split exchanges like 001UT or 001/322
 * */
		if ((fld == RST) && serial && (strlen(serial) > 0))
		{
			number = strtol (serial, &endptr, 10);
			if (strlen (endptr) > 0)
				fprintf(fp, "<STX_STRING:%zd>%s ", strlen(serial), serial);
			else
				fprintf(fp, "<STX:%zd>%s ", strlen(serial), serial);
		}
		if ((fld == MYRST) && serial && (strlen(serial) > 0))
		{
			number = strtol (serial, &endptr, 10);
			if (strlen (endptr) > 0)
				fprintf(fp, "<SRX_STRING:%zd>%s ", strlen(serial), serial);
			else
				fprintf(fp, "<SRX:%zd>%s ", strlen(serial), serial);
		}

	}
	fprintf(fp, "<EOR>\n");

	return 0;
}

/*
 * The adif3_mode2enum() will take any mode or submode as input, and it will
 * return the corresponding ADIF3 <MODE>.
 *
 * This is in contast to the mode2enum() will will return any legal mode or submode,
 * as described in the ADIF3 specification.
*/

gint adif3_mode2enum (gchar * str)
{
 	if 
 	( 
 		!g_ascii_strcasecmp  (str, "AM")     || 
 		!g_ascii_strncasecmp (str, "A3E", 3) || 
 		!g_ascii_strncasecmp (str, "A3", 2) 
 	) 
 	return MODE_AM;

 	if 
 	( 
 		!g_ascii_strcasecmp  (str, "ARDOP")
 	) 
 	return MODE_ARDOP;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "ATV") || 
 		!g_ascii_strcasecmp (str, "A5")	 || 
 		!g_ascii_strcasecmp (str, "C3F") 
 	) 
 	return MODE_ATV; 

 	if 
 	( 
 		!g_ascii_strcasecmp  (str, "C4FM")
 	) 
 	return MODE_C4FM;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "CHIP")    ||
 		!g_ascii_strcasecmp (str, "CHIP64")  ||
 		!g_ascii_strcasecmp (str, "CHIP128") 
 	) 
 	return MODE_CHIP; 

 	if  
 	( 
 		!g_ascii_strcasecmp (str, "CLO") || 
 		!g_ascii_strcasecmp (str, "CLOVER") 
 	) 
 	return MODE_CLO; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "CONTESTI") 
 	) 
 	return MODE_CONTESTI; 

 	if 
 	( 
 		!g_ascii_strcasecmp  (str, "CW")    || 
 		!g_ascii_strcasecmp  (str, "PCW")   || 
 		!g_ascii_strncasecmp (str, "A1", 2) || 
 		!g_ascii_strncasecmp (str, "A2", 2) 
 	) 
 	return MODE_CW; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "DIGITALVOICE") 
 	) 
 	return MODE_DIGITALVOICE; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "DOMINO")   ||
 		!g_ascii_strcasecmp (str, "DOMINOEX") ||
 		!g_ascii_strcasecmp (str, "DOMINOF") 
 	) 
 	return MODE_DOMINO; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "DSTAR") 
 	) 
 	return MODE_DSTAR; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "FAX") 
 	) 
 	return MODE_FAX; 

 	if 
 	( 
 		!g_ascii_strcasecmp  (str, "FM")	  || 
 		!g_ascii_strncasecmp (str, "F3", 2)	  || 
 		!g_ascii_strcasecmp  (str, "20K0F3E") 
 	) 
 	return MODE_FM; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "FSK441") 
 	) 
 	return MODE_FSK441; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "FT8") 
 	) 
 	return MODE_FT8; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "FMHELL")  ||
 		!g_ascii_strcasecmp (str, "FSKHELL") ||
 		!g_ascii_strcasecmp (str, "HELL")    ||
 		!g_ascii_strcasecmp (str, "HELL80")  ||
 		!g_ascii_strcasecmp (str, "HFSK")    ||
 		!g_ascii_strcasecmp (str, "PSKHELL")
 	) 
 	return MODE_HELL; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "ISCAT")   ||
 		!g_ascii_strcasecmp (str, "ISCAT_A") ||
 		!g_ascii_strcasecmp (str, "ISCAT_B")
 	) 
 	return MODE_ISCAT;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "JT44") 
 	) 
 	return MODE_JT44; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "JT4")  ||
 		!g_ascii_strcasecmp (str, "JT4A") ||
 		!g_ascii_strcasecmp (str, "JT4B") ||
 		!g_ascii_strcasecmp (str, "JT4C") ||
 		!g_ascii_strcasecmp (str, "JT4D") ||
 		!g_ascii_strcasecmp (str, "JT4E") ||
 		!g_ascii_strcasecmp (str, "JT4F") ||
 		!g_ascii_strcasecmp (str, "JT4G")
 	) 
 	return MODE_JT4;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "JT6M") 
 	) 
 	return MODE_JT6M; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "JT65")   ||
 		!g_ascii_strcasecmp (str, "JT65A")  ||
 		!g_ascii_strcasecmp (str, "JT65B")  ||
 		!g_ascii_strcasecmp (str, "JT65B2") ||
 		!g_ascii_strcasecmp (str, "JT65C")  ||
 		!g_ascii_strcasecmp (str, "JT65C2") 
 	) 
 	return MODE_JT65; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "JT9")         ||
 		!g_ascii_strcasecmp (str, "JT9_1")       ||
 		!g_ascii_strcasecmp (str, "JT9_2")       ||
 		!g_ascii_strcasecmp (str, "JT9_5")       ||
 		!g_ascii_strcasecmp (str, "JT9_10")      ||
 		!g_ascii_strcasecmp (str, "JT9_30")      ||
 		!g_ascii_strcasecmp (str, "JT9A")        ||
 		!g_ascii_strcasecmp (str, "JT9B")        ||
 		!g_ascii_strcasecmp (str, "JT9C")        ||
 		!g_ascii_strcasecmp (str, "JT9D")        ||
 		!g_ascii_strcasecmp (str, "JT9E")        ||
 		!g_ascii_strcasecmp (str, "JT9E_FAST")   ||
 		!g_ascii_strcasecmp (str, "JT9F")        ||
 		!g_ascii_strcasecmp (str, "JT9F_FAST")   ||
 		!g_ascii_strcasecmp (str, "JT9G")        ||
 		!g_ascii_strcasecmp (str, "JT9G_FAST")   ||
 		!g_ascii_strcasecmp (str, "JT9H")        ||
 		!g_ascii_strcasecmp (str, "JT9H_FAST") 
 	) 
 	return MODE_JT9; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "FSQCALL") ||
 		!g_ascii_strcasecmp (str, "FST4")    ||
 		!g_ascii_strcasecmp (str, "FT4")     ||
 		!g_ascii_strcasecmp (str, "JS8")     ||
 		!g_ascii_strcasecmp (str, "MFSK")    ||
 		!g_ascii_strcasecmp (str, "MFSK4")   ||
 		!g_ascii_strcasecmp (str, "MFSK8")   ||
 		!g_ascii_strcasecmp (str, "MFSK11")  ||
 		!g_ascii_strcasecmp (str, "MFSK16")  ||
 		!g_ascii_strcasecmp (str, "MFSK22")  ||
 		!g_ascii_strcasecmp (str, "MFSK31")  ||
 		!g_ascii_strcasecmp (str, "MFSK32")  ||
 		!g_ascii_strcasecmp (str, "MFSK64")  ||
 		!g_ascii_strcasecmp (str, "MFSK128")
 	) 
 	return MODE_MFSK;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "MSK144") 
 	) 
 	return MODE_MSK144;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "MT63") 
 	) 
 	return MODE_MT63; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "OLIVIA")         ||
 		!g_ascii_strcasecmp (str, "OLIVIA_4_125")   ||
 		!g_ascii_strcasecmp (str, "OLIVIA_4_250")   ||
 		!g_ascii_strcasecmp (str, "OLIVIA_8_250")   ||
 		!g_ascii_strcasecmp (str, "OLIVIA_8_500")   ||
 		!g_ascii_strcasecmp (str, "OLIVIA_16_500")  ||
 		!g_ascii_strcasecmp (str, "OLIVIA_16_1000") ||
 		!g_ascii_strcasecmp (str, "OLIVIA_32_1000") 
 	) 
 	return MODE_OLIVIA; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "OPERA")        || 
 		!g_ascii_strcasecmp (str, "OPERA_BEACON") || 
 		!g_ascii_strcasecmp (str, "OPERA_QSO")
 	) 
 	return MODE_OPERA;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "PAC")  || 
 		!g_ascii_strcasecmp (str, "PAC2") || 
 		!g_ascii_strcasecmp (str, "PAC3") || 
 		!g_ascii_strcasecmp (str, "PAC4") || 
 		!g_ascii_strcasecmp (str, "PACTOR") 
 	) 
 	return MODE_PAC;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "PAX")  ||
 		!g_ascii_strcasecmp (str, "PAX2")
 	) 
 	return MODE_PAX; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "PKT") || 
 		!g_ascii_strcasecmp (str, "PACKET") 
 	) 
 	return MODE_PKT; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "FSK31")     ||
 		!g_ascii_strcasecmp (str, "PSK")       ||
 		!g_ascii_strcasecmp (str, "PSK10")     ||
 		!g_ascii_strcasecmp (str, "PSK31")     ||
 		!g_ascii_strcasecmp (str, "PSK63")     ||
 		!g_ascii_strcasecmp (str, "PSK63F")    ||
 		!g_ascii_strcasecmp (str, "PSK125")    ||
 		!g_ascii_strcasecmp (str, "PSK250")    ||
 		!g_ascii_strcasecmp (str, "PSK500")    ||
 		!g_ascii_strcasecmp (str, "PSK1000")   ||
 		!g_ascii_strcasecmp (str, "PSKAM10")   ||
 		!g_ascii_strcasecmp (str, "PSKAM31")   ||
 		!g_ascii_strcasecmp (str, "PSKAM50")   ||
 		!g_ascii_strcasecmp (str, "PSKFEC31")  ||
 		!g_ascii_strcasecmp (str, "QPSK31")    ||
 		!g_ascii_strcasecmp (str, "QPSK63")    ||
 		!g_ascii_strcasecmp (str, "QPSK125")   ||
 		!g_ascii_strcasecmp (str, "QPSK250")   ||
 		!g_ascii_strcasecmp (str, "QPSK500")   ||
 		!g_ascii_strcasecmp (str, "SIM31") 
 	) 
 	return MODE_PSK;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "PSK2K") 
 	) 
 	return MODE_PSK2K; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "Q15")
 	) 
 	return MODE_Q15; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "QRA64")   ||
 		!g_ascii_strcasecmp (str, "QRA64A")  ||
 		!g_ascii_strcasecmp (str, "QRA64B")  ||
 		!g_ascii_strcasecmp (str, "QRA64C")  ||
 		!g_ascii_strcasecmp (str, "QRA64D")  ||
 		!g_ascii_strcasecmp (str, "QRA64E")
 	) 
 	return MODE_QRA64;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "ROS")     ||
 		!g_ascii_strcasecmp (str, "ROS_EME") ||
 		!g_ascii_strcasecmp (str, "ROS_HF")  ||
 		!g_ascii_strcasecmp (str, "ROS_MF") 
 	) 
 	return MODE_ROS; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "ASCI")   || 
 		!g_ascii_strcasecmp (str, "ASCII")  ||
 		!g_ascii_strcasecmp  (str, "RTTY")  || 
 		!g_ascii_strncasecmp (str, "F1", 2) || 
 		!g_ascii_strncasecmp (str, "F2", 2) 
 	) 
 	return MODE_RTTY;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "RTTYM") 
 	) 
 	return MODE_RTTYM; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "SSB")   ||
 		!g_ascii_strcasecmp (str, "USB")   ||
 		!g_ascii_strcasecmp (str, "LSB")   ||
 		!g_ascii_strcasecmp (str, "J3E")   ||
 		!g_ascii_strcasecmp (str, "A3J")   ||
 		!g_ascii_strcasecmp (str, "R3E")   ||
 		!g_ascii_strcasecmp (str, "H3E")   ||
 		!g_ascii_strcasecmp (str, "A3R")   ||
 		!g_ascii_strcasecmp (str, "PHONE") ||
 		!g_ascii_strcasecmp (str, "VOICE") ||
 		!g_ascii_strcasecmp (str, "A3H") 
 	) 
 	return MODE_SSB; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "SSTV") 
 	) 
 	return MODE_SSTV; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "T10") 
 	) 
 	return MODE_T10; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "THOR") 
 	) 
 	return MODE_THOR; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "THRB")  ||
 		!g_ascii_strcasecmp (str, "THRBX") ||
 		!g_ascii_strcasecmp (str, "THROB") 
 	) 
 	return MODE_THRB; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "AMTORFEC") ||
 		!g_ascii_strcasecmp (str, "GTOR")     ||
 		!g_ascii_strcasecmp (str, "TOR")
 	) 
 	return MODE_TOR;

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "V4") 
 	) 
 	return MODE_V4; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "VOI") 
 	) 
 	return MODE_VOI; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "WINMOR") 
 	) 
 	return MODE_WINMOR; 

 	if 
 	( 
 		!g_ascii_strcasecmp (str, "WSPR") 
 	) 
 	return MODE_WSPR; 

 	return -1; 
 } 
