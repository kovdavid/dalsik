package Dalsik;

use strict;
use warnings;
use FindBin qw[ $Bin ];

our $keycode_str_to_dec = {};
our $dec_to_keycode_str = {};

BEGIN {
    my $file = "$Bin/keycode_str_to_dec.txt";
    open my $fh, "<", $file or die "Could not open $file: $!";
    while (my $line = <$fh>) {
        my ($str, $dec) = split(/\s+/, $line);
        $keycode_str_to_dec->{$str} = $dec;
        $dec_to_keycode_str->{$dec} = $str;
    }
    close $fh;
}

sub open_serial {
    my ($serial) = @_;

    `stty -F $serial 9600 raw -echo`;
    open my $fh, "+<:raw", $serial or die "Could not open $serial";

    return $fh;
}

sub get_cmd {
    my ($cmd, @args) = @_;

    if ($cmd eq 'GET_KEY') {
        return "DALSIK-\x01".chr($args[0]).chr($args[1]).chr($args[2])."\x00\x00";
    } elsif ($cmd eq 'GET_KEYMAP') {
        return "DALSIK-\x02".join("", ("\x00")x5);
    } elsif ($cmd eq 'SET_KEY') {
        return "DALSIK-\x03".chr($args[0]).chr($args[1]).chr($args[2]).$args[3].$args[4];
    } elsif ($cmd eq 'CLEAR_KEYMAP') {
        return "DALSIK-\x04".join("", ("\x00")x5);
    } elsif ($cmd eq 'NUM_ROWS') {
        return "DALSIK-\x05".join("", ("\x00")x5);
    } elsif ($cmd eq 'NUM_COLS') {
        return "DALSIK-\x06".join("", ("\x00")x5);
    } elsif ($cmd eq 'PING') {
        return "DALSIK-\x07".join("", ("\x00")x5);
    } elsif ($cmd eq 'GET_LAYER') {
        return "DALSIK-\x08".join("", ("\x00")x5);
    }
}

sub to_hex {
    my ($str) = @_;
    my $hex = join("-", map { unpack "H*", $_ } split("", $str));
    return $hex;
}

sub type_and_key_to_str {
    my ($type, $key) = @_;

    if ($type eq 'KEY_UNSET') {
        return 'KC_NO';
    } elsif ($type eq 'KEY_NORMAL') {
        return $dec_to_keycode_str->{$key} // '';
    } elsif ($type eq 'KEY_DUAL_LCTRL') {
        return "MT(LCTL,". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_DUAL_RCTRL') {
        return "MT(RCTL,". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_DUAL_LSHIFT') {
        return "MT(LSFT,". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_DUAL_RSHIFT') {
        return "MT(RSFT,". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_DUAL_LGUI') {
        return "MT(LGUI,". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_DUAL_RGUI') {
        return "MT(RGUI,". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_DUAL_LALT') {
        return "MT(LALT,". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_DUAL_RALT') {
        return "MT(RALT,". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_LAYER_PRESS') {
        return "MO($key)";
    } elsif ($type eq 'KEY_LAYER_TOGGLE') {
        return "TG($key)";
    } elsif ($type eq 'KEY_LAYER_HOLD_OR_TOGGLE') {
        return "TT($key)";
    } elsif ($type eq 'KEY_WITH_MOD_LCTRL') {
        return "LCTL(". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_WITH_MOD_RCTRL') {
        return "RCTL(". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_WITH_MOD_LSHIFT') {
        return "LSFT(". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_WITH_MOD_RSHIFT') {
        return "RSFT(". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_WITH_MOD_LGUI') {
        return "LGUI(". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_WITH_MOD_RGUI') {
        return "RGUI(". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_WITH_MOD_LALT') {
        return "LALT(". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_WITH_MOD_RALT') {
        return "RALT(". type_and_key_to_str('KEY_NORMAL', $key) .")";
    } elsif ($type eq 'KEY_TRANSPARENT') {
        return 'KC_TRNS';
    }

    return "";
}

sub str_to_type_and_key {
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
            my $dec = $keycode_str_to_dec->{$fields->[1]->{id}};
            if (defined $dec) {
                return "\x02".chr($dec);
            }
        } elsif ($fields->[0] == 2) { # "SHIFT"
            my $dec = $keycode_str_to_dec->{$fields->[1]->{id}};
            if (defined $dec) {
                return "\x04".chr($dec);
            }
        } elsif ($fields->[0] == 4) { # "ALT"
            my $dec = $keycode_str_to_dec->{$fields->[1]->{id}};
            if (defined $dec) {
                return "\x08".chr($dec);
            }
        } elsif ($fields->[0] == 8) { # "GUI"
            my $dec = $keycode_str_to_dec->{$fields->[1]->{id}};
            if (defined $dec) {
                return "\x06".chr($dec);
            }
        }
    } elsif ($id eq 'LCTL()') {
        my $dec = $keycode_str_to_dec->{$fields->[0]->{id}};
        if (defined $dec) {
            return "\x0D".chr($dec);
        }
    } elsif ($id eq 'RCTL()') {
        my $dec = $keycode_str_to_dec->{$fields->[0]->{id}};
        if (defined $dec) {
            return "\x0E".chr($dec);
        }
    } elsif ($id eq 'LSFT()') {
        my $dec = $keycode_str_to_dec->{$fields->[0]->{id}};
        if (defined $dec) {
            return "\x0F".chr($dec);
        }
    } elsif ($id eq 'RSFT()') {
        my $dec = $keycode_str_to_dec->{$fields->[0]->{id}};
        if (defined $dec) {
            return "\x10".chr($dec);
        }
    } elsif ($id eq 'LGUI()') {
        my $dec = $keycode_str_to_dec->{$fields->[0]->{id}};
        if (defined $dec) {
            return "\x11".chr($dec);
        }
    } elsif ($id eq 'RGUI()') {
        my $dec = $keycode_str_to_dec->{$fields->[0]->{id}};
        if (defined $dec) {
            return "\x12".chr($dec);
        }
    } elsif ($id eq 'LALT()') {
        my $dec = $keycode_str_to_dec->{$fields->[0]->{id}};
        if (defined $dec) {
            return "\x13".chr($dec);
        }
    } elsif ($id eq 'RALT()') {
        my $dec = $keycode_str_to_dec->{$fields->[0]->{id}};
        if (defined $dec) {
            return "\x14".chr($dec);
        }
    } elsif (exists ($keycode_str_to_dec->{$id})) {
        my $dec = $keycode_str_to_dec->{$id};
        return "\x01".chr($dec);
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

1;
