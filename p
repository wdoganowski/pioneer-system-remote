pigs wvclr

# set 22 to '1'
echo set to 1
pigs w 22 1 

# define '0'
echo add '0'
pigs wvag 0 0x400000 560 0x400000 0 22400 
pigs wvcre

# define '1'
echo add '1'
pigs wvag 0 0x400000 560 0x400000 0 560 
pigs wvcre

# define 'start'
echo add 'start'
pigs wvag 0 0x400000 9000 0x400000 0 4500 
pigs wvcre

# define 'stop'
echo add 'stop'
pigs wvag 0 0x400000 560 0x400000 0 560 
pigs wvcre

# define start - a5 - stop
echo transmit
pigs wvtx 2 wvtx 1 wvtx 0 wvtx 1 wvtx 0 wvtx 0 wvtx 1 wvtx 0 wvtx 1 wvtx 3 mils 1000

