list=`ls -d -- */`
for fn in ${list}; do
	if [ -f ${fn}"Makefile" ]; then
		echo "-------------------"${fn}"-----------------"
    	echo ${fn}"Makefile";
		cd ${fn};
		make clean;make;
		cd ../;
		echo
		echo
	fi
done
