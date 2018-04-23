#!/usr/bin/perl -w
#
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#

#use strict;
use Caraca;
use Sys::Syslog qw(:DEFAULT setlogsock);
use Storable;
use POSIX qw(setsid strftime);
#use Data::Dumper;
 
$STATE = '/var/state/caraca_key_mapping';

my $_programname=$0;
$_programname=~s{^.*/}{};
setpriority 0,0,-20;

setlogsock('unix');
openlog($_programname, 'pid', 'daemon');

my $c = Caraca->new();
my $ir_id = 0;
$c->filter(0,14);     #keypress
$c->filter(0,15);     #remote
#$c->filter(0,17);     #Send output
#get key messages from all nodes

$c->filter(0,30);
#get boot messages from all nodes
eval {
%output_state = %{ retrieve($STATE) };
};

setsid();
fork && exit;
syslog('info','started');

&catch_signals;
&read_config;

while (1) {
	my $answer='';

	$c->{SOCK}->recv($answer,100,0);
	#$answer = '23.30:0,4';
	#$answer = '23.14:1,3';
	$answer =~ /(\d+)\.(\d+):(\d+),(\d+)/;
	#$1 - node, $2 - function 
	SWITCH: {
		if ($2 == 14) { do_keys(); last SWITCH; }
		if ($2 == 15) { do_remote(); last SWITCH; }
		if ($2 == 30) { process_boot_msg(); last SWITCH; }
        }
	#sleep 1;
}

sub read_config {
	eval `cat /etc/caraca/key_mapping.conf`
		or do 'key_mapping.conf'
		or die "config error; $@\n";
}

sub process_boot_msg {
	#restore state of outputs
	syslog('warning',"Node $1 booted. Setting outputs. "); 
	$c->send($1,1,$output_state{"$1:16"},16);
	$c->send($1,1,$output_state{"$1:32"},32);
	#check node...
}

sub toggle {
	my $light = shift;
	foreach $action (split(/\//,$light)) {
		my ($node,$output) = split(/:/,$action);
#toggle state and save into hash
		$output_state{$action} = $output_state{$action}?0:1; 
		$c->send($node,1,$output_state{$action},$output);
		select(undef, undef, undef, 0.1);
		$dim_node = $node;
		$msg .= "$node:$output (";
		$msg .= $output_state{$action}?'off) ':'on) ';
	}
}
sub dim {
	my $direction = shift;
	$brightness{$dim_node} += $direction;
	$brightness{$dim_node} = 0 if $brightness{$dim_node}<0;
	$brightness{$dim_node} = 92 if $brightness{$dim_node}>92;
	$c->send($dim_node, 2, $brightness{$dim_node});
	$msg .= "dim $dim_node:".$brightness{$dim_node};
}

sub send1 {
	my $node = shift;
	my $function = shift;
	my $param = shift;
	$c->send( $node, $function, $param);
}

sub do_remote {

#toggle output on every keypress
	my $lookup="$1:$3";
	my $id=$1.$3.$4;	 #$4 toggles between 71 and 79
	$msg = "remote $lookup --> ";
	return if (($4 | 13) != 79);
	return if ($id == $ir_id); 
	if (defined($remote{$lookup})) {
		eval {
			#print Dumper( $remote{$lookup}); 
			&{$remote{$lookup}}();
		};
		warn "Error: $@\n" if $@ ne '';
		syslog('debug',$msg);
		$ir_id = $id;
	} else {
		syslog('warning',$msg.'Not assigned'); 
	}
}


sub do_keys { 
	#$3 - 1=key pressed 2=key released
	#$4 - Number of Key (1..4)
	
	if ($3 == 1) {
		#toggle output on every keypress
		my $lookup="$1:$4";
		my $msg = "key  $lookup --> ";
                if (defined($lights{$lookup})) {
			foreach $action (split(/\//,$lights{$lookup})) {
				my ($node,$output) = split(/:/,$action);
				#toggle state and save into hash
				$output_state{$action} = $output_state{$action}?0:1; 
				$c->send($node,1,$output_state{$action},$output);
		        	$msg .= "$node:$output (";
				$msg .= $output_state{$action}?'off) ':'on) ';
				#test, if node reacts properly
				#reset bus, if not?
			}
			syslog('debug',$msg);
		} else {
			syslog('warning',$msg.'Not assigned'); 
		}
	}	
}


sub catch_signals {
	$SIG{'INT'} = 'SIGNAL_CLEANUP';
	$SIG{'HUP'} = 'SIGNAL_REREAD';
	$SIG{'QUIT'} = 'SIGNAL_CLEANUP';
	$SIG{'TERM'} = 'SIGNAL_CLEANUP';
}

sub SIGNAL_REREAD {
	syslog('info','config reread');
	read_config();
}

sub SIGNAL_CLEANUP {
	syslog('info','stopped');
	store(\%output_state, $STATE);

	exit 2;
}

__END__  
