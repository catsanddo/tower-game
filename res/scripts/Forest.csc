#0020 - Enter
:TRK0003
:EVE0021
:FDI :END

#0021 - Hide fruit
:EFR0300,0002 :FDI :END

#0100 - Town exits
:LCK :FDO :MAP0011,0016,0043
#0101
:LCK :FDO :MAP0011,0017,0043

#0110 - Beach exits
:LCK :FDO :MAP0000,0021,0010
#0111
:LCK :FDO :MAP0000,0022,0010

#0120 - Cave2 exits
:LCK :FDO :MAP0002,0023,0032

#0130 - Cave1 exits
:LCK :FDO :MAP0001,0004,0025

#0200 - Game start
:LCK :TXT "You dream of\nfalling..." :PAU :TCL
:SFX0002 :TRK0003 :EVE0021 :FDI :END

#0300 - Sign (fruit)
:LCK :TXT
"Welcome to the"
"\ndemo!" :PAU :TCR
"Story & Music by"
"\nCasual hero" :PAU
"\n\n\n\n" :TCR
"Programming by"
"\nCat" :PAU
"\n\n\n\n" :TCR
"Characters by"
"\nAnt Hands" :PAU :TCR
"Feedback is"
"\nappreciated" :PAU :TCL
:END
