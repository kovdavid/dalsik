#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];
use Getopt::Long qw( GetOptions );

use lib "$Bin";

use Dalsik;

my $serial = '/dev/ttyACM0';
my $index = 0;
GetOptions(
    "serial=s" => \$serial,
    "index=i" => \$index,
);

my $fh = Dalsik::open_serial($serial);

for my $tap (1..3) {
    print "Sending:\n\tGET_TAPDANCE_KEY tap:$tap\n";
    my $cmd = Dalsik::get_cmd('GET_TAPDANCE_KEY', $index, $tap);
    unless (print $fh $cmd) {
        die "Could not send GET_TAPDANCE_KEY: $!";
    }

    my $res = <$fh>;
    my $cmd_res = <$fh>;

    print "Received:\n";
    print "\t$res";
    print "\t$cmd_res";
}

close $fh;
