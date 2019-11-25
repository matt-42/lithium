
IODDIR=$1/libraries
INCDIR=$2
mkdir -p $INCDIR/iod

[ -h $INCDIR/iod/metamap ]  || ln -sf $IODDIR/metamap/metamap $INCDIR/iod/metamap;
[ -h $INCDIR/iod/symbol ]   || ln -sf $IODDIR/symbol/symbol $INCDIR/iod/symbol
[ -h $INCDIR/iod/metajson ] || ln -sf $IODDIR/metajson/metajson $INCDIR/iod/metajson
[ -h $INCDIR/iod/silicon ] || ln -sf $IODDIR/silicon/silicon $INCDIR/iod/silicon
[ -h $INCDIR/iod/http_client ] || ln -sf $IODDIR/http_client/http_client $INCDIR/iod/http_client
[ -h $INCDIR/iod/di ] || ln -sf $IODDIR/di/di $INCDIR/iod/di
[ -h $INCDIR/iod/sqlite ] || ln -sf $IODDIR/sqlite $INCDIR/iod/sqlite
[ -h $INCDIR/iod/callable_traits ] || ln -sf $IODDIR/callable_traits $INCDIR/iod/callable_traits
