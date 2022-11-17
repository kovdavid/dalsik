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

# print "Sending:\n\tGET_LAYER_INDEX\n";
my $cmd = Dalsik::get_cmd('GET_LAYER_INDEX');
unless (print $fh $cmd) {
    die "Could not send GET_LAYER_INDEX: $!";
}

# print "Received:\n";
while (my $line = <$fh>) {
    if ($line =~ /^CMD/) {
        last;
    }

    print "$line";
}

close $fh;
