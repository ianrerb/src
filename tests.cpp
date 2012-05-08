#define SPOT 1412.52
#include "calibration.h"
#include "surfaces.h"
#include<iostream>
#include<vector>
#include<cmath>
#include "../../transform_model/include/transform.h"
#define DIM 5

using namespace std;
using namespace calibration;
using namespace surface;

int main()
{ 
  vector<double> T;
  T.push_back(.1);
  while(T.back() < .8){ T.push_back(T.back() + .05); }

  VGSA TheModel(.0374,.215,-.0946,7.06,2.48,9.456);
  FrFFT TheEngine(256,1.5,.15,.001);
  
  TheModel.RiskFreeRate(.005);
  TheModel.DividendRate(.02);

  for(vector<double>::iterator it = T.begin(); it!= T.end(); ++it)
  {  
    TheModel.TimeToExpiry(*it);
    cout<<CalendarSpread(SPOT,1000,TheEngine,TheModel)<<"\n";
  }
  
  return 0;
}

