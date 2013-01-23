#ifndef ANT_HILL_PROJECT_FILE_HEADER
#define ANT_HILL_PROJECT_FILE_HEADER

#include <QString>
#include <QMap>

class AntHillFile {
public:
	AntHillFile( const QString &, const QString &, const QString &, const QMap< QString, QString > & ) ;
	bool is_valid() const { return !_error ; }
private:
	bool _error ;
} ;

#endif
