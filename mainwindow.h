#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tubeview.h"
#include "ibex/ibex.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_pushButton_start_clicked();

private:
  Ui::MainWindow *ui;

  TubeView *tubeX,*tubeY,*tubeA;

  ibex::IntervalVector iEvoEuler(ibex::IntervalVector vI,double dt);
  ibex::IntervalVector iEvoRK4(ibex::IntervalVector vI,double dt);
  ibex::Vector vEvoEuler(ibex::Vector v,double dt);
  ibex::Vector vEvoRK4(ibex::Vector v,double dt);

  ibex::IntervalVector iEvoEulerR(ibex::IntervalVector vI,double dt);
  ibex::IntervalVector iEvoRK4R(ibex::IntervalVector vI,double dt);
  ibex::Vector vEvoEulerR(ibex::Vector v, double dt, double du1, double du2);
  ibex::Vector vEvoRK4R(ibex::Vector v,double dt, double du1, double du2);

  ibex::Vector positionR(ibex::Vector A,ibex::Vector B);
};

#endif // MAINWINDOW_H
