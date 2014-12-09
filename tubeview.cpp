#include "tubeview.h"
#include "ui_tubeview.h"

TubeView::TubeView(const QString &name, QWidget *parent) :
    QWidget(parent,Qt::Window),
    ui(new Ui::TubeView),
    m_mode(Tube)
{
    ui->setupUi(this);

    ui->comboBox_mode->addItem("Bars");
    ui->comboBox_mode->addItem("Tube");
    ui->comboBox_mode->setCurrentIndex(1);
    ui->comboBox_mode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed );
    connect(ui->comboBox_mode,SIGNAL(currentIndexChanged(int)),
            SLOT(setMode(int)));

    ui->qwtPlot_tube->setObjectName(name);
    ui->qwtPlot_tube->setTitle(name);

    ui->qwtPlot_tube->setAxisTitle(QwtPlot::xBottom, "Time");
    ui->qwtPlot_tube->setAxisTitle(QwtPlot::yRight, "Intervals");

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setPalette( Qt::darkGray );
    canvas->setBorderRadius( 10 );
    ui->qwtPlot_tube->setCanvas(canvas);

    ui->qwtPlot_tube->insertLegend(new QwtLegend(), QwtPlot::RightLegend);

    // LeftButton for the zooming
    // MidButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size
    QwtPlotZoomer* zoomer = new QwtPlotZoomer(canvas);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
        Qt::RightButton );

    QwtPlotPanner *panner = new QwtPlotPanner( canvas );
    panner->setMouseButton( Qt::MidButton );
}

TubeView::~TubeView()
{
    delete ui;
}

void TubeView::clear()
{
    ui->qwtPlot_tube->detachItems();
}

void TubeView::insertCurve(const QString &title,
                           const QVector<QPointF> &samples,
                           const QColor &color)
{
    d_curve = new QwtPlotCurve( title );
    d_curve->setRenderHint( QwtPlotItem::RenderAntialiased );
    d_curve->setStyle( QwtPlotCurve::NoCurve );
    d_curve->setLegendAttribute( QwtPlotCurve::LegendShowSymbol );

    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::XCross );
    symbol->setSize( 4 );
    symbol->setPen( color );
    d_curve->setSymbol( symbol );

    d_curve->setSamples( samples );
    d_curve->attach(ui->qwtPlot_tube);

    ui->qwtPlot_tube->replot();
}

void TubeView::insertTube(const QString &title,
                          const QVector<QwtIntervalSample> &samples,
                          const QColor &color)
{
    QwtPlotIntervalCurve* tubeCurve = new QwtPlotIntervalCurve( title );
    tubeCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
    tubeCurve->setPen( Qt::white );

    QColor bg( color );
    bg.setAlpha( 150 );
    tubeCurve->setBrush( QBrush( bg ) );
    tubeCurve->setStyle( QwtPlotIntervalCurve::Tube );

    tubeCurve->setSamples( samples );

    tubeCurve->attach(ui->qwtPlot_tube);

    ui->qwtPlot_tube->replot();
    ui->comboBox_mode->setCurrentIndex(1);
}

void TubeView::setMode(int style)
{
    m_mode=style;
    for(int i = 0;i<ui->qwtPlot_tube->itemList(QwtPlotItem::Rtti_PlotIntervalCurve).size();++i){
        QwtPlotIntervalCurve* t = (QwtPlotIntervalCurve *) ui->qwtPlot_tube->itemList(QwtPlotItem::Rtti_PlotIntervalCurve)[i];
        if(style==Tube){
            t->setStyle(QwtPlotIntervalCurve::Tube);
            t->setSymbol(NULL);
            t->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        } else {
            t->setStyle(QwtPlotIntervalCurve::NoCurve);

            QColor c( t->brush().color().rgb() ); // skip alpha
            QwtIntervalSymbol *errorBar =
                    new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
            errorBar->setWidth( 8 ); // should be something even
            errorBar->setPen( c );

            t->setSymbol( errorBar );
            t->setRenderHint( QwtPlotItem::RenderAntialiased, false );
        }
    }
    ui->qwtPlot_tube->replot();
}

void TubeView::on_pushButton_save_clicked()
{
    QwtPlotRenderer renderer;
    renderer.exportTo(ui->qwtPlot_tube,ui->qwtPlot_tube->title().text().append(".pdf"));
}
