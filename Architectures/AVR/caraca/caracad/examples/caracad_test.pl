#!/usr/bin/perl -w
use strict;
use Socket;

#Simple test for interfacing with caracad

#open connection:
socket(SOCK, PF_UNIX, SOCK_STREAM, 0)     || die "socket: $!";
my $NAME = "/var/tmp/$$";
socket(SOCK,PF_UNIX,SOCK_STREAM,0)        || die "socket: $!";
unlink($NAME);
bind  (SOCK, sockaddr_un($NAME))          || die "bind: $!";
chmod 0700, $NAME;
connect(SOCK, sockaddr_un('/tmp/caracad'))|| die "connect: $!";

#set filter
send(SOCK,"filter 5.0.0 7.0.0 0\0",0);
#get everything with type 5 (temp messages) in ascii-Format

my $answer = '';
recv(SOCK, $answer, 84, 0)        || die "recv: $!";
print "$answer\n";


#write CAN-msg with RTR bit set - to get sensor value
send(SOCK,"RTR 5.1.0\0",0);
recv(SOCK, $answer, 84, 0)        || die "recv: $!";
print "$answer\n";

#get CAN-msg with sensor value
recv(SOCK, $answer, 84, 0)        || die "recv: $!";
print "$answer\n";
#blocks if sensor don't answer

