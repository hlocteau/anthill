///
/// Compilation
/// g++ misc-test/number_encoding.cpp -o ~/bin/number_encoding -I/usr/include/qt4 -I/usr/include/qt4/QtCore -lQtCore
///

#include <QFile>
#include <QDataStream>
#include <armadillo>
#include <iomanip>

void writing( const char * ) ;
void reading( const char * ) ;

using std::setw ;

int main( int narg, char **argv ) {
	writing( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::hex | std::ios::showbase );
	reading( "/tmp/encoding.bin" ) ;
	std::cout.flags ( std::ios::dec );
	reading( "/tmp/encoding.bin" ) ;

	return 0 ;
}

#define width_value 11

void reading ( const char * filename ) {
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

void writing( const char * filename ) {
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
	aSignedChar = 128 ;			stream << (qint8) aSignedChar ;
	aSignedChar = 255 ;			stream << (qint8) aSignedChar ;
	
	/// char - 16 bits
	anUnsignedChar = 0 ;		stream << (qint16) anUnsignedChar ;
	anUnsignedChar = 127 ;		stream << (qint16) anUnsignedChar ;
	anUnsignedChar = 128 ;		stream << (qint16) anUnsignedChar ;
	anUnsignedChar = 255 ;		stream << (qint16) anUnsignedChar ;
	
	aSignedChar = 0 ;			stream << (qint16) aSignedChar ;
	aSignedChar = 127 ;			stream << (qint16) aSignedChar ;
	aSignedChar = 128 ;			stream << (qint16) aSignedChar ;
	aSignedChar = 255 ;			stream << (qint16) aSignedChar ;
	
	/// char - 32 bits
	anUnsignedChar = 0 ;		stream << (qint32) anUnsignedChar ;
	anUnsignedChar = 127 ;		stream << (qint32) anUnsignedChar ;
	anUnsignedChar = 128 ;		stream << (qint32) anUnsignedChar ;
	anUnsignedChar = 255 ;		stream << (qint32) anUnsignedChar ;
	
	aSignedChar = 0 ;			stream << (qint32) aSignedChar ;
	aSignedChar = 127 ;			stream << (qint32) aSignedChar ;
	aSignedChar = 128 ;			stream << (qint32) aSignedChar ;
	aSignedChar = 255 ;			stream << (qint32) aSignedChar ;





	/// short - 16 bis
	anUnsignedShort = 0 ;		stream << (qint16) anUnsignedShort ;
	anUnsignedShort = 32767 ;	stream << (qint16) anUnsignedShort ;
	anUnsignedShort = 32768 ;	stream << (qint16) anUnsignedShort ;
	anUnsignedShort = 65535 ;	stream << (qint16) anUnsignedShort ;

	aSignedShort = 0 ;			stream << (qint16) aSignedShort ;
	aSignedShort = 32767 ;		stream << (qint16) aSignedShort ;
	aSignedShort = 32768 ;		stream << (qint16) aSignedShort ;
	aSignedShort = 65535 ;		stream << (qint16) aSignedShort ;

	/// short - 32 bis
	anUnsignedShort = 0 ;		stream << (qint32) anUnsignedShort ;
	anUnsignedShort = 32767 ;	stream << (qint32) anUnsignedShort ;
	anUnsignedShort = 32768 ;	stream << (qint32) anUnsignedShort ;
	anUnsignedShort = 65535 ;	stream << (qint32) anUnsignedShort ;

	aSignedShort = 0 ;			stream << (qint32) aSignedShort ;
	aSignedShort = 32767 ;		stream << (qint32) aSignedShort ;
	aSignedShort = 32768 ;		stream << (qint32) aSignedShort ;
	aSignedShort = 65535 ;		stream << (qint32) aSignedShort ;

	/// int - 32 bits
	anUnsignedInt = 0 ;			stream << (qint32) anUnsignedInt ;
	anUnsignedInt = 2147483647 ;stream << (qint32) anUnsignedInt ;
	anUnsignedInt = 2147483648 ;stream << (qint32) anUnsignedInt ;
	anUnsignedInt = 4294967295 ;stream << (qint32) anUnsignedInt ;

	aSignedInt = 0 ;			stream << (qint32) aSignedInt ;
	aSignedInt = 2147483647 ;	stream << (qint32) aSignedInt ;
	aSignedInt = 2147483648 ;	stream << (qint32) aSignedInt ;
	aSignedInt = 4294967295 ;	stream << (qint32) aSignedInt ;
	file.close() ;
}
