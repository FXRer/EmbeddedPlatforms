package Caraca;

require 5.000;

=head1 NAME

Caraca - Object interface to CAN communication via caracad

=head1 SYNOPSIS

    use Caraca;

=head1 DESCRIPTION

C<Caraca> provides an object interface to communicate over the CAN-Bus. It
is built upon the L<IO::Socket> interface.

=head1 CONSTRUCTOR

=over 4

=item new ( )

Creates an connection via port 7776 to caracad (our message dispatcher for
CAN-Messages.

=back
=cut

use IO::Socket;
use Carp;
use strict;
use vars qw(@ISA $VERSION);
use Exporter;

# legacy

$VERSION = 0.61;

sub import {
    my $pkg = shift;
    my $callpkg = caller;
    Exporter::export 'Caraca', $callpkg, @_;
}

sub new {
    my ($class,%arg) = @_;
    my $self = {};
    $arg{PeerAddr} = 'localhost' if !exists $arg{PeerAddr};
    $self->{SOCK} = IO::Socket::INET->new(PeerPort  => 7776,
		    Type => SOCK_STREAM, %arg);
    #TODO: where to get error string?
    if (!defined($self->{SOCK})) {
 	    croak 'No connection to caracad@'.$arg{PeerAddr};
    }
    my $t= $self->{SOCK}->timeout(1);

    bless ($self, $class);
    return $self;
}

sub DESTROY{
    my $self = shift;
    $self->{SOCK}->close();
#    print "DESTROY\n";
}

sub sendmsg {
	my $self = shift;
	my $answer='';
	$self->{SOCK}->send(shift);
	eval {
		local $SIG{ALRM} = sub { die "alarm\n" }; 
		alarm 2;
		$self->{SOCK}->recv($answer,100,0);
	       	alarm 0;
	};
	return 0 if $answer =~ m/^OK/;
	warn $answer;
}

sub rtr {
	my $self = $_[0];
	my $answer='';
	$self->sendmsg('RTR '.$_[1]." 0\0");
	eval {
		local $SIG{ALRM} = sub { die "alarm\n" };
		alarm 2;
		$self->{SOCK}->recv($answer,100,0);
		alarm 0;
	};
	if ($@) {
		die unless $@ eq "alarm\n";
		$answer = "9999,0";
	} else {
		$answer =~ s/.*://;
	}
	return $answer;
}

sub request{
	my $self = shift;
	my $answer='';
	$self->send(@_);
	eval {
		local $SIG{ALRM} = sub { die "alarm\n" };
		alarm 2;
		$self->{SOCK}->recv($answer,100,0);
		alarm 0;
	};
	if ($@) {
		die unless $@ eq "alarm\n";
		$answer = "0,9999,0";
	} else {
		$answer =~ s/.*://;
	}
	return $answer;
}

sub send{
	my $self = shift;
	my $node = shift;
	my $function = shift;
        if ((my $length = $#_ + 1) <= 8 ) {
		#pack it into struct canmsg_t (see can_proto.h)	
		my $msg = pack("iiLllsC*\@32",0,0,($node & 63) + (($function & 31) << 6),0,0,$length,@_);
		#$function is shifted 6 bit, because it's transmitted first
		# C*\@32   as many unsigned chars yout can find, null padding until 32 (sizeof(canmsg_t)) 
		# $#_      length (size of array)
		$self->sendmsg('S '.$msg."\0");
	}
}

sub filter {
	my $self = shift;
	my $node = shift;
	my $function = shift;
	my $mask = 0;
	my $flags= 0;
	my $id = ($node & 63) + (($function & 31) << 6);
	$mask |= 63 if $node;
	$mask |= ( 31) << 6 if $function;
		
	$self->sendmsg("F $id,$mask,$flags\0");
}

1;

__END__
=head1 METHODS

=over 4

=item filter(id,mask)

We set a filter to receive Messages from the CAN-Bus. If 

=item rtr(id)

Sends an Message with the given id and RTR-Bit set. So we can query our sensors.

=item send(node, function, Byte Array)

Sends an Message with given function to node.


=back

=head1 SEE ALSO

L<IO::Socket::UNIX>

=head1 AUTHOR

Konrad Riedel E<lt>F<k.riedel@gmx.de>E<gt>

=head1 COPYRIGHT

Copyright (c) 2000 Konrad Riedel <k.riedel@gmx.de>. All rights reserved.
This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
