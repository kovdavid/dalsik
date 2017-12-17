#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];
use Getopt::Long qw( GetOptions );

use lib "$Bin";

use Dalsik;

my $serial = '/dev/ttyACM0';
GetOptions(
    "serial=s" => \$serial,
);

my $fh = Dalsik::open_serial($serial);

my $cmd = Dalsik::get_cmd("PING");

for my $iteration (1..32) {
    unless (print $fh $cmd) {
        die "Iteration $iteration: Could not send PING: $!";
    }
    my $res = <$fh>;
    if ($res =~ /^PONG/) {
        print "cmd_buffer cleared; exiting\n";
        exit;
    }

    print $fh "\x00";
}

close $fh;
