
IODDIR=$1/libraries
INCDIR=$2
mkdir -p $INCDIR/iod

[ -h $INCDIR/iod/metamap ]  || ln -sf $IODDIR/metamap/metamap $INCDIR/iod/metamap;
[ -h $INCDIR/iod/symbol ]   || ln -sf $IODDIR/symbol/symbol $INCDIR/iod/symbol
[ -h $INCDIR/iod/metajson ] || ln -sf $IODDIR/metajson/metajson $INCDIR/iod/metajson
