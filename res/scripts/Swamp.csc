#0020 - Enter
:TRK0005
:FDI :END

#0100 - Beach exits
:LCK :FDO :MAP0000,0035,0027,0000
#0101
:LCK :FDO :MAP0000,0036,0027,0000

#0110 - Cave1 exit
:LCK :FDO :MAP0001,0025,0029,0003

#0200 - Mook
:LCK :FLJ0600 :EVE0201
:TXB "Well,this is a"
"\nsurprise!" :PAU
"\nNot many people"
"\ncome to visit old  "
"\nMook at his pond;" :PAU
"\nnot since all those"
"\nfolks left"
"\ntopwards,haha!" :PAU
:TCL :FLS0600 :END
#0201 - after first
:FLJ0027 :EVE0202
:TXB "Nope,nothing quite"
"\nlike the feeling of"
"\nwarm clay!" :PAU
:TCL :END
#0202 - Got fruit
:FLJ0601 :EVE0203
:TXB "I'm going to bring"
"\nsome mudpies to my"
"\nlittle friend." :PAU
:TCL :FLS0601 :END
#0203 - after got fruit
:FLJ0028 :EVE0204
:TXB "You don't have to"
"\ntake off your shoes"
"\nwhen you come to my"
"\nhouse." :PAU
:TCL :END
#0204 - Gave fruit
:FLJ0602 :EVE0205
:TXB "The tower?" :PAU
"\nNo,I can't say I"
"\nknow much about"
"\nthat." :PAU
"\nEver since they"
"\nbuilt it,they've"
"\njust been sitting"
"\naround!" :PAU
:TCL :FLS0602 :END
#0205 - after gave fruit
:FLJ0029 :EVE0206
:TXB "Talk about a lazy"
"\nafternoon!" :PAU
:TCL :END
#0206 - Visited ruins
:FLJ0603 :EVE0207
:TXB "I've been to the"
"\nruins,but there is"
"\nnothing quite like"
"\nsitting on your own" :PAU
"\nfront porch." :PAU
:TCL :FLS0603 :END
#0207 - after visited ruins
:FLJ0024 :EVE0208
:TXB "Don't forget this:" :PAU
"\neverything's better"
"\nwhen you're with a"
"\nfriend." :PAU
:TCL :END
#0208 - Got star
:FLJ0604 :EVE0209
:TXB "Oh,me?" :PAU
"\nI'm not one of"
"\nthose golems,hehe." :PAU
"\nI just like it"
"\nhere.This pond is"
"\nnice and warm." :PAU
:TCL :FLS0604 :END
#0209 - after got star
:TXB "Will you be going"
"\nback to where you"
"\ncame from?" :PAU
:TCL :END

#0999 - magic seed; set fruit
:LCK :SFX0001
:FLJ0027 :EVE1000
:FLS0027 :END
#1000 - set elder
:FLJ0028 :EVE1001
:FLS0028 :END
#1001 - set ruins
:FLJ0029 :EVE1002
:FLS0029 :END
#1002 - set orb
:FLS0024 :END
#1004 - lever of doom
:LCK :SFX0002
:FLR0027 :FLR0028 :FLR0029 :FLR0024 :FLR0025
:FLR0100 :FLR0101 :FLR0102 :FLR0103 :FLR0104
:FLR0200 :FLR0201 :FLR0202 :FLR0203 :FLR0204
:FLR0300 :FLR0301 :FLR0302 :FLR0303 :FLR0304
:FLR0400 :FLR0401 :FLR0402 :FLR0403 :FLR0404
:FLR0500 :FLR0501 :FLR0502 :FLR0503 :FLR0504
:FLR0600 :FLR0601 :FLR0602 :FLR0603 :FLR0604
:FLR0700 :FLR0701 :FLR0702 :FLR0703 :FLR0704
:END
