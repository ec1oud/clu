/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>
   Copyright (C) 2021        Andy Stewart <kb1oiq@arrl.net>

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
 * dxcc.c - dxcc lookups and creation of the hashtable
 */

//
// As described on this website:
// http://n1mm.hamdocs.com/tiki-index.php?page=Customizing+the+DXCC+List
//
// CTY.DAT file format
// The format from CTY.DAT is as follows:
// Column	Length	Description
// 1	26	Country name terminated by a colon character.
// 27	5	CQ zone terminated by a colon character.
// 32	5	ITU zone terminated by a colon character.
// 37	5	2-letter continent abbreviation terminated by a colon character.
// 42	9	Latitude in degrees, + for North terminated by a colon character.
// 51	9	Longitude in degrees, + for West terminated by a colon character.
// 61	9	Local time offset from GMT terminated by a colon character.
// 69	6	Primary DXCC Prefix terminated by a colon character.
// next line(s)	List of prefixes assigned to that country, each one separated
//              by a comma and terminated by a semicolon.
//
// The fields are aligned in columns and spaced out for readability only.
// It is the ":" at the end of each field that acts as a delimiter for that field.
//
// Alias DXCC prefixes (including the primary one) follow on consecutive lines,
// separated by ",". If there is more than one line, subsequent lines begin
// with the "&" continuation character. A ";" terminates the last prefix in
// the list.
//
// "Prefixes" which start with "=" are not prefixes, but are full callsigns
// and seem to be exceptions to the rules.
//
// If the country spans multiple zones, then the prefix may be followed by a
// CQWW zone number in parenthesis, and it may also be followed by an ITU zone
// number in square brackets, or both, but the CQ zone number in parenthesis
// must precede the ITU zone number in square brackets.
//
// The following special characters can be applied to an alias prefix:
// (#) Override CQ zone where # is the zone number
// [#] Override ITU zone where # is the zone number
//

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "dxcc.h"
#include "gc.h"
#include "cfg.h"
#include "clu_enum.h"
#include "awards_enum.h"
#include "utils.h"

static const char *cty_location = "/usr/share/xlog/dxcc/cty.dat";
static const char *area_location = "/usr/share/xlog/dxcc/area.dat";

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
} programstatetype;

programstatetype programstate;
GPtrArray *dxcc, *area;
GHashTable *prefixes, *full_callsign_exceptions;
int excitu, exccq;

/* ushort: 0 - 65534, one extra element for all bands scoring */
ushort dxcc_w[400][MAX_BANDS + 1];
ushort dxcc_c[400][MAX_BANDS + 1];
ushort waz_w[MAX_ZONES][MAX_BANDS + 1];
ushort waz_c[MAX_ZONES][MAX_BANDS + 1];
ushort wac_w[MAX_CONTINENTS][MAX_BANDS + 1];
ushort wac_c[MAX_CONTINENTS][MAX_BANDS + 1];
ushort was_w[MAX_STATES][MAX_BANDS + 1];
ushort was_c[MAX_STATES][MAX_BANDS + 1];
GHashTable *iota_w[MAX_BANDS + 1];
GHashTable *iota_c[MAX_BANDS + 1];
GHashTable *loc_w[MAX_BANDS + 1];
GHashTable *loc_c[MAX_BANDS + 1];

/* free memory used by the dxcc array */
void
cleanup_dxcc (void)
{
  int i;

  /* free the dxcc array */
  if (dxcc)
    {
      for (i = 0; i < dxcc->len; i++)
	{
	  dxcc_data *d = g_ptr_array_index (dxcc, i);
	  g_free (d->countryname);
	  g_free (d->px);
	  g_free (d->exceptions);
	  g_free (d);
	}
      g_ptr_array_free (dxcc, TRUE);
    }
  if (prefixes) g_hash_table_destroy (prefixes);
  if (full_callsign_exceptions) g_hash_table_destroy (full_callsign_exceptions);

  for (i = 0; i < MAX_BANDS; i++)
    {
      if (iota_w[i]) g_hash_table_destroy (iota_w[i]);
      if (iota_c[i]) g_hash_table_destroy (iota_c[i]);
      if (loc_w[i]) g_hash_table_destroy (loc_w[i]);
      if (loc_c[i]) g_hash_table_destroy (loc_c[i]);
    }
}

/* free memory used by the area array */
void
cleanup_area (void)
{
  int i;

  /* free the dxcc array */
  if (area)
    {
      for (i = 0; i < area->len; i++)
	{
	  area_data *a = g_ptr_array_index (area, i);
	  g_free (a->countryname);
	  g_free (a->continent);
	  g_free (a->px);
	  g_free (a);
	}
      g_ptr_array_free (area, TRUE);
    }
}

/*
 * go through exception string and stop when end of prefix
 * is reached (BT3L(23)[33] -> BT3L)
 */
static char *
findpfx_in_exception (char * pfx)
{
  char *end, *j;

  g_strstrip (pfx);
  end = pfx + strlen (pfx);
  for (j = pfx; j < end; ++j)
    {
      switch (*j)
	{
	case '(':
	case '[':
	case ';':
	  *j = '\0';
	  break;
	}
    }

// BOZO!!!!!!!
  if (pfx[0] == '=')
    return pfx + 1;
  return pfx;
}

/* replace callsign area (K0AR/2 -> K2AR) so we can do correct lookups */
static char *
change_area (char *callsign, int area)
{
  char *end, *j;

  end = callsign + strlen (callsign);
  for (j = callsign; j < end; ++j)
    {
      switch (*j)
	{
	case '0' ... '9':
	  if ((j - callsign) > 1)
	    *j = area + 48;
	  break;
	}
    }

  return(g_strdup(callsign));
}

/*
   extract prefix from a callsign with a forward slash:
   - check if callsign has a '/'
   - replace callsign area's (K0AR/2 -> K2AR)
   - skip /mm, /am and /qrp
   - return string after slash if it is shorter than string before
 */
static char *
getpx (char *checkcall)
{

  char *pxstr = NULL, **split;

  /* characters after '/' might contain a country */
  if (strchr(checkcall, '/'))
    {
      split = g_strsplit(checkcall, "/", 2);
      if (split[1]) /* we might be typing */
	{
	  if ((strlen(split[1]) > 1) && (strlen(split[1]) < strlen(split[0])))
	    /* this might be a candidate */
	    {
	      if ((g_ascii_strcasecmp(split[1], "AM") == 0)
		  || (g_ascii_strcasecmp(split[1], "MM") == 0))
		pxstr = NULL; /* don't know location */
	      else if (g_ascii_strcasecmp(split[1], "QRP") == 0)
		pxstr = g_strdup(split[0]);
	      else pxstr = g_strdup(split[1]);
	    }
	  else if ((strlen(split[1]) == 1) &&
		   split[1][0] >= '0' && split[1][0] <= '9')
	    /* callsign area changed */
	    {
	      pxstr = change_area(split[0], atoi(split[1]));
	    }
	  else pxstr = g_strdup(split[0]);
	}
      else pxstr = g_strdup(split[0]);
      g_strfreev(split);
    }
  else
    pxstr = g_strdup(checkcall);

  return (pxstr);
}


/* parse an exception and extract the CQ and ITU zone */
static char *
findexc(char *exception)
{
  char *end, *j;

  excitu = 0;
  exccq = 0;
  end = exception + strlen (exception);
  for (j = exception; j < end; ++j)
    {
      switch (*j)
	{
	case '(':
	  if (*(j+2) == 41)
	    exccq = *(j+1) - 48;
	  else if (*(j+3) == 41)
	    exccq = ((*(j+1) - 48) * 10) + (*(j+2) - 48);
	case '[':
	  if (*(j+2) == 93)
	    excitu = *(j+1) - 48;
	  else if (*(j+3) == 93)
	    excitu = ((*(j+1) - 48) * 10) + (*(j+2) - 48);
	case ';':
	  *j = '\0';
	  break;
	}
    }
  return (exception);
}

/*
 * return the country number cq zone and itu zone directly from the array
 */
struct info
lookupcountry_by_prefix (char *px)
{
  int index;
  struct info lookup;

  lookup.country = 0;
  lookup.itu = 0;
  lookup.cq = 0;
  lookup.continent = 99;

  for (index = 0; index < dxcc->len; index++)
    {
      dxcc_data *d = g_ptr_array_index (dxcc, index);
      if (g_ascii_strcasecmp (d->px, px) == 0)
	{
	  lookup.country = index;
	  lookup.itu = d -> itu;
	  lookup.cq = d -> cq;
	  lookup.continent = d -> continent;
	  break;
	}
    }
  return lookup;
}

/*
 * go through the hashtable with the current callsign and return the country number
 * cq zone and itu zone - this also goes through the exceptionlist
 */
struct info
lookupcountry_by_callsign (char * callsign)
{
  int ipx, iexc;
  char *px;
  char **excsplit, *exc;
  char *searchpx = NULL;
  struct info lookup;

  lookup.country = 0;

  /* first check complete callsign exceptions list*/
  lookup.country = GPOINTER_TO_INT(g_hash_table_lookup (full_callsign_exceptions, callsign));

  if (lookup.country == 0) {
    /* Next, check the list of prefixes */
    lookup.country = GPOINTER_TO_INT(g_hash_table_lookup (prefixes, callsign));
  }

  if (lookup.country == 0 && (px = getpx (callsign)))
    {	/* start with full callsign and truncate it until a correct lookup */
      for (ipx = strlen (px); ipx > 0; ipx--)
	{
	  searchpx = g_strndup (px, ipx);
	  lookup.country = GPOINTER_TO_INT(g_hash_table_lookup (prefixes, searchpx));
	  if (lookup.country > 0) break;
	}
      g_free (px);
    }
  else
    searchpx = g_strdup (callsign);

  dxcc_data *d = g_ptr_array_index (dxcc, lookup.country);
  lookup.itu = d -> itu;
  lookup.cq = d -> cq;
  lookup.continent = d -> continent;

  printf("country is %s lat %d lon %d px %s exc %s\n", d->countryname, d->latitude, d->longitude, d->px, d->exceptions);

  /* look for CQ/ITU zone exceptions */
  if (strchr(d->exceptions, '(') || strchr(d->exceptions, '['))
    {
      excsplit = g_strsplit (d->exceptions, ",", -1);
      for (iexc = 0 ;; iexc++)
		{
		  if (!excsplit[iexc]) break;
		  exc = findexc (excsplit[iexc]);
		  if (g_ascii_strcasecmp (searchpx, exc) == 0)
			{
			  if (excitu > 0) lookup.itu = excitu;
			  if (exccq > 0) lookup.cq = exccq;
			}
		}
      g_strfreev(excsplit);
	}
  g_free (searchpx); // Bug #60022
  return lookup;
}

/* add an item from cty.dat to the dxcc array */
static void
dxcc_add (char *c, int w, int i, int cont, int lat, int lon,
	  int tz, char *p, char *e)
{
  dxcc_data *new_dxcc = g_new (dxcc_data, 1);

  new_dxcc -> countryname = g_strdup (c);
  new_dxcc -> cq = w;
  new_dxcc -> itu = i;
  new_dxcc -> continent = cont;
  new_dxcc -> latitude = lat;
  new_dxcc -> longitude = lon;
  new_dxcc -> timezone = tz;
  new_dxcc -> px = g_strdup (p);
  new_dxcc -> exceptions = g_strdup (e);
  g_ptr_array_add (dxcc, new_dxcc);
}

/* add an item from area.dat to the area array */
static void
area_add (char *c, int w, int i, char *cont, int lat, int lon,
	  int tz, char *p)
{
  area_data *new_area = g_new (area_data, 1);

  new_area -> countryname = g_strdup (c);
  new_area -> cq = w;
  new_area -> itu = i;
  new_area -> continent = g_strdup (cont);
  new_area -> latitude = lat;
  new_area -> longitude = lon;
  new_area -> timezone = tz;
  new_area -> px = g_strdup (p);
  g_ptr_array_add (area, new_area);
}

int
readctyversion (void)
{
  char buf[256000], *ver, *ch;
  FILE *fp;

  if ((fp = g_fopen (cty_location, "r")) == NULL)
      return (1);
  int n = fread (buf, 1, 256000, fp);
  buf[n] = '\0';
  ver = strstr (buf, "VER2");
  if (ver)
    {
      ch = strstr (ver, ",");
      if (ch)
	{
	  *ch = '\0';
	  fclose (fp);
	  return atoi(ver+3);
	}
    }
  fclose (fp);
  return 0;
}

/* fill the hashtable with all of the prefixes from cty.dat */
int
readctydata (void)
{

  char buf[65536], *pfx, **split, **pfxsplit;
  int ichar = 0, dxccitem = 0, ipfx = 0, ch = 0;
  gboolean firstcolon = FALSE;
  char tmp[20];
  int i;
  FILE *fp;

  if ((fp = g_fopen (cty_location, "r")) == NULL)
    {
		printf("didn't find %s\n", cty_location);
      return (1);
    }

  dxcc = g_ptr_array_new ();
  prefixes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  full_callsign_exceptions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  /* first field in case hash_table_lookup returns NULL */
  dxcc_add ("Unknown", 0, 0, 99, 0, 0, 0, "", "");
  programstate.countries = 1;

  while (!feof (fp))
    {
      while (ch != ';')
	{
	  ch = fgetc (fp);
	  /* ignore space (exept in the country string), new line, carriage return and semicolon */
	  if (ch == EOF) break;
	  if (ch == ':') firstcolon = TRUE;
	  if (ch == ' ' && firstcolon) continue;
	  if (ch == '\r') continue;
	  if (ch == '\n') continue;
	  if (ch == ';') continue;
	  buf[ichar++] = ch;
	}
      if (ch == EOF) break;

      tmp[0] = '\0';
      buf[ichar] = '\0';
      ichar = 0;
      ch = 0;
      firstcolon = FALSE;

      /* split up the first line */
      split = g_strsplit (buf, ":", 9);

      /* ignore WAE countries */
      /* WAE countries count for CQ contests, but not for ARRL contests. */
      if (!g_strrstr (split[7], "*"))
	{
	  for (dxccitem = 0; dxccitem < 9; dxccitem++)
	    g_strstrip (split[dxccitem]);

	  dxcc_add (split[0], atoi(split[1]), atoi(split[2]), cont_to_enum(split[3]),
		    (int)(strtod(split[4], NULL) * 100), (int)(strtod(split[5], NULL) * 100),
		    (int)(strtod(split[6], NULL) * 10), split[7], split[8]);

	  /* NOTE: split[7] is the description prefix, it is not added to the hashtable */
	  /* With this addition, the user can enter the "callsign" like "3D2/R" and the */
	  /* scoring window locator box will show the proper DXCC entity, even though   */
	  /* this isn't a "real" callsign.  AMS 24-feb-2013 */

	  if (strchr(split[7], '/'))
	    {
	      for (i=0; i<strlen(split[7]); i++)
		{
		  tmp[i] = toupper(split[7][i]);
		}
	      tmp[i] = '\0';
	      g_hash_table_insert (prefixes, g_strdup (tmp), GINT_TO_POINTER (programstate.countries));
	    }

	  /* split up the second line */
	  /* The second line is made up of prefixes AND exceptions which start with '=' */
	  /* As of 2.0.11, there are two hashes, one for prefixes and one for           */
	  /* callsign exceptions due to a recently discovered bug.                      */

	  pfxsplit = g_strsplit (split[8], ",", 0);
	  for (ipfx = 0;; ipfx++)
	  {
	    if (!pfxsplit[ipfx]) break;

	    pfx = findpfx_in_exception (pfxsplit[ipfx]);
	    if (!strncmp(pfxsplit[ipfx], "=", 1)) {
	      g_hash_table_insert (full_callsign_exceptions, g_strdup (pfx), GINT_TO_POINTER (programstate.countries));
	    } else {
	      g_hash_table_insert (prefixes, g_strdup (pfx), GINT_TO_POINTER (programstate.countries));
	    }
	  }
	  g_strfreev (pfxsplit);
	  programstate.countries++;
	}
	g_strfreev (split);
    }
  fclose (fp);
  return (0);
}

/* fill the hashtable with all of the prefixes from area.dat */
int
readareadata (void)
{

  char buf[4096], **split;
  int ichar = 0, ch = 0;
  FILE *fp;

  if ((fp = g_fopen (area_location, "r")) == NULL)
    {
		printf("failed to find %s\n", area_location);
      return (1);
    }

  area = g_ptr_array_new ();

  while (!feof (fp))
    {
      while (ch != 59)
	{
	  ch = fgetc (fp);
	  if (ch == EOF) break;
	  if (ch == 59) continue;
	  buf[ichar++] = ch;
	}
      if (ch == EOF) break;

      buf[ichar] = '\0';
      ichar = 0;
      ch = 0;

      /* split up the line */
      split = g_strsplit (buf, ":", 9);
      area_add (split[0], atoi(split[1]), atoi(split[2]), split[3],
		(int)(strtod(split[4], NULL) * 100), (int)(strtod(split[5], NULL) * 100),
		(int)(strtod(split[6], NULL) * 10), split[7]);
      g_strfreev (split);
    }
  fclose (fp);
  return (0);
}

/* search a callsign and return the callsign area */
char lookuparea (char *callsign)
{
  char *end, *j, *slash;

  end = callsign + strlen (callsign);
  if ((slash = strchr(callsign, '/')))
    {
      for (j = slash; j < end; ++j)
	switch (*j)
	  {
	  case '0' ... '9':
	    return (*j);
	  }
    }
  end = callsign + strlen (callsign);
  for (j = callsign; j < end; ++j)
    {
      switch (*j)
	{
	case '0' ... '9':
	  return (*j);
	}
    }
  return '?';
}

void hash_inc(GHashTable *hash_table, const char *key)
{
  gpointer p, value;
  p = g_hash_table_lookup (hash_table, key);
  if (!p)
    value = GINT_TO_POINTER (1);
  else
    value = GINT_TO_POINTER(GPOINTER_TO_INT(p) + 1);

  g_hash_table_insert (hash_table, g_strdup(key), value);

}

void hash_dec(GHashTable *hash_table, const char *key)
{
  gpointer p, value;

  p = g_hash_table_lookup (hash_table, key);
  if (!p)
    return;

  value = GINT_TO_POINTER(GPOINTER_TO_INT(p) - 1);
  if (value > 0)
    g_hash_table_insert (hash_table, g_strdup(key), value);
  else
    g_hash_table_remove (hash_table, key);

}

char *loc_norm(const char *locator)
{
  char *loc4;

  if (!locator || strlen(locator) < 4)
    return NULL;

  /* Make all locators uppercase - no dupe,
   * and keep only first 4 chars
   */
  loc4 = g_ascii_strup(locator, -1);
  loc4[4] = '\0';

  return loc4;
}

void loc_new_qso(const char *locator, int f, gboolean qslconfirmed)
{
  char *loc4;

  if (!locator || strlen(locator) < 4) return;
  loc4 = loc_norm(locator);
  if (!loc4) return;

  hash_inc(loc_w[f], loc4);
  hash_inc(loc_w[MAX_BANDS], loc4);

  if (qslconfirmed)
    {
      hash_inc(loc_c[f], loc4);
      hash_inc(loc_c[MAX_BANDS], loc4);
    }

  g_free(loc4);
}

void loc_del_qso(const char *locator, int f, gboolean qslconfirmed)
{
  char *loc4;

  if (!locator || strlen(locator) < 4) return;
  loc4 = loc_norm(locator);
  if (!loc4) return;

  hash_dec(loc_w[f], loc4);
  hash_dec(loc_w[MAX_BANDS], loc4);

  if (qslconfirmed)
    {
      hash_dec(loc_c[f], loc4);
      hash_dec(loc_c[MAX_BANDS], loc4);
    }
}

void iota_new_qso(uint iota, int f, gboolean qslconfirmed)
{
  const char *iotastr = num_to_iota(iota);
  if (!iotastr)
    return;

  hash_inc(iota_w[f], iotastr);
  hash_inc(iota_w[MAX_BANDS], iotastr);

  if (qslconfirmed)
    {
      hash_inc(iota_c[f], iotastr);
      hash_inc(iota_c[MAX_BANDS], iotastr);
    }
}

void iota_del_qso(uint iota, int f, gboolean qslconfirmed)
{
  const char *iotastr = num_to_iota(iota);
  if (!iotastr)
    return;

  hash_dec(iota_w[f], iotastr);
  hash_dec(iota_w[MAX_BANDS], iotastr);

  if (qslconfirmed)
    {
      hash_dec(iota_c[f], iotastr);
      hash_dec(iota_c[MAX_BANDS], iotastr);
    }
}

static void init_scoring (void)
{
  int i, j;

  for (i = 0; i <= programstate.countries; i++)
    for (j = 0; j <= MAX_BANDS; j++)
      {
	dxcc_w[i][j] = 0;
	dxcc_c[i][j] = 0;
      }
  for (i = 0; i < MAX_CONTINENTS; i++)
    for (j = 0; j <= MAX_BANDS; j++)
      {
	wac_w[i][j] = 0;
	wac_c[i][j] = 0;
      }
  for (i = 0; i < MAX_STATES; i++)
    for (j = 0; j <= MAX_BANDS; j++)
      {
	was_w[i][j] = 0;
	was_c[i][j] = 0;
      }
  for (i = 0; i < MAX_ZONES; i++)
    for (j = 0; j <= MAX_BANDS; j++)
      {
	waz_w[i][j] = 0;
	waz_c[i][j] = 0;
      }
  for (j = 0; j <= MAX_BANDS; j++)
    {
      iota_w[j] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
      iota_c[j] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    }
  for (j = 0; j <= MAX_BANDS; j++)
    {
      loc_w[j] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
      loc_c[j] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    }
}
