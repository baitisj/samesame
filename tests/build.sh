CP=cp
ECHO=echo
LN=ln
MKDIR=mkdir
RM=rm
SLEEP=sleep

WORK=${srcdir}/work
TEST1=${WORK}/test1
TEST2=${WORK}/test2
LIST1=${WORK}/test1.lst
LIST2=${WORK}/test2.lst
OUTPUT=${srcdir}/work/test.out

TEST1_FILE1A=${TEST1}/file1a
TEST1_FILE1B=${TEST1}/file1b
TEST1_FILE2A=${TEST1}/file2a
TEST1_FILE2B=${TEST1}/file2b
TEST1_FILE2C=${TEST1}/file2c
TEST1_FILE3A=${TEST1}/file3a
TEST2_FILE1A=${TEST2}/file1a
TEST2_FILE1B=${TEST2}/file1b
TEST2_FILE2A=${TEST2}/file2a
TEST2_FILE2B=${TEST2}/file2b
TEST2_FILE2C=${TEST2}/file2c
TEST2_FILE3A=${TEST2}/file3a

${RM} -rf ${TEST1} ${TEST2}
${MKDIR} ${TEST1} ${TEST2}
${ECHO} "1234567890" > ${OUTPUT}

				${CP} ${OUTPUT}	${TEST1_FILE1A}
${ECHO} -n '.';	${SLEEP} 1;	${CP} ${OUTPUT}	${TEST1_FILE3A}
${ECHO} -n '.';	${SLEEP} 1;	${CP} ${OUTPUT}	${TEST1_FILE2A}
${ECHO} -n '.';	${SLEEP} 1;	${LN} ${TEST1_FILE2A}	${TEST1_FILE2C}
				${LN} ${TEST1_FILE2A}	${TEST1_FILE2B}
				${LN} ${TEST1_FILE1A}	${TEST1_FILE1B}

${ECHO} "${TEST1_FILE1A}" > ${LIST1}
${ECHO} "${TEST1_FILE3A}" >> ${LIST1}
${ECHO} "${TEST1_FILE2A}" >> ${LIST1}
${ECHO} "${TEST1_FILE2C}" >> ${LIST1}
${ECHO} "${TEST1_FILE2B}" >> ${LIST1}
${ECHO} "${TEST1_FILE1B}" >> ${LIST1}

${ECHO} "${TEST1_FILE1A}" > ${LIST2}
${ECHO} "${TEST1_FILE3A}" >> ${LIST2}
${ECHO} "${TEST1_FILE2A}" >> ${LIST2}
${ECHO} "${TEST1_FILE2C}" >> ${LIST2}
${ECHO} "${TEST1_FILE2B}" >> ${LIST2}
${ECHO} "${TEST1_FILE1B}" >> ${LIST2}

				${CP} ${OUTPUT}	${TEST2_FILE1A}
${ECHO} -n '.';	${SLEEP} 1;	${CP} ${OUTPUT}	${TEST2_FILE3A}
${ECHO} -n '.';	${SLEEP} 1;	${CP} ${OUTPUT}	${TEST2_FILE2A}
${ECHO} -n '.';	${SLEEP} 1;	${CP} ${OUTPUT}	${TEST2_FILE2C}
${ECHO} -n '.';	${SLEEP} 1;	${CP} ${OUTPUT}	${TEST2_FILE2B}
${ECHO} -n '.';	${SLEEP} 1;	${CP} ${OUTPUT}	${TEST2_FILE1B}
${ECHO} '';	

${ECHO} "${TEST2_FILE1A}" >> ${LIST2}
${ECHO} "${TEST2_FILE3A}" >> ${LIST2}
${ECHO} "${TEST2_FILE2A}" >> ${LIST2}
${ECHO} "${TEST2_FILE2C}" >> ${LIST2}
${ECHO} "${TEST2_FILE2B}" >> ${LIST2}
${ECHO} "${TEST2_FILE1B}" >> ${LIST2}


# samefile -A:
(	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2A}	=	2	3"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE3A}	=	2	1"
) > ${WORK}/samefile-A.out

# samefile -Z:
(	${ECHO} "11	${TEST1_FILE3A}	${TEST1_FILE2A}	=	1	3"
	${ECHO} "11	${TEST1_FILE3A}	${TEST1_FILE1A}	=	1	2"
) > ${WORK}/samefile-Z.out

# samefile -At:
(	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE3A}	=	2	1"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2A}	=	2	3"
) > ${WORK}/samefile-At.out

# samefile -Zt:
(	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE3A}	=	3	1"
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE1A}	=	3	2"
) > ${WORK}/samefile-Zt.out

# samefile -L:
(	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE1A}	=	3	2"
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE3A}	=	3	1"
) > ${WORK}/samefile-L.out

# samefile -iA:
(	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2A}	=	2	3"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2B}	=	2	3"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2C}	=	2	3"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE3A}	=	2	1"
) > ${WORK}/samefile-iA.out

# samefile -iZ:
(	${ECHO} "11	${TEST1_FILE3A}	${TEST1_FILE2A}	=	1	3"
	${ECHO} "11	${TEST1_FILE3A}	${TEST1_FILE2B}	=	1	3"
	${ECHO} "11	${TEST1_FILE3A}	${TEST1_FILE2C}	=	1	3"
	${ECHO} "11	${TEST1_FILE3A}	${TEST1_FILE1A}	=	1	2"
	${ECHO} "11	${TEST1_FILE3A}	${TEST1_FILE1B}	=	1	2"
) > ${WORK}/samefile-iZ.out

# samefile -iAt:
(	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE3A}	=	2	1"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2A}	=	2	3"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2B}	=	2	3"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2C}	=	2	3"
) > ${WORK}/samefile-iAt.out

# samefile -iZt:
(	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE3A}	=	3	1"
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE1A}	=	3	2"
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE1B}	=	3	2"
) > ${WORK}/samefile-iZt.out

# samefile -iL:
(
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE1A}	=	3	2"
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE1B}	=	3	2"
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE1A}	=	3	1"
) > ${WORK}/samefile-iL.out

# samefile -ix:
(	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2A}	=	2	3"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2B}	=	2	3"
	${ECHO} "11	${TEST1_FILE1A}	${TEST1_FILE2C}	=	2	3"
	${ECHO} "11	${TEST1_FILE1B}	${TEST1_FILE2A}	=	2	3"
	${ECHO} "11	${TEST1_FILE1B}	${TEST1_FILE2B}	=	2	3"
	${ECHO} "11	${TEST1_FILE1B}	${TEST1_FILE2C}	=	2	3"
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE3A}	=	2	1"
	${ECHO} "11	${TEST1_FILE2B}	${TEST1_FILE3A}	=	2	1"
	${ECHO} "11	${TEST1_FILE2A}	${TEST1_FILE3A}	=	3	1"
	${ECHO} "11	${TEST1_FILE2B}	${TEST1_FILE3A}	=	3	1"
	${ECHO} "11	${TEST1_FILE2C}	${TEST1_FILE3A}	=	3	1"
) > ${WORK}/samefile-ix.out
