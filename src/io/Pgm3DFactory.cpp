//#include <io/Pgm3dFactory.h>
#include <billon.h>
#include <QtCore/qendian.h>
#include <QtCore/QFile>
#include <io/IOPgm3d.h>

//namespace io {
	template <typename T> BillonTpl<T> * Pgm3dFactory<T>::read( const QString & fileName ) {
		std::map< int, QString > 	desc ;
		QFile 						imageFile(fileName);
		/// read header
		if ( ! pgmheader( imageFile, desc ) ) {
			std::cerr<<"Error : "<<desc[ PGM_HEADER_ERROR ].toStdString()<<" reading file "<<fileName.toStdString()<<std::endl;
			return 0 ;
		}
			
		/// \warning do not control that all information are available
			
		bool bErrorOnFormat = ( desc[ PGM_HEADER_TYPE ].toStdString().size() != 2 ) ;
		if ( !bErrorOnFormat )
			bErrorOnFormat = ( desc[ PGM_HEADER_TYPE ].toStdString()[ 0 ] != 'p' ) ;
		
		size_t idtype = std::string( "25789ABCD" ).find( desc[ PGM_HEADER_TYPE ].toStdString()[ 1 ] ) ;
		if ( !bErrorOnFormat )
			bErrorOnFormat = ( idtype == std::string::npos ) ;
			
		if ( bErrorOnFormat ) {
			std::cerr<<"Error : unknown pgm format ["<<desc[ PGM_HEADER_TYPE ].toStdString()[1]<<"]"<<std::endl;
			return 0 ;
		}
		if ( false && desc[ PGM_HEADER_NUMBER_OF_COLORS ].toInt() > std::numeric_limits<T>::max() ) {
			std::cerr<<"Error : encoding type of the image object is smaller than the maximum value of the file data."<<std::endl;
			return 0 ;
		}
		BillonTpl<T> *img = new BillonTpl<T>( desc[ PGM_HEADER_IMAGE_HEIGHT ].toInt(), desc[ PGM_HEADER_IMAGE_WIDTH ].toInt(), desc[ PGM_HEADER_IMAGE_DEPTH ].toInt() ) ;
		img->fill(0);
		img->setMaxValue( desc[ PGM_HEADER_NUMBER_OF_COLORS ].toInt() );
			
		float 	voxelWidth = 1.f,
				voxelHeight = 1.f,
				voxelDepth = 1.f;
		if ( desc.find( PGM_HEADER_VOXEL_WIDTH ) != desc.end() ) 	voxelWidth = desc[ PGM_HEADER_VOXEL_WIDTH ].toFloat() ;
		if ( desc.find( PGM_HEADER_VOXEL_HEIGHT ) != desc.end() ) 	voxelHeight = desc[ PGM_HEADER_VOXEL_HEIGHT ].toFloat() ;
		if ( desc.find( PGM_HEADER_VOXEL_DEPTH ) != desc.end() ) 	voxelDepth = desc[ PGM_HEADER_VOXEL_DEPTH ].toFloat() ;
		img->setVoxelSize( voxelWidth, voxelHeight, voxelDepth );
		switch ( idtype ) {
		case ASCII_CHAR:
			IOPgm3d< T, char, true >::readAscii( imageFile, img ) ;
			img->setMaxValue( 1 ) ;
			break;
		case BINARY_CHAR:
		case BINARY_CHAR_SECOND:
			IOPgm3d< T, qint8, false >::readBinary( imageFile, img ) ;
			img->setMaxValue( 1 ) ;
			break;
		case ASCII_INT:
			IOPgm3d< T, qint32, true >::readAscii( imageFile, img ) ;
			break;
		case BINARY_INT:
			IOPgm3d< T, qint32, false >::readBinary( imageFile, img ) ;
			break;
		case BINARY_FLOAT:
			IOPgm3d< T, float, true >::readBinary( imageFile, img ) ;
			break;
		case ASCII_FLOAT:
			IOPgm3d< T, float, false >::readAscii( imageFile, img ) ;
			break;
		case BINARY_DOUBLE:
			IOPgm3d< T, double, true >::readBinary( imageFile, img ) ;
			break;
		case ASCII_DOUBLE:
			IOPgm3d< T, double, false >::readAscii( imageFile, img ) ;
			break;
		}
		_lasttype = (TEncoding)idtype ;
		return img ;
	}
		
	template <typename T> void Pgm3dFactory<T>::correctEncoding( BillonTpl<T> * img ) {
		switch ( _lasttype ) {
			case BINARY_CHAR:
			case BINARY_CHAR_SECOND:
				*img *= -1 ;
				break ;
			case BINARY_INT:
				uint32_t max_value = 0 ;
				for ( uint z=0;z<img->n_slices;z++)
					for ( uint y=0;y<img->n_rows;y++)
						for ( uint x=0;x<img->n_cols;x++) {
							int32_t value = (int32_t) ( img->at(y,x,z) );
							img->at(y,x,z) = qbswap<qint32>( value ) ; //qFromBigEndian< qint32 >( (const uchar*)( &value ) ) ;
							if ( max_value < img->at(y,x,z) ) max_value = img->at(y,x,z) ;
						}
				img->setMaxValue( max_value ) ;
				break;
			default:
				std::cerr<<"Warning : No change for type "<<_lasttype<<" ("<<__FUNCTION__<<")"<<std::endl;
			}
		}
//}
