#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace ibex;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  double dU1=ui->doubleSpinBox_uA->value();
  double dU2=ui->doubleSpinBox_uB->value();

  Variable A(4),B(4);
  m_f = new Function(A,
                     Return(cos(A[2]),
                            sin(A[2]),
                            A[3],
                            Interval(0.0)));
  m_fR = new Function(A,
                      Return((Interval(-1)+cos(A[2])+dU1*A[1]),
                             (sin(A[2])-dU1*A[0]),
                             Interval(dU2-dU1)));
  m_dist = new Function(A,B,
                        sqrt(sqr(A[0]-B[0])+sqr(A[1]-B[1])));
  m_posR = new Function(A,B,
                        Return(cos(A[2])*(B[0]-A[0])+sin(A[2])*(B[1]-A[1]),
                               -sin(A[2])*(B[0]-A[0])+cos(A[2])*(B[1]-A[1]),
                               B[2]-A[2]));

  tubeX = new TubeView("Pos X",this);
  tubeY = new TubeView("Pos Y",this);
  tubeA = new TubeView("Angle",this);
  tubeE = new TubeView("Error",this);

  tubeX->show();
  tubeY->show();
  tubeA->show();
  tubeE->show();
}

MainWindow::~MainWindow()
{
  delete tubeA;
  delete tubeX;
  delete tubeY;
  delete tubeE;

  delete m_f;
  delete m_fR;
  delete m_dist;
  delete m_posR;

  delete ui;
}

void MainWindow::on_pushButton_start_clicked()
{
  tubeX->clear();
  tubeY->clear();
  tubeA->clear();
  tubeE->clear();

  double dU1=ui->doubleSpinBox_uA->value();
  double dU2=ui->doubleSpinBox_uB->value();

  {
    delete m_f;
    delete m_fR;
    delete m_dist;
    delete m_posR;

    Variable A(4),B(4);
    m_f = new Function(A,
                       Return(cos(A[2]),
                              sin(A[2]),
                              A[3],
                              Interval(0.0)));
    m_fR = new Function(A,
                        Return((Interval(-1)+cos(A[2])+dU1*A[1]),
                               (sin(A[2])-dU1*A[0]),
                               Interval(dU2-dU1)));
    m_dist = new Function(A,B,
                          sqrt(sqr(A[0]-B[0])+sqr(A[1]-B[1])));
    m_posR = new Function(A,B,
                          Return(cos(A[2])*(B[0]-A[0])+sin(A[2])*(B[1]-A[1]),
                                 -sin(A[2])*(B[0]-A[0])+cos(A[2])*(B[1]-A[1]),
                                 B[2]-A[2]));
  }


  double dt=ui->doubleSpinBox_dt->value();
  double time=ui->doubleSpinBox_time->value();
  int samples = time/dt;

  bool RK4 = ui->radioButton_RK4->isChecked();
  bool euler = ui->radioButton_euler->isChecked();

  IntervalVector iA(4), iB(4);
  Vector vA(4), vB(4);

  cout.precision(12);

  iA[0]=ui->doubleSpinBox_xA->value();
  iA[1]=ui->doubleSpinBox_yA->value();
  iA[2]=ui->doubleSpinBox_aA->value();
  iA[3]=ui->doubleSpinBox_uA->value();
  iB[0]=ui->doubleSpinBox_xB->value();
  iB[1]=ui->doubleSpinBox_yB->value();
  iB[2]=ui->doubleSpinBox_aB->value();
  iB[3]=ui->doubleSpinBox_uB->value();
  vA[0]=ui->doubleSpinBox_xA->value();
  vA[1]=ui->doubleSpinBox_yA->value();
  vA[2]=ui->doubleSpinBox_aA->value();
  vA[3]=ui->doubleSpinBox_uA->value();
  vB[0]=ui->doubleSpinBox_xB->value();
  vB[1]=ui->doubleSpinBox_yB->value();
  vB[2]=ui->doubleSpinBox_aB->value();
  vB[3]=ui->doubleSpinBox_uB->value();


  IntervalVector iAB(8);
  for (int i = 0; i < 4; ++i) {
      iAB[i]=iA[i];
      iAB[i+4]=iB[i];
    }
  Interval d = m_dist->eval(iAB);

  IntervalVector iB_A(3), iB_R(3);
  Vector vB_A(3), vB_R(3);

  iB_A = iPositionR(iA,iB);
  iB_R = iPositionR(iA,iB);
  vB_A = vPositionR(vA,vB);
  vB_R = vPositionR(vA,vB);

  QVector<QwtIntervalSample> xEul(samples);
  QVector<QwtIntervalSample> yEul(samples);
  QVector<QwtIntervalSample> aEul(samples);
  QVector<QPointF> xEul_p(samples);
  QVector<QPointF> yEul_p(samples);
  QVector<QPointF> aEul_p(samples);

  QVector<QwtIntervalSample> xRK4(samples);
  QVector<QwtIntervalSample> yRK4(samples);
  QVector<QwtIntervalSample> aRK4(samples);
  QVector<QPointF> xRK4_p(samples);
  QVector<QPointF> yRK4_p(samples);
  QVector<QPointF> aRK4_p(samples);


  QVector<QwtIntervalSample> xRel(samples);
  QVector<QwtIntervalSample> yRel(samples);
  QVector<QwtIntervalSample> aRel(samples);
  QVector<QPointF> xRel_p(samples);
  QVector<QPointF> yRel_p(samples);
  QVector<QPointF> aRel_p(samples);

  QVector<QwtIntervalSample> xErr(samples);
  QVector<QwtIntervalSample> yErr(samples);
  QVector<QwtIntervalSample> aErr(samples);
  QVector<QPointF> xErr_p(samples);
  QVector<QPointF> yErr_p(samples);
  QVector<QPointF> aErr_p(samples);


  IntervalVector stateNoise(3,Interval(0).inflate(0.1));
  stateNoise[2]=Interval(0);


  for (int i = 0; i < samples; ++i)
    {
      //error computation
      IntervalVector E(3);
      E = iB_R-iB_A;
      Vector vE = vB_R-vB_A;
      xErr[i]=QwtIntervalSample(double(i)*dt,QwtInterval(E[0].lb(),E[0].ub()));
      xErr_p[i]=QPointF(double(i)*dt,vE[0]);
      yErr[i]=QwtIntervalSample(double(i)*dt,QwtInterval(E[1].lb(),E[1].ub()));
      yErr_p[i]=QPointF(double(i)*dt,vE[1]);
      aErr[i]=QwtIntervalSample(double(i)*dt,QwtInterval(E[2].lb(),E[2].ub()));
      aErr_p[i]=QPointF(double(i)*dt,vE[2]);



      xRel[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_R[0].lb(),iB_R[0].ub()));
      xRel_p[i]=QPointF(double(i)*dt,vB_R[0]);
      yRel[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_R[1].lb(),iB_R[1].ub()));
      yRel_p[i]=QPointF(double(i)*dt,vB_R[1]);
      aRel[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_R[2].lb(),iB_R[2].ub()));
      aRel_p[i]=QPointF(double(i)*dt,vB_R[2]);
      if(euler){
          xEul[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[0].lb(),iB_A[0].ub()));
          xEul_p[i]=QPointF(double(i)*dt,vB_A[0]);
          yEul[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[1].lb(),iB_A[1].ub()));
          yEul_p[i]=QPointF(double(i)*dt,vB_A[1]);
          aEul[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[2].lb(),iB_A[2].ub()));
          aEul_p[i]=QPointF(double(i)*dt,vB_A[2]);
        }
      if(RK4){
          xRK4[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[0].lb(),iB_A[0].ub()));
          xRK4_p[i]=QPointF(double(i)*dt,vB_A[0]);
          yRK4[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[1].lb(),iB_A[1].ub()));
          yRK4_p[i]=QPointF(double(i)*dt,vB_A[1]);
          aRK4[i]=QwtIntervalSample(double(i)*dt,QwtInterval(iB_A[2].lb(),iB_A[2].ub()));
          aRK4_p[i]=QPointF(double(i)*dt,vB_A[2]);
        }


      if(euler){
          iA = iEvoEuler(iA,dt);
          vA = vEvoEuler(vA,dt);

          iB = iEvoEuler(iB,dt);
          vB = vEvoEuler(vB,dt);

          iB_A = iEvoEulerR(iB_A,dt);
          vB_A = vEvoEulerR(vB_A,dt,dU1,dU2);
        }
      if(RK4){
          iA = iEvoRK4(iA,dt);
          vA = vEvoRK4(vA,dt);

          iB = iEvoRK4(iB,dt);
          vB = vEvoRK4(vB,dt);

          iB_A = iEvoRK4R(iB_A,dt);
          vB_A = vEvoRK4R(vB_A,dt,dU1,dU2);
        }

      iB_R = iPositionR(iA,iB);
      vB_R = vPositionR(vA,vB);

//      for (int j = 0; j < 4; ++j)
//        {
//          iAB[j]=iA[j];
//          iAB[j+4]=iB[j];
//        }
//      d = m_dist->eval(iAB);

    }

  tubeX->insertTube("X Relative",xRel,Qt::white);
  tubeY->insertTube("Y Relative",yRel,Qt::white);
  tubeA->insertTube("A Relative",aRel,Qt::white);

  tubeE->insertTube("Errors' tube on X",xErr,Qt::yellow);
  tubeE->insertCurve("Errors on X",xErr_p,Qt::darkYellow);
  tubeE->insertTube("Errors' tube on Y",yErr,Qt::cyan);
  tubeE->insertCurve("Errors on Y",yErr_p,Qt::darkCyan);
  tubeE->insertTube("Errors' tube on Angle",aErr,Qt::magenta);
  tubeE->insertCurve("Errors on Angle",aErr_p,Qt::darkMagenta);

  tubeX->insertCurve("X Relative",xRel_p,Qt::white);
  tubeY->insertCurve("Y Relative",yRel_p,Qt::white);
  tubeA->insertCurve("A Relative",aRel_p,Qt::white);


  if(euler){
      tubeX->insertTube("X Euler",xEul,Qt::blue);
      tubeY->insertTube("Y Euler",yEul,Qt::blue);
      tubeA->insertTube("A Euler",aEul,Qt::blue);

      tubeX->insertCurve("X Euler",xEul_p,Qt::black);
      tubeY->insertCurve("Y Euler",yEul_p,Qt::black);
      tubeA->insertCurve("A Euler",aEul_p,Qt::black);
    }
  if(RK4){
      tubeX->insertTube("X RK4",xRK4,Qt::blue);
      tubeY->insertTube("Y RK4",yRK4,Qt::blue);
      tubeA->insertTube("A RK4",aRK4,Qt::blue);

      tubeX->insertCurve("X RK4",xRK4_p,Qt::black);
      tubeY->insertCurve("Y RK4",yRK4_p,Qt::black);
      tubeA->insertCurve("A RK4",aRK4_p,Qt::black);
    }
}

IntervalVector MainWindow::iEvoEuler(IntervalVector vI, double dt)
{
  IntervalVector r = vI;
  r += dt*m_f->eval_vector(r);
  return r;
}

IntervalVector MainWindow::iEvoRK4(IntervalVector vI, double dt)
{
  IntervalVector r = vI;
  IntervalVector k1=m_f->eval_vector(vI);
  IntervalVector k2=m_f->eval_vector(vI+(dt/2.)*k1);
  IntervalVector k3=m_f->eval_vector(vI+(dt/2.)*k2);
  IntervalVector k4=m_f->eval_vector(vI+dt*k3);
  r += (dt/6.)*(k1+2.*k2+2.*k3+k4);
  return r;
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
  Vector k1(4),k2(4),k3(4),k4(4), r(4);
  //k1
  k1[0]=cos(v[2]);
  k1[1]=sin(v[2]);
  k1[2]=v[3];
  k1[3]=0.0;
  //k2
  k2[0]=cos(v[2]+(dt/2.)*k1[2]);
  k2[1]=sin(v[2]+(dt/2.)*k1[2]);
  k2[2]=v[3]+(dt/2.)*k1[3];
  k2[3]=0.0;
  //k3
  k3[0]=cos(v[2]+(dt/2.)*k2[2]);
  k3[1]=sin(v[2]+(dt/2.)*k2[2]);
  k3[2]=v[3]+(dt/2.)*k2[3];
  k3[3]=0.0;
  //k4
  k4[0]=cos(v[2]+dt*k3[2]);
  k4[1]=sin(v[2]+dt*k3[2]);
  k4[2]=v[3]+dt*k3[3];
  k4[3]=0.0;
  // r 
  r = v + (dt/6.)*(k1+2.*k2+2.*k3+k4);
  return r;
}

IntervalVector MainWindow::iEvoEulerR(IntervalVector vI, double dt)
{
  IntervalVector r = vI;
  r += dt*m_fR->eval_vector(r);
  return r;
}

IntervalVector MainWindow::iEvoRK4R(IntervalVector vI, double dt)
{
  IntervalVector r = vI;
  IntervalVector k1=m_fR->eval_vector(vI);
  IntervalVector k2=m_fR->eval_vector(vI+(dt/2.)*k1);
  IntervalVector k3=m_fR->eval_vector(vI+(dt/2.)*k2);
  IntervalVector k4=m_fR->eval_vector(vI+dt*k3);
  r += (dt/6.)*(k1+2.*k2+2.*k3+k4);
  return r;
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
  Vector k1(3),k2(3),k3(3),k4(3), r(3);
  //k1
  k1[0]=-1+cos(v[2])+du1*v[1];
  k1[1]=sin(v[2])-du1*v[0];
  k1[2]=du2-du1;
  //k2
  k2[0]=-1+cos(v[2]+(dt/2.)*k1[2])+du1*(v[1]+(dt/2.)*k1[1]);
  k2[1]=sin(v[2]+(dt/2.)*k1[2])-du1*(v[0]+(dt/2.)*k1[0]);
  k2[2]=du2-du1;
  //k3
  k3[0]=-1+cos(v[2]+(dt/2.)*k2[2])+du1*(v[1]+(dt/2.)*k2[1]);
  k3[1]=sin(v[2]+(dt/2.)*k2[2])-du1*(v[0]+(dt/2.)*k2[0]);
  k3[2]=du2-du1;
  //k4
  k4[0]=-1+cos(v[2]+dt*k3[2])+du1*(v[1]+dt*k3[1]);
  k4[1]=sin(v[2]+dt*k3[2])-du1*(v[0]+dt*k3[0]);
  k4[2]=du2-du1;
  // r 
  r = v + (dt/6.)*(k1+2.*k2+2.*k3+k4);
  return r;
}

IntervalVector MainWindow::iPositionR(IntervalVector A, IntervalVector B)
{
  IntervalVector AB(8);
  for(int i=0; i<4; ++i){
      AB[i]=A[i];
      AB[i+4]=B[i];
    }
  return m_posR->eval_vector(AB);
}

Vector MainWindow::vPositionR(Vector A, Vector B)
{
  Vector r(3);
  r[0]=cos(A[2])*(B[0]-A[0])+sin(A[2])*(B[1]-A[1]);
  r[1]=-sin(A[2])*(B[0]-A[0])+cos(A[2])*(B[1]-A[1]);
  r[2]=B[2]-A[2];
  return r;
}



void MainWindow::on_pushButton_drawError_clicked()
{
  tubeE->clear();
  double dU1=ui->doubleSpinBox_uA->value();
  double dU2=ui->doubleSpinBox_uB->value();

  bool RK4 = ui->radioButton_RK4->isChecked();
  bool euler = ui->radioButton_euler->isChecked();

  {
    delete m_f;
    delete m_fR;
    delete m_dist;
    delete m_posR;

    Variable A(4),B(4);
    m_f = new Function(A,
                       Return(cos(A[2]),
                              sin(A[2]),
                              A[3],
                              Interval(0.0)));
    m_fR = new Function(A,
                        Return((Interval(-1)+cos(A[2])+dU1*A[1]),
                               (sin(A[2])-dU1*A[0]),
                               Interval(dU2-dU1)));
    m_dist = new Function(A,B,
                          sqrt(sqr(A[0]-B[0])+sqr(A[1]-B[1])));
    m_posR = new Function(A,B,
                          Return(cos(A[2])*(B[0]-A[0])+sin(A[2])*(B[1]-A[1]),
                                 -sin(A[2])*(B[0]-A[0])+cos(A[2])*(B[1]-A[1]),
                                 B[2]-A[2]));
  }

  IntervalVector iA(4), iB(4);
  Vector vA(4), vB(4);
  iA[0]=ui->doubleSpinBox_xA->value();
  iA[1]=ui->doubleSpinBox_yA->value();
  iA[2]=ui->doubleSpinBox_aA->value();
  iA[3]=ui->doubleSpinBox_uA->value();
  iB[0]=ui->doubleSpinBox_xB->value();
  iB[1]=ui->doubleSpinBox_yB->value();
  iB[2]=ui->doubleSpinBox_aB->value();
  iB[3]=ui->doubleSpinBox_uB->value();
  vA[0]=ui->doubleSpinBox_xA->value();
  vA[1]=ui->doubleSpinBox_yA->value();
  vA[2]=ui->doubleSpinBox_aA->value();
  vA[3]=ui->doubleSpinBox_uA->value();
  vB[0]=ui->doubleSpinBox_xB->value();
  vB[1]=ui->doubleSpinBox_yB->value();
  vB[2]=ui->doubleSpinBox_aB->value();
  vB[3]=ui->doubleSpinBox_uB->value();

  cout.precision(12);

  IntervalVector iB_A(3), iB_R(3);
  Vector vB_A(3), vB_R(3);
  iB_A = iPositionR(iA,iB);
  iB_R = iPositionR(iA,iB);
  vB_A = vPositionR(vA,vB);
  vB_R = vPositionR(vA,vB);

  QVector<QwtIntervalSample> Err;
  QVector<QPointF> Err_p;

  double dtM=ui->doubleSpinBox_dt->value();
  for(double dt = .0001; dt<dtM; dt+=0.0001)
    {

      if(euler){
          iA = iEvoEuler(iA,dt);
          vA = vEvoEuler(vA,dt);

          iB = iEvoEuler(iB,dt);
          vB = vEvoEuler(vB,dt);

          iB_A = iEvoEulerR(iB_A,dt);
          vB_A = vEvoEulerR(vB_A,dt,dU1,dU2);
        }
      if(RK4){
          iA = iEvoRK4(iA,dt);
          vA = vEvoRK4(vA,dt);

          iB = iEvoRK4(iB,dt);
          vB = vEvoRK4(vB,dt);

          iB_A = iEvoRK4R(iB_A,dt);
          vB_A = vEvoRK4R(vB_A,dt,dU1,dU2);
        }

      iB_R = iPositionR(iA,iB);
      vB_R = vPositionR(vA,vB);

      IntervalVector E(3);
      E = iB_R-iB_A;
      double vE = (vB_R-vB_A).norm()/dt;

      Err.append(QwtIntervalSample(dt,QwtInterval(E[0].lb(),E[0].ub())));
      Err_p.append(QPointF(dt,vE));
    }
  tubeE->insertCurve("Norm of errors",Err_p,Qt::black);
}
