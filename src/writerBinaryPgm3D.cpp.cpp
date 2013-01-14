/**
 * \file writerBinaryPgm3D.cpp
 */
#include <writerBinaryPgm3D.h>

bool IOPgm3d::write( const std::vector< Z3i::Object> > &objCC, QString fileName ) {
	assert( !objCC.empty() ) ;
	assert( dimensions() == 3 ) ;
	uint 	width 	= objCC[0].domain().upperBound().at(0),
			height 	= objCC[0].domain().upperBound().at(1),
			depth	= objCC[0].domain().upperBound().at(2) ;
	
	arma::icube allImage = arma::icube( width, height, depth );
	std::vector< Z3i::Object >::const_iterator iter = objCC.begin() ;
	int nbComp=0;

	while( iter != objCC.end() ) {
		for ( Z3i::DigitalSet::ConstIterator pt = (*iter).pointSet().begin() ; pt != (*iter).pointSet().end() ; pt++ ) {
			allImage( (*pt).at(0), (*pt).at(1), (*pt).at(1)) = nbComp;
		}
		iter++ ;
	}

	QFile file(fileName);
	if( !file.open(QFile::WriteOnly) ) {
		std::cerr << tr("ERREUR : %1 n'a pas pu être écrit.").arg(fileName).toStdString() << std::endl;
		return false;
	}
	QTextStream out(&file);
	out << "P3d\n";
	out << QString("%1 %2 %3").arg( width ).arg( height ).arg( depth ) << endl;
	out << QString("%1").arg( objCC.size() ) << endl;
	QDataStream dout(&file);
	for ( int k=0 ; k< depth ; k++ ) {
		for ( int j=0 ; j< height ; j++ ) {
			for ( int i=0 ; i< width ; i++ ) {
				dout << (qint16) allImage(i,j,k);
			}
		}
	}
	file.close();
	return true ;
}
