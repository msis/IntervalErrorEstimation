#ifndef TUBEVIEW_H
#define TUBEVIEW_H

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_scale_div.h>
#include <qwt_series_data.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_legend.h>
#include <qwt_interval_symbol.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_renderer.h>
#include <ibex.h>

namespace Ui {
class TubeView;
}

class TubeView : public QWidget
{
    Q_OBJECT
public:
    enum Mode
    {
        Bars,
        Tube
    };

public:
    explicit TubeView(const QString &name, QWidget *parent = 0);
    ~TubeView();


    void clear();

    void insertCurve( const QString &title,
                      const QVector<QPointF> &, const QColor & );

    void insertTube( const QString &title,
                     const QVector<QwtIntervalSample> &,
                     const QColor &color );

public Q_SLOTS:
    void setMode( int );


private slots:
    void on_pushButton_save_clicked();

private:
    Ui::TubeView *ui;

    int m_mode;

//    vector<QwtPlotIntervalCurve*> d_tubeCurve;
    QwtPlotCurve *d_curve;
};

#endif // TUBEVIEW_H
