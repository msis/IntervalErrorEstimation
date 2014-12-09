#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace ibex;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  tubeX = new TubeView("Pos X",this);
  tubeY = new TubeView("Pos Y",this);
  tubeA = new TubeView("Angle",this);

  tubeX->show();
  tubeY->show();
  tubeA->show();
}

MainWindow::~MainWindow()
{
  delete tubeA;
  delete tubeX;
  delete tubeY;
  delete ui;
}

void MainWindow::on_pushButton_start_clicked()
{
  tubeX->clear();
  tubeY->clear();
  tubeA->clear();

  double dt=ui->doubleSpinBox_dt->value();
  double time=ui->doubleSpinBox_time->value();
  int samples = time/dt;
  double dU1=5.0;
  double dU2=0.0;

  bool RK4 = ui->radioButton_RK4->isChecked();
  bool euler = ui->radioButton_euler->isChecked();

  IntervalVector iA(4), iB(4);
  Vector vA(4), vB(4);

  cout.precision(12);

  iA[0]=0;
  iB[0]=-15;
  iA[1]=10;
  iB[1]=-5;
  iA[2]=1.74;
  iB[2]=.52;
  iA[3]=dU1;
  iB[3]=dU2;
  vA[0]=0;
  vB[0]=-15;
  vA[1]=10;
  vB[1]=-5;
  vA[2]=1.74;
  vB[2]=.52;
  vA[3]=dU1;
  vB[3]=dU2;

  Variable A(4),B(4);

  Function f(A,Return(cos(A[2]),
             sin(A[2]),
      A[3],
      Interval(0.0)));
  Function fR(A,Return((Interval(-1)+cos(A[2])+dU1*A[1]),
      (sin(A[2])-dU1*A[0]),
      Interval(dU2-dU1)));

  Function dist(A,B,sqrt(sqr(A[0]-B[0])+sqr(A[1]-B[1])));
  Function posR(A,B,Return(cos(A[2])*(B[0]-A[0])+sin(A[2])*(B[1]-A[1]),
      -sin(A[2])*(B[0]-A[0])+cos(A[2])*(B[1]-A[1]),
      B[2]-A[2]));

  IntervalVector iAB(8);
  for (int i = 0; i < 4; ++i) {
      iAB[i]=iA[i];
      iAB[i+4]=iB[i];
    }
  Interval d = dist.eval(iAB);

  IntervalVector iB_A(3), iB_R(3);
  Vector vB_A(3), vB_R(3);
  vB_A[0]=vB[0];
  vB_A[1]=vB[1];
  vB_A[2]=vB[2];
  vB_R[0]=vB[0];
  vB_R[1]=vB[1];
  vB_R[2]=vB[2];

  iB_A = posR.eval_vector(iAB);
  iB_R = posR.eval_vector(iAB);

  QVector<QwtIntervalSample> xRK4(samples);
  QVector<QwtIntervalSample> yRK4(samples);
  QVector<QwtIntervalSample> aRK4(samples);
  QVector<QPointF> xRK4_p(samples);
  QVector<QPointF> yRK4_p(samples);
  QVector<QPointF> aRK4_p(samples);

  QVector<QwtIntervalSample> xEul(samples);
  QVector<QwtIntervalSample> yEul(samples);
  QVector<QwtIntervalSample> aEul(samples);
  QVector<QPointF> xEul_p(samples);
  QVector<QPointF> yEul_p(samples);
  QVector<QPointF> aEul_p(samples);


  QVector<QwtIntervalSample> xRel(samples);
  QVector<QwtIntervalSample> yRel(samples);
  QVector<QwtIntervalSample> aRel(samples);
  QVector<QPointF> xRel_p(samples);
  QVector<QPointF> yRel_p(samples);
  QVector<QPointF> aRel_p(samples);

  IntervalVector stateNoise(3,Interval(0).inflate(0.1));
  stateNoise[2]=Interval(0);


  for (int i = 0; i < samples; ++i)
    {
      cout << "d[" << i << "]=" << d << endl;
      cout << "iA[" << i << "]=" << iA << endl;
      cout << "iB[" << i << "]=" << iB << endl;
      cout << "iB_R[" << i << "]=" << iB_R << endl;
      cout << "iB_A[" << i << "]=" << iB_A << endl;

      if(euler){
          // Euler
          iA += dt*f.eval_vector(iA);// + stateNoise;
          iB += dt*f.eval_vector(iB);// + stateNoise;
          vA = vEvoEuler(vA,dt);
          vB = vEvoEuler(vB,dt);
        }
      if(RK4){
          //RK4
          IntervalVector k1=f.eval_vector(iA);
          IntervalVector k2=f.eval_vector(iA+(dt/2.)*k1);
          IntervalVector k3=f.eval_vector(iA+(dt/2.)*k2);
          IntervalVector k4=f.eval_vector(iA+dt*k3);
          iA += (dt/6.)*(k1+2.*k2+2.*k3+k4);
          k1 = f.eval_vector(iB);
          k2 = f.eval_vector(iB+(dt/2.)*k1);
          k3 = f.eval_vector(iB+(dt/2.)*k2);
          k4 = f.eval_vector(iB+dt*k3);
          iB += (dt/6.)*(k1+2.*k2+2.*k3+k4);
        }

      for (int j = 0; j < 4; ++j)
        {
          iAB[j]=iA[j];
          iAB[j+4]=iB[j];
        }
      d = dist.eval(iAB);


      iB_R = posR.eval_vector(iAB);
      xRel[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_R[0].lb(),iB_R[0].ub()));
      yRel[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_R[1].lb(),iB_R[1].ub()));
      aRel[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_R[2].lb(),iB_R[2].ub()));


      if(euler){
          // Euler
          iB_A += dt*fR.eval_vector(iB_A) + stateNoise;
          xEul[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[0].lb(),iB_A[0].ub()));
          yEul[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[1].lb(),iB_A[1].ub()));
          aEul[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[2].lb(),iB_A[2].ub()));
          vB_A = vEvoEulerR(vB_A,dt,dU1,dU2);
        }
      if(RK4){
          //RK4
          IntervalVector k1_r = fR.eval_vector(iB_A);
          IntervalVector k2_r = fR.eval_vector(iB_A+(dt/2.)*k1_r);
          IntervalVector k3_r = fR.eval_vector(iB_A+(dt/2.)*k2_r);
          IntervalVector k4_r = fR.eval_vector(iB_A+dt*k3_r);
          iB_A += (dt/6.)*(k1_r+2.*k2_r+2.*k3_r+k4_r);
          xRK4[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[0].lb(),iB_A[0].ub()));
          yRK4[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[1].lb(),iB_A[1].ub()));
          aRK4[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[2].lb(),iB_A[2].ub()));
        }

      IntervalVector E = iB_R-iB_A;
      cout << "E[" << i << "]=" << E << endl;

      cout << endl;
    }
  tubeX->insertTube("X Relative",xRel,Qt::white);
  tubeY->insertTube("Y Relative",yRel,Qt::white);
  tubeA->insertTube("A Relative",aRel,Qt::white);

  if(euler){
      tubeX->insertTube("X Euler",xEul,Qt::blue);
      tubeY->insertTube("Y Euler",yEul,Qt::blue);
      tubeA->insertTube("A Euler",aEul,Qt::blue);
    }
  if(RK4){
      tubeX->insertTube("X RK4",xRK4,Qt::blue);
      tubeY->insertTube("Y RK4",yRK4,Qt::blue);
      tubeA->insertTube("A RK4",aRK4,Qt::blue);
    }
}

IntervalVector MainWindow::iEvoEuler(IntervalVector vI, double dt)
{

}

IntervalVector MainWindow::iEvoRK4(IntervalVector vI, double dt)
{

}

Vector MainWindow::vEvoEuler(Vector v, double dt)
{
  Vector f(4);
  f[0]=cos(v[2]);
  f[1]=sin(v[2]);
  f[2]=v[3];
  f[3]=0.0;
  Vector r = v + dt*f;
  return r;
}

Vector MainWindow::vEvoRK4(Vector v, double dt)
{

}

IntervalVector MainWindow::iEvoEulerR(IntervalVector vI, double dt)
{

}

IntervalVector MainWindow::iEvoRK4R(IntervalVector vI, double dt)
{

}

Vector MainWindow::vEvoEulerR(Vector v, double dt, double du1, double du2)
{
  Vector f(3);
  f[0]=-1+cos(v[2])+du1*v[1];
  f[1]=sin(v[2])-du1*v[0];
  f[2]=du2-du1;
  Vector r = v + dt*f;
  return r;
}

Vector MainWindow::vEvoRK4R(Vector v, double dt, double du1, double du2)
{

}

Vector MainWindow::positionR(Vector A, Vector B)
{
  Vector r(3);

}
