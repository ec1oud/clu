/*
   clu - Callsign Looker Upper
   code extracted from xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>
   Copyright (C) 2021        Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2025        Shawn Rutledge <s@ecloud.org>

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

#include <glib.h>

#ifndef uchar
#define uchar unsigned char
#endif

/*
 * dxcc.h
 */
#ifndef DXCC_H
#define DXCC_H

/* struct for dxcc information from cty.dat */
typedef struct
{
	char* countryname;
	uchar cq; /* uchar max=255 */
	uchar itu;
	uchar continent;
	int latitude;
	int longitude;
	short timezone;
	char* px;
	char* exceptions;
	uint worked;
	uint confirmed;
} dxcc_data;

/* struct for dxcc information from area.dat */
typedef struct
{
	char* countryname;
	uchar cq; /* uchar max=255 */
	uchar itu;
	char* continent;
	int latitude;
	int longitude;
	short timezone;
	char* px;
} area_data;

struct info {
	uint country;
	uint cq;
	uint itu;
	uint continent;
};

void cleanup_dxcc(void);
void cleanup_area(void);
int readctyversion(void);
int readctydata(void);
int readareadata(void);
void updatedxccframe(char* item, gboolean byprefix, int st, int zone, int cont, uint iota);
void update_dxccscoring(void);
void update_wacscoring(void);
void update_wasscoring(void);
void update_wazscoring(void);
void update_iotascoring(void);
void update_locscoring(void);
void fill_scoring_arrays(void);
struct info lookupcountry_by_callsign(char* callsign);
struct info lookupcountry_by_prefix(char* px);

void hash_inc(GHashTable* hash_table, const char* key);
void hash_dec(GHashTable* hash_table, const char* key);
void iota_new_qso(uint iota, int f, gboolean qslconfirmed);
void iota_del_qso(uint iota, int f, gboolean qslconfirmed);
void loc_new_qso(const char* locator, int f, gboolean qslconfirmed);
void loc_del_qso(const char* locator, int f, gboolean qslconfirmed);
char* loc_norm(const char* locator);

#endif /* DXCC_H */
