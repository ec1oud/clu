/*
   clu - Callsign Looker Upper
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
#include <stdio.h>

#include "dxcc.h"
#include "locator.h"

bool show_prefix = false;
bool show_distance = false;

/* command line options */
static void
parsecommandline(int argc, char* argv[])
{
	int p;

	while ((p = getopt(argc, argv, "pdhv")) != -1) {
		switch (p) {
		case 'p':
			show_prefix = true;
			break;
		case 'd':
			show_distance = true;
			break;
		case 'v':
			printf("cty version %d\n", readctyversion());
			exit(0);
		case ':':
		case '?':
		case 'h':
			printf("Usage: clu [option] callsign\n");
			printf("	-p	Show prefix and exceptions for the country\n");
			printf("	-d	Show distance between two grids\n");
			printf("	-h	Display this help and exit\n");
			printf("	-v	Output version information and exit\n");
			exit(0);
		}
	}
}

/*!
	Expect a series of callsigns, alternating callsigns and grids,
	FT8 messages, etc. Detect the callsigns and grids and
	look up their countries and coordinates.
*/
int main(int argc, char* argv[])
{
	parsecommandline(argc, argv);
	if (optind + 1 >= argc)
		return -1;
	if (readctydata()) // error if not false
		return -2;
#ifdef USE_AREA_DAT
	readareadata();
#endif
	dxcc_data info;
	memset(&info, 0, sizeof(info));
	bool is_gr = is_grid(argv[optind]);
	char *callsign = 0;
	float last_lat = 999.0, last_lon = 999.0;
	for (int i = optind; i < argc; ++i) {
		bool is_cs = false;
		bool next_is_gr = i < argc && is_grid(argv[i + 1]);
		if (!is_gr) {
			info = lookupcountry_by_callsign(argv[i]);
			if (info.country && strlen(argv[i]) > strlen(info.px)) {
				// country was found and the candidate is longer than its prefix: must be a callsign
				is_cs = true;
				callsign = argv[i];
			}
		}
		//~ printf("    %s: cs? %d gr? %d\n", argv[i], is_cs, is_gr);
		if (!is_cs && is_gr) // refine the callsign's location by grid, if found
			set_location_from_grid(&info, argv[i]);
		if (is_gr && callsign) {
			if (show_prefix) {
				printf("%s @ %s: country %d '%s' cq %d itu %d continent %d lat %6.2f lon %6.2f prefix %s exceptions: %s\n",
					callsign, argv[i], info.country, info.countryname, info.cq, info.itu, info.continent, info.latitude, info.longitude, info.px, info.exceptions);
			} else {
				printf("%s @ %s: country %d '%s' cq %d itu %d continent %d lat %6.2f lon %6.2f\n",
					callsign, argv[i], info.country, info.countryname, info.cq, info.itu, info.continent, info.latitude, info.longitude);
			}
			callsign = 0;
			memset(&info, 0, sizeof(info));
		} else if (is_cs && !next_is_gr) {
			if (show_prefix) {
				printf("%s: country %d '%s' cq %d itu %d continent %d lat %6.2f lon %6.2f prefix %s exceptions: %s\n",
					callsign, info.country, info.countryname, info.cq, info.itu, info.continent, info.latitude, info.longitude, info.px, info.exceptions);
			} else {
				printf("%s: country %d '%s' cq %d itu %d continent %d lat %6.2f lon %6.2f\n",
					callsign, info.country, info.countryname, info.cq, info.itu, info.continent, info.latitude, info.longitude);
			}
			callsign = 0;
			memset(&info, 0, sizeof(info));
		} else if (is_gr) {
			int ret = RIG_OK - 999;
			if (show_distance && last_lat < 999.0) {
				double dist = 0.0, azimuth = 0.0;
				ret = qrb(last_lon, last_lat, info.longitude, info.latitude, &dist, &azimuth);
				if (ret == RIG_OK)
					printf("%s:\t%.2f,%.2f to %.2f,%.2f\t: distance %5.0lf azimuth %3.0lf\n",
						argv[i], last_lat, last_lon, info.latitude, info.longitude, dist, azimuth);
				else
					printf("distance calculation failed\n");
			}
			if (ret != RIG_OK) {
				printf("%s:\t%.2f,%.2f\n",
					argv[i], info.latitude, info.longitude);
			}
		}
		if (is_gr) {
			last_lat = info.latitude;
			last_lon = info.longitude;
		}
		is_gr = next_is_gr;
	}
#ifdef USE_AREA_DAT
	cleanup_area();
#endif
	cleanup_dxcc();
}
