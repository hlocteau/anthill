#-------------------------------------------------
#
# Project created by QtCreator 2012-11-07T11:43:23
#
#-------------------------------------------------

# Version d'ITK installée : itk3 | itk4
ITK_VERSION = itk4
ITK_NUMBER =  4.2

QT       += core gui xml opengl
OBJECTS_DIR = obj
MOC_DIR = moc
TARGET = ~/bin/antHouse_gui
TEMPLATE = app
CONFIG		*=	$${ITK_VERSION}

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/dicomreader.cpp \
    src/sliceview.cpp \
    src/slicealgorithm.cpp \
    src/globalfunctions.cpp \
    src/segmhouse.cpp \
    src/geom2d.cpp \
    src/AntHillManager.cpp \
    src/io/AntHillFile.cpp \
    src/io/IOPgm3d.cpp

HEADERS  += include/mainwindow.h \
    include/dicomreader.h \
    include/interval.h \
    include/coordinate.h \
    include/billon.h \
    include/define.h \
    include/sliceview.h \
    include/segmhouse.h \
    include/geom2d.h \
    include/io/Pgm3dFactory.ih \
    include/io/Pgm3dFactory.h \
    include/io/AntHillFile.hpp \
    include/AntHillManager.hpp \
    include/io/IOPgm3d.ih

FORMS    += mainwindow.ui

LIBS *= -lblas -llapack -larmadillo -lQGLViewer -lGLU -lDGtalIO -lDGtal -lboost_filesystem -lboost_system

# ITK
#----#
itk3 {
        # SI ITK_VERSION = itk3
        #----------------------#
        ITK_PATH	 =	/usr/local/include/InsightToolkit/
        INCLUDEPATH	*=	$${ITK_PATH}/ \
                                        $${ITK_PATH}/IO/ \
                                        $${ITK_PATH}/Common/ \
                                        $${ITK_PATH}/gdcm/src/ \
                                        $${ITK_PATH}/Utilities/ \
                                        $${ITK_PATH}/Utilities/vxl/vcl/ \
                                        $${ITK_PATH}/Utilities/vxl/core/

        LIBS *=	-lITKIO \
                        -litkgdcm \
                        -litkjpeg8 \
                        -litktiff \
                        -lITKMetaIO \
                        -lITKNrrdIO \
                        -litkpng \
                        -litkzlib \
                        -lITKDICOMParser \
                        -litkjpeg12 \
                        -litkjpeg16 \
                        -litkopenjpeg \
                        -lITKniftiio \
                        -lITKznz \
                        -lITKCommon \
                        -litksys \
                        -litkvnl_algo \
                        -litkv3p_netlib \
                        -litkvnl
}
else:itk4 {
        # SI ITK_VERSION = itk4
        #----------------------#
        ITK_PATH	 =	/usr/local/include/ITK-$${ITK_NUMBER}/
        INCLUDEPATH	*=	$${ITK_PATH}/ include

        LIBS *=	-lITKIOGDCM-$${ITK_NUMBER} \
                                -litkgdcmDICT-$${ITK_NUMBER} \
                                -litkgdcmMSFF-$${ITK_NUMBER} \
                                        -litkgdcmIOD-$${ITK_NUMBER} \
                                        -litkgdcmDSED-$${ITK_NUMBER} \
                                                -litkzlib-$${ITK_NUMBER} \
                                        -litkgdcmCommon-$${ITK_NUMBER} \
                                        -litkgdcmuuid-$${ITK_NUMBER} \
                                        -litkopenjpeg-$${ITK_NUMBER} \
                                        -litkgdcmjpeg12-$${ITK_NUMBER} \
                                        -litkgdcmjpeg16-$${ITK_NUMBER} \
                                        -litkgdcmjpeg8-$${ITK_NUMBER} \
                                -lITKIOImageBase-$${ITK_NUMBER} \
                                        -lITKCommon-$${ITK_NUMBER} \
                                                -litksys-$${ITK_NUMBER} \
                                                -litkvnl_algo-$${ITK_NUMBER} \
                                                -litkv3p_netlib-$${ITK_NUMBER} \
                                                -litkvnl-$${ITK_NUMBER}

}
