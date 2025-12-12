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

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "awards_enum.h"

uint cont_to_enum(char* str)
{
	if (strlen(str) < 2)
		return 99;
	switch (str[0]) {
	case 'N':
		return str[1] == 'A' ? CONTINENT_NA : 99;
	case 'E':
		return str[1] == 'U' ? CONTINENT_EU : 99;
	case 'S':
		return str[1] == 'A' ? CONTINENT_SA : 99;
	case 'O':
		return str[1] == 'C' ? CONTINENT_OC : 99;
	case 'A': {
		switch (str[1]) {
		case 'S':
			return CONTINENT_AS;
		case 'F':
			return CONTINENT_AF;
		}
		break;
	}
	}
	return 99;
}

const char* enum_to_cont(uint cont)
{
	switch (cont) {
	case CONTINENT_NA:
		return "NA";
	case CONTINENT_SA:
		return "SA";
	case CONTINENT_OC:
		return "OC";
	case CONTINENT_AS:
		return "AS";
	case CONTINENT_EU:
		return "EU";
	case CONTINENT_AF:
		return "AF";
	}
	return "--";
}

uint state_to_enum(char* str)
{
	if (strlen(str) < 2)
		return 99;
	switch (str[0]) {
	case 'A': {
		switch (str[1]) {
		case 'L':
			return STATE_AL;
		case 'K':
			return STATE_AK;
		case 'Z':
			return STATE_AZ;
		case 'R':
			return STATE_AR;
		}
		break;
	}
	case 'C': {
		switch (str[1]) {
		case 'A':
			return STATE_CA;
		case 'O':
			return STATE_CO;
		case 'T':
			return STATE_CT;
		}
		break;
	}
	case 'D': {
		return STATE_DE;
	}
	case 'F': {
		return STATE_FL;
	}
	case 'G': {
		return STATE_GA;
	}
	case 'H': {
		return STATE_HI;
	}
	case 'I': {
		switch (str[1]) {
		case 'L':
			return STATE_IL;
		case 'N':
			return STATE_IN;
		case 'A':
			return STATE_IA;
		case 'D':
			return STATE_ID;
		}
		break;
	}
	case 'K': {
		switch (str[1]) {
		case 'S':
			return STATE_KS;
		case 'Y':
			return STATE_KY;
		}
		break;
	}
	case 'L': {
		return STATE_LA;
	}
	case 'M': {
		switch (str[1]) {
		case 'E':
			return STATE_ME;
		case 'D':
			return STATE_MD;
		case 'A':
			return STATE_MA;
		case 'I':
			return STATE_MI;
		case 'N':
			return STATE_MN;
		case 'S':
			return STATE_MS;
		case 'O':
			return STATE_MO;
		case 'T':
			return STATE_MT;
		}
		break;
	}
	case 'N': {
		switch (str[1]) {
		case 'E':
			return STATE_NE;
		case 'V':
			return STATE_NV;
		case 'H':
			return STATE_NH;
		case 'J':
			return STATE_NJ;
		case 'M':
			return STATE_NM;
		case 'Y':
			return STATE_NY;
		case 'C':
			return STATE_NC;
		case 'D':
			return STATE_ND;
		}
		break;
	}
	case 'O': {
		switch (str[1]) {
		case 'H':
			return STATE_OH;
		case 'K':
			return STATE_OK;
		case 'R':
			return STATE_OR;
		}
		break;
	}
	case 'P': {
		return STATE_PA;
	}
	case 'R': {
		return STATE_RI;
	}
	case 'S': {
		switch (str[1]) {
		case 'C':
			return STATE_SC;
		case 'D':
			return STATE_SD;
		}
		break;
	}
	case 'T': {
		switch (str[1]) {
		case 'N':
			return STATE_TN;
		case 'X':
			return STATE_TX;
		}
		break;
	}
	case 'U': {
		return STATE_UT;
	}
	case 'V': {
		switch (str[1]) {
		case 'T':
			return STATE_VT;
		case 'A':
			return STATE_VA;
		}
		break;
	}
	case 'W': {
		switch (str[1]) {
		case 'A':
			return STATE_WA;
		case 'V':
			return STATE_WV;
		case 'I':
			return STATE_WI;
		case 'Y':
			return STATE_WY;
		}
		break;
	}
	}
	return 99;
}

/* Must be in same order as awards_enum.h */
static const char* usa_states[MAX_STATES] = {
	"AL",
	"AK",
	"AZ",
	"AR",
	"CA",
	"CO",
	"CT",
	"DE",
	"FL",
	"GA",
	"HI",
	"ID",
	"IL",
	"IN",
	"IA",
	"KS",
	"KY",
	"LA",
	"ME",
	"MD",
	"MA",
	"MI",
	"MN",
	"MS",
	"MO",
	"MT",
	"NE",
	"NV",
	"NH",
	"NJ",
	"NM",
	"NY",
	"NC",
	"ND",
	"OH",
	"OK",
	"OR",
	"PA",
	"RI",
	"SC",
	"SD",
	"TN",
	"TX",
	"UT",
	"VT",
	"VA",
	"WA",
	"WV",
	"WI",
	"WY"
};

const char* enum_to_state(uint st)
{
	if (st < MAX_STATES)
		return usa_states[st];

	return NULL;
}

uint iota_to_num(char* str)
{
	uint cont;

	if (!str || strlen(str) < 6 || str[2] != '-')
		return NOT_AN_IOTA;

	switch (str[0]) {
	case 'A':
		switch (str[1]) {
		case 'F':
			cont = IOTA_CONTINENT_AF;
			break;
		case 'N':
			cont = IOTA_CONTINENT_AN;
			break;
		case 'S':
			cont = IOTA_CONTINENT_AS;
			break;
		default:
			return NOT_AN_IOTA;
		}
		break;
	case 'E':
		cont = IOTA_CONTINENT_EU;
		break;
	case 'N':
		cont = IOTA_CONTINENT_NA;
		break;
	case 'O':
		cont = IOTA_CONTINENT_OC;
		break;
	case 'S':
		cont = IOTA_CONTINENT_SA;
		break;
	default:
		return NOT_AN_IOTA;
	}

	return cont * 1000 + (atoi(str + 3) % 1000);
}

/*!
    Note: you get the same buffer each time you call this, so
    1) don't call it from more than one thread, and
    2) don't use the returned pointer outside the scope of the calling function.
    Call strdup() if you need to keep the string for a longer time.
*/
const char* num_to_iota(uint num)
{
	const char* cont;
	static char buf[8];

	if (num == NOT_AN_IOTA)
		return NULL;
	switch (num / 1000) {
	case IOTA_CONTINENT_AF:
		cont = "AF";
		break;
	case IOTA_CONTINENT_AN:
		cont = "AN";
		break;
	case IOTA_CONTINENT_AS:
		cont = "AS";
		break;
	case IOTA_CONTINENT_EU:
		cont = "EU";
		break;
	case IOTA_CONTINENT_NA:
		cont = "NA";
		break;
	case IOTA_CONTINENT_OC:
		cont = "OC";
		break;
	case IOTA_CONTINENT_SA:
		cont = "SA";
		break;
	default:
		return NULL;
	}
	snprintf(buf, sizeof(buf), "%s-%03u", cont, num % 1000);
	return buf;
}

/* locator runs from AA00 to RR99 and returns 010100 to 181899 */
int locator_to_num(char* str)
{
	if (!str || strlen(str) < 4)
		return NOT_A_LOCATOR;

	int first;
	if (islower(str[0]))
		first = str[0] - 96;
	else
		first = str[0] - 64;
	int second;
	if (islower(str[1]))
		second = str[1] - 96;
	else
		second = str[1] - 64;

	return first * 10000 + second * 100 + (str[2] - 48) * 10 + str[3] - 48;
}

/*!
    Note: you get the same buffer each time you call this, so
    1) don't call it from more than one thread, and
    2) don't use the returned pointer outside the scope of the calling function.
    Call strdup() if you need to keep the string for a longer time.
*/
const char* num_to_locator(int num)
{
	static char locator[5];

	int first = num / 10000;
	num = num - first * 10000;
	int second = num / 100;
	num = num - second * 100;
	int third = num / 10;
	num = num - third * 10;

	locator[0] = (char)first + 96;
	locator[1] = (char)second + 96;
	locator[2] = (char)third + 48;
	locator[3] = (char)num + 48;
	locator[4] = 0;

	return locator;
}
