# $Id$
# This batch job does all the automatic postprocessing, 
# that can't be done in open office 
# (e.g. saving a png file with transparent background)
#


# export uracoli-usrl1.odg as l{1,2,3,4,5}raw.png
# with the aproriate number of bars colored blue.
OPTS="-resize 120 -transparent white"
for f in l1 l2 l3 l4 l5;
do
    cmd="convert ${OPTS} ${f}raw.png ${f}.png"
    echo $cmd
    $($cmd)
done

# antenna to faviocn
cmd="convert antennaraw.png -resize 16x16 favicon.ico"
echo $cmd
$($cmd)
