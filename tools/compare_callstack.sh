# Generates the call stack for crashes, to see whether the crash is the same as
# the target crash
BINARY=$1
TARGET_CRASH_FILE=$2
CRASH_DIR=$3

for c in `ls ${CRASH_DIR}/*`; do 
    gdb --batch --quiet -ex "thread apply all bt full" -ex "quit" \
    --args ${BINARY} $c > `basename $c`.out
    stat -c %y "$c" > `basename $c`.time
done;

for f in `ls *.out`; do sed '^[\#\&\*]/d' $f > $f.filter; done;
