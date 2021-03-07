DIFF=diff
ECHO=echo
SED=sed

WORK=${srcdir}/work
LIST1=${WORK}/test1.lst
OUTPUT=${WORK}/test.out
CHECK=${srcdir}/tests/samefile.out
CMD=${srcdir}/samefile

(	${ECHO} 'samefile -A:'
	${CMD} -A < ${LIST1}
	${ECHO}
	${ECHO} 'samefile -Z:'
	${CMD} -Z < ${LIST1}
	${ECHO}
	${ECHO} 'samefile -At:'
	${CMD} -At < ${LIST1}
	${ECHO}
	${ECHO} 'samefile -Zt:'
	${CMD} -Zt < ${LIST1}
	${ECHO}
	${ECHO} 'samefile -L:'
	${CMD} -L < ${LIST1}
	${ECHO}

	${ECHO} 'samefile -irA:'
	${CMD} -irA < ${LIST1}
	${ECHO}
	${ECHO} 'samefile -irZ:'
	${CMD} -irZ < ${LIST1}
	${ECHO}
	${ECHO} 'samefile -irAt:'
	${CMD} -irAt < ${LIST1}
	${ECHO}
	${ECHO} 'samefile -irZt:'
	${CMD} -irZt < ${LIST1}
	${ECHO}
	${ECHO} 'samefile -irL:'
	${CMD} -irL < ${LIST1}
	${ECHO}

	${ECHO} 'samefile -irx:'
	${CMD} -irx < ${LIST1}
) | ${SED} -e 's/${srcdir}/./g' > ${OUTPUT}

${DIFF} -u ${CHECK} ${OUTPUT}
