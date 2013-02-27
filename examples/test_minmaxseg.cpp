#include <SignalMinMax.hpp>
#include <QFile>
#include <QStringList>

#include <iomanip>


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
	}
	file.close() ;
	
	uint radius = 4 ;
	
	QList< double > Lblur ;
	blur_signal( L, Lblur, 5 ) ;
	SignalMinMax< double, char,true,false > SMM( Lblur, radius, 15 ) ;
	QList<char> Lw ;
	SMM.result( Lw ) ;
	std::cout<<"Index,Initial,Blurred,Fragment"<<std::endl;
	for ( uint i= 0;i<Lw.size();i++ )
		std::cout<<i<<","<<L.at(i)<<","<<std::setprecision(6)<<Lblur.at(i)<<","<<(int)Lw.at(i)<<std::endl;
	
	
	return 0 ;
}
