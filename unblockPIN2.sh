#!/bin/bash
if [ "$1" == "" ]
then
	READER=0
else
	READER=$1
fi
pkcs15-tool --reader $READER --unblock-pin --auth-id 2 --puk 17258403 --new-pin 00015
if [ "$?" == "0" ]
then
	pkcs15-tool --reader $READER --change-pin --auth-id 2 --pin 00015 --new-pin 01497
fi

