import os
import time

def get_num_lines(file_name):
    with open(file_name, "r") as fin:
        lines = fin.readlines()
        return len(lines)

if __name__ == '__main__':
    APP="../build-st_pattern-Desktop_Qt_5_4_1_clang_64bit-Release/st_pattern.app/Contents/MacOS/st_pattern"

    INTER_FILE_NAME='test'

    SEG_DATA="../test_files/gen/diff_s_large"
    SEG_SUFFIX='.txt'
    SEG_STEP=1.8
    SEG_USE_SED=1
    SEG_MIN_LEN=10.0
    SEG_USE_SEST=1
    SEG_DOTS_TH = 1000.0

    CLU_WEIGHT="0.001:0.001:0.001:0.001:0:0"
    CLU_THRESH=1.6
    CLU_MEM_LIM=500

    MINE_RADIUS=1000.0
    MINE_MIN_SUP=4
    MINE_MIN_PAT_LEN=2
    SPMF_OUTPUT="output"
    MINE_METHOD = "SCPM"
    
    _start = time.time()
    os.system("%s seg %s %s %s %f %d %f %d %f" % (APP, SEG_DATA, SEG_SUFFIX, INTER_FILE_NAME, SEG_STEP, SEG_USE_SED, SEG_MIN_LEN, SEG_USE_SEST, SEG_DOTS_TH))
    os.system("%s cluster %s %s %s %f %d" % (APP, INTER_FILE_NAME, CLU_WEIGHT, INTER_FILE_NAME, CLU_THRESH, CLU_MEM_LIM))
    if MINE_METHOD == "SCPM":
        os.system("%s mine %s %s %s %f %d %d" % (APP, INTER_FILE_NAME, INTER_FILE_NAME, INTER_FILE_NAME, MINE_RADIUS, MINE_MIN_SUP, MINE_MIN_PAT_LEN))
    else:
        numTrans = get_num_lines(INTER_FILE_NAME+".txt")
        if numTrans > 0:
            MINE_MIN_SUP_RATE = MINE_MIN_SUP/1.0/numTrans
            print("MINE_MIN_SUP_RATE = " + str(MINE_MIN_SUP_RATE))
            os.system("java -jar ../tools/spmf.jar run %s %s.txt %s.txt %f" % (MINE_METHOD, INTER_FILE_NAME, SPMF_OUTPUT, MINE_MIN_SUP_RATE))
            os.system("%s visualize %s %s %d" % (APP, SPMF_OUTPUT, INTER_FILE_NAME, MINE_MIN_PAT_LEN))
        else:
            print("There's no transactions at all.")
    _end = time.time()
    print('Time cost: %f seconds.' % (_end-_start,))