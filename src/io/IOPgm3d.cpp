#include <io/IOPgm3d.h>
#include <QStringList>

//namespace io {

template <> QString IOPgm3d<char,qint32,true>::_header = QString( "PB" ) ;
template <> QString IOPgm3d<int32_t,qint32,true>::_header = QString( "PB" ) ;
template <> QString IOPgm3d<char,qint32,false>::_header = QString( "P8" ) ;
template <> QString IOPgm3d<int32_t,qint32,false>::_header = QString( "P8" ) ;

template <> QString IOPgm3d<char,float,true>::_header = QString( "PA" ) ;
template <> QString IOPgm3d<int32_t,float,true>::_header = QString( "PA" ) ;
template <> QString IOPgm3d<char,float,false>::_header = QString( "P9" ) ;
template <> QString IOPgm3d<int32_t,float,false>::_header = QString( "P9" ) ;

template <> QString IOPgm3d<char,double,true>::_header = QString( "PD" ) ;
template <> QString IOPgm3d<int32_t,double,true>::_header = QString( "PD" ) ;
template <> QString IOPgm3d<char,double,false>::_header = QString( "PC" ) ;
template <> QString IOPgm3d<int32_t,double,false>::_header = QString( "PC" ) ;

template <> QString IOPgm3d<char,char,true>::_header = QString( "P2" ) ;
template <> QString IOPgm3d<int32_t,char,true>::_header = QString( "P2" ) ;

template <> QString IOPgm3d<char,qint8,false>::_header = QString( "P5" ) ;
template <> QString IOPgm3d<int32_t,qint8,false>::_header = QString( "P5" ) ;

template <> QString IOPgm3d<char,qint16,true>::_header = QString( "PB" ) ;
template <> QString IOPgm3d<int32_t,qint16,true>::_header = QString( "PB" ) ;
template <> QString IOPgm3d<char,qint16,false>::_header = QString( "P8" ) ;

template <> QString IOPgm3d<int32_t,qint16,false>::_header = QString( "P8" ) ;

/*
template <> QString IOPgm3d<short,qint16,false>::_header = QString( "P8" ) ;
*/
template <typename To,typename T, bool tAscii> QString IOPgm3d<To,T,tAscii>::_header = "";

bool pgmheader( QFile &imageFile, std::map<int,QString> &description ) {
	if (!imageFile.open(QIODevice::ReadOnly)) {
		description[ PGM_HEADER_ERROR ] = QString("can not open file");
		return false;
	}

	int width, height, depth, nbColors;
	float voxelWidth, voxelHeight, voxelDepth;
	QString currentLine;

	//Lecture de l'entete (type de fichier)
	currentLine = imageFile.readLine();
	description[ PGM_HEADER_TYPE ] = currentLine.trimmed().toLower() ;

	QStringList dims;

	// Lecture des dimensions des voxels
	currentLine = imageFile.readLine();
	
	while ( currentLine.startsWith('#') ) {
		// Lecture des commentaires
		// Certaines informations peuvent y être stockées
		if ( currentLine.size() > 6 ) {
			if ( currentLine.startsWith("#xdim ") ) {
				dims = currentLine.split(" ",QString::SkipEmptyParts);
				dims.takeFirst();
				description[ PGM_HEADER_VOXEL_WIDTH ] = (QString)dims.takeFirst() ;
			} else if ( currentLine.startsWith("#ydim ") ) {
				dims = currentLine.split(" ",QString::SkipEmptyParts);
				dims.takeFirst();
				description[ PGM_HEADER_VOXEL_HEIGHT ] = (QString)dims.takeFirst();
			} else if ( currentLine.startsWith("#zdim ") ) {
				dims = currentLine.split(" ",QString::SkipEmptyParts);
				dims.takeFirst();
				description[ PGM_HEADER_VOXEL_DEPTH ] = (QString)dims.takeFirst();
			}
		}
		do {
			currentLine = imageFile.readLine();
		} while ( currentLine.trimmed().isEmpty() ) ;
	}
	//Lecture des dimensions
	dims = currentLine.split(" ",QString::SkipEmptyParts);
	if ( dims.size() != 3 && dims.size() != 2 ) {
		description[ PGM_HEADER_ERROR ] = QString("number of dimensions");;
		return false ;
	}
	description[ PGM_HEADER_IMAGE_DEPTH ] = QString("%1").arg(1) ;
	description[ PGM_HEADER_IMAGE_WIDTH ] = (QString)dims.takeFirst() ;
	description[ PGM_HEADER_IMAGE_HEIGHT ] = (QString)dims.takeFirst() ;
	if ( !dims.empty() )
		description[ PGM_HEADER_IMAGE_DEPTH ] = (QString)dims.takeFirst() ;

	//Lecture du nombre de couleurs
	currentLine = imageFile.readLine();
	description[ PGM_HEADER_NUMBER_OF_COLORS ] = currentLine ;
	return true ;
}


//}
