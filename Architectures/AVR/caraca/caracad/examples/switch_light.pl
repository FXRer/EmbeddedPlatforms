#!/usr/bin/perl -w
use strict;
use Caraca;

my $c = Caraca->new();
$c->filter(0,14);
#get key messages from all nodes

while (1) {
	my $answer='';

	$c->{SOCK}->recv($answer,100,0);
	$answer =~ /(\d+)\.(\d+):(\d+),(\d+)/;
	#$1 - node, $2 - function (always 14)
	#$3 - 1=key pressed 2=key released
	#$4 - Number of Key (1..4)
	
	if ($3 == 2) {
		#toggle output on every keyrelease (same node)
		my $output=2<<$4;
                #input 1 -> output 4
                #input 2 -> output 8
                #input 3 -> output 16 (230V switch)
                #input 4 -> output 32 (dimmer)
		
		$c->send($1,1,2,$output);
		print "$1,1,2,$3,$output\n";
	}	
}

__END__  


