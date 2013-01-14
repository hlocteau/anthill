#include "thresholdsegmhouse.h"
#include <iostream>
#include "billon.h"
ThresholdSegmHouse::ThresholdSegmHouse( Billon * dicomSerie, const Interval<int> & range, int th ) : SegmHouse( dicomSerie )
{
    _name = "Basic Threshold";
    _range = range ;
    _threshold = th;

    /// apply segmentation scheme
    if ( ! run() ) {
        std::cerr<<"Error while segmenting the data using method "<< name().toStdString() <<std::endl;
    }
}

bool ThresholdSegmHouse::run() {
    /// \todo use find function
    std::cout<<"call "<<__FUNCTION__<<" of "<<name().toStdString()<<std::endl;
    for ( uint k=0; k<_rDicomSerie->n_slices ; k++ )
    {
        Slice &slice = _rDicomSerie->slice(k);
        /// iterate through pixels of layers
        for ( uint j=0; j< _rDicomSerie->n_cols; j++ )
        {
            for ( uint i=0; i< _rDicomSerie->n_rows ; i++ )
            {
                /// add each pixel that get value higher than the given value
                if ( _range.containsClosed( slice.at(j,i) ) )
                {
                    if ( ( (255*(slice.at(j,i) - _range.min() ) ) / _range.size() ) >= _threshold )
                    {
                        _pShapeSet->insertNew( Z3i::Point( i, j, k) ) ;
                    }
                }
            }
        }
    }
    return !_pShapeSet->empty() ;
}

bool ThresholdSegmHouse::updateParamters( const Interval<int> &range, int value ) {
    _pShapeSet->clear() ;
    _range = range ;
    _threshold = value ;
    return run() ;
}
