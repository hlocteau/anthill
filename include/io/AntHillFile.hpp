#ifndef ANT_HILL_PROJECT_FILE_HEADER
#define ANT_HILL_PROJECT_FILE_HEADER

#include <QString>
#include <QMap>

class AntHillFile {
public:
	AntHillFile( ) { _error = false ;}
	AntHillFile( const QString &, const QString &, const QString &, const QMap< QString, QString > & ) ;
	bool is_valid() const { return !_error ; }
	
	void setDicomFolder( const QString &dicomFolder ) {
		_dicomFolder = dicomFolder ;
	}
	const QString &dicomFolder( ) const {
		return _dicomFolder ;
	}
	void setUID( const QString &uid ) {
		_dicomUID = uid ;
	}
	const QString & uid( ) const {
		return _dicomUID ;
	}
	
	bool load( const QString & ) ;
	bool save( const QString & ) ;
	
	void setDictionary( const QMap< QString, QString > & dictionary) {
		_dictionary = dictionary ;
	}
	const QMap< QString, QString > & dictionary( ) const {
		return _dictionary ;
	}
	
	bool addProcess( const QString &, const QMap< QString, QString > & ) ;
	QMap< QString, QMap< QString,QString > >::const_iterator process_begin() const {
		return _process.begin() ;
	}
	QMap< QString, QMap< QString,QString > >::const_iterator process_end() const {
		return _process.end() ;
	}
private:
	bool _error ;
	QMap< QString, QString > _dictionary ;
	QString _dicomFolder ;
	QString _dicomUID ;
	QMap< QString, QMap< QString,QString > > _process ;
} ;

#endif
