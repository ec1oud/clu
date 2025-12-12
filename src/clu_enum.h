/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2019 Andy Stewart <kb1oiq@arrl.net>
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
 * xlog_enum.h
 */

#ifndef XLOG_ENUM_H
#define XLOG_ENUM_H

enum /* we have many amateur radio bands */
{
	BAND_2190,	/* 0.1357 - 0.1378 MHz experimental */
	BAND_630,   /* 0.472 - 0.479 MHz */
	BAND_560,   /* 0.501 - 0.504 MHz */
	BAND_160,
	BAND_80,
	BAND_60,	/* 5.06 - 5.45 MHz */
	BAND_40,
	BAND_30,
	BAND_20,
	BAND_17,
	BAND_15,
	BAND_12,
	BAND_10,
    BAND_8,    /* 40 - 45 MHz */
	BAND_6,
	BAND_5,    /* 54.000001 - 69.9 MHz */
	BAND_4,
	BAND_2,
	BAND_125CM, /* 220 - 225 MHz, region 2 */
	BAND_70CM,
	BAND_33CM,	/* 902 - 928 MHz, region 2 */
	BAND_23CM,
	BAND_13CM,
	BAND_9CM,
	BAND_6CM,
	BAND_3CM,
	BAND_12HMM,
	BAND_6MM,
	BAND_4MM,
	BAND_2HMM,
	BAND_2MM,
	BAND_1MM,
	BAND_SUBMM,
	MAX_BANDS	/* number of bands in the enumeration */
};

enum /* modes and submodes taken from ADIF spec, listed alphabetically */
{
  MODE_AM,
  MODE_AMTORFEC,
  MODE_ARDOP,
  MODE_ASCI,
  MODE_ATV,
  MODE_C4FM,
  MODE_CHIP,
  MODE_CHIP128,
  MODE_CHIP64,
  MODE_CLO,
  MODE_CONTESTI,
  MODE_CW,
  MODE_DIGITALVOICE,
  MODE_DOMINO,
  MODE_DOMINOEX,
  MODE_DOMINOF,
  MODE_DSTAR,
  MODE_FAX,
  MODE_FM,
  MODE_FMHELL,
  MODE_FSK31,
  MODE_FSK441,
  MODE_FSKHELL,
  MODE_FST4,
  MODE_FSQCALL,
  MODE_FT4,
  MODE_FT8,
  MODE_GTOR,
  MODE_HELL,
  MODE_HELL80,
  MODE_HFSK,
  MODE_ISCAT,
  MODE_ISCAT_A,
  MODE_ISCAT_B,
  MODE_JS8,
  MODE_JT4,
  MODE_JT44,
  MODE_JT4A,
  MODE_JT4B,
  MODE_JT4C,
  MODE_JT4D,
  MODE_JT4E,
  MODE_JT4F,
  MODE_JT4G,
  MODE_JT65,
  MODE_JT65A,
  MODE_JT65B,
  MODE_JT65B2,
  MODE_JT65C,
  MODE_JT65C2,
  MODE_JT6M,
  MODE_JT9,
  MODE_JT9_1,
  MODE_JT9_10,
  MODE_JT9_2,
  MODE_JT9_30,
  MODE_JT9_5,
  MODE_JT9A,
  MODE_JT9B,
  MODE_JT9C,
  MODE_JT9D,
  MODE_JT9E,
  MODE_JT9E_FAST,
  MODE_JT9F,
  MODE_JT9F_FAST,
  MODE_JT9G,
  MODE_JT9G_FAST,
  MODE_JT9H,
  MODE_JT9H_FAST,
  MODE_LSB,
  MODE_MFSK,
  MODE_MFSK11,
  MODE_MFSK128,
  MODE_MFSK16,
  MODE_MFSK22,
  MODE_MFSK31,
  MODE_MFSK32,
  MODE_MFSK4,
  MODE_MFSK64,
  MODE_MFSK8,
  MODE_MSK144,
  MODE_MT63,
  MODE_OLIVIA,
  MODE_OLIVIA_16_1000,
  MODE_OLIVIA_16_500,
  MODE_OLIVIA_32_1000,
  MODE_OLIVIA_4_125,
  MODE_OLIVIA_4_250,
  MODE_OLIVIA_8_250,
  MODE_OLIVIA_8_500,
  MODE_OPERA,
  MODE_OPERA_BEACON,
  MODE_OPERA_QSO,
  MODE_PAC,
  MODE_PAC2,
  MODE_PAC3,
  MODE_PAC4,
  MODE_PAX,
  MODE_PAX2,
  MODE_PCW,
  MODE_PKT,
  MODE_PSK,
  MODE_PSK10,
  MODE_PSK1000,
  MODE_PSK125,
  MODE_PSK250,
  MODE_PSK2K,
  MODE_PSK31,
  MODE_PSK500,
  MODE_PSK63,
  MODE_PSK63F,
  MODE_PSKAM10,
  MODE_PSKAM31,
  MODE_PSKAM50,
  MODE_PSKFEC31,
  MODE_PSKHELL,
  MODE_Q15,
  MODE_QPSK125,
  MODE_QPSK250,
  MODE_QPSK31,
  MODE_QPSK500,
  MODE_QPSK63,
  MODE_QRA64,
  MODE_QRA64A,
  MODE_QRA64B,
  MODE_QRA64C,
  MODE_QRA64D,
  MODE_QRA64E,
  MODE_ROS,
  MODE_ROS_EME,
  MODE_ROS_HF,
  MODE_ROS_MF,
  MODE_RTTY,
  MODE_RTTYM,
  MODE_SIM31,
  MODE_SSB,
  MODE_SSTV,
  MODE_T10,
  MODE_THOR,
  MODE_THRB,
  MODE_THRBX,
  MODE_TOR,
  MODE_USB,
  MODE_V4,
  MODE_VOI,
  MODE_WINMOR,
  MODE_WSPR,
  MAX_MODES /* number of modes in the enumeration */
};

char *band_enum2char (uint band_enum);
char *band_enum2cabrillochar (uint band_enum);
int freq2enum (char * str);
int hamlibfreq2enum (long long f);
char *band_enum2bandchar (int band_enum);
int meters2enum (char * str);
char *mode_enum2char (uint mode_enum);
int reportlen(uint mode_enum);
int mode2enum (char * str);
char *freq2khz (char *str);

#endif // XLOG_ENUM_H

