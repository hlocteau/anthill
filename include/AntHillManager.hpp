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


namespace fs = boost::filesystem ;

class AntHillManager {
public:
	enum { SLICING_ON_X, SLICING_ON_Y, SLICING_ON_Z };
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
	bool draw( const QString &resname, arma::Mat<uint8_t> &image, uint8_t axis, uint16_t coordinate, const Interval<int32_t> &range,bool normalize=true ) ;
	template <typename T> void draw( const BillonTpl< T > *data, arma::Mat<uint8_t> &image, uint8_t axis, uint16_t coordinate, const Interval<T> &range,bool normalize=true ) ;
	
	void reset() ;
	
	bool isContentOnly           ( QMap< QString, QString >::ConstIterator &res ) const ;
	bool isColorSelectionAllowed ( QMap< QString, QString >::ConstIterator &res ) const ;
	
	QString uid ( QMap< QString, QMap< QString, QString> >::ConstIterator process,
					QMap< QString, QString >::ConstIterator param ) const {
		return QString("%1:%2").arg(process.key()).arg(param.key()) ;
	}
	void getOffset( QMap< QString, QString >::ConstIterator &res, uint &row, uint &col, uint &slice ) const ;
	void getSize( QMap< QString, QString >::ConstIterator &res, uint &n_rows, uint &n_cols, uint &n_slices ) const ;
	
	void getSize( uint &n_rows, uint &n_cols, uint &n_slices ) const ;
	void getRange( Interval< arma::u16 > &range ) const ;
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


template <typename T>
void AntHillManager::draw( const BillonTpl< T > *data, arma::Mat<uint8_t> &image, uint8_t axis, uint16_t coordinate, const Interval<T> &range,bool normalize ) {
	arma::span rows, cols, slices ;
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
