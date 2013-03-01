BUILD = ${HOME}/bin
OBJ = obj
INC_ITK = -I/usr/local/include/ITK-4.2
INC_QT = -I/usr/include/qt4 -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtXml -I/usr/include/qt4/QtOpenGL -I/usr/include/qt4/QtGui
INC = -Iinclude ${INC_ITK} ${INC_QT}
COMPILE = g++ -O2 -c ${INC}
LIB_DGTAL = -lDGtalIO -lDGtal
LIB_QGLViewer = -lQGLViewer -lGLU
LIB_BOOST = -lboost_filesystem -lboost_system
LIB_BOOST_PARAM = -lboost_program_options
LIB_ITK = -lITKIOGDCM-4.2 -litkgdcmDICT-4.2 -litkgdcmMSFF-4.2 -litkgdcmIOD-4.2 -litkgdcmDSED-4.2 -litkzlib-4.2 -litkgdcmCommon-4.2 -litkgdcmuuid-4.2 -litkopenjpeg-4.2 -litkgdcmjpeg12-4.2 -litkgdcmjpeg16-4.2 -litkgdcmjpeg8-4.2 -lITKIOImageBase-4.2 -lITKCommon-4.2 -litksys-4.2 -litkvnl_algo-4.2 -litkv3p_netlib-4.2 -litkvnl-4.2 -lpthread -ldl



IS_64 = `gcc -dumpmachine | grep -c x86_64` # > /dev/null
LINK= g++ -O2 $(if ${IS_64},-Xlinker -zmuldefs)


# target files

${BUILD}/antHouse2D :  ${OBJ}/antHouse2D.o ${OBJ}/geom2d.o ${OBJ}/writerBinaryPgm3D.o
	${LINK} ${OBJ}/antHouse2D.o ${OBJ}/geom2d.o ${OBJ}/writerBinaryPgm3D.o -o "$@" ${LIB_DGTAL} ${LIB_QGLViewer} ${LIB_BOOST} -lQtCore

${BUILD}/antHouse3D :  ${OBJ}/antHouse3D.o ${OBJ}/geom2d.o  ${OBJ}/ccxExtractor.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/antHouse3D.o ${OBJ}/geom2d.o  ${OBJ}/ccxExtractor.o ${OBJ}/IOPgm3d.o -o "$@" ${LIB_DGTAL} ${LIB_QGLViewer} ${LIB_BOOST} -lQtCore -larmadillo

${BUILD}/decomp3DSkel : ${OBJ}/decomp3DSkel.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/decomp3DSkel.o ${OBJ}/IOPgm3d.o -o "$@" -lQtCore ${LIB_BOOST_PARAM}

${BUILD}/squareshuntthin : ${OBJ}/squareshuntthin.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/squareshuntthin.o ${OBJ}/IOPgm3d.o -o "$@" ${LIB_DGTAL} ${LIB_QGLViewer}

${BUILD}/consecutivecc : ${OBJ}/consecutivecc.o ${OBJ}/IOPgm3d.o ${OBJ}/utils.o
	${LINK} ${OBJ}/consecutivecc.o ${OBJ}/IOPgm3d.o ${OBJ}/utils.o -o "$@" ${LIB_DGTAL} -lQtCore ${LIB_BOOST_PARAM} -larmadillo

clean:
	rm ${OBJ}/*.o
# compilation files

${OBJ}/GatherFolderImg.o : src/io/GatherFolderImg.cpp
	${COMPILE} $< -o "$@"

${OBJ}/geom2d.o : src/geom2d.cpp
	${COMPILE} $< -o "$@"

${OBJ}/antHouse2D.o : src/antHouse2D.cpp
	${COMPILE} $< -o "$@"

${OBJ}/antHouse3D.o : src/antHouse3D.cpp
	${COMPILE} $< -o "$@"

${OBJ}/IOPgm3d.o : src/io/IOPgm3d.cpp
	${COMPILE} $< -o "$@"

${OBJ}/squareshuntthin.o : misc-test/squareshuntthin.cpp
	${COMPILE} $< -o "$@"

${OBJ}/utils.o : src/utils.cpp
	${COMPILE} $< -o "$@"

${OBJ}/consecutivecc.o : misc-test/consecutive_z_floors.cpp
	${COMPILE} $< -o "$@"

${OBJ}/IOUtils.o : src/io/IOUtils.cpp
	${COMPILE} $< -o "$@"

${OBJ}/rag.o : src/rag.cpp
	${COMPILE} -g $< -o "$@"
