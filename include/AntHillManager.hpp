#ifndef ANT_HILL_PROJECT_MANAGER_HEADER
#define ANT_HILL_PROJECT_MANAGER_HEADER

#include <io/AntHillFile.hpp>
#include <QVector>
#include <def_billon.h>
#include <billon.h>
#include <interval.h>
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
	bool binarization( const Billon *data, const Interval<__billon_type__> &range, int th) ;
	QVector< QString >::ConstIterator series_begin() const {
		return _series.begin() ;
	}
	QVector< QString >::ConstIterator series_end() const {
		return _series.end() ;
	}
	
	fs::path defaultProjectLocation( ) const ;
	fs::path projectLocation( ) const {
		return _projectLocation ;
	}
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
	QList< BillonTpl< char > * > 	_im_s8 ;
	QList< BillonTpl< qint16 > * > 	_im_s16 ;
	QList< BillonTpl< quint16 > * > _im_u16 ;
    AntHillFile *_project ;
} ;

#endif
