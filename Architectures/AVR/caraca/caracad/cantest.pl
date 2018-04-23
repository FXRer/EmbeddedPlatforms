#! /usr/bin/perl

use strict vars;
use Caraca;

my $c = Caraca->new();
my $node;

for (6..12) {
	$c->send($_,4,$_);
	sleep(1);
}
