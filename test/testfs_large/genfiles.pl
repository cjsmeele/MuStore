#!/usr/bin/env perl

use 5.12.0;
use warnings;

use autodie "open";
use FindBin;

my $ROOT = "$FindBin::RealBin";

$| = 1;

my $clean = ($ARGV[0] // '') eq "clean";

unlink "$ROOT/rtdir100/huge.txt" if $clean and -e "$ROOT/rtdir100/huge.txt";

for my $i (1..200) {

    my $name = sprintf "%s/rtdir%03d", $ROOT, $i;

    if ($clean) {
        rmdir $name if -d $name;
    } else {
        mkdir $name unless -d $name;
    }
}

exit if $clean;

open my $fh, '>' => "$ROOT/rtdir100/huge.txt";

for my $sector (0..20) {
    my $i = 0;
    while ($i < 512) {
        my $s = sprintf "Ik ben sector %04d\r\n", $sector;
        if ($i + length($s) <= 512) {
            print $fh $s;
            $i += length $s;
        } elsif ($sector < 19) {
            print $fh " "x(512 - $i);
            $i = 512;
        } else {
            exit;
        }
        last if ($i >= 512)
    }
}

close $fh;
