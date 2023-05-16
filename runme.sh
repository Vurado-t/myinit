#!/bin/bash

make -C ./src;

BASE_DIR=/tmp/vurado/myinit;
BASE_TESTS_DIR=$BASE_DIR/tests;

mkdir -p $BASE_DIR/tests/configs;
cp ./tests/data/runme_with_one_process.config $BASE_TESTS_DIR/configs/runme_with_one_process.config;
cp ./tests/data/runme_with_3_processes.config $BASE_TESTS_DIR/configs/runme_with_3_process.config;

cp -r ./tests/scripts $BASE_TESTS_DIR;

mkdir $BASE_TESTS_DIR/buffers;

for i in {1..4}; do
  touch $BASE_TESTS_DIR/buffers/test-script"$i".in;
  touch $BASE_TESTS_DIR/buffers/test-script"$i".out;
done;

printf "" > result.txt;


cp $BASE_TESTS_DIR/configs/runme_with_3_process.config $BASE_TESTS_DIR/configs/myinit.config;
./out/myinitd $BASE_TESTS_DIR/configs/myinit.config
sleep 2;

myinit_pid=$(pgrep myinitd);

{ printf "\n===== THREE CHILD PROCESSES =====\n"; ps --ppid "$myinit_pid"; } >> result.txt

printf "\n===== IMMEDIATELY AFTER KILL SCRIPT 2 =====\n" >> result.txt
pkill -KILL test-script2.sh;
ps --ppid "$myinit_pid" >> result.txt;

printf "\n===== SECOND AFTER KILL SCRIPT 2 =====\n" >> result.txt
sleep 2;
ps --ppid "$myinit_pid" >> result.txt;

printf "\n===== CONFIG RELOADING =====\n" >> result.txt
cp $BASE_TESTS_DIR/configs/runme_with_one_process.config $BASE_TESTS_DIR/configs/myinit.config;
kill -HUP "$myinit_pid";
sleep 2;
ps --ppid "$myinit_pid" >> result.txt;

printf "\n===== LOG =====\n" >> result.txt
tail -n 100 /tmp/myinit.log >> result.txt;


kill -KILL -"$myinit_pid";
rm -r /tmp/vurado;
rm /tmp/myinit.log;
