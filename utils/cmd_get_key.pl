#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];

use lib "$Bin";

use Dalsik;

my $serial = shift || "/dev/ttyACM0";

my $fh = Dalsik::open_serial($serial);

my $layer = 0;
my $row = 0;
my $col = 1;

print "Sending:\n\tGET_KEY\n";
my $cmd = Dalsik::get_cmd('GET_KEY', $layer, $row, $col);
unless (print $fh $cmd) {
    die "Could not send GET_KEY: $!";
}

my $res = <$fh>;
my $cmd_res = <$fh>;

print "Received:\n";
print "\t$res";
print "\t$cmd_res";

close $fh;
