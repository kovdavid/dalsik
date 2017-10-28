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

my $serial = $opt_s || "/dev/ttyACM0";
my $json_file = $opt_j;
unless ($json_file && -f $json_file) {
    die "Usage: $0 [-s /dev/ttyACM0] -j path_to_json_file";
}

open my $json_fh, "<", $json_file or die "Could not open json($json_file): $!";

my $json = do { local $/ = undef; <$json_fh> };
close $json_fh;

$json =~ s/^\xef\xbb\xbf//g;

my $json_data = eval { decode_json($json) };
if ($@) {
    die "Could not decode json($json_file): $@";
}

my $fh = Dalsik::open_serial($serial);

my ($num_rows, $num_cols) = get_serial_num_rows_cols($fh);

my $keys = $json_data->{keyboard}->{keys};

my ($json_num_rows, $json_num_cols) = get_json_num_rows_cols($keys);

if ($json_num_rows != $num_rows || $json_num_cols != $num_cols) {
    die "Wrong JSON config! serial[num_rows:$num_rows num_cols:$num_cols] json[num_rows:$json_num_rows num_cols:$json_num_cols]";
}

for my $key (@{ $keys }) {
    my $row = $key->{row};
    my $col = $key->{col};

    my $layer = 0;
    for my $keycode (@{ $key->{keycodes} }) {
        my $type_and_key = Dalsik::get_type_and_key($keycode);

        unless ($type_and_key) {
            die "Could not get type and key for layer:$layer row:$row col:$col keycode:".Dumper($keycode);
        }

        my ($type, $key) = split("", $type_and_key);

        my $cmd = Dalsik::get_cmd('SET_KEY', $layer, $row, $col, $type, $key);
        # my $hex = Dalsik::to_hex($cmd);

        unless (print $fh "$cmd") {
            die "Could not set key for layer:$layer row:$row col:$col type_and_key:$type_and_key";
        }

        my $res = <$fh>;
        if ($res !~ /^CMD_OK/) {
            die "Could not set key for layer:$layer row:$row col:$col type_and_key:$type_and_key $res";
        }

        $layer++;
        if ($layer == 6) {
            last;
        }
    }
}

close $fh;

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

    if ($cols_res =~ /^COLS<(\d)>$/) {
        $num_cols = $1;
    } else {
        die "ERROR DALSIK-NUM_COLS: $cols_res$res";
    }

    return ($num_rows, $num_cols);
}

sub get_json_num_rows_cols {
    my ($keys) = @_;

    my $max_col = 0;
    my $max_row = 0;

    for my $key (@{ $keys }) {
        my $row = $key->{row}+1;
        my $col = $key->{col}+1;

        if ($row > $max_row) {
            $max_row = $row;
        }
        if ($col > $max_col) {
            $max_col = $col;
        }
    }

    return ($max_row, $max_col);
}
