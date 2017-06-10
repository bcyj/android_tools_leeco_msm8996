set -e
RUNCOUNT=$1
LOOPCOUNT=$2

echo "Starting $RUNCOUNT concurrent instances of $LOOPCOUNT iterations" >&2

range=$(seq 1 $RUNCOUNT)
for i in $range
do
   ./loop.sh $LOOPCOUNT &
done
echo "Waiting for tests to complete" >&2
wait
