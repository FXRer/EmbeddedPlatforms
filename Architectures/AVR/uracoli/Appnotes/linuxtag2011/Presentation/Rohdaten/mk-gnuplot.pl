#! /usr/bin/perl

use warnings;
use strict;

use Getopt::Std;

use vars qw(%opts);
# -sN - start at time N
# -pN - stop at time N
# -m  - smooth output
# -oX - omit sensor X
# -fN - output to file N (rather than displaying on X11)
getopts('f:s:p:mo:', \%opts);

use vars qw(%skip);
%skip = ();

if (defined($opts{o})) {
    my @x = split(/,/, $opts{o});
    foreach my $v (@x) {
	$skip{$v} = 1;
    }
}
if (defined($opts{s})) {
    $opts{s} *= 3600.0;
}
if (defined($opts{p})) {
    $opts{p} *= 3600.0;
}

my @fieldnames;
my $headline_printed = 0;

my $time;
my $offset = 0;
my @fields;
my $lineno = 0;

open(PLOT, ">/tmp/plotfile") || die "Cannot create /tmp/plotfile: $!\n";

while (<>) {
    chomp;
    s/\r//g;
    $lineno++;
    if (/^$/) {
	next if !$time || !@fieldnames;
	if (!$headline_printed) {
	    $headline_printed = 1;
	    for (my $i = 0; $i <= $#fieldnames; $i++) {
		print PLOT "#time\t" if $i == 0;
		print PLOT $fieldnames[$i];
		if ($i == $#fieldnames) {
		    print PLOT "\n";
		} else {
		    print PLOT "\t";
		}
	    }
	}
printout:
	if ((defined($opts{s}) && ($time + $offset < $opts{s})) ||
	    (defined($opts{p}) && ($time + $offset > $opts{p}))) {
	    # do print nothing
	} else {
	    print PLOT ($time + $offset) / 3600 . "\t";
	    for (my $j = 0; $j <= $#fields; $j++) {
		my $temp = $fields[$j];
		$temp -= 256 if $temp > 200;
		if ($temp == 0 && $fieldnames[$j] =~ '^TMP') {
		    # omit failed measurements for TMP141 devices
		    print PLOT "''";
		} else {
		    print PLOT $temp;
		}
		if ($j == $#fields) {
		    print PLOT "\n";
		} else {
		    print PLOT "\t";
		}
	    }
	}
	@fields = ();
	next;
    }
    if (/^[+](\d+)d (\d\d):(\d\d):(\d\d)/){
	$time = $1 * 86400 + $2 * 3600 + $3 * 60 + $4;
	next;
    }
    if (/^([^ ]+) +([-0-9.]+)/) {
	push @fieldnames, $1 unless $headline_printed;
	push @fields, $2;
	next;
    }
    if (/^Temperature logger booted/) {
	# ouch, reboot
	$offset += $time + 5;
	$time = 0;
	@fields = ();
	printf STDERR "setting offset to %.3f h\n", $offset / 3600;
	next;
    }
    die "unexpected pattern in input: $_, line $lineno\n";
}
close(PLOT);

open(GNUPLOT, ">/tmp/gnuplot.cmds") || die "Cannot create /tmp/gnuplot.cmds: $!\n";
print GNUPLOT 'set xlabel "Zeit"' . "\n";
print GNUPLOT 'set ylabel "Temperatur"' . "\n";
if (defined $opts{f}) {
    # perform SVG output
    print GNUPLOT "set terminal svg\n";
    print GNUPLOT "set output \"" . $opts{f} . "\"\n";
}
print GNUPLOT "plot ";
my $first = 1;
for (my $i = 0; $i <= $#fieldnames; $i++) {
    next if defined($skip{$fieldnames[$i]});	# skip this sensor
    if ($first) {
	$first = 0;
    } else {
	print GNUPLOT ", ";
    }
    print GNUPLOT '"/tmp/plotfile" using 1:' . ($i + 2);
    if (defined($opts{m})) {
	print GNUPLOT ' smooth bezier title "' . $fieldnames[$i] . '"';
    } else {
	print GNUPLOT ' with lines title "' . $fieldnames[$i] . '"';
    }
}
print GNUPLOT "\n";
if (!defined $opts{f}) {
    print GNUPLOT "pause mouse keypress\n";
}
close GNUPLOT;
system("gnuplot /tmp/gnuplot.cmds");
