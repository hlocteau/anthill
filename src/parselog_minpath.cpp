#include <QMap>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QFile>
#include <stdint.h>

#include <io/Pgm3dFactory.h>
#include <unistd.h>

int main( int narg, char **argv ) {
	char *logfile = argv[1] ;
	char *skelfile = argv[2] ;
	
	QMap< uint32_t,QList<uint32_t> > all_relations ;
	QList< QMap< uint32_t,QList<uint32_t> > > relations ;
	QList< QPair< uint32_t, uint32_t > > between ;

	Pgm3dFactory<short> factory ;
	BillonTpl<short> *pskel = factory.read( QString("%1").arg(skelfile) ) ;
	factory.correctEncoding( pskel ) ;
	int n_rows = pskel->n_rows,
		n_cols = pskel->n_cols,
		n_slices = pskel->n_slices;
	
	QString currentLine;
	
	QFile file( QString("%1").arg(logfile) );
	if (!file.open(QIODevice::ReadOnly)) {
		return -1;
	}
	
	currentLine = file.readLine();
	
	while ( !currentLine.startsWith( "separating seeds" ) ) {
		currentLine = file.readLine();
	}
	QStringList words, coords ;
	uint32_t s,t ;
	while ( currentLine.startsWith( "separating seeds" ) ) {
		words =	currentLine.split(" ",QString::SkipEmptyParts);
		s = words.at(2).toInt();
		t = words.at(4).toInt();
		relations.append( QMap< uint32_t,QList<uint32_t> >() ) ;
		between.push_back( QPair<uint32_t,uint32_t> ( s,t ) ) ;
		currentLine = file.readLine();
		while ( currentLine.startsWith( "remove") ) {
			words =	currentLine.split(" ",QString::SkipEmptyParts);
			std::string str = words.at(5).toStdString() ;
			str = str.substr( 1, str.length()-3);
			coords = QString( "%1").arg(str.c_str()).split(",",QString::SkipEmptyParts);
			s = ( coords.at(2).toInt() * n_rows + coords.at(1).toInt() ) * n_cols + coords.at(0).toInt();
			if ( s == 103600 ) {
				std::cerr<<"warning : get "<<s<<" from "<<coords.at(0).toInt()<<" / "<<coords.at(1).toInt()<<" / "<<coords.at(2).toInt()<<std::endl
											<<"table "<<coords.at(0).toStdString()<<" ; "<<coords.at(1).toStdString()<<" ; "<<coords.at(2).toStdString()<<std::endl
											<<"string "<<str<<std::endl
											<<"qstring "<<words.at(5).toStdString()<<std::endl;
			}
			str = words.at(6).toStdString() ;
			str = str.substr( 1, str.length()-3);
			coords = QString( "%1").arg(str.c_str()).split(",",QString::SkipEmptyParts);
			t = ( coords.at(2).toInt() * n_rows + coords.at(1).toInt() ) * n_cols + coords.at(0).toInt();
			if ( t == 103600 ) {
				std::cerr<<"warning : get "<<s<<" from "<<coords.at(0).toInt()<<" / "<<coords.at(1).toInt()<<" / "<<coords.at(2).toInt()<<std::endl
											<<"table "<<coords.at(0).toStdString()<<" ; "<<coords.at(1).toStdString()<<" ; "<<coords.at(2).toStdString()<<std::endl
											<<"string "<<str<<std::endl
											<<"qstring "<<words.at(6).toStdString()<<std::endl;
			}
			
			if ( !all_relations.contains( s ) ) all_relations.insert( s, QList<uint32_t>() ) ;
			all_relations[ s ].append( t ) ;
			if ( !all_relations.contains( t ) ) all_relations.insert( t, QList<uint32_t>() ) ;
			all_relations[ t ].append( s ) ;
			if ( !relations.back().contains( s ) ) relations.back().insert( s,QList<uint32_t>() ) ;
			relations.back()[ s ].append( t ) ;
			if ( !relations.back().contains( t ) ) relations.back().insert( t,QList<uint32_t>() ) ;
			relations.back()[ t ].append( s ) ;
			currentLine = file.readLine();
			if ( file.atEnd() ) break ;
		}
		if ( file.atEnd() ) break ;
	}
	file.close();
	std::cout<<between.size()<<" context"<<std::endl ;
	size_t num_context = between.size() ;
	for ( int iContext=0;iContext<num_context;iContext++ ) {
		std::cout<<relations.at(iContext).size()<<" relations"<<std::endl;
	}
	pskel->fill(0);
	for ( QMap< uint32_t,QList<uint32_t> >::ConstIterator it = all_relations.begin(); it != all_relations.end(); it++ ) {
		(*pskel)( (it.key() / n_cols) % n_rows,it.key() % n_cols,(it.key() / n_cols) / n_rows ) = 10;
		//if ( ( (it.key() / n_cols) / n_rows ) == 0 ) std::cerr<<it.key()<<" leads to 0z"<<std::endl;
	}
	
	IOPgm3d<short,qint8,false>::write( *pskel, QString("/tmp/loc.pgm3d"), IOPgm3d<int32_t,qint8,false>::_header ) ;
	
	
	std::cout<<getcwd(0,0)<<std::endl;
	
	delete pskel ;
	return 0 ;
}
