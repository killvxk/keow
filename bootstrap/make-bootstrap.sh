#!/bin/sh
#
#  Script to make a Keow bootstrap image.
#  Builds debian based images (eg ubuntu)
#

distribution=`lsb_release -s -i | tr '[:upper:]' '[:lower:]' | tr '[:blank:]' '-'`
codename=`lsb_release -s -c | tr '[:upper:]' '[:lower:]' | tr '[:blank:]' '-'`

archive=bootstrap-$distribution-$codename.zip
tmpdir=tmp.$distribution.$codename.$$


if [ -z "$distribution" -o -z "$codename" ]; then
    echo "Unable to determine distribution name" 1>&2
    exit 1
fi

#check dependancies that may not be installed
for cmd in zip fakeroot fakechroot debootstrap; do
    if [ -z "`which $cmd`" ]; then
        echo "Cannot find $cmd. Please install it" 1>&2
        exit 1
    fi
done

#prepare tmp - and cleanup at exit
trap "echo -e '\nCleaning up'; rm -rf $tmpdir; echo 'Done.'" 0
mkdir -p "$tmpdir/linux"

# build minimal system
echo "Creating minimal system"
fakeroot fakechroot debootstrap --arch=i386 --components=main --variant=fakechroot $codename $tmpdir/linux
#find linux | cpio -p $tmpdir

#remove some stuff we don't need (can be rebuilt)
echo "Pruning unnessesary files"
rm -f $tmpdir/linux/var/cache/apt/archives/*.deb
rm -f $tmpdir/linux/var/lib/apt/lists/*
#files with invalid windows chars can be skipped!
find $tmpdir/linux -name \*:\* -exec rm {} \;

#create archive of bootstrap
echo "Creating archice of bootstrap files"
tar -C $tmpdir -czf $tmpdir/linux.tgz linux
rm -rf "$tmpdir/linux"

#zip up the results
echo "Creating ZIP of bootstrap files"
rm $archive #remove old one
zip -j -0 $archive INSTALL-bootstrap.txt README.txt Extract.cmd busybox "$tmpdir/linux.tgz"

#see result
du -sh $archive

#end
