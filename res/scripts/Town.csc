#0020
:TRK0008
:FDI :END

#0100 - Forest exits
:LCK :FDO :MAP0003,0078,0008
#0101
:LCK :FDO :MAP0003,0079,0008

#0110 - Tower Cave exit
:LCK :FDO :MAP0007,0007,0020

#0200 - Sword guy
:LCK :FLJ0100 :EVE0201
:TXB "A new face?" :PAU
"\nMake sure you don't"
"\ncause any trouble"
"\nhere, because I'm"
"\nwatching you!" :PAU
:TCL :FLS0100 :END
#0201 - after first
:FLJ0027 :EVE0202
:TXB "I've been guarding"
"\nthis town for as"
"\nlong as I've lived!" :PAU
:TCL :END
#0202 - got fruit
:FLJ0101 :EVE0203
:TXB "Oh, this sword?" :PAU
"\nYeah, I bet you'd"
"\nlike to hold it,"
"\nhuh?" :PAU
"\nIt's definitely too"
"\nheavy for you, but"
"\nit isn't even a"
"\nproblem for me." :PAU
:TCL :FLS0101 :END
#0203 - after got fruit
:FLJ0028 :EVE0204
:TXB "Heh, I can tell"
"\nyou're impressed." :PAU
:TCL :END
#0204 - gave fruit
:FLJ0102 :EVE0205
:TXB "So you're leaving,"
"\nthen?" :PAU
"\nYou're probably"
"\neager to follow"
"\nthose other folk, I"
"\nguess." :PAU
:TCL :FLS0102 :END
#0205 - after gave fruit
:FLJ0029 :EVE0206
:TXB "Maybe I'd join"
"\nyou..." :PAU
"\nbut I can't leave"
"\nmy post." :PAU
:TCL :END
#0206 - visited ruins
:FLJ0103 :EVE0207
:TXB "I bet you've seen"
"\nsome pretty cool"
"\nstuff around here,"
"\nright?" :PAU
"\nMaybe you can tell"
"\nabout it some time?" :PAU
:TCL :FLS0103 :END
#0207 - after visited ruins
:FLJ0024 :EVE0208
:TXB "I've never actually"
"\nleft the town." :PAU
:TCL :END
#0208 - got orb
:FLJ0104 :EVE0209
:TXB "Hey, do you think"
"\nyou could take me"
"\nwith you sometime?" :PAU
"\nWe shouldn't go too"
"\nfar from the town,"
"\nbut maybe just"
"\noutside the gate?" :PAU
:TCL :FLS0104 :END
#0209 - after got orb
:TXB "Well, I'm not going"
"\nanywhere in this"
"\nstate." :PAU
"\nI haven't moved in"
"\nyears." :PAU
:TCL :END


#0300 - Ghost girl
:LCK :FLJ0200 :EVE0301
:TXB "Aaaaah!" :PAU
:TCR "You scared me..." :PAU
:TCL :FLS0200 :END
#0301 - after first
:FLJ0027 :EVE0302
:TXB "You shouldn't sneak"
"\nup on people..." :PAU
:TCL :END
#0302 - got fruit
:FLJ0201 :EVE0303
:TXB "Oh!" :PAU
"\nSomeone used to"
"\nbring me pala fruit"
"\nevery day." :PAU
"\nIt's been so long"
"\nthat I can't even"
"\nremember his face.\n" :PAU
:TCL :FLS0201 :END
#0303 - after got fruit
:FLJ0028 :EVE0304
:TXB "You'll come and"
"\nvisit me again..."
"\nright?" :PAU
:TCL :END
#0304 - gave fruit
:FLJ0202 :EVE0305
:TXB "Do you know where"
"\neveryone went?" :PAU
:TCL :FLS0202 :END
#0305 - after gave fruit
:FLJ0029 :EVE0306
:TXB "Well, nevermind..." :PAU
:TCL :END
#0306 - visited ruins
:FLJ0203 :EVE0307
:TXB "It's you again!" :PAU
"\nI remember your"
"\nface!" :PAU
:TCL :FLS0203 :END
#0307 - after visted ruins
:FLJ0024 :EVE0308
:TXB "Today I've been"
"\nthinking about all"
"\nof my favorite"
"\ntypes of flowers." :PAU
:TCL :END
#0308 - got orb
:FLJ0204 :EVE0309
:TXB "It's really good to"
"\nsee you!" :PAU :TCR
"Today I remembered"
"\na song that I used"
"\nto sing." :PAU :TCR
"Maybe I can sing"
"\nit for you..." :PAU
"\nBut you can't"
"\nwatch me!" :PAU
:TCL :FLS0204 :END
#0309 - after got orb
:TXB "Hey, there is"
"\nsomething I want"
"\nto tell you..." :PAU
:TCL :END

#0400 - Ant man
:LCK :SFX0002 :TXB "\n. . ." :PAU :TCL :SFX0001 :END

#0500 - Skuller
:LCK :SFX0002 :TXB "\n. . ." :PAU :TCL :SFX0001 :FLS0028 :END

#0600 - Dream worm
:LCK :SFX0002 :TXB "\n. . ." :PAU :TCL :SFX0001 :END
