#include "globalfunctions.h"

#include "interval.h"

#include <QtGlobal>

namespace TKD
{
	__billon_type__ restrictedValue( __billon_type__ value , const Interval<int> &intensityInterval )
	{
		return intensityInterval.containsOpen(value) ? value : intensityInterval.min();
	}
}
