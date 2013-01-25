#include "sliceview.h"

#include "billon.h"
#include "globalfunctions.h"
//#include "slicealgorithm.h"

#include <QImage>
#include <QPainter>
#include <QVector2D>

#ifndef PROOF_COORD
#define PROOF_COORD(a,b) (b<0?0:(b>=a?a-1:b))
#endif

SliceView::SliceView(){
}

/*******************************
 * Public setters
 *******************************/

void SliceView::drawSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval, const int v )
{
	if ( sliceIndex < billon.n_slices )
	{
        drawCurrentSlice( image, billon, sliceIndex, intensityInterval, v );
	}
}

void SliceView::drawOverSlice( QImage &image, const BillonTpl<char> &segm, const uint &sliceIndex,int x_shift, int y_shift, int z_shift, bool content,const QRgb color  )
{
	if ( sliceIndex >= z_shift && sliceIndex < z_shift + segm.n_slices)
	{
	drawOverCurrentSlice( image, segm, sliceIndex,x_shift, y_shift, z_shift,content,color);
	}
}
/*******************************
 * Private functions
 *******************************/

void SliceView::drawCurrentSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval, const int v )
{
	const Slice &slice = billon.slice(sliceIndex);
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;
	const int minIntensity = intensityInterval.min();
    const qreal fact = 255.0/(intensityInterval.size()==0?1:intensityInterval.size());

	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j;

	for ( j=0 ; j<height ; j++)
	{
		for ( i=0 ; i<width ; i++)
		{
            color = (TKD::restrictedValue(slice.at(j,i),intensityInterval)-minIntensity)*fact;
            if ( v < intensityInterval.max() ) color = ( color > v ? 255 : 0 );
			*(line++) = qRgb(color,color,color);
		}
	}
}
void SliceView::drawOverCurrentSlice( QImage &image, const BillonTpl<char> &segm, const uint &sliceIndex,int x_shift, int y_shift, int z_shift, bool content, const QRgb color )
{
	QRgb * line = (QRgb *) image.bits();
	int x,y,shift;
	for ( shift = 0 ; shift < y_shift * image.width() ; shift++ )
		line++ ;
	for ( y = 0;y<segm.n_rows;y++ ) {
	    for ( shift=0;shift<x_shift ; shift++ )
			line++ ;
	    for ( x=0 ; x<segm.n_cols; x++) {
			if ( segm(y,x,sliceIndex-z_shift) != 0 ) {
				if ( content )
					*(line) = color;
				else {
					if ( segm(PROOF_COORD( segm.n_rows,y-1),x,sliceIndex-z_shift) == 0 ||
						segm(PROOF_COORD( segm.n_rows,y+1),x,sliceIndex-z_shift) == 0 ||
						segm(y,PROOF_COORD( segm.n_cols,x+1),sliceIndex-z_shift) == 0 ||
						segm(y,PROOF_COORD( segm.n_cols,x-1),sliceIndex-z_shift) == 0 )
						*(line) = color;
				}
			}
			line++ ;
		}
		for ( shift=0;shift<image.width()-segm.n_cols-x_shift;shift++) line++ ;
	}
}
