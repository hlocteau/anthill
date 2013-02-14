///
/// Compilation
/// g++ misc-test/number_encoding.cpp -o ~/bin/number_encoding -I/usr/include/qt4 -I/usr/include/qt4/QtCore -lQtCore
///

#include <QFile>
#include <QDataStream>
#include <armadillo>
#include <iomanip>

void writingup( const char * ) ;
void readingup( const char * ) ;

void writingdown( const char * ) ;
void readingdown( const char * ) ;


using std::setw ;

/**
 * \note found at url : http://stackoverflow.com/questions/5123314/compile-time-checking-if-right-shift-is-arithmetic-on-signed-types
 */
template < typename T > bool is_signed() {
	T vp = -1 ;
	T vn = -1 ;
	vn = vn >> 1 ;
	return ( vp  == vn );
}

template < typename T > bool is_signed2() {
	return std::numeric_limits<T>::is_signed ;
}

template < typename SRC, typename DST > DST cast_integer( SRC value ) {
	uint size_t src_size = sizeof( SRC ) ;
	uint size_t dst_size = sizeof( DST ) ;
	if ( src_size == dst_size ) {
	
	} else if ( src_size > dst_size ) {
	
	} else {
	
	}
}

int main( int narg, char **argv ) {
	std::cout<<"u8 : "<<is_signed<arma::u8>()<<" "<<is_signed2<arma::u8>()<<std::endl ;
	std::cout<<"u16 : "<<is_signed<arma::u16>()<<" "<<is_signed2<arma::u16>()<<std::endl ;
	std::cout<<"u32 : "<<is_signed<arma::u32>()<<" "<<is_signed2<arma::u32>()<<std::endl ;
	std::cout<<"s8 : "<<is_signed<arma::s8>()<<" "<<is_signed2<arma::s8>()<<std::endl ;
	std::cout<<"s16 : "<<is_signed<arma::s16>()<<" "<<is_signed2<arma::s16>()<<std::endl ;
	std::cout<<"s32 : "<<is_signed<arma::s32>()<<" "<<is_signed2<arma::s32>()<<std::endl ;
	return 0 ;

	writingup( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::hex | std::ios::showbase );
	readingup( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::dec );
	readingup( "/tmp/encoding.bin" ) ;

	writingdown( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::hex | std::ios::showbase );
	readingdown( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::dec );
	readingdown( "/tmp/encoding.bin" ) ;

	return 0 ;
}

#define width_value 11

void readingdown( const char * filename ) {
	arma::u8 anUnsignedChar ;
	arma::s8 aSignedChar ;
	arma::u16 anUnsignedShort ;
	arma::s16 aSignedShort ;
	arma::u32 anUnsignedInt ;
	arma::s32 aSignedInt ;

	qint8 aQtChar ;
	qint16 aQtShort ;
	qint32 aQtInt ;

	QFile file( filename );
	file.open( QIODevice::ReadOnly ) ;
	QDataStream stream( &file ) ;
	uint isSigned, iSpecificValue, iType ;
	
	for ( isSigned = 0 ; isSigned < 2 ; isSigned++ )
		for ( iType = 0 ; iType < 2 ; iType++ )
			for ( iSpecificValue = 0 ; iSpecificValue < 5 ; iSpecificValue++ ) {
				stream >> aQtInt ;
				anUnsignedChar  = aQtInt ;
				aSignedChar     = aQtInt ;
				anUnsignedShort = aQtInt ;
				aSignedShort    = aQtInt ;
				anUnsignedInt   = aQtInt ;
				aSignedInt      = aQtInt ;
				std::cout<<"["<<setw(width_value)<<aQtInt<<"]["<<setw(width_value)<<anUnsignedInt<<"]["
																<<setw(width_value)<<aSignedInt<<"]["
																<<setw(width_value)<<anUnsignedShort<<"]["
																<<setw(width_value)<<aSignedShort<<"]";
				if ( iType == 0 ) std::cout						<<"["<<setw(width_value)<<anUnsignedChar<<"]["
																<<setw(width_value)<<aSignedChar<<"]";
				std::cout										<<std::endl;

				stream >> aQtShort ;
				anUnsignedChar  = aQtShort ;
				aSignedChar     = aQtShort ;
				anUnsignedShort = aQtShort ;
				aSignedShort    = aQtShort ;
				anUnsignedInt   = aQtShort ;
				aSignedInt      = aQtShort ;
				std::cout<<"["<<setw(width_value)<<aQtShort<<"]["<<setw(width_value)<<anUnsignedInt<<"]["
																<<setw(width_value)<<aSignedInt<<"]["
																<<setw(width_value)<<anUnsignedShort<<"]["
																<<setw(width_value)<<aSignedShort<<"]";
				if ( iType == 0 ) std::cout						<<"["<<setw(width_value)<<anUnsignedChar<<"]["
																<<setw(width_value)<<aSignedChar<<"]";
				std::cout										<<std::endl;
				
				if ( iType == 0 ) {
					stream >> aQtChar ;
					anUnsignedChar  = aQtChar ;
					aSignedChar     = aQtChar ;
					anUnsignedShort = aQtChar ;
					aSignedShort    = aQtChar ;
					anUnsignedInt   = aQtChar ;
					aSignedInt      = aQtChar ;
					std::cout<<"["<<setw(width_value)<<aQtChar<<"]["<<setw(width_value)<<anUnsignedInt<<"]["
																	<<setw(width_value)<<aSignedInt<<"]["
																	<<setw(width_value)<<anUnsignedShort<<"]["
																	<<setw(width_value)<<aSignedShort<<"]["
																	<<setw(width_value)<<anUnsignedChar<<"]["
																	<<setw(width_value)<<aSignedChar<<"]"<<std::endl;
				}
			}
	file.close();
}

void writingdown( const char * filename ) {
	arma::u16 anUnsignedShort ;
	arma::s16 aSignedShort ;
	arma::u32 anUnsignedInt ;
	arma::s32 aSignedInt ;

	QFile file( filename );
	file.open( QIODevice::WriteOnly ) ;
	QDataStream stream( &file ) ;

	aSignedInt = 0 ;		stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ;		stream << (qint8) aSignedInt ;
	aSignedInt = 127 ;		stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ; 	stream << (qint8) aSignedInt ;
	aSignedInt = -128 ;		stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ; 	stream << (qint8) aSignedInt ;
	aSignedInt = -1 ;		stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ; 	stream << (qint8) aSignedInt ;
	aSignedInt = 255 ;		stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ; 	stream << (qint8) aSignedInt ;

	aSignedInt = 256 ;		stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ;
	aSignedInt = 32766 ;	stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ;
	aSignedInt = -32766 ;	stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ;
	aSignedInt = 32767 ;	stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ;
	aSignedInt = -32767 ;	stream << (qint32) aSignedInt ;		stream << (qint16) aSignedInt ;

	
	anUnsignedInt = 0 ;		stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;	stream << (qint8) anUnsignedInt ;
	anUnsignedInt = 127 ;	stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;	stream << (qint8) anUnsignedInt ;
	anUnsignedInt = 128 ;	stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;	stream << (qint8) anUnsignedInt ;
	anUnsignedInt = 255 ;	stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;	stream << (qint8) anUnsignedInt ;
	anUnsignedInt = 1 ;		stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;	stream << (qint8) anUnsignedInt ;
	
	anUnsignedInt = 256 ;	stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;
	anUnsignedInt = 32766 ;	stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;
	anUnsignedInt = 32767 ;	stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;
	anUnsignedInt = 65534 ;	stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;
	anUnsignedInt = 65535 ;	stream << (qint32) anUnsignedInt ;	stream << (qint16) anUnsignedInt ;

	file.close();
}

void readingup ( const char * filename ) {
	arma::u8 anUnsignedChar ;
	arma::s8 aSignedChar ;
	arma::u16 anUnsignedShort ;
	arma::s16 aSignedShort ;
	arma::u32 anUnsignedInt ;
	arma::s32 aSignedInt ;

	qint8 aQtChar ;
	qint16 aQtShort ;
	qint32 aQtInt ;

	QFile file( filename );
	file.open( QIODevice::ReadOnly ) ;
	QDataStream stream( &file ) ;
	uint isSigned, iSpecificValue, iType ;
	
	for ( iType = 0 ; iType < 3 ; iType++ ) {
		if ( iType == 0 )
			for ( isSigned = 0 ; isSigned < 2 ; isSigned++ )
				for ( iSpecificValue = 0 ; iSpecificValue < 4 ; iSpecificValue++ ) {
					stream >> aQtChar ;
					anUnsignedChar  = aQtChar ;
					aSignedChar     = aQtChar ;
					anUnsignedShort = aQtChar ;
					aSignedShort    = aQtChar ;
					anUnsignedInt   = aQtChar ;
					aSignedInt      = aQtChar ;
					std::cout<<"["<<setw(width_value)<<aQtChar<<"]["	<<setw(width_value)<<anUnsignedInt<<"]["
																		<<setw(width_value)<<aSignedInt<<"]["
																		<<setw(width_value)<<anUnsignedShort<<"]["
																		<<setw(width_value)<<aSignedShort<<"]["
																		<<setw(width_value)<<anUnsignedChar<<"]["
																		<<setw(width_value)<<aSignedChar<<"]"<<std::endl;
				}
		if ( iType <= 1 )
			for ( isSigned = 0 ; isSigned < 2 ; isSigned++ )
				for ( iSpecificValue = 0 ; iSpecificValue < 4 ; iSpecificValue++ ) {
					stream >> aQtShort ;
					anUnsignedShort = aQtShort ;
					aSignedShort    = aQtShort ;
					anUnsignedInt   = aQtShort ;
					aSignedInt      = aQtShort ;
					std::cout<<"["<<setw(width_value)<<aQtShort<<"]["	<<setw(width_value)<<anUnsignedInt<<"]["
																		<<setw(width_value)<<aSignedInt<<"]["
																		<<setw(width_value)<<anUnsignedShort<<"]["
																		<<setw(width_value)<<aSignedShort<<"]"<<std::endl;
				}
		for ( isSigned = 0 ; isSigned < 2 ; isSigned++ )
			for ( iSpecificValue = 0 ; iSpecificValue < 4 ; iSpecificValue++ ) {
				stream >> aQtInt ;
				anUnsignedInt   = aQtInt ;
				aSignedInt      = aQtInt ;
				std::cout<<"["<<setw(width_value)<<aQtInt<<"]["		<<setw(width_value)<<anUnsignedInt<<"]["
																	<<setw(width_value)<<aSignedInt<<"]"<<std::endl;
			}
	}
	file.close() ;
}

void writingup( const char * filename ) {
	arma::u8 anUnsignedChar ;
	arma::s8 aSignedChar ;
	arma::u16 anUnsignedShort ;
	arma::s16 aSignedShort ;
	arma::u32 anUnsignedInt ;
	arma::s32 aSignedInt ;

	QFile file( filename );
	file.open( QIODevice::WriteOnly ) ;
	QDataStream stream( &file ) ;
	
	/// char - 8 bits
	anUnsignedChar = 0 ;		stream << (qint8) anUnsignedChar ;
	anUnsignedChar = 127 ;		stream << (qint8) anUnsignedChar ;
	anUnsignedChar = 128 ;		stream << (qint8) anUnsignedChar ;
	anUnsignedChar = 255 ;		stream << (qint8) anUnsignedChar ;

	aSignedChar = 0 ;			stream << (qint8) aSignedChar ;
	aSignedChar = 127 ;			stream << (qint8) aSignedChar ;
	aSignedChar = -128 ;		stream << (qint8) aSignedChar ;
	aSignedChar = -1 ;			stream << (qint8) aSignedChar ;
	
	/// char - 16 bits
	anUnsignedChar = 0 ;		stream << (qint16) anUnsignedChar ;
	anUnsignedChar = 127 ;		stream << (qint16) anUnsignedChar ;
	anUnsignedChar = 128 ;		stream << (qint16) anUnsignedChar ;
	anUnsignedChar = 255 ;		stream << (qint16) anUnsignedChar ;
	
	aSignedChar = 0 ;			stream << (qint16) aSignedChar ;
	aSignedChar = 127 ;			stream << (qint16) aSignedChar ;
	aSignedChar = -128 ;		stream << (qint16) aSignedChar ;
	aSignedChar = -1 ;			stream << (qint16) aSignedChar ;
	
	/// char - 32 bits
	anUnsignedChar = 0 ;		stream << (qint32) anUnsignedChar ;
	anUnsignedChar = 127 ;		stream << (qint32) anUnsignedChar ;
	anUnsignedChar = 128 ;		stream << (qint32) anUnsignedChar ;
	anUnsignedChar = 255 ;		stream << (qint32) anUnsignedChar ;
	
	aSignedChar = 0 ;			stream << (qint32) aSignedChar ;
	aSignedChar = 127 ;			stream << (qint32) aSignedChar ;
	aSignedChar = -128 ;		stream << (qint32) aSignedChar ;
	aSignedChar = -1 ;			stream << (qint32) aSignedChar ;





	/// short - 16 bis
	anUnsignedShort = 0 ;		stream << (qint16) anUnsignedShort ;
	anUnsignedShort = 32767 ;	stream << (qint16) anUnsignedShort ;
	anUnsignedShort = 32768 ;	stream << (qint16) anUnsignedShort ;
	anUnsignedShort = 65535 ;	stream << (qint16) anUnsignedShort ;

	aSignedShort = 0 ;			stream << (qint16) aSignedShort ;
	aSignedShort = 32767 ;		stream << (qint16) aSignedShort ;
	aSignedShort = -32768 ;		stream << (qint16) aSignedShort ;
	aSignedShort = -1 ;			stream << (qint16) aSignedShort ;

	/// short - 32 bis
	anUnsignedShort = 0 ;		stream << (qint32) anUnsignedShort ;
	anUnsignedShort = 32767 ;	stream << (qint32) anUnsignedShort ;
	anUnsignedShort = 32768 ;	stream << (qint32) anUnsignedShort ;
	anUnsignedShort = 65535 ;	stream << (qint32) anUnsignedShort ;

	aSignedShort = 0 ;			stream << (qint32) aSignedShort ;
	aSignedShort = 32767 ;		stream << (qint32) aSignedShort ;
	aSignedShort = -32768 ;		stream << (qint32) aSignedShort ;
	aSignedShort = -1 ;			stream << (qint32) aSignedShort ;

	/// int - 32 bits
	anUnsignedInt = 0 ;			stream << (qint32) anUnsignedInt ;
	anUnsignedInt = 2147483647 ;stream << (qint32) anUnsignedInt ;
	anUnsignedInt = 2147483648 ;stream << (qint32) anUnsignedInt ;
	anUnsignedInt = 4294967295 ;stream << (qint32) anUnsignedInt ;

	aSignedInt = 0 ;			stream << (qint32) aSignedInt ;
	aSignedInt = 2147483647 ;	stream << (qint32) aSignedInt ;
	aSignedInt = -2147483648 ;	stream << (qint32) aSignedInt ;
	aSignedInt = -1 ;			stream << (qint32) aSignedInt ;
	file.close() ;
}
