#ifndef ANT_HILL_PROJECT_MANAGER_HEADER
#define ANT_HILL_PROJECT_MANAGER_HEADER

#include <io/AntHillFile.hpp>
#include <QVector>
#include <def_billon.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem ;

class AntHillManager {
public:
	AntHillManager() {_currentSeries=-1;_project = 0;};
	~AntHillManager();
	void importDicom( const QString &folderName ) ;
	bool load( const QString &name ) ;
	void openInitialInput( Billon ** img ) ;
	
	QVector< QString >::ConstIterator series_begin() const {
		return _series.begin() ;
	}
	QVector< QString >::ConstIterator series_end() const {
		return _series.end() ;
	}
	
	fs::path defaultProjectLocation( ) const ;
	void setFileName( const QString &filename ) ;
	AntHillFile * project() {
		return _project ;
	}
	
	void reset() ;
private:
	fs::path _projectLocation ;
	fs::path _filename ;
	QVector< QString > _series ;
	uint _currentSeries ;
	    
    AntHillFile *_project ;
} ;

#endif
