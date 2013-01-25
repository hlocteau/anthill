#include <SignalMinMax.hpp>
#include <QFile>
#include <QStringList>




int main( int narg, char **argv ) {
	QList< int > L ;
	
	QFile file( argv[1] );
	if (!file.open(QIODevice::ReadOnly)) return -1 ;

	QStringList values;
	QString currentLine;
	
	while ( ! file.atEnd() ) {
		currentLine = file.readLine();
		values = currentLine.split(" ",QString::SkipEmptyParts);
		L.append( values.takeFirst().toInt() ) ;
		std::cout<<">> "<<L.back()<<std::endl;
	}
	file.close() ;
	
	QList< char > W ;
	uint radius = 2 ;
	
	QList< double > Lblur ;
	blur_signal( L, Lblur, 5 ) ;
	for ( uint i=0;i<Lblur.size();i++ ) std::cout<<i<<":"<<Lblur.at(i)<<std::endl;
	SignalMinMax< double, char,true > SMM( Lblur, radius, 35 ) ;
	QList<char> Lw ;
	SMM.result( Lw ) ;
	for ( uint i= 0;i<Lw.size();i++ ) std::cout<<i<<":"<<(int)Lw.at(i)<<" ";
	std::cout<<std::endl;
	
	
	return 0 ;
}
