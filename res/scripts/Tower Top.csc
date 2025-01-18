#0020 - Enter
:TRK0001
:FDI :END

#0100 - Tower exit
:LCK :FDO :MAP0010,0058,0051,0003

#0200 - Game ending trigger
:LCK :FDO :TRK0011
:WAT1000
:FLJ0024 :EVE0201
:TXT "You return whence"
"\nyou came..." :PAU
:EVE0203
#0201 - selfish
:FLJ0025 :EVE0202
:TXT "You are wicked." :PAU
:EVE0203
#0202 - selfless
:TXT "In time all things"
"\nreturn to their"
"\nplace." :PAU
:EVE0203
#0203 - end
:TCR "\n      The End      " :PAU :TCL :FDI :TRK0001 :END

#0999 - debugger of doom
:FLJ0024 :EVE1000
:FLS0024 :SFX0001 :END
#1000
:FLJ0025 :EVE1001
:FLS0025 :SFX0001 :END
#1001
:FLR0024 :FLR0025
:SFX0002 :END
