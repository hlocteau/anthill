BUILD = ${HOME}/bin
OBJ = ../obj
SRC = ../src
INC_ITK = -I/usr/local/include/ITK-4.2
INC_QT = -I/usr/include/qt4 -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtXml -I/usr/include/qt4/QtOpenGL -I/usr/include/qt4/QtGui
INC = -I../include ${INC_ITK} ${INC_QT}
COMPILE = g++ -O2 -c ${INC}
LIB_DGTAL = -lDGtalIO -lDGtal
LIB_QGLViewer = -lQGLViewer -lGLU
LIB_BOOST = -lboost_filesystem -lboost_system
LIB_BOOST_PARAM = -lboost_program_options
LIB_ITK = -lITKIOGDCM-4.2 -litkgdcmDICT-4.2 -litkgdcmMSFF-4.2 -litkgdcmIOD-4.2 -litkgdcmDSED-4.2 -litkzlib-4.2 -litkgdcmCommon-4.2 -litkgdcmuuid-4.2 -litkopenjpeg-4.2 -litkgdcmjpeg12-4.2 -litkgdcmjpeg16-4.2 -litkgdcmjpeg8-4.2 -lITKIOImageBase-4.2 -lITKCommon-4.2 -litksys-4.2 -litkvnl_algo-4.2 -litkv3p_netlib-4.2 -litkvnl-4.2 -lpthread -ldl



IS_64 = `gcc -dumpmachine | grep -c x86_64` # > /dev/null
LINK= g++ -O2 $(if ${IS_64},-Xlinker -zmuldefs)


# target files
${BUILD}/test_boundVol : ${OBJ}/test_boundVol.o ${OBJ}/geom2d.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/test_boundVol.o ${OBJ}/geom2d.o ${OBJ}/IOPgm3d.o -o "$@" ${LIB_DGTAL} ${LIB_QGLViewer} -larmadillo ${LIB_BOOST_PARAM}

${BUILD}/test_minmax : ${OBJ}/test_minmax.o
	${LINK} ${OBJ}/test_minmax.o -o "$@" -lQtCore

${BUILD}/test_rag : ${OBJ}/test_rag.o ${OBJ}/rag.o ${OBJ}/IOPgm3d.o
	${LINK} -g ${OBJ}/test_rag.o ${OBJ}/rag.o ${OBJ}/IOPgm3d.o -o "$@" -lQtCore ${LIB_DGTAL}

${BUILD}/pgmcrop : ${OBJ}/pgmcrop.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/pgmcrop.o ${OBJ}/IOPgm3d.o -o "$@" -lQtCore ${LIB_BOOST} ${LIB_BOOST_PARAM}

# compilation files

${OBJ}/test_boundVol.o : test_boundvol.cpp
	${COMPILE} $< -o "$@"

${OBJ}/test_minmax.o : test_minmaxseg.cpp
	${COMPILE} $< -o "$@"

${OBJ}/test_rag.o : test_rag.cpp
	${COMPILE} $< -o "$@"

${OBJ}/pgmcrop.o : pgmcrop.cpp
	${COMPILE} $< -o "$@"

.PHONY: ${OBJ}/rag.o ${OBJ}/IOPgm3d.o ${OBJ}/geom2d.o

${OBJ}/rag.o : 
	cd .. && make obj/rag.o

${OBJ}/IOPgm3d.o : 
	cd .. && make obj/IOPgm3d.o

${OBJ}/geom2d.o : 
	cd .. && make obj/geom2d.o
