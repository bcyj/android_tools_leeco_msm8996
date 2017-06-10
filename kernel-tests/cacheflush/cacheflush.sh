echo "Cacheflush test starting"
tests="$*"

chmod 777 _cacheflush.sh
chmod 777 loop.sh
chmod 777 cacheflush

RUNCOUNT=2
LOOPCOUNT=3000

if [ "x$tests" == "x" ]
then
	echo "Running nominal tests."
	tests='-n'
fi

for i in $tests
do
	case $i in
		-n|--nominal)
			RUNCOUNT=2
			LOOPCOUNT=1500
			;;
		-s|--stress)
			RUNCOUNT=5
			LOOPCOUNT=5000
			;;
	esac
done

EXP=`expr $RUNCOUNT \* $LOOPCOUNT`
echo "Expecting $EXP successful runs."

VAL=`./_cacheflush.sh $RUNCOUNT $LOOPCOUNT |grep OK |wc -l`

echo "Done"

if [ $EXP == $VAL ]
then
	echo "Cacheflush test passed."
else
	echo "Observed $VAL successful runs."
	echo "Cacheflush test FAILED."
	exit 1
fi

exit 0
