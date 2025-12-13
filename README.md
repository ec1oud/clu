# clu

*Callsign (and grid) Looker Upper*

I was looking for a cty.dat parser, preferably in C,
for the [sbitx/zbitx](https://github.com/ec1oud/sbitx) project.
https://www.country-files.com/big-cty/ pointed out that 
[xlog](https://github.com/kareiva/xlog) has one.  This started with
that parser and related functionality from xlog, with everything
else removed. And then I brought in the grid parser and distance calculator
from [hamlib](https://github.com/Hamlib/Hamlib).

This could be incorporated into something else (like I'm doing).
But if you need a command-line utility, just build it and then e.g.
```
$ clu CQ AA1BCD CQ LB2JK JO59 73 de K7IHZ DM43 call PM34ab
AA1BCD: country 179 USA 'United States' cq 5 itu 8 continent 0 lat  37.60 lon -91.87
LB2JK @ JO59: country 197 Norway 'Norway' cq 14 itu 18 continent 4 lat  59.50 lon  11.00
K7IHZ @ DM43: country 179 USA 'United States' cq 5 itu 8 continent 0 lat  33.50 lon -111.00
PM34ab: 34.06,126.04
$ clu -d DM43 PM95ab FN55 JO59 MK75
DM43:   33.50,-111.00
PM95ab: 33.50,-111.00 to 35.06,138.04   : distance  9543 azimuth 310
FN55:   35.06,138.04 to 45.50,-69.00    : distance 10654 azimuth  19
JO59:   45.50,-69.00 to 59.50,11.00     : distance  5276 azimuth  43
MK75:   59.50,11.00 to 15.50,75.00      : distance  7072 azimuth 105
```
assuming for now that cty.dat is in the same place where xlog would have it
(e.g. you can install xlog from your system package manager to get that):
/usr/share/xlog/dxcc/cty.dat
This repo also includes a cty.dat but I don't promise to keep it updated;
so if you need to download it separately, get the latest from country-files.com

