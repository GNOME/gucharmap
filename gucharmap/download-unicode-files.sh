#!/bin/sh

FILES='UnicodeData.txt Unihan.zip NamesList.txt Blocks.txt Scripts.txt'
DIR='unicode'

mkdir -p $DIR

for x in $FILES; do
	echo -n '.'
	#if [ -e "$DIR/$x" ]; then continue fi
	wget "http://www.unicode.org/Public/UNIDATA/$x" -O "$DIR/$x"
done

echo 'Done.'

