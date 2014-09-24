#!/bin/sh
#
# make a release of keow
# (currently does not run IN keow because we don't currently
#  support mounting other host directories within the tree)
# using cygwin currently.

# use zip so windows users can access easily

rm -f keow.zip
[ ! -d keow ] && mkdir keow
cp Release/*.exe Release/*.dll Build/INSTALL-keow.txt keow
zip -r9 keow.zip keow

#do this manually - we need to keep the windows .lnk file intact
#rm -f bootstrap-linux.zip
#zip bootstrap-linux.zip INSTALL-bootstrap.txt keow-bash.cmd
#(cd bootstrap; find linux | grep -v '/CVS' | zip -9 ../bootstrap-linux.zip -@ )
