#ifndef ANT_HILL_PROJECT_FILE_HEADER
#define ANT_HILL_PROJECT_FILE_HEADER

#include <QString>
#include <QMap>

#define ANTHILL_TAG_BILEVEL "bilevel"
#define ANTHILL_TAG_FEATURE "feature"
#define ANTHILL_TAG_LABEL   "label"

#define ANTHILL_OVERALL_INPUT_PROCESS_NAME "import"
#define ANTHILL_DEFAULT_OUTPUT_NAME        "opgm"
#define ANTHILL_SLICE_NAME                 "slice-%1.pgm"
#define ANTHILL_PRE_MASK_NAME              "premask.pgm"
#define ANTHILL_MASK_NAME                  "mask.pgm"

#define ANTHILL_SKEL_NAME                  "content.skeleucl.pgm3d"
#define ANTHILL_DT_HULL_NAME               "hull.dt.pgm3d"
#define ANTHILL_DT_CONTENT_NAME            "content.dt.pgm3d"
#define ANTHILL_INNER_SKEL_NAME            "inner.skel.pgm3d"
#define ANTHILL_INIT_SCENE_NAME            "content.pgm3d"

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
	QMap< QString, QMap< QString,QString > >::ConstIterator process_begin() const {
		return _process.begin() ;
	}
	QMap< QString, QMap< QString,QString > >::ConstIterator process_end() const {
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
