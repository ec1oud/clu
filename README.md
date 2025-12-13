# clu

*Callsign (and grid) Looker Upper*

I was looking for a cty.dat parser, preferably in C,
for the [sbitx/zbitx](https://github.com/ec1oud/sbitx) project.
https://www.country-files.com/big-cty/ pointed out that 
[xlog](https://github.com/kareiva/xlog) has one.  This is little more
than that parser and related functionality from xlog, with everything
else removed. And then I added a Maidenhead grid parser.

This could be incorporated into something else (like I'm doing).
But if you need a command-line utility, just build it and then e.g.
```
$ clu CQ AA1BCD CQ LB2JK JO59 73 de K7IHZ DM43 call PM34ab
AA1BCD: country 179 'United States' cq 5 itu 8 continent 0 lat 3760 lon -9187
LB2JK @ JO59: country 197 'Norway' cq 14 itu 18 continent 4 lat 6450 lon 2100
K7IHZ @ DM43: country 179 'United States' cq 5 itu 8 continent 0 lat 3850 lon -10100
PM34ab: lat 3956 lon 13704
$ clu -d DM43 call PM95ab FN55 JO59  MK75
DM43:   38.50,-101.00
PM95ab: 38.50,-101.00 to 40.56,149.04   : distance  8713 azimuth 313
FN55:   40.56,149.04 to 50.50,-59.00    : distance  9528 azimuth  17
JO59:   50.50,-59.00 to 64.50,21.00     : distance  4662 azimuth  39
MK75:   64.50,21.00 to 20.50,85.00      : distance  6724 azimuth 105
```
assuming for now that cty.dat is in the same place where xlog would have it
(e.g. you can install xlog from your system package manager to get that):
/usr/share/xlog/dxcc/cty.dat
This repo also includes a cty.dat but I don't promise to keep it updated;
so if you need to download it separately, get the latest from country-files.com

