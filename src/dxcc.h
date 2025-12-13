// SPDX-License-Identifier: GPL-3.0-or-later
/*
   clu - Callsign Looker Upper
   code extracted from xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>
   Copyright (C) 2021        Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2025        Shawn Rutledge <s@ecloud.org>
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
	const char *countryname;
	uint country;
	uchar cq; /* uchar max=255 */
	uchar itu;
	uchar continent;
	float latitude;
	float longitude;
	short timezone;
	const char *px;
	const char *exceptions;
#ifdef USE_AREA_DAT
	const char *area; // TODO fill it
#endif
}
dxcc_data;

#ifdef USE_AREA_DAT
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

int readareadata(void);
void cleanup_area(void);
#endif

void cleanup_dxcc(void);
int readctyversion(void);
int readctydata(void);
int readabbrev(void);
bool is_grid(const char* grid);
dxcc_data lookupcountry_by_callsign(const char* callsign);
const char *abbreviate_country(const char *country);
bool set_location_from_grid(dxcc_data* info, const char* grid);

void list_all_countries();
void hash_inc(GHashTable* hash_table, const char* key);
void hash_dec(GHashTable* hash_table, const char* key);
char* loc_norm(const char* locator);

#endif /* DXCC_H */
