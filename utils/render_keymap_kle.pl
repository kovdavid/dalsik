#!/usr/bin/env perl

use v5.10;
use strict;
use warnings;

use JSON;
use FindBin qw[ $Bin ];
use lib "$Bin";
use utf8;

use Dalsik;

use constant {
    KLE_POSITION_CENTER => 7,
    KLE_COLOR_WHITE => "#cccccc",
    KLE_COLOR_ORANGE => "#f77800",
};

my $json = JSON->new()->canonical(1);

binmode(STDOUT, ":utf8");

my $fh;

my $file = shift;
if ($file) {
    open $fh, "<", $file or die "Unable to open file($file): $!";
} else {
    $fh = *STDIN;
}

my $data = {};
my $column_lengths = {};
my $num_keys_on_layers = {};
my $tapdances = {};

my $num_rows = 0;
my $num_cols = 0;

while (my $line = <$fh>) {
    if ($line =~ /KEY<L(\d+)-R(\d+)-C(\d+)\|(\w+)\|(\w+)>/) {
        my $layer = $1+0;
        my $row = $2+0;
        my $col = $3+0;
        my $type_str = $4;
        my $key_hex = $5;

        my $str = Dalsik::type_and_key_to_str($type_str, $key_hex);
        if ($str ne 'UNSET' && $str ne 'TRANS' && $str ne 'KC_NO') {
            $num_keys_on_layers->{$layer}++;
        }

        $str =~ s/\bMINUS\b/-/;
        $str =~ s/\bSCOLON\b/:/;
        $str =~ s/\bSLSH\b/\//;
        $str =~ s/\bQUOTE\b/'/;
        $str =~ s/\bEQUAL\b/=/;
        $str =~ s/\bBSLS\b/\\/;
        $str =~ s/\bDOT\b/./;
        $str =~ s/\bCOMMA\b/,/;
        $str =~ s/\bKC_NO\b/ /;
        $str =~ s/\bLBRC\b/[/;
        $str =~ s/\bRBRC\b/]/;
        $str =~ s/\bGRV\b/`/;
        $str =~ s/\bTRANS\b//;

        $data->{$layer}->{$row}->{$col} = $str;
        if (
            !$column_lengths->{$col}
            || length($str) > $column_lengths->{$col}
        ) {
            $column_lengths->{$col} = length($str);
        }

        if ($row+1 > $num_rows) {
            $num_rows = $row+1;
        }
        if ($col+1 > $num_cols) {
            $num_cols = $col+1;
        }
    } elsif ($line =~ /TAPDANCE<I(\d+)-T(\d+)\|(\w+)\|(\w+)>/) {
        my $index = $1+0;
        my $tap = $2+0;
        my $type_str = $3;
        my $key_hex = $4;

        my $str = Dalsik::type_and_key_to_str($type_str, $key_hex);
        if ($str ne 'UNSET' && $str ne 'TRANS' && $str ne 'KC_NO') {
            $tapdances->{$index}->{$tap} = $str;
        }
    }
}
close $fh;

for my $layer (sort keys %{ $data }) {
    next unless $num_keys_on_layers->{$layer};
    render_layer($layer, $data->{$layer});
    print "\n";
}

sub render_layer {
    my ($layer, $rows) = @_;

    my $layer_header_props = {
        c => KLE_COLOR_ORANGE,
        a => KLE_POSITION_CENTER,
        fa => [8],
        y => 0.25,
    };

    say $json->encode([$layer_header_props, "$layer"]).",";

    for (my $row = 0; $row < $num_rows; $row++) {

        my $row_data = [];

        for (my $col = 0; $col < $num_cols; $col++) {
            my $color = KLE_COLOR_WHITE;
            if ($row == 3 || $col == 0 || $col == 11) {
                $color = KLE_COLOR_ORANGE;
            }

            my $key = $rows->{$row}->{$col};
            my $props = {
                c => $color,
                fa => [2],
                a => KLE_POSITION_CENTER,
            };
            if ($row == 0 && $col == 0) {
                $props->{y} = 0.25;
            }
            if ($col == 6) {
                $props->{x} = 1;
            }
            if ($col > 1 && $col < 11) {
                delete $props->{c};
            }

            if ($key =~ m/^KC\((\S+)\)$/) {
                $key = handle_kc_key($1, $props);
            } elsif ($key =~ m/^D(S?)\(LAYER_(\d),(\S+)\)$/) {
                my $single = $1 || "";
                my $layer = $2;
                my $dual_key = handle_kc_key($3, $props);

                my $buffer = [];
                $buffer->[8] = "LP$single($layer)";
                $buffer->[10] = $dual_key;

                $key = join("\n", map { $_ // "" } @{ $buffer });

                $props->{a} = 0;
            } elsif ($key =~ m/^D(S?)\((\S+),(\S+)\)$/) {
                my $single = $1 || "";
                my $modifier = translate_key($2);
                my $dual_key = handle_kc_key($3, $props);

                my $buffer = [];
                $buffer->[8] = "$modifier";
                $buffer->[10] = $dual_key;

                $key = join("\n", map { $_ // "" } @{ $buffer });

                $props->{a} = 0;
            } elsif ($key =~ m/^(\S+)\((\S+)\)$/) {
                my $modifier = translate_key($1);
                my $dual_key = handle_kc_key($2, $props);

                if ($1 =~ /SHIFT/ && $dual_key ne 'INS') {
                    my $dict = {
                        1 => '!',
                        2 => '@',
                        3 => '#',
                        4 => '$',
                        5 => '%',
                        6 => '^',
                        7 => '&',
                        8 => '*',
                        9 => '(',
                        0 => ')',
                        '\\' => '|',
                        '[' => '{',
                        ']' => '}',
                        '-' => '_',
                        ',' => '<',
                        '.' => '>',
                        '/' => '?',
                        '=' => '+',
                        ':' => ';',
                    };

                    $key = handle_kc_key($dict->{$dual_key} // "undef $dual_key");
                } else {
                    my $buffer = [];
                    $buffer->[8] = "$modifier";
                    $buffer->[9] = "+";
                    $buffer->[10] = $dual_key;

                    $key = join("\n", map { $_ // "" } @{ $buffer });

                    $props->{a} = 0;
                }
            }

            push @{ $row_data }, $props;
            push @{ $row_data }, $key;
        }

        say $json->encode($row_data).",";
    }
}

sub translate_key {
    my ($key) = @_;

    my $kc_key_name_mapping = {
        "ENTER" => "↩",
        "DELETE" => "DEL",
        "ESCAPE" => "ESC",
        "LSHIFT" => "⇧",
        "LCTRL" => "CTRL",
        "LALT" => "ALT",
        "LGUI" => "GUI",
        "RSHIFT" => "⇧",
        "RCTRL" => "CTRL",
        "RALT" => "ALT",
        "RGUI" => "GUI",
        "NO" => "",
        "LEFT" => "←",
        "RIGHT" => "→",
        "DOWN" => "↓",
        "UP" => "↑",
        "TAB" => "⭾",
        "BSPC" => "⌫",
        "TRANS" => "",
        "INSERT" => "INS",
        "PGDOWN" => "PGDN",
    };

    return $kc_key_name_mapping->{$key} // $key;
}

sub handle_kc_key {
    my ($key, $props) = @_;

    $key = translate_key($key);

    if (length($key) > 6) {
        $props->{fa} = [2];
    } elsif (length($key) > 3) {
        $props->{fa} = [3];
    } elsif (length($key) > 2) {
        $props->{fa} = [5];
    } else {
        $props->{fa} = [8];
    }

    return $key;
}

__END__


open keyboard_layout_editor.com -> raw_data, click into the input and run:

./utils/render_keymap.pl /clean_daily/keymap.out  | xclip -selection clipboard ; xdotool key super+Tab ; sleep 0.1; xdotool key ctrl+a ; sleep 0.1 ; xdotool key BackSpace ; sleep 0.1 ; xdotool key shift+Insert ; sleep 0.1 ; xdotool key super+Tab

inotifyrun utils -- "./utils/render_keymap_kle.pl /clean_daily/keymap.out  | xclip -selection clipboard ; xdotool key super+Tab ; sleep 0.1; xdotool key ctrl+a ; sleep 0.1 ; xdotool key BackSpace ; sleep 0.1 ; xdotool key shift+Insert ; sleep 0.1 ; xdotool key super+Tab"
