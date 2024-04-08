#!/bin/bash

check_directory(){

    #caut directorul dat prin $1

    while IFS= read -r line;
    do
	file=$(echo $line | awk '{print $9}')
	if [[ $file == $1 ]]
	then
	    ls_curent=$line
	fi
    done <<< $(ls -al)

    #functia de cautat cea mai recenta arhiva produsa de program

    look_for_last_tar(){
	maxim=0
	while IFS= read -r line;
	do
	    file=$(echo $line | awk '{print $9}')
	    if [[ $file =~ .tar$ ]]
	    then
		nr=$(echo $file | awk -F'_' '{print $NF}' | sed 's/.tar//g')
		if test $nr -gt $maxim
		then
		    maxim=$nr
		fi
	    fi
	done <<< $(ls -l)
	echo $maxim
    }

    #caut prin directorul 'versions'(folderul cu arhive) daca exista un director cu numele $1(inseamna ca a mai fost arhivat in trecut)
    #intru in el si caut ultima arhiva(arh), pe care o dezarhivez folosind 'tar -xf $arh'

    cd versions
    arhive_exists=0
    while IFS= read -r line;
    do
	file=$(echo $line | awk '{print $9}')
	if [[ $file == $1 ]]
	then
	    cd $file   #intru in folderul numit $1 care contine toate arhivele lui $1
	    arh=$(look_for_last_tar)   #aflu numarul ultimei arhive(cea mai recenta creata de program) cu ajutorul functiei de mai sus
            tar -xf $1_$arh.tar   #dezarhivez arhiva gasita
	    arhive_exists=1
	fi
    done <<< $(ls -al)

    if test $arhive_exists -eq 0
    then
	mkdir $1   #creez directory-ul ce va contine toate arhivele viitoare ale argumentului $1
	cd ..   #ma reintorc in directory-ul unde se afla folder-ul pe care vrem sa il arhivam
	tar -czf "versions/$1/$1_1.tar" "$1"   #creez prima arhiva a directory-ului, in caz in care nu exista alte arhive ale lui
	echo "$1: ARHIVA NUMARUL 1 CREATA CU SUCCES"
	return
    fi

    #caut folderul pe care l-am dezarhivat mai devreme

    while IFS= read -r line;
    do
	file=$(echo $line | awk '{print $9}')
	if [[ $file == $1 ]]
	then
            ls_tar=$line
	fi
    done <<< $(ls -al)

    #TODO sa sterg folderul creat

    #memorez datele ultimelor modificari pentru folderul $1 si pentru ultima sa arhiva

    month1=$(echo $ls_curent | awk '{print $6}')   #folderul initial
    day1=$(echo $ls_curent | awk '{print $7}')
    hour1=$(echo $ls_curent | awk '{print $8}')

    month2=$(echo $ls_tar | awk '{print $6}')   #ultima arhiva
    day2=$(echo $ls_tar | awk '{print $7}')
    hour2=$(echo $ls_tar | awk '{print $8}')

    #ma reintorc in directory-ul unde se afla folder-ul pe care vrem sa il arhivam

    cd ..
    cd ..

    #verific daca exista sau nu modificari de la ultima arhiva(in caz ca da, creez o noua arhiva)
    
    if [[ $month1 != $month2 || $day1 != $day2 || $hour1 != $hour2 ]]
    then
	arh=$(($arh+1))   #adunam 1 la index-ul arhivei cele mai recente
	tar -czf "versions/$1/$1_$arh.tar" "$1"   #creez arhiva, aceasta va fi considerata cea mai recenta la urmatoarea rulare a programului
	echo "$1: ARHIVA NUMARUL $arh CREATA CU SUCCES"
    else
	echo "$1: NU EXISTA MODIFICARI DE LA ULTIMA ARHIVARE"
    fi

}

echo ' '

for arg in $@
do
    check_directory $arg
done

echo ' '
