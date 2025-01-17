#0020 - Enter
:TRK0007
:FLS0029
:FDI :END

#0100 - Beach exit
:LCK :FDO :MAP0000,0021,0052,0003

#0200 - Ruins orb
:LCK :FLJ0025 :EVE0203
:TXT "Seems like there is"
"\na small depression"
"\nhere." :PAU
:FLJ0024 :EVE0201
:TCL :END
#0201 - prompt place orb
:TCR "Place the orb?" :PYS :EVE0202
:TCL :END
#0202 - place orb
:TCL :EAN0200,0001 :TRK0000 :WAT1000
:SFX0003 :QUK3000 :WAT3000
:FLS0025 :END
#0203 - post placed
:TXT "The orb has been"
"\nplaced. You cannot"
"\nremove it now." :PAU
:TCL :END

#0999 - debug
:LCK :SFX0001 :FLR0025 :FLS0024 :EAN0200,0000 :END
