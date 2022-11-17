package Dalsik;

use v5.10;
use strict;
use warnings;
use FindBin qw[ $Bin ];

our $key_types_array = [];
our $type_to_byte = {};
our $byte_to_alias = {};
our $key_to_byte = {};
our $byte_to_key = {};
our $aliases = {};

# my $file = "$Bin/../lib/key_definitions/key_definitions.h";
# open my $fh, "<", $file or die "Could not open $file: $!";
# while (my $line = <$fh>) {
    # chomp $line;

    # if ($line =~ m/^#define KEY_/) {
        # my (undef, $key_type, $byte_str, undef, $alias, undef) = split(/\s+/, $line);
        # my $byte = hex($byte_str);

        # push @{ $key_types_array }, $alias;
        # $type_to_byte->{$key_type} = $byte;
        # $type_to_byte->{$alias} = $byte;
        # $byte_to_alias->{$byte} = $alias;
    # } elsif ($line =~ m/^#define KC_(\S+)\s+(\S+)/) {
        # my $key = $1;
        # my $byte = hex($2);
        # $key_to_byte->{$key} = $byte;
        # $byte_to_key->{$byte} = $key;
    # } elsif ($line =~ m/^#define ALIAS_(\S+)\s+(\S+)/) {
        # my $name = $1;
        # my $alias = $2;
        # $aliases->{$name} = $alias;
    # }
# }
# close $fh;

sub set_key_command {
    my ($layer, $row, $col, $key_str) = @_;

    $key_str = $aliases->{$key_str} || $key_str;

    my ($type, $key) = _keystr_to_type_and_arg($key_str);
    return get_cmd('SET_KEY', $layer, $row, $col, $type, $key);
}

sub set_tapdance_command {
    my ($tapdance, $tap_keys) = @_;

    if ($tapdance =~ m/^T(\d+)$/) {
        my $tapdance_index = $1;
        my @result;

        for my $i (0..2) {
            if (!defined($tap_keys->[$i])) {
                next;
            }
            my ($type, $key) = _keystr_to_type_and_arg($tap_keys->[$i]);
            push @result, get_cmd('SET_TAPDANCE_KEY', $tapdance_index, $i+1, $type, $key);
        }

        return @result;
    }

    die "Invalid tapdance $tapdance";
}

sub _keystr_to_type_and_arg {
    my ($key_str) = @_;

    for my $type (@{ $key_types_array }) {
        if ($key_str !~ m/^\Q$type\E/) {
            next;
        }
        if ($type =~ m/\(|\,$/) {
            if ($key_str =~ m/^\Q$type\E(.*?)\)$/) {
                my $arg = $1;
                if (defined($key_to_byte->{$arg})) {
                    return ($type_to_byte->{$type}, $key_to_byte->{$arg});
                } elsif ($arg =~ m/^0x[0-9A-Fa-f]+$/ || $arg =~ m/^\d+$/) {
                    return ($type_to_byte->{$type}, hex($arg));
                } elsif ($arg =~ m/^T(\d+)$/) {
                    return ($type_to_byte->{$type}, hex($1));
                } else {
                    die "Could not parse $key_str - invalid argument\n";
                }
            } else {
                die "Could not parse $key_str - invalid argument\n";
            }
        } else {
            return ($type_to_byte->{$type}, 0);
        }
    }

    die "Could not parse $key_str";
}

sub type_and_key_to_str {
    my ($type, $key) = @_;

    my $type_alias = $byte_to_alias->{hex($type)};
    if (!defined($type_alias)) {
        die "Could not get type alias for type $type";
    }

    my $key_byte = hex("0x$key");
    my $key_str = $byte_to_key->{$key_byte};

    if (
        $type_alias eq 'LP('
        || $type_alias eq 'LT('
        || $type_alias eq 'LHT('
        || $type_alias eq 'TD('
        || $type_alias eq 'KC_M0('
        || $type_alias eq 'KC_M1('
        || $type_alias eq 'KC_M2('
    ) {
        return "${type_alias}0x$key)";
    } elsif ($type_alias =~ m/\(|\,$/) {
        return "${type_alias}$key_str)";
    } else {
        return "$type_alias";
    }

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
    } elsif ($cmd eq 'NUM_ROWS') {
        return "DALSIK-\x05".join("", ("\x00")x5);
    } elsif ($cmd eq 'NUM_COLS') {
        return "DALSIK-\x06".join("", ("\x00")x5);
    } elsif ($cmd eq 'PING') {
        return "DALSIK-\x07".join("", ("\x00")x5);
    } elsif ($cmd eq 'GET_LAYER') {
        return "DALSIK-\x08".join("", ("\x00")x5);
    } elsif ($cmd eq 'GET_TAPDANCE_KEY') {
        return "DALSIK-\x0B".chr($args[0]).chr($args[1])."\x00\x00\x00";
    } elsif ($cmd eq 'GET_TAPDANCE_KEYMAP') {
        return "DALSIK-\x0C".join("", ("\x00")x5);
    } elsif ($cmd eq 'GET_FULL_KEYMAP') {
        return "DALSIK-\x0E".join("", ("\x00")x5);
    } elsif ($cmd eq 'GET_KEYBOARD_SIDE') {
        return "DALSIK-\x0F".join("", ("\x00")x5);
    } elsif ($cmd eq 'SET_KEYBOARD_SIDE') {
        return "DALSIK-\x10".$args[0]."\x00\x00\x00\x00";
    } elsif ($cmd eq 'GET_LAYER_INDEX') {
        return "DALSIK-\x11".join("", ("\x00")x5);
    }
}

sub to_hex {
    my ($str) = @_;
    my $hex = join("-", map { unpack "H*", $_ } split("", $str));
    return $hex;
}

1;
