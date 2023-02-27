#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];
use Getopt::Long qw( GetOptions );

use lib "$Bin";

use Dalsik;

my $serial = '/dev/ttyACM0';
GetOptions("serial=s" => \$serial);

my $fh = Dalsik::open_serial($serial);

my $cmd = Dalsik::get_cmd('GET_CHECKSUM');
unless (print $fh $cmd) {
    die "Could not send PING: $!";
}

my $res = <$fh>;
my $cmd_res = <$fh>;

print "$res";

close $fh;
