DIFF=diff
SED=sed

WORK=${srcdir}/work
TEST1=${WORK}/test1
TEST2=${WORK}/test2
LIST2=${WORK}/test2.lst
OUTPUT=${WORK}/test.out
CHECK=${srcdir}/tests/samearchive.out
CMD=${srcdir}/samearchive

${CMD} -i ${TEST1} ${TEST2} < ${LIST2} | ${SED} -e 's/${srcdir}/./g' > ${OUTPUT}
${DIFF} -u ${CHECK} ${OUTPUT}
