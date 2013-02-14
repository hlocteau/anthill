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
	size_t src_size = sizeof( SRC ) ;
	size_t dst_size = sizeof( DST ) ;
	if ( src_size == dst_size ) {
		return (DST) value ;
	} else if ( src_size < dst_size ) {
		DST cast_value = value ;
		if ( !std::numeric_limits<DST>::is_signed ) {
			DST mask = 0x00ff ;
			uint shift = src_size-1 ;
			while ( shift != 0 ) {
				mask = mask << 8 | mask ;
				shift-= 1 ;
			}
			cast_value = cast_value & mask ;
		}
		return cast_value ;
	} else {
		return (DST) value ; ///?
	}
}
template <typename T,typename TQT> void test_encoding( void *values, uint n_values ) {
	if      ( sizeof(T) == 1  ) std::cout<<"From  Char " ;
	else if ( sizeof(T) == 2 ) std::cout<<"From Short " ;
	else if ( sizeof(T) == 4 ) std::cout<<"From  Int  " ;

	if      ( sizeof(TQT) == 1  ) std::cout<<"via  Char " ;
	else if ( sizeof(TQT) == 2 ) std::cout<<"via Short " ;
	else if ( sizeof(TQT) == 4 ) std::cout<<"via  Int  " ;

std::cout<<std::endl;

	T v ;
	TQT vqt ;
	QFile fileOUT( QString("/tmp/enc%1.bin").arg((n_values/6)*8 ) );
	fileOUT.open( QIODevice::WriteOnly ) ;
	QDataStream streamOUT( &fileOUT ) ;
	for ( uint i_value = 0 ; i_value < n_values; i_value++ ) {
		//streamOUT << (TQT) ((T *) values)[ i_value ] ;
		streamOUT << cast_integer<T,TQT>( ((T *) values)[ i_value ] );
	}
	fileOUT.close();
	
	QFile fileIN( QString("/tmp/enc%1.bin").arg((n_values/6)*8 ) );
	fileIN.open( QIODevice::ReadOnly ) ;
	QDataStream streamIN( &fileIN ) ;
	for ( uint i_value = 0 ; i_value < n_values; i_value++ ) {
		streamIN >> vqt ;
		v=vqt;
		if ( std::numeric_limits<T>::is_signed ) {
			arma::s8 v8 = cast_integer<TQT,arma::s8>(v) ;
			arma::s16 v16 = cast_integer<TQT,arma::s16>(v) ;
			arma::s32 v32 = cast_integer<TQT,arma::s32>(v) ;
			std::cout<<(int)((T *) values)[ i_value ]<<" s8="<<(int)v8<<" s16="<<v16<<" s32="<<v32<<std::endl;
		} else {
			arma::u8 v8 = cast_integer<TQT,arma::u8>(v) ;
			arma::u16 v16 = cast_integer<TQT,arma::u16>(v) ;
			arma::u32 v32 = cast_integer<TQT,arma::u32>(v) ;
			std::cout<<(int)((T *) values)[ i_value ]<<" u8="<<(int)v8<<" u16="<<v16<<" u32="<<v32<<std::endl;
		}
	}
	fileIN.close();
}

int main( int narg, char **argv ) {
/*
	writingup( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::hex | std::ios::showbase );
	readingup( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::dec );
	readingup( "/tmp/encoding.bin" ) ;
*/
/*
	writingdown( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::hex | std::ios::showbase );
	readingdown( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::dec );
	readingdown( "/tmp/encoding.bin" ) ;
*/	
	char tc8[]  ={0x88,      0x7e,      0x7f,      0x80,      0x81,      0xff};
	
	short tc16[]={0x0088,    0x007e,    0x007f,    0x0080,    0x0081,    0x00ff,	
	              0x8800,    0x7ffe,    0x8000,    0x8001,    0xfffe,    0xffff};
	
	int tc32[]  ={0x00000088,0x0000007e,0x0000007f,0x00000080,0x00000081,0x000000ff,
	              0x00008800,0x00007ffe,0x00008000,0x00008001,0x0000fffe,0x0000ffff,
	              0x00880000,0x007ffffe,0x00800000,0x00800001,0x00fffffe,0x00ffffff,
	              0x88000000,0x7ffffffe,0x80000000,0x80000001,0xfffffffe,0xffffffff};
	
	test_encoding<arma::u8,qint8>( (void*)tc8, 6 ) ;
	test_encoding<arma::s8,qint8>( (void*)tc8, 6 ) ;
	test_encoding<arma::u8,qint16>( (void*)tc8, 6 ) ;
	test_encoding<arma::s8,qint16>( (void*)tc8, 6 ) ;
	test_encoding<arma::u8,qint32>( (void*)tc8, 6 ) ;
	test_encoding<arma::s8,qint32>( (void*)tc8, 6 ) ;

	test_encoding<arma::u16,qint8>( (void*)tc16, 6 ) ;
	test_encoding<arma::s16,qint8>( (void*)tc16, 6 ) ;
	test_encoding<arma::u16,qint16>( (void*)tc16, 6 ) ;
	test_encoding<arma::s16,qint16>( (void*)tc16, 6 ) ;
	test_encoding<arma::u16,qint32>( (void*)tc16, 6 ) ;
	test_encoding<arma::s16,qint32>( (void*)tc16, 6 ) ;

	test_encoding<arma::u16,qint16>( (void*)tc16, 6*2 ) ;
	test_encoding<arma::s16,qint16>( (void*)tc16, 6*2 ) ;
	test_encoding<arma::u16,qint32>( (void*)tc16, 6*2 ) ;
	test_encoding<arma::s16,qint32>( (void*)tc16, 6*2 ) ;

	test_encoding<arma::u32,qint8>( (void*)tc32, 6 ) ;
	test_encoding<arma::s32,qint8>( (void*)tc32, 6 ) ;
	test_encoding<arma::u32,qint16>( (void*)tc32, 6*2 ) ;
	test_encoding<arma::s32,qint16>( (void*)tc32, 6*2 ) ;
	test_encoding<arma::u32,qint32>( (void*)tc32, 6*4 ) ;
	test_encoding<arma::s32,qint32>( (void*)tc32, 6*4 ) ;
	
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
