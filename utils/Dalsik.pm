package Dalsik;

use strict;
use warnings;

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
