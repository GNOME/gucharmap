#!/usr/bin/perl -w
#
# reads UnicodeData.txt on stdin, prints unicode_categories.cI on stdout
#
# Sticks ranges of codepoints together. In UnicodeData.txt, a character
# name in '<>' means the start or end of a range. This script recognizes
# these ranges. In all other cases, it sticks together only runs of
# adjacent codepoints with the same category.

use strict;

# Map general category code onto symbolic name.
my %mappings =
(
    # Normative.
    'Lu' => "G_UNICODE_UPPERCASE_LETTER",
    'Ll' => "G_UNICODE_LOWERCASE_LETTER",
    'Lt' => "G_UNICODE_TITLECASE_LETTER",
    'Mn' => "G_UNICODE_NON_SPACING_MARK",
    'Mc' => "G_UNICODE_COMBINING_MARK",
    'Me' => "G_UNICODE_ENCLOSING_MARK",
    'Nd' => "G_UNICODE_DECIMAL_NUMBER",
    'Nl' => "G_UNICODE_LETTER_NUMBER",
    'No' => "G_UNICODE_OTHER_NUMBER",
    'Zs' => "G_UNICODE_SPACE_SEPARATOR",
    'Zl' => "G_UNICODE_LINE_SEPARATOR",
    'Zp' => "G_UNICODE_PARAGRAPH_SEPARATOR",
    'Cc' => "G_UNICODE_CONTROL",
    'Cf' => "G_UNICODE_FORMAT",
    'Cs' => "G_UNICODE_SURROGATE",
    'Co' => "G_UNICODE_PRIVATE_USE",
    'Cn' => "G_UNICODE_UNASSIGNED",

    # Informative.
    'Lm' => "G_UNICODE_MODIFIER_LETTER",
    'Lo' => "G_UNICODE_OTHER_LETTER",
    'Pc' => "G_UNICODE_CONNECT_PUNCTUATION",
    'Pd' => "G_UNICODE_DASH_PUNCTUATION",
    'Ps' => "G_UNICODE_OPEN_PUNCTUATION",
    'Pe' => "G_UNICODE_CLOSE_PUNCTUATION",
    'Pi' => "G_UNICODE_INITIAL_PUNCTUATION",
    'Pf' => "G_UNICODE_FINAL_PUNCTUATION",
    'Po' => "G_UNICODE_OTHER_PUNCTUATION",
    'Sm' => "G_UNICODE_MATH_SYMBOL",
    'Sc' => "G_UNICODE_CURRENCY_SYMBOL",
    'Sk' => "G_UNICODE_MODIFIER_SYMBOL",
    'So' => "G_UNICODE_OTHER_SYMBOL"
);

print "/* unicode_categories.cI */\n";
print "/* THIS IS A GENERATED FILE. */\n";
print "/* http://www.unicode.org/Public/UNIDATA/UnicodeData.txt */\n";
print "\n";
print "const UnicodeCategory unicode_categories[] =\n";
print "{\n";

my ($codepoint, $last_codepoint, $start_codepoint) = (-99, -99, -99);
my ($category, $last_category) = ("G_XXX", "G_YYY");
my $name;
my ($started_range, $finished_range) = (undef, undef);

while (my $line = <>)
{
    $line =~ /^([0-9A-F]*);([^;]*);([^;]*);/;

    my $codepoint = hex $1;
    my $name = $2;
    my $category = $mappings{$3};

    if ($finished_range or ($category ne $last_category) 
        or (not $started_range and $codepoint != $last_codepoint + 1))
    {
        if ($last_codepoint >= 0) 
        {
            printf("  { 0x%4.4X, 0x%4.4X, %s },\n", 
                   $start_codepoint, $last_codepoint, $last_category);
        } 

        $start_codepoint = $codepoint;
    }

    if ($name =~ /^<.*First>$/) {
        $started_range = 1;
        $finished_range = undef;
    }
    elsif ($name =~ /^<.*Last>$/) {
        $started_range = undef;
        $finished_range = 1;
    }
    elsif ($finished_range) {
        $finished_range = undef;
    }


    $last_codepoint = $codepoint;
    $last_category = $category;
}

printf("  { 0x%4.4X, 0x%4.4X, %s },\n", 
       $start_codepoint, $last_codepoint, $last_category);

print "};\n\n";

exit;
