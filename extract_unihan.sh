#!/bin/sh
#
# $Id$
#

#
# looks at Unihan.txt information and defines a unihan_t for each character
# sample output:
#
#   { 0x4E00, "one; a, an; alone", "YAT1", "YI1", "qit4", "IL", "", "ICHI ITSU" },
#   { 0x4E01, "male adult; robust, vigorous; the fourth heavenly stem", "DING1 JANG1", "DING1 ZHENG1", "deng1", "CENG", "", "TEI CHOU TOU" },
#   { 0x4E02, "obstruction of breath (qi) as it seeks release; variant of other characters", "", "KAO3 QIAO3 YU2", "", "KYO", "", "KOU" },
#   { 0x4E03, "seven", "CHAT1", "QI1 SHANG3", "tsit4", "CHIL", "", "SHICHI SHITSU" },
#

#
# sample usage: ./extract_unihan.sh < Unihan.txt
#

#
# typedef struct 
# {
#   gunichar index;
#   gchar *kDefinition;
#   gchar *kCantonese;
#   gchar *kMandarin;
#   gchar *kTang;
#   gchar *kKorean;
#   gchar *kJapeneseKun
#   gchar *kJapaneseOn;
# } 
# unihan_t;
#

#
# assumes there are no quotes or other characters that should be escaped in
# any of the values (which is not true, but there are only a couple
# exceptions, which can be easily fixed)
#

kDefinition=""
kCantonese=""
kMandarin=""
kTang=""
kKorean=""
kJapeneseKun=""
kJapaneseOn=""

curr_index="U+3400"

while read index property value
do
    case $index in
        "#") continue ;;
        "#*") continue ;;
    esac

    if [ $index != $curr_index ] 
    then
        hex=`echo $curr_index | sed 's/^U+/0x/'`
        curr_index=$index

        if [ "x$kDefinition" = "x" ] && [ "x$kCantonese" = "x" ] && [ "x$kMandarin" = "x" ] && [ "x$kTang" = "x" ] && [ "x$kKorean" = "x" ] && [ "x$kJapeneseKun" = "x" ] && [ "x$kJapaneseOn" = "x" ] ;
        then
            continue;
        fi

        echo "  { $hex, \"$kDefinition\", \"$kCantonese\", \"$kMandarin\", \"$kTang\", \"$kKorean\", \"$kJapeneseKun\", \"$kJapaneseOn\" },"

        kDefinition=""
        kCantonese=""
        kMandarin=""
        kTang=""
        kKorean=""
        kJapeneseKun=""
        kJapaneseOn=""
    fi

    case $property in
        "kDefinition") kDefinition=$value ;;
        "kCantonese") kCantonese=$value ;;
        "kMandarin") kMandarin=$value ;;
        "kTang") kTang=$value ;;
        "kKorean") kKorean=$value ;;
        "kJapeneseKun") kJapeneseKun=$value ;;
        "kJapaneseOn") kJapaneseOn=$value ;;
    esac
done
