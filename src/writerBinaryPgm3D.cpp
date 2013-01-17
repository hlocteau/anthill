/**
 * \file writerBinaryPgm3D.cpp
 */
#include <writerBinaryPgm3D.h>
#include <billon.h>
#include <QFile>
#include <QDataStream>
#include <QTextStream>

IOPgm3d::IOPgm3d( ) {
}

bool IOPgm3d::write( const std::vector< DGtal::Z3i::Object6_18*> &objCC, QString fileName ) {
	assert( objCC.size()>1 ) ;
	uint 	width 	= objCC[1]->domain().upperBound().at(0),
			height 	= objCC[1]->domain().upperBound().at(1),
			depth	= objCC[1]->domain().upperBound().at(2) ;

	Billon allImage( width, height, depth );
	std::vector< DGtal::Z3i::Object6_18* >::const_iterator iter = objCC.begin() ;
	int nbComp=1;

	while( iter != objCC.end() ) {
		for ( DGtal::Z3i::DigitalSet::ConstIterator pt = (*iter)->pointSet().begin() ; pt != (*iter)->pointSet().end() ; pt++ ) {
			allImage( (*pt).at(0), (*pt).at(1), (*pt).at(2)) = nbComp;
		}
		iter++ ;
		nbComp++ ;
	}

	return write( allImage, fileName ) ;
}

bool IOPgm3d::write( const Billon &img, QString fileName ) {
	uint 	width 	= img.n_cols,
			height 	= img.n_rows,
			depth	= img.n_slices ;

	QFile file(fileName);
	if( !file.open(QFile::WriteOnly) ) {
		std::cerr << QString("ERREUR : %1 n'a pas pu être écrit.").arg(fileName).toStdString() << std::endl;
		return false;
	}
	QTextStream out(&file);
	out << "P3d\n";
	out << QString("%1 %2 %3").arg( width ).arg( height ).arg( depth ) << endl;
	out << QString("%1").arg( img.maxValue() ) << endl;
	QDataStream dout(&file);
	for ( int k=0 ; k< depth ; k++ ) {
		for ( int j=0 ; j< height ; j++ ) {
			for ( int i=0 ; i< width ; i++ ) {
				dout << (qint16) img(i,j,k);
			}
		}
	}
	file.close();
	return true ;
}
