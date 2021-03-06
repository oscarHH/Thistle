#ifndef AXISCHART_P_H
#define AXISCHART_P_H

#include "abstractchart_p.h"
#include "Axis/abstractaxis.h"
#include "serieformat.h"

#include <QMap>

namespace Thistle
{
class AxisChart;
struct AxisChartPrivate : public AbstractChartPrivate
{
    AbstractAxis* axis;
    QMap<int, SerieFormat> style;
    int x;

    AxisChartPrivate( AxisChart* q );
};

}

#endif // AXISCHART_P_H
