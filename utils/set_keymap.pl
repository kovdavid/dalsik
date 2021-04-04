#!/usr/bin/env perl

use strict;
use warnings;

use Data::Dumper;
use Getopt::Std;
use JSON qw[ decode_json ];

use FindBin qw[ $Bin ];
use lib "$Bin";
use Dalsik;

our ($opt_s, $opt_j);
getopts('s:j:');

my ($max_rows, $max_cols) = (99, 99);
my $serial_fh = \*STDOUT;

my $serial = $opt_s || "/dev/ttyACM0";
if ($serial ne 'STDOUT') {
    $serial_fh = Dalsik::open_serial($serial);
    ($max_rows, $max_cols) = get_serial_num_rows_cols($serial_fh);
}
my $json_file = $opt_j;
unless ($json_file && -f $json_file) {
    die "json_file($json_file) does not exist!\nUsage: $0 [-s /dev/ttyACM0] -j path_to_json_file";
}

open my $json_fh, "<", $json_file or die "Could not open json($json_file): $!";
my $json = do { local $/ = undef; <$json_fh> };
close $json_fh;

$json =~ s/^\xef\xbb\xbf//g;

my $json_data = eval { decode_json($json) };
if ($@) {
    die "Could not decode json($json_file): $@";
}

my $json_keys = $json_data->{keys};
for my $location (sort keys %{ $json_keys }) {
    my ($layer, $row, $col);
    if ($location =~ /L(\d+)R(\d+)C(\d+)$/) {
        $layer = $1;
        $row = $2;
        $col = $3;
    } else {
        die "Invalid location:$location";
    }

    if ($row >= $max_rows || $col >= $max_cols) {
        die "Exceeded max row/col numbers! row:$row max_row:$max_rows col:$col max_col:$max_cols\n";
    }
    my $cmd = Dalsik::set_key_command($layer, $row, $col, $json_keys->{$location});
    if ($serial eq 'STDOUT') {
        print $serial_fh Dalsik::to_hex($cmd)."\n";
    } else {
        print $serial_fh $cmd;
        my $res = <$serial_fh>;
        my $cmd_res = <$serial_fh>;
        print "\t$res";
        print "\t$cmd_res";
    }
}

my $json_tapdances = $json_data->{tapdances};
for my $tapdance (sort keys %{ $json_tapdances }) {
    my @commands = Dalsik::set_tapdance_command($tapdance, $json_tapdances->{$tapdance});
    if ($serial eq 'STDOUT') {
        print $serial_fh Dalsik::to_hex($_)."\n" for @commands;
    } else {
        print $serial_fh $_ for @commands;
    }
}

close $serial_fh;

sub get_serial_num_rows_cols {
    my ($fh) = @_;

    my $num_rows = 0;
    my $num_cols = 0;

    my $cmd = Dalsik::get_cmd('NUM_ROWS');
    unless (print $fh $cmd) {
        die "Error DALSIK-NUM_ROWS: $!";
    }

    my $rows_res = <$fh>;
    my $res = <$fh>;

    if ($res !~ /^CMD_OK/) {
        die "ERROR DALSIK-NUM_ROWS: $rows_res$res";
    }

    if ($rows_res =~ /^ROWS<(\d)>$/) {
        $num_rows = $1;
    } else {
        die "ERROR DALSIK-NUM_ROWS: $rows_res$res";
    }

    $cmd = Dalsik::get_cmd('NUM_COLS');
    unless (print $fh $cmd) {
        die "Error DALSIK-NUM_COLS: $!";
    }

    my $cols_res = <$fh>;
    $res = <$fh>;

    if ($res !~ /^CMD_OK/) {
        die "ERROR DALSIK-NUM_COLS: $cols_res$res";
    }

    if ($cols_res =~ /^COLS<(\d+)>$/) {
        $num_cols = $1;
    } else {
        die "ERROR DALSIK-NUM_COLS: $cols_res$res";
    }

    return ($num_rows, $num_cols);
}
