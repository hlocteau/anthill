BUILD = ${HOME}/bin
OBJ = ../obj
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

${BUILD}/labelingcc : ${OBJ}/labelingcc.o ${OBJ}/IOPgm3d.o ${OBJ}/utils.o
	${LINK} ${OBJ}/labelingcc.o ${OBJ}/IOPgm3d.o ${OBJ}/utils.o -o "$@" -lQtCore ${LIB_BOOST_PARAM} -lDGtal

${BUILD}/view3dPgm : ${OBJ}/view3dPgm.o ${OBJ}/IOPgm3d.o ${OBJ}/utils.o
	${LINK} ${OBJ}/view3dPgm.o ${OBJ}/IOPgm3d.o ${OBJ}/utils.o -o "$@" ${LIB_DGTAL} ${LIB_QGLViewer} ${LIB_BOOST_PARAM}

${BUILD}/rebuildcc : ${OBJ}/rebuild.o ${OBJ}/IOPgm3d.o ${OBJ}/utils.o ${OBJ}/geom2d.o
	${LINK} ${OBJ}/rebuild.o ${OBJ}/IOPgm3d.o ${OBJ}/utils.o ${OBJ}/geom2d.o -o "$@" ${LIB_DGTAL} -lQtCore ${LIB_BOOST_PARAM} -larmadillo

${BUILD}/buildScene : ${OBJ}/buildScene.o ${OBJ}/utils.o ${OBJ}/geom2d.o ${OBJ}/GatherFolderImg.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/buildScene.o ${OBJ}/utils.o ${OBJ}/geom2d.o ${OBJ}/GatherFolderImg.o ${OBJ}/IOPgm3d.o -o "$@" ${LIB_DGTAL} ${LIB_QGLViewer} -larmadillo ${LIB_BOOST} -lQtCore

${BUILD}/innerSkelOnly : ${OBJ}/innerSkelOnly.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/innerSkelOnly.o ${OBJ}/IOPgm3d.o -o "$@" -lQtCore ${LIB_BOOST} ${LIB_BOOST_PARAM} ${LIB_DGTAL}

${BUILD}/innerScene : ${OBJ}/innerScene.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/innerScene.o ${OBJ}/IOPgm3d.o -o "$@" -lQtCore ${LIB_BOOST} ${LIB_BOOST_PARAM} ${LIB_DGTAL}

${BUILD}/minpath : ${OBJ}/minpath.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/minpath.o ${OBJ}/IOPgm3d.o -o "$@" -lQtCore -lDGtal ${LIB_BOOST}

${BUILD}/catchSkel : ${OBJ}/catchskel.o ${OBJ}/IOPgm3d.o
	${LINK} ${OBJ}/catchskel.o ${OBJ}/IOPgm3d.o -o "$@" -lQtCore ${LIB_BOOST} ${LIB_BOOST_PARAM} ${LIB_DGTAL}

${BUILD}/segSkelMinDepth : ${OBJ}/segSkelMinDepth.o ${OBJ}/IOPgm3d.o ${OBJ}/geom2d.o ${OBJ}/IOUtils.o ${OBJ}/rag.o
	${LINK} ${OBJ}/segSkelMinDepth.o ${OBJ}/IOPgm3d.o ${OBJ}/geom2d.o ${OBJ}/IOUtils.o ${OBJ}/rag.o -o "$@" -lQtCore ${LIB_BOOST} ${LIB_BOOST_PARAM} ${LIB_DGTAL} -larmadillo

clean:
	rm ${OBJ}/*.o
# compilation files

${OBJ}/innerScene.o : innerScene.cpp
	${COMPILE} $< -o "$@"

${OBJ}/segSkelMinDepth.o : segSkelMinDepth.cpp
	${COMPILE} $< -o "$@"

${OBJ}/catchskel.o : catchskel.cpp
	${COMPILE} $< -o "$@"

${OBJ}/minpath.o : minpath.cpp
	${COMPILE} $< -o "$@"

${OBJ}/innerSkelOnly.o : keepInnerSkelOnly.cpp
	${COMPILE} $< -o "$@"

${OBJ}/buildScene.o : buildScene.cpp
	${COMPILE} $< -o "$@"
	
${OBJ}/labelingcc.o : labelingcc.cpp
	${COMPILE} $< -o "$@"

${OBJ}/rebuild.o : rebuild.cpp
	${COMPILE} -DDEBUG_REMOVAL_EXT $< -o "$@"

${OBJ}/view3dPgm.o : view3dPgm.cpp
	${COMPILE} $< -o "$@"

.PHONY: ${OBJ}/rag.o ${OBJ}/GatherFolderImg.o ${OBJ}/IOPgm3d.o ${OBJ}/IOUtils.o ${OBJ}/geom2d.o ${OBJ}/rag.o ${OBJ}/utils.o

${OBJ}/rag.o : 
	cd .. && make obj/rag.o

${OBJ}/GatherFolderImg.o :
	cd .. && make obj/GatherFolderImg.o

${OBJ}/IOPgm3d.o :
	cd .. && make obj/IOPgm3d.o

${OBJ}/IOUtils.o :
	cd .. && make obj/IOUtils.o

${OBJ}/geom2d.o :
	cd .. && make obj/geom2d.o

${OBJ}/utils.o :
	cd .. && make obj/utils.o
