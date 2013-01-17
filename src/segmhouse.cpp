#include "segmhouse.h"
#include "billon.h"
SegmHouse::SegmHouse( Billon *dicomserie ) : _rDicomSerie( dicomserie ), _pShapeSet( 0 ), _name( "undefined")
{
    Z3i::Point pt_low( 0,0,0),
            pt_up( _rDicomSerie->n_slices, _rDicomSerie->n_rows, _rDicomSerie->n_cols ) ;
    Z3i::Domain domain( pt_low, pt_up ) ;
    _pShapeSet = new Z3i::DigitalSet( domain ) ;
}
SegmHouse::~SegmHouse() {
    _rDicomSerie = 0 ;
    delete _pShapeSet ;
}
