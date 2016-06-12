#!/bin/sh

APP=../build-st_pattern-Desktop_Qt_5_4_1_clang_64bit-Release/st_pattern.app/Contents/MacOS/st_pattern

INTER_FILE_NAME=test

SEG_DATA="../test_files/gen/diff_t"
SEG_SUFFIX=.txt
SEG_STEP=1.8
SEG_USE_SED=1
SEG_MIN_LEN=10.0
SEG_USE_SEST=1

CLU_WEIGHT="0.001:0.001:0.001:0.001:0.1:0.01"
CLU_THRESH=0.4
CLU_MEM_LIM=500

MINE_RADIUS=150.0
MINE_MIN_SUP=3.0
MINE_MIN_PAT_LEN=2
SPMF_OUTPUT=output


$APP seg $SEG_DATA $SEG_SUFFIX $INTER_FILE_NAME $SEG_STEP $SEG_USE_SED $SEG_MIN_LEN $SEG_USE_SEST

$APP cluster $INTER_FILE_NAME $CLU_WEIGHT $INTER_FILE_NAME $CLU_THRESH $CLU_MEM_LIM

#$APP mine $INTER_FILE_NAME $INTER_FILE_NAME $INTER_FILE_NAME $MINE_RADIUS $MINE_MIN_SUP $MINE_MIN_PAT_LEN

#transNum=$(cat $INTER_FILE_NAME.txt |wc -l)
#echo There are $transNum transactions.
#sup=$(($MINE_MIN_SUP/$transNum))
#echo $sup

java -jar ../tools/spmf.jar run BIDE+ $INTER_FILE_NAME.txt $SPMF_OUTPUT.txt 0.2
$APP visualize $SPMF_OUTPUT $INTER_FILE_NAME 10

#rm *.cluster
rm *.s2c
rm *.seg
rm *.stp
rm *.tinc
rm *.tins