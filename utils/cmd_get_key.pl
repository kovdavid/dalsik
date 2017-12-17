#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];
use Getopt::Long qw( GetOptions );

use lib "$Bin";

use Dalsik;

my $serial = '/dev/ttyACM0';
my $layer = 0;
my $row = 0;
my $col = 0;

GetOptions(
    "serial=s" => \$serial,
    "layer=i"  => \$layer,
    "row=i"    => \$row,
    "col=i"    => \$col,
);

my $fh = Dalsik::open_serial($serial);

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
