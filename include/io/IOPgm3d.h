/**
 * \file BinaryPgm3D.h
 */
#ifndef WRITER_BINARY_PGM_3D_HEADER
#define WRITER_BINARY_PGM_3D_HEADER
#include <QString>
#include <QFile>
#include <def_billon.h>
#include <armadillo>
#include <map>
//namespace io {

enum { PGM_HEADER_IMAGE_WIDTH=0, PGM_HEADER_IMAGE_HEIGHT=1, PGM_HEADER_IMAGE_DEPTH=2, PGM_HEADER_VOXEL_WIDTH=3, PGM_HEADER_VOXEL_HEIGHT=4, PGM_HEADER_VOXEL_DEPTH=5, PGM_HEADER_NUMBER_OF_COLORS=6, PGM_HEADER_TYPE=7, PGM_HEADER_ERROR=8};
/**
 * \note may also be used for 2D pgm
 */
template <typename To, typename T, bool tAscii>
class IOPgm3d {
public:
	static const bool _sAscii = tAscii ;
	static bool write( const BillonTpl<To> &img, const QString &fileName, const QString &header=_header ) ;
	static bool write( const arma::Cube<To> &img, const QString &fileName, const QString &header=_header ) ;
	static BillonTpl<To> * read( const QString &fileName ) ;
	static void read( QFile &file, BillonTpl<To> * ) ;
	static void readAscii( QFile &file, BillonTpl<To> * ) ;
	static void readBinary( QFile &file, BillonTpl<To> * ) ;
	static QString _header ;
} ;

bool pgmheader( QFile &imageFile, std::map<int,QString> &description ) ;

#include <io/IOPgm3d.ih>
//}
#endif
