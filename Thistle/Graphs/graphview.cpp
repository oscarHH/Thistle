/*
This file is part of Thistle.
Thistle is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License.
Thistle is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
Lesser GNU General Public License for more details.
You should have received a copy of the Lesser GNU General Public License
along with Thistle.    If not, see <http://www.gnu.org/licenses/>.
Thistle    Copyright (C) 2013    Dimitry Ernot & Romha Korev
*/
#include "graphview.h"
#include <QScrollBar>

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <qmath.h>

#include "graphmodel.h"
#include "abstractgraphalgorithm.h"
#include "graphalgorithm.h"
#include "graphview_p.h"

#include "../kernel/itemdelegate.h"
static const qreal Pi = 3.14159265358979323846264338327950288419717;

namespace Thistle
{

GraphView::GraphView(QWidget *parent) : AbstractItemView( new GraphViewPrivate(), parent )
{
    Q_D( GraphView );
    setDragEnabled( true );
    ItemDelegate* delegate = new ItemDelegate( this );
    ItemStyle style = delegate->itemStyle();
    style.setShape( Thistle::Ellipse );
    delegate->setItemStyle( style );
    this->setItemDelegate( delegate );

    d->algorithm = new GraphAlgorithm( this );

    connect( d->algorithm, SIGNAL( updated() ), this, SLOT( update() ) );
    connect( d->algorithm, SIGNAL( updated() ), this->viewport(), SLOT( update() ) );

}

GraphView::~GraphView()
{
}



QModelIndex GraphView::indexAt( const QPoint& point ) const
{
    const Q_D( GraphView );
    QPoint p = point    + QPoint( horizontalOffset(), verticalOffset() );

    QAbstractItemModel* model = this->model();

    if ( model == 0 ) return QModelIndex();

    for (int row = 0; row < model->rowCount(); ++row )
    {
        for (int col = 0; col < model->columnCount(); ++col )
        {
            QModelIndex index = model->index( row, col );
            QPainterPath path = this->itemPath( index );
            if ( path.contains( p ) )
            {
                return index;
            }
        }
    }
    return QModelIndex();
}


QPainterPath GraphView::itemPath( const QModelIndex& index ) const
{
    const Q_D( GraphView );
    QPainterPath path;
    //const Node& node = d->itemPos.value( index );
    Node node = d->algorithm->node( index );
    if ( node.isNull() )
    {
        return path;
    }
    QPointF pos = node.pos();
    path.addRect( QRect(-20,-20,40,40).translated( pos.x(), pos.y() ) );
    return path;
}


GraphModel* GraphView::model() const
{
    const Q_D( GraphView );
    return d->model;
}

#if 0
void GraphView::mouseMoveEvent( QMouseEvent* event )
{
    Q_D( GraphView );
    QPoint p = QPoint( this->horizontalOffset(), this->verticalOffset() );
    if ( d->movableItem == true && d->movedItem.isValid() )
    {
        if ( !d->dragDropTime.isNull() )
        {
            QTime current = QTime::currentTime();
            if ( d->dragDropTime.msecsTo( current ) > 50 )
            {
                //d->itemPos[ d->movedItem ].setPos( event->pos() + p );

            }
        }
    }
    AbstractItemView::mouseMoveEvent( event );
}


void GraphView::mousePressEvent( QMouseEvent* event )
{
    Q_D( GraphView );
    if ( d->movableItem == true )
    {
        QModelIndex idx = this->indexAt( event->pos() );
        if ( idx.isValid() )
        {
            d->movedItem = idx;
            d->dragDropTime = QTime::currentTime();
        }
    }
    AbstractItemView::mousePressEvent( event );
}


void GraphView::mouseReleaseEvent( QMouseEvent* event )
{
    Q_D( GraphView );
    if ( d->movableItem == true )
    {
        if ( d->elasticItem == true )
        {
        }
        else
        {
            d->movedItem = QModelIndex();
        }
        d->dragDropTime = QTime();
    }
    AbstractItemView::mouseReleaseEvent( event );
}
#endif

void GraphView::paintEdges( QPainter& painter, const QPointF& offset ) const
{
    const Q_D( GraphView );
    Q_UNUSED( offset )
    painter.save();
    painter.setPen( QPen( QColor( Thistle::Colors::Gray ), 3 ) );
    painter.setBrush( QColor( Thistle::Colors::Gray ) );
    Q_FOREACH( Edge edge, d->model->edges() )
    {
        edge.paintEdge( painter, this->itemRect( edge.leftIndex ), this->itemRect( edge.rightIndex ) );
    }
    painter.restore();
}


void GraphView::paintEvent( QPaintEvent* /*event*/ )
{
    Q_D( GraphView );
    if ( this->model() == 0 )
    {
        return;
    }
    QPainter painter( viewport() );
    painter.translate( -this->horizontalOffset(), -this->verticalOffset() );
    painter.setRenderHint( QPainter::Antialiasing );
    this->paintEdges( painter );
    this->paintItems( painter );
}


void GraphView::paintItems( QPainter& painter, const QPointF& offset ) const
{
    const Q_D( GraphView );
    for (int r = 0; r < this->model()->rowCount(); ++r )
    {
        QModelIndex idx = this->model()->index( r, 0 );
        QStyleOptionViewItem option;
        option.rect = this->itemRect( idx ).translated( offset.x(), offset.y() ).toRect();
        this->itemDelegate()->paint( &painter, option, idx );
    }
}

void GraphView::setScrollBarValues()
{
    Q_D( GraphView );

    QSizeF realSize = d->boundingRect.size() + ( this->itemRect( this->model()->index( 0, 0 ) ).size() );;
    this->horizontalScrollBar()->setMinimum( d->boundingRect.x() - this->itemRect( this->model()->index( 0, 0 ) ).width() );
    this->verticalScrollBar()->setMinimum( d->boundingRect.y() - this->itemRect( this->model()->index( 0, 0 ) ).height() );
    this->horizontalScrollBar()->setMaximum( realSize.width() - this->width() + 50 );
    this->verticalScrollBar()->setMaximum( realSize.height() - this->height() + 50 );
}


void GraphView::setModel( GraphModel* model )
{
    Q_D( GraphView );
    d->model = model;
    AbstractItemView::setModel( model );
    connect( model, SIGNAL(updateEdges()), this, SLOT(updateValues()));
    AbstractItemView::setModel(model);
}


void GraphView::updateValues()
{
    Q_D( GraphView );
    d->algorithm->run();
}


AbstractGraphAlgorithm* GraphView::algorithm() const
{
    const Q_D( GraphView );
    return d->algorithm;
}

void GraphView::setAlgorithm( AbstractGraphAlgorithm* algorithm )
{
    Q_D ( GraphView );
    d->algorithm = algorithm;
    connect( d->algorithm, SIGNAL( updated() ), this, SLOT( update() ) );
    connect( d->algorithm, SIGNAL( updated() ), this->viewport(), SLOT( update() ) );
}

}