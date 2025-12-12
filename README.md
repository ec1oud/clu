# clu

*Callsign Looker Upper*

I was looking for a cty.dat parser, preferably in C,
for the [sbitx/zbitx](https://github.com/ec1oud/sbitx) project.
https://www.country-files.com/big-cty/ pointed out that 
[xlog](https://github.com/kareiva/xlog) has one.  This is little more
than that parser and related functionality from xlog, with everything
else removed.

This could be incorporated into something else (like I'm doing).
But if you need a command-line utility, just build it and then e.g.
```
$ clu LB2JK K7IHZ
LB2JK: country 197 'Norway' cq 14 itu 18 continent 4 lat 6100 lon -900
K7IHZ: country 179 'United States' cq 5 itu 8 continent 0 lat 3760 lon 9187
```
assuming for now that cty.dat is in the same place where xlog would have it
(e.g. you can install xlog from your system package manager to get that):
/usr/share/xlog/dxcc/cty.dat
This repo also includes a cty.dat but I don't promise to keep it updated;
so if you need to download it separately, get the latest from country-files.com

