
while :
do
	MODE=`mosquitto_sub -h 10.10.4.1  -C 1 -t /scanner/scan`
	echo $MODE

	case "$MODE" in
		"color")
			echo "scanning in color";
			/usr/bin/hp-scan --adf --mode=color --size=a4
			FILE=`ls -tp hp*.pdf | grep -v /$ | head -1`
			mv $FILE temp.pdf
			/usr/bin/ps2pdf temp.pdf $FILE
			rm temp.pdf
			;;
		"grayscale")
			echo "scanning in grayscale";
			/usr/bin/hp-scan --adf --mode=gray --size=a4
			FILE=`ls -tp hp*.pdf | grep -v /$ | head -1`
			mv $FILE temp.pdf
			/usr/bin/ps2pdf temp.pdf $FILE
			rm temp.pdf
			;;

		*)
			echo "bogus input - not scanning";
			;;
	esac
done	
