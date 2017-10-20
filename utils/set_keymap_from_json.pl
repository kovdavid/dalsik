#!/usr/bin/env perl

use strict;
use warnings;

use Data::Dumper;
use Getopt::Std;
use JSON qw[ decode_json ];

our $opt_s, $opt_j;
getopts('s:j:');

my $serial = $opt_s || "/dev/ttyACM0";
my $json_file = $opt_j;
unless ($json_file && -f $json_file) {
    die "Usage: $0 [-s /dev/ttyACM0] -j path_to_json_file";
}

open my $json_fh, "<", $json_file or die "Could not open json($json_file): $!";
my $json = do { local $/ = undef; <$json_fh> };
close $json_fh;

my $json_data = eval { decode_json($json) };
if ($@) {
    die "Could not decode json($json_file): $@";
}

`stty -F $serial 9600 raw -echo`;

open my $fh, "+<:raw", $serial or die "Could not open serial($serial): $!";

my $cmd_prefix = "DALSIK-SET_KEY";

my $keys = $json_data->{keyboard}->{keys};
for my $key (@{ $keys }) {
    my $row = $key->{row};
    my $col = $key->{col};

    my $layer = 0;
    for my $keycode (@{ $key->{keycodes} }) {
        my $type_and_key = get_type_and_key($keycode);

        unless ($type_and_key) {
            die "Could not get type and key for layer:$layer row:$row col:$col keycode:".Dumper($keycode);
        }

        my $cmd = "${cmd_prefix}".chr($layer).chr($row).chr($col).$type_and_key;
        unless (print $fh "$cmd\n") {
            die "Could not set key for layer:$layer row:$row col:$col type_and_key:$type_and_key";
        }

        $layer++;
    }
}

close $fh;

sub get_type_and_key {
    my ($keycode) = @_;

    my $id = $keycode->{id};
    my $fields = $keycode->{fields};

    if ($id eq 'KC_TRNS') {
        return "\xFF\x00";
    } elsif ($id eq 'KC_NO') {
        return "\x00\x00";
    } elsif ($id eq 'MO()') {
        return "\x0A".chr($fields->[0]);
    } elsif ($id eq 'TG()') {
        return "\x0B".chr($fields->[0]);
    } elsif ($id eq 'TT()') {
        return "\x0C".chr($fields->[0]);
    } elsif ($id eq 'MT()') {
        if ($fields->[0] == 1) { # CTRL
            # $fields->[1]->{id}
            return "\x02";
        } elsif ($fields->[0] == 2) { # "SHIFT"
            return "\x04";
        } elsif ($fields->[0] == 4) { # "ALT"
            return "\x08";
        } elsif ($fields->[0] == 8) { # "GUI"
            return "\x06";
        }
    } elsif ($id eq 'LCTL()') {
        # $fields->[1]->{id}
        return "\x0D";
    } elsif ($id eq 'RCTL()') {
        return "\x0E";
    } elsif ($id eq 'LSFT()') {
        return "\x0F";
    } elsif ($id eq 'RSFT()') {
        return "\x10";
    } elsif ($id eq 'LGUI()') {
        return "\x11";
    } elsif ($id eq 'RGUI()') {
        return "\x12";
    } elsif ($id eq 'LALT()') {
        return "\x13";
    } elsif ($id eq 'RALT()') {
        return "\x14";
    } elsif ($id =~ /^KC_/) {

    }

    return;
}

# KC_TAB, KC_U -keycode - KEY_NORMAL
# KC_TRNS - KEY_TRANSPARENT
# KC_NO - KEY_UNSET
# MO(1) - KEY_LAYER_PRESS
# TG(1) - KEY_LAYER_TOGGLE
# TT(1) - KEY_LAYER_TAP_TOGGLE

# MT(LCTL, kc) - KEY_DUAL_LCTRL
# MT(RCTL, kc) - KEY_DUAL_RCTRL
# MT(LSFT, kc) - KEY_DUAL_LSHIFT
# MT(RSFT, kc) - KEY_DUAL_RSHIFT
# MT(LGUI, kc) - KEY_DUAL_LGUI
# MT(RGUI, kc) - KEY_DUAL_RGUI
# MT(LALT, kc) - KEY_DUAL_LALT
# MT(RALT, kc) - KEY_DUAL_RALT

# LCTL(kc) - KEY_WITH_MOD_LCTRL
# RCTL(kc) - KEY_WITH_MOD_RCTRL
# LSFT(kc) - KEY_WITH_MOD_LSHIFT
# RSFT(kc) - KEY_WITH_MOD_RSHIFT
# LGUI(kc) - KEY_WITH_MOD_LGUI
# RGUI(kc) - KEY_WITH_MOD_RGUI
# LALT(kc) - KEY_WITH_MOD_LALT
# RALT(kc) - KEY_WITH_MOD_RALT
