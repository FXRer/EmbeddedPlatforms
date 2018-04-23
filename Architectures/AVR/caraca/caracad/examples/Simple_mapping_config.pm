# This is CARACA's systemwide configuration file. 
# This file provides 
# a list of nodes             (Name -> Number)
# a list of lamps eg. outputs (Name -> Address) 
# a list of keys              (Name -> Address)
# several mappings
#
# This is perl syntax!

# here we have a mapping between keyrelease and output toggle in the following syntax
# 'NODE:KEY' => 'NODE:TOGGLE_MASK'
# so you can toggle any number of outputs on one node at once
#%nodes = ( '
%lights = ( '20:1' => '21:16',
	    '20:2' => '20:48',
	    '20:4' => '20:32',
	    '23:2' => '23:16',
	    '23:3' => '23:32',
	    '24:2' => '24:16',
	    '24:3' => '24:32',#Bad OG
	     '2:3' =>  '2:16',#Toi Eg
	    '16:1' => '16:16',#Toi EG
	    '16:3' => '16:32',#AZ  EG
	    '19:3' => '12:16',
	    '19:4' => '12:32',
	    '15:2' => '23:32', 
	    '15:3' => '15:32', #OG Kizi/flur rechter Taste => Kizi Licht
	    '15:4' => '15:32', #OG Kizi/flur Tür => Kizi Licht
	    '11:3' => '11:32',
	    '11:2' => '23:16'
          );

1;
__END__

