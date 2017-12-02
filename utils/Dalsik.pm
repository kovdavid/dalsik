package Dalsik;

use v5.10;
use strict;
use warnings;
use FindBin qw[ $Bin ];

our $modifier_to_byte = {};
our $byte_to_modifier = {};
our $key_types_array = [];
our $type_to_alias = {};
our $type_to_byte = {};
our $byte_to_type = {};
our $key_to_byte = {};
our $byte_to_key = {};
our $aliases      = {};

my $file = "$Bin/../key_definitions.h";
open my $fh, "<", $file or die "Could not open $file: $!";
while (my $line = <$fh>) {
    chomp $line;

    if ($line =~ m/^#define MODIFIER_(\w+)\s+(\S+)/) {
        my $modifier = $1;
        my $byte = hex($2);
        $modifier_to_byte->{$modifier} = $byte;
        $byte_to_modifier->{$byte} = $modifier;
    } elsif ($line =~ m/^#define KEY_/) {
        my (undef, $key_type, $byte_str, undef, $alias, undef) = split(/\s+/, $line);
        my $byte = hex($byte_str);
        push @{ $key_types_array }, $alias;
        $type_to_byte->{$key_type} = $byte;
        $type_to_byte->{$alias} = $byte;
        $byte_to_type->{$byte} = $alias;
        $type_to_alias->{$key_type} = $alias;
    } elsif ($line =~ m/^#define KC_(\S+)\s+(\S+)/) {
        my $key = $1;
        my $byte = hex($2);
        $key_to_byte->{$key} = $byte;
        $byte_to_key->{$byte} = $key;
    } elsif ($line =~ m/^#define ALIAS_(\S+)\s+(\S+)/) {
        my $name = $1;
        my $alias = $2;
        $aliases->{$name} = $alias;
    }
}
close $fh;

sub set_key_command {
    my ($layer, $row, $col, $key_str) = @_;

    $key_str = $aliases->{$key_str} || $key_str;

    for my $type (@{ $key_types_array }) {
        if ($key_str !~ m/^\Q$type\E/) {
            next;
        }
        if ($type =~ m/\(|\,$/) {
            if ($key_str =~ m/^\Q$type\E(.*?)\)$/) {
                my $arg = $1;
                if (defined($key_to_byte->{$arg})) {
                    return get_cmd('SET_KEY', $layer, $row, $col, $type_to_byte->{$type}, $key_to_byte->{$arg});
                } elsif ($arg =~ m/^0x\d\d$/) {
                    return get_cmd('SET_KEY', $layer, $row, $col, $type_to_byte->{$type}, hex($arg));
                } else {
                    die "Could not parse $key_str - invalid argument\n";
                }
            } else {
                die "Could not parse $key_str - invalid argument\n";
            }
        } else {
            return get_cmd('SET_KEY', $layer, $row, $col, $type_to_byte->{$type}, 0);
        }
    }

    die "Could not parse $key_str";
}

sub type_and_key_to_str {
    my ($type, $key) = @_;

    my $type_alias = $type_to_alias->{$type};
    if (!defined($type_alias)) {
        die "Could not get type alias for type $type";
    }

    my $key_byte = hex("0x$key");
    my $key_str = $byte_to_key->{$key_byte};

    if ($type_alias =~ m/\(|\,$/) {
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
    } elsif ($cmd eq 'SET_KEY') {
        return "DALSIK-\x03".chr($args[0]).chr($args[1]).chr($args[2]).chr($args[3]).chr($args[4]);
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

1;
