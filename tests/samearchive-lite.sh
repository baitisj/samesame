DIFF=diff
SED=sed

WORK=${srcdir}/work
TEST1=${WORK}/test1
TEST2=${WORK}/test2
LIST1=${WORK}/test1.lst
OUTPUT=${WORK}/test.out
CHECK=${srcdir}/tests/samearchive-lite.out
CMD=${srcdir}/samearchive-lite

${CMD} ${TEST1} ${TEST2} < ${LIST1} | ${SED} -e 's/${srcdir}/./g' > ${OUTPUT}
${DIFF} -u ${CHECK} ${OUTPUT}
