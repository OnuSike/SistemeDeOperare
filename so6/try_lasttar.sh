look_for_last_tar(){
    maxim=0
    while IFS= read -r line;
    do
	file=$(echo $line | awk '{print $9}')
	if [[ $file =~ .tar$ ]]
	then
	    nr=$(echo $file | awk -F'_' '{print $NF}' | sed 's/.tar//g')
	    echo $line
	    echo "file: " $file
	    echo "number: " $nr
	    echo $maxim
	    if test $nr -gt $maxim
	    then
		maxim=$nr
		echo "Maxim: " $maxim
	    else
		echo error2
	    fi
	else
	    echo "NU SE EXISTA" $line
	fi
    done <<< $(ls -l)
    
}

cd versions
cd dir
look_for_last_tar
