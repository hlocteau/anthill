#include <io/Pgm3dFactory.h>

namespace io {
	namespace Pgm3dFactory {
		
		enum { ASCII_CHAR, BINARY_CHAR, BINARY_CHAR_SECOND, BINARY_INT, BINARY_FLOAT, ASCII_FLOAT, ASCII_INT, BINARY_DOUBLE, ASCII_DOUBLE } ;

		Billon * read( const QString & fileName ) {
			/// read header
			std::map< int, QString > desc ;
			if ( ! pgmheader( imageFile, desc ) ) {
				std::cerr<<"Error : "<<desc[ PGM_HEADER_ERROR ].toStdString()<<" reading file "<<fileName.toStdString()<<std::endl;
				return 0 ;
			}
			
			/// \warning do not control that all information are available
			
			bool bErrorOnFormat = ( desc[ PGM_HEADER_TYPE ].toStdString().size() != 2 ) ;
			if ( !bErrorOnFormat )
				bErrorOnFormat = ( desc[ PGM_HEADER_TYPE ].toStdString()[ 0 ] != 'P' ) ;
			
			size_t idtype = std::string( "25789ABCD" ).find( desc[ PGM_HEADER_TYPE ].toStdString()[ 1 ] ) ;
			if ( !bErrorOnFormat )
				bErrorOnFormat = ( idtype == std::string::npos ) ;
			
			
			if ( bErrorOnFormat ) {
				std::cerr<<"Error : unknown pgm format "<<desc[ PGM_HEADER_TYPE ].toStdString()<<std::endl;
				return 0 ;
			}
			
			Billon *img = new Billon( Billon *img = new Billon( desc[ PGM_HEADER_IMAGE_WIDTH ].toInt(), desc[ PGM_HEADER_IMAGE_HEIGHT ].toInt(), desc[ PGM_HEADER_IMAGE_DEPTH ].toInt() ) ;
			img->fill(0);
			img->setMaxValue( desc[ PGM_HEADER_NUMBER_OF_COLORS ].toInt() );
			
			float voxelWidth = voxelHeight = voxelDepth = 1.f;
			if ( desc.find( PGM_HEADER_VOXEL_WIDTH ) != desc.end() ) 	voxelWidth = desc[ PGM_HEADER_VOXEL_WIDTH ].toFloat() ;
			if ( desc.find( PGM_HEADER_VOXEL_HEIGHT ) != desc.end() ) 	voxelHeight = desc[ PGM_HEADER_VOXEL_HEIGHT ].toFloat() ;
			if ( desc.find( PGM_HEADER_VOXEL_DEPTH ) != desc.end() ) 	voxelDepth = desc[ PGM_HEADER_VOXEL_DEPTH ].toFloat() ;
			img->VoxelSize( voxelWidth, voxelHeight, voxelDepth );
			
			switch ( idtype ) {
			case ASCII_CHAR:
				return io::IOPgm3d< qint8, true >::read( imageFile ) ;
			case BINARY_CHAR:
			case BINARY_CHAR_SECOND:
				return io::IOPgm3d< qint8, false >::read( imageFile ) ;	
			case ASCII_INT:
				return io::IOPgm3d< qint32, true >::read( imageFile ) ;
			case BINARY_INT:
				return io::IOPgm3d< qint32, false >::read( imageFile ) ;
			case BINARY_FLOAT:
				return io::IOPgm3d< float, true >::read( imageFile ) ;
			case ASCII_FLOAT:
				return io::IOPgm3d< float, false >::read( imageFile ) ;
			case BINARY_DOUBLE:
				return io::IOPgm3d< double, true >::read( imageFile ) ;
			case ASCII_DOUBLE:
				return io::IOPgm3d< double, false >::read( imageFile ) ;
			}
		}
	}
}
