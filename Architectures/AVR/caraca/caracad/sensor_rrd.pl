#! /usr/bin/perl

use strict vars;
use vars qw($a $b $c);
use RRDs;
use Getopt::Long;
use Caraca;

# Poll interval (in seconds).  This value should be greater than
# the number of retries times the timeout value.
my $interval = 300;
my $rrd = "/home/skr/caraca/temperature.rrd";
my $command = shift;

my $range = shift || "7d";

eval {
	&{$command};
};
if ($@) {
	print "$@\n" ;
        print <<"USAGE";
usage: $0 create - creates a new database
          update - reads temperature sensors into rrd file
          test   - prints temperature sensors readings
	  graph  RANGE - writes test.png (default range 7d)
USAGE
}

sub create {

	RRDs::create($rrd, qw(-s 300        
				DS:temp1:GAUGE:600:-10:100   
				DS:temp2:GAUGE:600:-10:100   
				RRA:AVERAGE:0.5:1:1600      
				RRA:AVERAGE:0.5:6:600      
				RRA:AVERAGE:0.5:24:600     
				RRA:AVERAGE:0.5:288:732    
				RRA:MAX:0.5:1:1600          
				RRA:MAX:0.5:6:600          
				RRA:MAX:0.5:24:600         
				RRA:MAX:0.5:288:732
				RRA:MIN:0.5:1:1600          
				RRA:MIN:0.5:6:600          
				RRA:MIN:0.5:24:600         
				RRA:MIN:0.5:288:732));
}
# every  5 minutes for 1600*300=133 Stunden
# every 30 minutes for 600*.5  =300 Stunden
# every  2 hours   for 50 days
# every  1 day     for 2 years
sub graph {
	my @defs;
	my @lines;
	push @defs,"DEF:ds0=$rrd:temp1:AVERAGE";
#	push @defs,"DEF:ds0=$rrd:temp1:MIN";
        push @defs,"DEF:ds0mx=$rrd:temp1:MAX";
        push @defs,"DEF:ds0mn=$rrd:temp1:MIN";
#	push @defs,"DEF:ds1=$rrd:temp2:MIN";
	push @defs,"DEF:ds1=$rrd:temp2:AVERAGE";
        push @lines,"LINE2:ds0#ff0000:Aussen-Temperatur";
	push @lines,'GPRINT:ds0:MAX:max %2.1lf %s';
	push @lines,'GPRINT:ds0:AVERAGE:avg %2.1lf %s';
	push @lines,'GPRINT:ds0:MIN:min %2.1lf %s';
	push @lines,'GPRINT:ds0:LAST:last %2.1lf %s)\l';
#	push @lines,"LINE1:ds0mx#2f0000:max CPU";
#	push @lines,"LINE1:ds0mn#2f0000:min CPU";
        push @lines,'LINE2:ds1#0000ff: Innen-Temperatur';
	push @lines,'GPRINT:ds1:MAX:(max %2.1lf %s';
	push @lines,'GPRINT:ds1:AVERAGE:avg %2.1lf %s';
	push @lines,'GPRINT:ds1:MIN:min %2.1lf %s';
	push @lines,'GPRINT:ds1:LAST:last %2.1lf %s)';
        push @lines,'PRINT:ds0:MIN:min %2.1lf %s';

        my(@args) = ("test.png", '--start', "-$range",
				 '--height', 500,
                                 @defs, @lines);

#        warn ("RRDs::graph " . join(" ", @args));
        my($avg, $w, $h) = RRDs::graph(@args);
	warn join(" ", @$avg); 
        if (my $error = RRDs::error) {
                warn("Unable to create graph: $error\n");
        }


}
sub ds1621 {
	my $temp = shift;
        $temp =~ /(\d+),(\d+),(\d+)/;
	$2 + $3/256;
}

sub test {
	my $count = shift || 1;
	#my $c = Caraca->new(PeerAddr => 'pandora');
	my $c = Caraca->new();
	$c->filter(0,20);
	for (1..$count) {
	        for (0..7) {
		print "$_: " . ds1621($c->request(12,10,$_)) . "\n";
		}
	}
	#Dump Filters to syslog
	$c->sendmsg("D \0");		

}

sub update {
	
	my $c = Caraca->new(PeerAddr => 'pandora');
	$c->filter(0,20);
	my $temp1 = 0;#ds1621($c->request(1,10,0));
	my $temp2 = ds1621($c->request(1,10,7));
	
	RRDs::update($rrd, "N:$temp1:$temp2");
	#warn "Temp1: $temp1, Temp2: $temp2\n";
}

