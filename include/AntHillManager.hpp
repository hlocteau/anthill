#ifndef ANT_HILL_PROJECT_MANAGER_HEADER
#define ANT_HILL_PROJECT_MANAGER_HEADER
#include <stdint.h>
#include <io/AntHillFile.hpp>
#include <QVector>
#include <def_billon.h>
#include <billon.h>
#include <interval.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#define SIGNED_TINY_INT		1
#define UNSIGNED_TINY_INT	2
#define SIGNED_SHORT_INT	3
#define UNSIGNED_SHORT_INT	4
#define SIGNED_INT			5
#define UNSIGNED_INT		6

namespace fs = boost::filesystem ;

class AntHillManager {
public:
	enum { SLICING_ON_X, SLICING_ON_Y, SLICING_ON_Z };
	AntHillManager() {_currentSeries=-1;_project = 0;};
	~AntHillManager();
	void importDicom( const QString &folderName ) ;
	bool load( const QString &name ) ;
	void openInitialInput( Billon ** img ) ;
	bool binarization( const Interval<arma::s16> &range, int th) ;
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
	bool draw( const QString &resname, arma::Mat<uint8_t> &image, uint8_t axis, uint16_t coordinate, const Interval<int32_t> &range,bool normalize=true ) ;
	template <typename T> void draw( const BillonTpl< T > *data, arma::Mat<uint8_t> &image, uint8_t axis, uint16_t coordinate, const Interval<T> &range,bool normalize=true ) ;
	template <typename T> BillonTpl<T> * getRessourceByUID( const QString &uid, bool &valid ) ;
	void reset() ;
	
	bool isContentOnly           ( QMap< QString, QString >::ConstIterator &res ) const ;
	bool isColorSelectionAllowed ( QMap< QString, QString >::ConstIterator &res ) const ;
	
	QString uid ( QMap< QString, QMap< QString, QString> >::ConstIterator process,
					QMap< QString, QString >::ConstIterator param ) const {
		return QString("%1:%2").arg(process.key()).arg(param.key()) ;
	}
	void getOffset( QMap< QString, QString >::ConstIterator &res, uint &row, uint &col, uint &slice ) const ;
	void getSize( QMap< QString, QString >::ConstIterator &res, uint &n_rows, uint &n_cols, uint &n_slices ) const ;
	
	bool getSize( uint &n_rows, uint &n_cols, uint &n_slices ) ;
	template <typename T> bool getRange( Interval< T > &range ) ;
	template <typename T> bool getRange( const QString &resname, Interval< T > &range ) ;
	bool load_ressource( const QString &resname ) ;
private:
	typedef struct _TImageProperty {
		void*	_adr ;
		uint8_t _type ;
		uint8_t _meaning ;
	} TImageProperty ;
	
	fs::path _projectLocation ;
	fs::path _filename ;
	QVector< QString > _series ;
	uint _currentSeries ;
	QList< BillonTpl< arma::s8 > * >  _im_8s ;
	QList< BillonTpl< arma::u8 > * >  _im_8u ;
	QList< BillonTpl< arma::s16 > * > _im_16s ;
	QList< BillonTpl< arma::u16 > * > _im_16u ;
	QList< BillonTpl< arma::s32 > * > _im_32s ;
	QList< BillonTpl< arma::u32 > * > _im_32u ;
	QMap< QString, TImageProperty >   _prop ;
    AntHillFile *_project ;
} ;

template <typename T> bool AntHillManager::getRange( Interval< T > &range ) {
	QString main_input_name = QString("%1:%2").arg( ANTHILL_OVERALL_INPUT_PROCESS_NAME ).arg( ANTHILL_DEFAULT_OUTPUT_NAME ) ;
	return getRange<T>( main_input_name, range ) ;
}

template <typename T> bool AntHillManager::getRange( const QString &resname, Interval< T > &range ) {
	if ( !load_ressource( resname ) ) return false ;
	switch ( _prop[ resname ]._type ) {
	case SIGNED_TINY_INT :
		{
			BillonTpl< arma::s8 > *img = (BillonTpl< arma::s8 >*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case UNSIGNED_TINY_INT :
		{
			BillonTpl< arma::u8 >  *img = (BillonTpl< arma::u8>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case SIGNED_SHORT_INT :
		{
			BillonTpl< arma::s16 > *img = (BillonTpl< arma::s16>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case UNSIGNED_SHORT_INT :
		{
			BillonTpl< arma::u16 >  *img = (BillonTpl< arma::u16>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case SIGNED_INT :
		{
			BillonTpl< arma::s32 >  *img = (BillonTpl< arma::s32>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	case UNSIGNED_INT :
		{
			BillonTpl< arma::u32 >  *img = (BillonTpl< arma::u32>*) _prop[ resname ]._adr ;
			range.setBounds( img->min(), img->max() ) ;
		}
		break;
	default:
		std::cerr<<"[ Error ] : unmanaged type "<< _prop[ resname ]._type <<" for the project file wrt the ressource "<<resname.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
		return false;
	}
	return true ;
}

template <typename T> BillonTpl<T> * AntHillManager::getRessourceByUID( const QString &uid, bool &valid ) {
	if ( !_prop.contains( uid ) ) {
		if (!load_ressource( uid ) ) {
			valid = false ;
			return 0 ;
		}
	}
	valid = true ;
	return (BillonTpl<T>*) _prop[ uid ]._adr ;
}

template <typename T>
void AntHillManager::draw( const BillonTpl< T > *data, arma::Mat<uint8_t> &image, uint8_t axis, uint16_t coordinate, const Interval<T> &range,bool normalize ) {
	arma::span rows, cols, slices ;
	//std::cout<<"[ Debug ] : "<<__FUNCTION__<<" on "<<(void*)data<<std::endl;
	if ( axis == SLICING_ON_Y ) {
		rows = arma::span(coordinate) ;
		cols = arma::span(0,data->n_cols-1) ;
		slices = arma::span(0,data->n_slices-1) ;
	} else if ( axis == SLICING_ON_X ) {
		rows = arma::span(0,data->n_rows-1) ;
		cols = arma::span(coordinate) ;
		slices = arma::span(0,data->n_slices-1) ;
	} else {
		rows = arma::span(0,data->n_rows-1) ;
		cols = arma::span(0,data->n_cols-1) ;
		slices = arma::span(coordinate) ;
	}
	const typename arma::Cube< T > v_data = (*data)( rows,cols,slices ) ;
	typename arma::Cube< T >::const_iterator readIter = v_data.begin(),
									readEnd = v_data.end() ;
	arma::Mat<uint8_t>::iterator writeIter = image.begin();
	/** \warning
	 * QImage's data are stored row by row while cube's data are stored column by column!
	 * that is why we draw on a matrice that we can next transpose for display
	 */
	while ( readIter != readEnd ) {
		if ( range.containsClosed( *readIter ) ) {
			T value = *readIter ;
			if ( normalize )
				value = floor( ( ( value - range.min() ) * 255. ) / range.size() ) ;
			*writeIter = (uint8_t)value ;
		} else
			*writeIter = 0 ;
		readIter++ ;
		writeIter++;
	}
	//std::cout<<"[ Info ] : finish drawing view "<<coordinate<<" on axis "<<(axis==0?"x":(axis==1?"y":"z"))<<std::endl;
}
#endif
