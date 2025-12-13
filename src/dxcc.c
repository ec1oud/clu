// SPDX-License-Identifier: GPL-3.0-or-later
/*
   clu - Callsign Looker Upper
   code extracted from xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>
   Copyright (C) 2021        Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2025        Shawn Rutledge <s@ecloud.org>
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
#include "locator.h"
#include "awards_enum.h"

#ifdef USE_AREA_DAT
static const char* area_location = "/usr/share/xlog/dxcc/area.dat";
#endif

typedef struct
{
	int countries; /* number of countries loaded */
} programstatetype;

programstatetype programstate;
GPtrArray *dxcc, *area;
GHashTable *prefixes, *full_callsign_exceptions, *abbreviations;
int excitu, exccq;

/* free memory used by the dxcc array */
void cleanup_dxcc(void)
{
	int i;

	/* free the dxcc array */
	if (dxcc) {
		for (i = 0; i < dxcc->len; i++) {
			dxcc_data* d = g_ptr_array_index(dxcc, i);
			g_free((char*)d->countryname);
			g_free((char*)d->px);
			g_free((char*)d->exceptions);
			g_free(d);
		}
		g_ptr_array_free(dxcc, TRUE);
	}
	if (prefixes)
		g_hash_table_destroy(prefixes);
	if (full_callsign_exceptions)
		g_hash_table_destroy(full_callsign_exceptions);
	if (abbreviations)
		g_hash_table_destroy(abbreviations);
}

#ifdef USE_AREA_DAT
/* free memory used by the area array */
void cleanup_area(void)
{
	int i;

	/* free the dxcc array */
	if (area) {
		for (i = 0; i < area->len; i++) {
			area_data* a = g_ptr_array_index(area, i);
			g_free(a->countryname);
			g_free(a->continent);
			g_free(a->px);
			g_free(a);
		}
		g_ptr_array_free(area, TRUE);
	}
}
#endif

/*
 * go through exception string and stop when end of prefix
 * is reached (BT3L(23)[33] -> BT3L)
 */
static char*
findpfx_in_exception(char* pfx)
{
	char *end, *j;

	g_strstrip(pfx);
	end = pfx + strlen(pfx);
	for (j = pfx; j < end; ++j) {
		switch (*j) {
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
static char*
change_area(char* callsign, int area)
{
	char *end, *j;

	end = callsign + strlen(callsign);
	for (j = callsign; j < end; ++j) {
		switch (*j) {
		case '0' ... '9':
			if ((j - callsign) > 1)
				*j = area + 48;
			break;
		}
	}

	return (g_strdup(callsign));
}

/*
   extract prefix from a callsign with a forward slash:
   - check if callsign has a '/'
   - replace callsign area's (K0AR/2 -> K2AR)
   - skip /mm, /am and /qrp
   - return string after slash if it is shorter than string before
 */
static char* getpx(const char* checkcall)
{

	char *pxstr = NULL, **split;

	/* characters after '/' might contain a country */
	if (strchr(checkcall, '/')) {
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
				else
					pxstr = g_strdup(split[1]);
			} else if ((strlen(split[1]) == 1) && split[1][0] >= '0' && split[1][0] <= '9')
			/* callsign area changed */
			{
				pxstr = change_area(split[0], atoi(split[1]));
			} else
				pxstr = g_strdup(split[0]);
		} else
			pxstr = g_strdup(split[0]);
		g_strfreev(split);
	} else
		pxstr = g_strdup(checkcall);

	return (pxstr);
}

/* parse an exception and extract the CQ and ITU zone */
static char*
findexc(char* exception)
{
	char *end, *j;

	excitu = 0;
	exccq = 0;
	end = exception + strlen(exception);
	for (j = exception; j < end; ++j) {
		switch (*j) {
		case '(':
			if (*(j + 2) == 41)
				exccq = *(j + 1) - 48;
			else if (*(j + 3) == 41)
				exccq = ((*(j + 1) - 48) * 10) + (*(j + 2) - 48);
		case '[':
			if (*(j + 2) == 93)
				excitu = *(j + 1) - 48;
			else if (*(j + 3) == 93)
				excitu = ((*(j + 1) - 48) * 10) + (*(j + 2) - 48);
		case ';':
			*j = '\0';
			break;
		}
	}
	return (exception);
}

/*!
    Look up information related to the given \a callsign.
    Note: strings in the returned struct are static constants;
    copy them if you need to keep them independently.
 */
dxcc_data lookupcountry_by_callsign(const char* callsign)
{
	int ipx, iexc;
	char* px;
	char **excsplit, *exc;
	char* searchpx = NULL;
	uint country_i = 0;

	/* first check complete callsign exceptions list*/
	country_i = GPOINTER_TO_INT(g_hash_table_lookup(full_callsign_exceptions, callsign));

	if (country_i == 0) {
		/* Next, check the list of prefixes */
		country_i = GPOINTER_TO_INT(g_hash_table_lookup(prefixes, callsign));
	}

	if (country_i == 0 && (px = getpx(callsign))) { /* start with full callsign and truncate it until a correct lookup */
		for (ipx = strlen(px); ipx > 0; ipx--) {
			searchpx = g_strndup(px, ipx);
			country_i = GPOINTER_TO_INT(g_hash_table_lookup(prefixes, searchpx));
			if (country_i > 0)
				break;
		}
		g_free(px);
	} else
		searchpx = g_strdup(callsign);

	dxcc_data* d = g_ptr_array_index(dxcc, country_i);
	dxcc_data ret = *d;
	ret.country = country_i;

	/* look for CQ/ITU zone exceptions */
	if (strchr(d->exceptions, '(') || strchr(d->exceptions, '[')) {
		excsplit = g_strsplit(d->exceptions, ",", -1);
		for (iexc = 0;; iexc++) {
			if (!excsplit[iexc])
				break;
			exc = findexc(excsplit[iexc]);
			if (g_ascii_strcasecmp(searchpx, exc) == 0) {
				if (excitu > 0)
					ret.itu = excitu;
				if (exccq > 0)
					ret.cq = exccq;
			}
		}
		g_strfreev(excsplit);
	}
	g_free(searchpx); // Bug #60022
	return ret;
}

const char *abbreviate_country(const char *country)
{
	gpointer p = g_hash_table_lookup(abbreviations, country);
	//~ printf("for '%s' found %p\n", country, p);
	return p;
}

/* add an item from cty.dat to the dxcc array */
static void
dxcc_add(char* c, int w, int i, int cont, int lat, int lon,
    int tz, char* p, char* e)
{
	dxcc_data* new_dxcc = g_new(dxcc_data, 1);

	new_dxcc->countryname = g_strdup(c);
	new_dxcc->cq = w;
	new_dxcc->itu = i;
	new_dxcc->continent = cont;
	new_dxcc->latitude = lat / 100.0;
	new_dxcc->longitude = lon / -100.0;
	new_dxcc->timezone = tz;
	new_dxcc->px = g_strdup(p);
	new_dxcc->exceptions = g_strdup(e);
	g_ptr_array_add(dxcc, new_dxcc);
}

#ifdef USE_AREA_DAT
/* add an item from area.dat to the area array */
static void
area_add(char* c, int w, int i, char* cont, int lat, int lon,
    int tz, char* p)
{
	area_data* new_area = g_new(area_data, 1);

	new_area->countryname = g_strdup(c);
	new_area->cq = w;
	new_area->itu = i;
	new_area->continent = g_strdup(cont);
	new_area->latitude = lat;
	new_area->longitude = lon;
	new_area->timezone = tz;
	new_area->px = g_strdup(p);
	g_ptr_array_add(area, new_area);
}
#endif

int readctyversion(const char *cty_dat_path)
{
	char buf[256000], *ver, *ch;
	FILE* fp;

	if ((fp = g_fopen(cty_dat_path, "r")) == NULL)
		return (1);
	int n = fread(buf, 1, 256000, fp);
	buf[n] = '\0';
	ver = strstr(buf, "VER2");
	if (ver) {
		ch = strstr(ver, ",");
		if (ch) {
			*ch = '\0';
			fclose(fp);
			return atoi(ver + 3);
		}
	}
	fclose(fp);
	return 0;
}

/* fill the hashtable with all of the prefixes from cty.dat */
int readctydata(const char *cty_dat_path)
{

	char buf[65536], *pfx, **split, **pfxsplit;
	int ichar = 0, dxccitem = 0, ipfx = 0, ch = 0;
	gboolean firstcolon = FALSE;
	char tmp[20];
	int i;
	FILE* fp;

	if ((fp = g_fopen(cty_dat_path, "r")) == NULL) {
		printf("didn't find %s\n", cty_dat_path);
		return (1);
	}

	dxcc = g_ptr_array_new();
	prefixes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	full_callsign_exceptions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	/* first field in case hash_table_lookup returns NULL */
	dxcc_add("Unknown", 0, 0, 99, 0, 0, 0, "", "");
	programstate.countries = 1;

	while (!feof(fp)) {
		while (ch != ';') {
			ch = fgetc(fp);
			/* ignore space (exept in the country string), new line, carriage return and semicolon */
			if (ch == EOF)
				break;
			if (ch == ':')
				firstcolon = TRUE;
			if (ch == ' ' && firstcolon)
				continue;
			if (ch == '\r')
				continue;
			if (ch == '\n')
				continue;
			if (ch == ';')
				continue;
			buf[ichar++] = ch;
		}
		if (ch == EOF)
			break;

		tmp[0] = '\0';
		buf[ichar] = '\0';
		ichar = 0;
		ch = 0;
		firstcolon = FALSE;

		/* split up the first line */
		split = g_strsplit(buf, ":", 9);

		/* ignore WAE countries */
		/* WAE countries count for CQ contests, but not for ARRL contests. */
		if (!g_strrstr(split[7], "*")) {
			for (dxccitem = 0; dxccitem < 9; dxccitem++)
				g_strstrip(split[dxccitem]);

			dxcc_add(split[0], atoi(split[1]), atoi(split[2]), cont_to_enum(split[3]),
			    (int)(strtod(split[4], NULL) * 100), (int)(strtod(split[5], NULL) * 100),
			    (int)(strtod(split[6], NULL) * 10), split[7], split[8]);

			/* NOTE: split[7] is the description prefix, it is not added to the hashtable */
			/* With this addition, the user can enter the "callsign" like "3D2/R" and the */
			/* scoring window locator box will show the proper DXCC entity, even though   */
			/* this isn't a "real" callsign.  AMS 24-feb-2013 */

			if (strchr(split[7], '/')) {
				for (i = 0; i < strlen(split[7]); i++) {
					tmp[i] = toupper(split[7][i]);
				}
				tmp[i] = '\0';
				g_hash_table_insert(prefixes, g_strdup(tmp), GINT_TO_POINTER(programstate.countries));
			}

			/* split up the second line */
			/* The second line is made up of prefixes AND exceptions which start with '=' */
			/* As of 2.0.11, there are two hashes, one for prefixes and one for           */
			/* callsign exceptions due to a recently discovered bug.                      */

			pfxsplit = g_strsplit(split[8], ",", 0);
			for (ipfx = 0;; ipfx++) {
				if (!pfxsplit[ipfx])
					break;

				pfx = findpfx_in_exception(pfxsplit[ipfx]);
				if (!strncmp(pfxsplit[ipfx], "=", 1)) {
					g_hash_table_insert(full_callsign_exceptions, g_strdup(pfx), GINT_TO_POINTER(programstate.countries));
				} else {
					g_hash_table_insert(prefixes, g_strdup(pfx), GINT_TO_POINTER(programstate.countries));
				}
			}
			g_strfreev(pfxsplit);
			programstate.countries++;
		}
		g_strfreev(split);
	}
	fclose(fp);
	return (0);
}

int readabbrev(const char *abbrev_tsv_path)
{
	char buf[64];
	FILE* fp;
	int count = 0;

	if ((fp = g_fopen(abbrev_tsv_path, "r")) == NULL) {
		printf("didn't find %s\n", abbrev_tsv_path);
		return (1);
	}

	abbreviations = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	while (!feof(fp)) {
		if (!fgets(buf, sizeof(buf), fp))
			break;
		int len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = 0;
		char *tab = strchr(buf, '\t');
		if (!tab)
			break;
		*tab = 0;
		//~ printf("insert '%s' : '%s'\n", tab + 1, buf);
		g_hash_table_insert(abbreviations, g_strdup(tab + 1), g_strdup(buf));
		++count;
	}

	fclose(fp);
	//~ printf("read %d abbreviations\n", count);
	return (0);
}

#ifdef USE_AREA_DAT
/* fill the hashtable with all of the prefixes from area.dat */
int readareadata(void)
{

	char buf[4096], **split;
	int ichar = 0, ch = 0;
	FILE* fp;

	if ((fp = g_fopen(area_location, "r")) == NULL) {
		printf("failed to find %s\n", area_location);
		return (1);
	}

	area = g_ptr_array_new();

	while (!feof(fp)) {
		while (ch != 59) {
			ch = fgetc(fp);
			if (ch == EOF)
				break;
			if (ch == 59)
				continue;
			buf[ichar++] = ch;
		}
		if (ch == EOF)
			break;

		buf[ichar] = '\0';
		ichar = 0;
		ch = 0;

		/* split up the line */
		split = g_strsplit(buf, ":", 9);
		area_add(split[0], atoi(split[1]), atoi(split[2]), split[3],
		    (int)(strtod(split[4], NULL) * 100), (int)(strtod(split[5], NULL) * 100),
		    (int)(strtod(split[6], NULL) * 10), split[7]);
		g_strfreev(split);
	}
	fclose(fp);
	return (0);
}
#endif

/*!
    Search a callsign and return the callsign area (just the number as a character).
    Note: this does *not* search area.dat.
*/
char lookuparea(const char* callsign)
{
	const char *end, *j, *slash;

	end = callsign + strlen(callsign);
	if ((slash = strchr(callsign, '/'))) {
		for (j = slash; j < end; ++j)
			switch (*j) {
			case '0' ... '9':
				return (*j);
			}
	}
	end = callsign + strlen(callsign);
	for (j = callsign; j < end; ++j) {
		switch (*j) {
		case '0' ... '9':
			return (*j);
		}
	}
	return '?';
}

void list_all_countries()
{
	if (dxcc) {
		for (int i = 0; i < dxcc->len; i++) {
			dxcc_data* d = g_ptr_array_index(dxcc, i);
			const char *abbrev = abbreviate_country(d->countryname);
			if (abbrev)
				printf("%s\t%s\n", abbrev, d->countryname);
			else
				printf("\t%s\n", d->countryname);
		}
	}
}

void hash_inc(GHashTable* hash_table, const char* key)
{
	gpointer p, value;
	p = g_hash_table_lookup(hash_table, key);
	if (!p)
		value = GINT_TO_POINTER(1);
	else
		value = GINT_TO_POINTER(GPOINTER_TO_INT(p) + 1);

	g_hash_table_insert(hash_table, g_strdup(key), value);
}

void hash_dec(GHashTable* hash_table, const char* key)
{
	gpointer p, value;

	p = g_hash_table_lookup(hash_table, key);
	if (!p)
		return;

	value = GINT_TO_POINTER(GPOINTER_TO_INT(p) - 1);
	if (value > 0)
		g_hash_table_insert(hash_table, g_strdup(key), value);
	else
		g_hash_table_remove(hash_table, key);
}

char* loc_norm(const char* locator)
{
	char* loc4;

	if (!locator || strlen(locator) < 4)
		return NULL;

	/* Make all locators uppercase - no dupe,
	 * and keep only first 4 chars
	 */
	loc4 = g_ascii_strup(locator, -1);
	loc4[4] = '\0';

	return loc4;
}

bool is_grid(const char* grid)
{
	if (!grid)
		return false;
    const int len = strlen(grid);
	if (!grid || len < 4 || len % 2 != 0)
		return false;

	for (int i = 0; i < len; i++) {
		// First square is base-18 and traditionally uses uppercase
		if (i < 2) {
			if (!(grid[i] >= 'A' && grid[i] <= 'R'))
				return false;
			continue;
		}
		// Subsequently, letter pairs are base-24 (upper or lowercase),
		// and digit pairs are in 00..99 range
		if (i % 4 < 2) {
			//~ printf("        %d %c: subsquare?\n", i, grid[i]);
			if (!((grid[i] >= 'A' && grid[i] <= 'X') || (grid[i] >= 'a' && grid[i] <= 'x')))
				return false;
		} else {
			//~ printf("        %d %c: digit?\n", i, grid[i]);
			if (grid[i] < '0' || grid[i] > '9')
				return false;
		}
	}

	return true;
}

/*!
	Set latitude and longitude from Maidenhead locator \a grid.

	Expect a Maidenhead locator like "FN31pr" (can be 2,4,6,... chars).
	Probably is_grid() has already verified that it's a grid, so just
	dive in and try to make sense of the letters and digits.

	Algorithm:
	- Start at lon = -180, lat = -90
	- Initial steps: lon_step = 20.0, lat_step = 10.0
	- For group g (pair index: 0 = first pair, 1 = second pair, ...):
		- character at pos 2*g contributes to longitude
		- character at pos 2*g+1 contributes to latitude
		- character types:
			g == 0 : letters A-R (18 values)  -> next divider = 10
			g  > 0 : if g is odd -> digits 0-9  -> next divider = 24
					 if g is even -> letters a-x (24) -> next divider = 10
		- add (value + 0.5) * step to get the center of the cell

	Returns true on success (info updated), false on invalid input.
*/
bool set_location_from_grid(dxcc_data* info, const char* grid)
{
	if (!info || !grid)
		return false;

	double lat, lon;
	int ret = locator2longlat(&lon, &lat, grid);
	if (ret == RIG_OK) {
		info->latitude = lat;
		info->longitude = lon;
		return true;
	}

	return false;
}
