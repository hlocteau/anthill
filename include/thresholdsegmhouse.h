#ifndef THRESHOLDSEGMHOUSE_H
#define THRESHOLDSEGMHOUSE_H

#include "segmhouse.h"

class ThresholdSegmHouse : public SegmHouse
{
public:
    ThresholdSegmHouse( Billon * dicomSerie, const Interval<int> & range, int th );
    bool updateParamters( const Interval<int> & range, int value ) ;
protected:
    bool run();
private:
    Interval<int> _range ;
    int _threshold ;
};

#endif // THRESHOLDSEGMHOUSE_H
