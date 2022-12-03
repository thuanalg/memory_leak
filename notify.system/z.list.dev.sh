i=0
while [ $i -lt 10 ];
do
	((i+=1))
	echo $i"|"$(uuidgen)
done
