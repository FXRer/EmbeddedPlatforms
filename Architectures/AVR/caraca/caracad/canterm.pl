#!/usr/bin/perl -w
#use strict var, subs;
use Tk;
use Tk::BrowseEntry;
use Carp;
use Caraca;

my $c;                                    # Connection to caracad
# Widgets
my $w_start;                              # start button widget
my $w_top;                                # top level widget
my $w_list;                               # canvas

my @msg;

my $lb;

sub init {
	$w_top = MainWindow->new(-title => 'CARACA - Explorer');

	my $menubar = $w_top->Frame(-relief => 'raised', -borderwidth => 2);
	$menubar->pack(side => 'top',  -fill => 'x');
	my $f = $menubar->Menubutton(qw/-text File -justify left -underline 0 -menuitems/ =>
			[
			[Button => 'Quit',        command => sub {exit(0)}],
			])->grid(qw/-row 0 -column 0 -sticky w/);

	# Create directory scrollbox, place at the left
	$lb = $w_top->Component(ScrlListbox    => 'dir_list', 
			-scrollbars    => 'se');
	$lb->pack(side => 'left', -expand => 1, -fill => 'both');
	$lb->bind('<Button-1>' => [\&Accept_dir, Ev(['getSelected'])]);
	$lb->bind('<space>' => [\&Accept_dir, Ev(['getSelected'])]);
	my $f = $w_top->Frame(qw/-borderwidth 2/);
	my $e =  $f->BrowseEntry(relief => 'sunken', width => 15, -variable => \$Id);
	my $l = $f->Label(-text => 'Function:');
	open(FUNC,"</etc/caraca/functions");
        %func = map {chomp;($a,$b,$c)=split/:/;$a => $b;} <FUNC>;
        close(FUNC);

	foreach (sort keys %func) {
		$e->insert("end", $_ . $func{$_});
	}  

	$f->pack(qw/-side top -fill x/);
	$e->pack(qw/-side right/);       
	$l->pack(qw/-side left/);        

	$f = $w_top->Frame(qw/-borderwidth 2/);
        $e =  $f->BrowseEntry(relief => 'sunken', width => 15, -variable => \$Node);
        $l = $f->Label(-text => 'Node:');
        open(NODE,"</etc/caraca/nodes");
        %node = map {chomp;($a,$b,$c)=split/:/;$a => $b;} <NODE>;
        close(NODE);

        foreach (sort keys %node) {
                $e->insert("end", $_ . $node{$_});
        }

        $f->pack(qw/-side top -fill x/);
        $e->pack(qw/-side right/);
        $l->pack(qw/-side left/);

	
	for (0..3) {
		my $f = $w_top->Frame(qw/-borderwidth 2/);
		my $e = $f->Entry(relief => 'sunken', width => 8, textvariable => eval('\$msg[$_]'));
		my $l = $f->Label(-text => "Byte$_:");
		$f->pack(qw/-side top -fill x/);
		$e->pack(qw/-side right/);       
		$l->pack(qw/-side left/);        
	}        


	$w_start = $w_top->Button(text => 'Send', command => \&send, underline =>0);
#  my $w_quit = $w_top->Button(text => 'Quit', command => sub {exit(0)});
#  my $w_quit = $w_top->Button(text => 'Quit', command => \&quit_play);

	$w_start->pack('-side'=> 'left', '-fill' => 'x', '-expand' => 'x');

#  $w_top->bind('<Control-s>' => \&send);

	$c = Caraca->new();
}

sub send {
	my $func = substr($Id,0,2);
	my $node = substr($Node,0,2);
	$c->send($node,$func,@msg);
}

sub bintodec {
	unpack("N", pack("B32", substr("0" x 32 . shift, -32)));
}

$|=1;

init();

MainLoop();

__END__  







