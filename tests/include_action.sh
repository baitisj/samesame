CAT=cat
DIFF=diff
ECHO=echo
SED=sed

WORK=${srcdir}/work
OUTPUT=${WORK}/test.out
CHECK=${srcdir}/tests/${NAME}.out
CMD=${srcdir}/${NAME}

(	${ECHO} "${NAME} -A:"
	${CMD} -nvA ${PARAM} < ${WORK}/samefile-A.out 2>&1
	${ECHO}
	${ECHO} "${NAME} -Z:"
	${CMD} -nvZ ${PARAM} < ${WORK}/samefile-Z.out 2>&1
	${ECHO}
	${ECHO} "${NAME} -At:"
	${CMD} -nvAt ${PARAM} < ${WORK}/samefile-At.out 2>&1
	${ECHO}
	${ECHO} "${NAME} -Zt:"
	${CMD} -nvZt ${PARAM} < ${WORK}/samefile-Zt.out 2>&1
	${ECHO}
	${ECHO} "${NAME} -L:"
	${CMD} -nvL ${PARAM} < ${WORK}/samefile-L.out 2>&1
) | ${SED} -e 's/${srcdir}/./g' > ${OUTPUT}

${DIFF} -u ${CHECK} ${OUTPUT}
