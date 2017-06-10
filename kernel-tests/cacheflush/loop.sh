COUNT=$1

for i in $(seq 1 $COUNT)
do
   ./cacheflush
done