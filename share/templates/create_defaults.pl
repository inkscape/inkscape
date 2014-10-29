#!/usr/bin/perl

# Purpose: To create internationalized versions of default.svg.

# Usage:  create_defaults.pl

use strict;
use warnings;
use utf8;

my $count = 0;

# Data base

my @data = (
	["be",	"Пласт 1",	"layer1"],
	["ca",	"Capa 1",	"capa1"],
	["cs",	"Vrstva 1",	"layer1"],
	["de",	"Ebene 1",	"layer1"],
	["eo",	"Tavolo 1",	"layer1"],
	["es",	"Capa 1",	"layer1"],
	["eu",	"Capa 1",	"layer1"],
	["fi",	"Taso 1",	"layer1"],
	["fr",	"Calque 1",	"layer1"],
	["hu",	"1. réteg",	"layer1"],
	["it",	"Livello 1",	"layer1"],
	["ja",	"レイヤー 1",	"layer1"],
	["lt",	"Sluoksnis 1",	"layer1"],
	["nl",	"Laag 1",	"layer1"],
	["pl",	"Warstwa 1",	"layer1"],
	["pt_BR",	"Camada 1",	"layer1"],
	["ro",	"Strat 1",	"layer1"],
	["sk",	"Vrstva 1",	"layer1"],
	["zh_TW",	"圖層 1",	"layer1"]
    );

foreach my $lang (@data ) {

    ++$count;

    my @values = @{$lang};
    print "$values[0]\n";

    my $filename = "default." . $values[0] . ".svg";
    open( OUTPUT, '>:encoding(UTF-8)', $filename ) or die "Cannot open $filename.\n";

    # Open input again to return to top...
    open (INPUT, '<:encoding(UTF-8)', "default.svg") or die 'Cannot open input\n';

    while( my $line = <INPUT> ) {

	if( $line =~ /inkscape:label=/ ) {
	    $line =~ s/Layer 1/$values[1]/;
	}
	print OUTPUT $line;

    }

    close( INPUT );
    close( OUTPUT );
}


print "Created $count files.\n";
