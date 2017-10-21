#!/usr/bin/env perl

use strict;
use warnings;

use Data::Dumper;
use Getopt::Std;
use JSON qw[ decode_json ];

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

my $keycodes = {
    "KC_NO"             => "\x00",
    "KC_ROLL_OVER"      => "\x01",
    "KC_POST_FAIL"      => "\x02",
    "KC_UNDEFINED"      => "\x03",
    "KC_A"              => "\x04",
    "KC_B"              => "\x05",
    "KC_C"              => "\x06",
    "KC_D"              => "\x07",
    "KC_E"              => "\x08",
    "KC_F"              => "\x09",
    "KC_G"              => "\x0a",
    "KC_H"              => "\x0b",
    "KC_I"              => "\x0c",
    "KC_J"              => "\x0d",
    "KC_K"              => "\x0e",
    "KC_L"              => "\x0f",
    "KC_M"              => "\x10",
    "KC_N"              => "\x11",
    "KC_O"              => "\x12",
    "KC_P"              => "\x13",
    "KC_Q"              => "\x14",
    "KC_R"              => "\x15",
    "KC_S"              => "\x16",
    "KC_T"              => "\x17",
    "KC_U"              => "\x18",
    "KC_V"              => "\x19",
    "KC_W"              => "\x1a",
    "KC_X"              => "\x1b",
    "KC_Y"              => "\x1c",
    "KC_Z"              => "\x1d",
    "KC_1"              => "\x1e",
    "KC_2"              => "\x1f",
    "KC_3"              => "\x20",
    "KC_4"              => "\x21",
    "KC_5"              => "\x22",
    "KC_6"              => "\x23",
    "KC_7"              => "\x24",
    "KC_8"              => "\x25",
    "KC_9"              => "\x26",
    "KC_0"              => "\x27",
    "KC_ENTER"          => "\x28",
    "KC_ESCAPE"         => "\x29",
    "KC_BSPACE"         => "\x2a",
    "KC_TAB"            => "\x2b",
    "KC_SPACE"          => "\x2c",
    "KC_MINUS"          => "\x2d",
    "KC_EQUAL"          => "\x2e",
    "KC_LBRACKET"       => "\x2f",
    "KC_RBRACKET"       => "\x30",
    "KC_BSLASH"         => "\x31",
    "KC_NONUS_HASH"     => "\x32",
    "KC_SCOLON"         => "\x33",
    "KC_QUOTE"          => "\x34",
    "KC_GRAVE"          => "\x35",
    "KC_COMMA"          => "\x36",
    "KC_DOT"            => "\x37",
    "KC_SLASH"          => "\x38",
    "KC_CAPSLOCK"       => "\x39",
    "KC_F1"             => "\x3a",
    "KC_F2"             => "\x3b",
    "KC_F3"             => "\x3c",
    "KC_F4"             => "\x3d",
    "KC_F5"             => "\x3e",
    "KC_F6"             => "\x3f",
    "KC_F7"             => "\x40",
    "KC_F8"             => "\x41",
    "KC_F9"             => "\x42",
    "KC_F10"            => "\x43",
    "KC_F11"            => "\x44",
    "KC_F12"            => "\x45",
    "KC_PSCREEN"        => "\x46",
    "KC_SCROLLLOCK"     => "\x47",
    "KC_PAUSE"          => "\x48",
    "KC_INSERT"         => "\x49",
    "KC_HOME"           => "\x4a",
    "KC_PGUP"           => "\x4b",
    "KC_DELETE"         => "\x4c",
    "KC_END"            => "\x4d",
    "KC_PGDOWN"         => "\x4e",
    "KC_RIGHT"          => "\x4f",
    "KC_LEFT"           => "\x50",
    "KC_DOWN"           => "\x51",
    "KC_UP"             => "\x52",
    "KC_NUMLOCK"        => "\x53",
    "KC_KP_SLASH"       => "\x54",
    "KC_KP_ASTERISK"    => "\x55",
    "KC_KP_MINUS"       => "\x56",
    "KC_KP_PLUS"        => "\x57",
    "KC_KP_ENTER"       => "\x58",
    "KC_KP_1"           => "\x59",
    "KC_KP_2"           => "\x5a",
    "KC_KP_3"           => "\x5b",
    "KC_KP_4"           => "\x5c",
    "KC_KP_5"           => "\x5d",
    "KC_KP_6"           => "\x5e",
    "KC_KP_7"           => "\x5f",
    "KC_KP_8"           => "\x60",
    "KC_KP_9"           => "\x61",
    "KC_KP_0"           => "\x62",
    "KC_KP_DOT"         => "\x63",
    "KC_NONUS_BSLASH"   => "\x64",
    "KC_APPLICATION"    => "\x65",
    "KC_POWER"          => "\x66",
    "KC_KP_EQUAL"       => "\x67",
    "KC_F13"            => "\x68",
    "KC_F14"            => "\x69",
    "KC_F15"            => "\x6a",
    "KC_F16"            => "\x6b",
    "KC_F17"            => "\x6c",
    "KC_F18"            => "\x6d",
    "KC_F19"            => "\x6e",
    "KC_F20"            => "\x6f",
    "KC_F21"            => "\x70",
    "KC_F22"            => "\x71",
    "KC_F23"            => "\x72",
    "KC_F24"            => "\x73",
    "KC_EXECUTE"        => "\x74",
    "KC_HELP"           => "\x75",
    "KC_MENU"           => "\x76",
    "KC_SELECT"         => "\x77",
    "KC_STOP"           => "\x78",
    "KC_AGAIN"          => "\x79",
    "KC_UNDO"           => "\x7a",
    "KC_CUT"            => "\x7b",
    "KC_COPY"           => "\x7c",
    "KC_PASTE"          => "\x7d",
    "KC_FIND"           => "\x7e",
    "KC__MUTE"          => "\x7f",
    "KC__VOLUP"         => "\x80",
    "KC__VOLDOWN"       => "\x81",
    "KC_LOCKING_CAPS"   => "\x82",
    "KC_LOCKING_NUM"    => "\x83",
    "KC_LOCKING_SCROLL" => "\x84",
    "KC_KP_COMMA"       => "\x85",
    "KC_KP_EQUAL_AS400" => "\x86",
    "KC_INT1"           => "\x87",
    "KC_INT2"           => "\x88",
    "KC_INT3"           => "\x89",
    "KC_INT4"           => "\x8a",
    "KC_INT5"           => "\x8b",
    "KC_INT6"           => "\x8c",
    "KC_INT7"           => "\x8d",
    "KC_INT8"           => "\x8e",
    "KC_INT9"           => "\x8f",
    "KC_LANG1"          => "\x90",
    "KC_LANG2"          => "\x91",
    "KC_LANG3"          => "\x92",
    "KC_LANG4"          => "\x93",
    "KC_LANG5"          => "\x94",
    "KC_LANG6"          => "\x95",
    "KC_LANG7"          => "\x96",
    "KC_LANG8"          => "\x97",
    "KC_LANG9"          => "\x98",
    "KC_ALT_ERASE"      => "\x99",
    "KC_SYSREQ"         => "\x9a",
    "KC_CANCEL"         => "\x9b",
    "KC_CLEAR"          => "\x9c",
    "KC_PRIOR"          => "\x9d",
    "KC_RETURN"         => "\x9e",
    "KC_SEPARATOR"      => "\x9f",
    "KC_OUT"            => "\xa0",
    "KC_OPER"           => "\xa1",
    "KC_CLEAR_AGAIN"    => "\xa2",
    "KC_CRSEL"          => "\xa3",
    "KC_EXSEL"          => "\xa4",
    "KC_LCTRL"          => "\xE0",
    "KC_LSHIFT"         => "\xE1",
    "KC_LALT"           => "\xE2",
    "KC_LGUI"           => "\xE3",
    "KC_RCTRL"          => "\xE4",
    "KC_RSHIFT"         => "\xE5",
    "KC_RALT"           => "\xE6",
    "KC_RGUI"           => "\xE7",
    "KC_LCTL"           => "\xE0",
    "KC_RCTL"           => "\xE4",
    "KC_LSFT"           => "\xE1",
    "KC_RSFT"           => "\xE5",
    "KC_ESC"            => "\x29",
    "KC_BSPC"           => "\x2a",
    "KC_ENT"            => "\x28",
    "KC_DEL"            => "\x4c",
    "KC_INS"            => "\x49",
    "KC_CAPS"           => "\x39",
    "KC_CLCK"           => "\x39",
    "KC_RGHT"           => "\x4f",
    "KC_PGDN"           => "\x4e",
    "KC_PSCR"           => "\x46",
    "KC_SLCK"           => "\x47",
    "KC_PAUS"           => "\x48",
    "KC_BRK"            => "\x48",
    "KC_NLCK"           => "\x53",
    "KC_SPC"            => "\x2c",
    "KC_MINS"           => "\x2d",
    "KC_EQL"            => "\x2e",
    "KC_GRV"            => "\x35",
    "KC_RBRC"           => "\x30",
    "KC_LBRC"           => "\x2f",
    "KC_COMM"           => "\x36",
    "KC_BSLS"           => "\x31",
    "KC_SLSH"           => "\x38",
    "KC_SCLN"           => "\x33",
    "KC_QUOT"           => "\x34",
    "KC_APP"            => "\x65",
    "KC_NUHS"           => "\x32",
    "KC_NUBS"           => "\x64",
    "KC_LCAP"           => "\x82",
    "KC_LNUM"           => "\x83",
    "KC_LSCR"           => "\x84",
    "KC_ERAS"           => "\x99",
    "KC_CLR"            => "\x9c",
    "KC_P1"             => "\x59",
    "KC_P2"             => "\x5a",
    "KC_P3"             => "\x5b",
    "KC_P4"             => "\x5c",
    "KC_P5"             => "\x5d",
    "KC_P6"             => "\x5e",
    "KC_P7"             => "\x5f",
    "KC_P8"             => "\x60",
    "KC_P9"             => "\x61",
    "KC_P0"             => "\x62",
    "KC_PDOT"           => "\x63",
    "KC_PCMM"           => "\x85",
    "KC_PSLS"           => "\x54",
    "KC_PAST"           => "\x55",
    "KC_PMNS"           => "\x56",
    "KC_PPLS"           => "\x57",
    "KC_PEQL"           => "\x67",
    "KC_PENT"           => "\x58",
    "KC_EXEC"           => "\x74",
    "KC_SLCT"           => "\x77",
    "KC_AGIN"           => "\x79",
    "KC_PSTE"           => "\x7d",
};

`stty -F $serial 9600 raw -echo`;
open my $fh, "+<:raw", $serial or die "Could not open serial($serial): $!";

# open my $fh, ">:raw", "/tmp/keymap" or die "Could not open serial($serial): $!";

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

        # TODO read back response

        $layer++;
        if ($layer == 6) {
            last;
        }
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
    } elsif ($id eq 'TO()') {
        return "\x0A".chr($fields->[0]);
    } elsif ($id eq 'TG()') {
        return "\x0B".chr($fields->[0]);
    } elsif ($id eq 'TT()') {
        return "\x0C".chr($fields->[0]);
    } elsif ($id eq 'MT()') {
        if ($fields->[0] == 1) { # CTRL
            if (exists $keycodes->{$fields->[1]->{id}}) {
                return "\x02$keycodes->{$fields->[1]->{id}}";
            }
        } elsif ($fields->[0] == 2) { # "SHIFT"
            if (exists $keycodes->{$fields->[1]->{id}}) {
                return "\x04$keycodes->{$fields->[1]->{id}}";
            }
        } elsif ($fields->[0] == 4) { # "ALT"
            if (exists $keycodes->{$fields->[1]->{id}}) {
                return "\x08$keycodes->{$fields->[1]->{id}}";
            }
        } elsif ($fields->[0] == 8) { # "GUI"
            if (exists $keycodes->{$fields->[1]->{id}}) {
                return "\x06$keycodes->{$fields->[1]->{id}}";
            }
        }
    } elsif ($id eq 'LCTL()') {
        if (exists $keycodes->{$fields->[0]->{id}}) {
            return "\x0D$keycodes->{$fields->[0]->{id}}";
        }
    } elsif ($id eq 'RCTL()') {
        if (exists $keycodes->{$fields->[0]->{id}}) {
            return "\x0E$keycodes->{$fields->[0]->{id}}";
        }
    } elsif ($id eq 'LSFT()') {
        if (exists $keycodes->{$fields->[0]->{id}}) {
            return "\x0F$keycodes->{$fields->[0]->{id}}";
        }
    } elsif ($id eq 'RSFT()') {
        if (exists $keycodes->{$fields->[0]->{id}}) {
            return "\x10$keycodes->{$fields->[0]->{id}}";
        }
    } elsif ($id eq 'LGUI()') {
        if (exists $keycodes->{$fields->[0]->{id}}) {
            return "\x11$keycodes->{$fields->[0]->{id}}";
        }
    } elsif ($id eq 'RGUI()') {
        if (exists $keycodes->{$fields->[0]->{id}}) {
            return "\x12$keycodes->{$fields->[0]->{id}}";
        }
    } elsif ($id eq 'LALT()') {
        if (exists $keycodes->{$fields->[0]->{id}}) {
            return "\x13$keycodes->{$fields->[0]->{id}}";
        }
    } elsif ($id eq 'RALT()') {
        if (exists $keycodes->{$fields->[0]->{id}}) {
            return "\x14$keycodes->{$fields->[0]->{id}}";
        }
    } elsif (exists ($keycodes->{$id})) {
        return "\x01$keycodes->{$id}";
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
