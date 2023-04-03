#!/usr/bin/env perl

use strict;
use warnings;
use Getopt::Long qw( GetOptions );

my $serial = '/dev/ttyACM0';
my $clear_cmd_buffer = 0;
GetOptions({
    "serial=s" => \$serial,
    "clear=c" => \$clear_cmd_buffer,
});

my $fh = open_serial($serial);

my $cmd = "DALSIK-\x01";

if ($clear_cmd_buffer) {
    for my $iteration (1..32) {
        unless (print $fh $cmd) {
            die "Iteration $iteration: Could not send command: $!";
        }
        my $res = <$fh>;
        if (length($res || "") == 7) {
            print "cmd_buffer cleared; exiting\n";
            exit;
        }

        print $fh "\x00";
    }
} else {
    unless (print $fh $cmd) {
        die "Could not send command: $!";
    }

    my $res = <$fh>;
    my $cmd_res = <$fh>;

    print "$res";
}

close $fh;

sub open_serial {
    my ($serial) = @_;

    `stty -F $serial 9600 raw -echo`;
    open my $fh, "+<:raw", $serial or die "Could not open $serial";

    return $fh;
}
