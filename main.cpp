#define SPOT 1412.52
#include "calibration.h"
#include<iostream>
#include<vector>
#include<cmath>
#include "../../transform_model/include/transform.h"
#define DIM 5

using namespace std;
using namespace calibration;

double VGSA_error( quote_list &quotes, param_list &params )
{  
  if(params.size()!=6){  cout<<"Error. Incorrect Number of Parameter Arguments."; return -1; }
  
  double sum = 0;
  FrFFT Engine(256,1.5,.15,.001);
  VGSA PriceModel(params[0], params[1], params[2], params[3], params[4], params[5]);  
  
  for(quote_list_iterator it = quotes.begin(); it!=quotes.end(); ++it)
  {  
    if(it->type == "Call"){ Engine.Alpha(1.5); } else { Engine.Alpha(-1.5); }
    PriceModel.RiskFreeRate(it->rate);
    PriceModel.DividendRate(it-> q);
    double mat = (it->maturity)/365.0;
    PriceModel.TimeToExpiry( mat );
    double C = exp((it->q - it->rate)*mat); 
    double computed_value = Engine.Price(SPOT,it->strike,C,PriceModel);
    double market_value = (it->bid + it->ask)/2.0;
    //cout<<it->strike<<"  "<<mat<<"   Model: "<<computed_value<<"  Market: "<<it->bid<<"\n";
    sum += abs(computed_value - market_value)/(it->ask - it->bid);
  } 
    return sum;
}

int main()
{ 
  vector<param_list> grid_params;
  
  grid_params.push_back(ParameterRange(DIM,.05,.3)); //sigma
  grid_params.push_back(ParameterRange(DIM,.1,.3));  //nu
  grid_params.push_back(ParameterRange(DIM,-.15,0));  //theta
  grid_params.push_back(ParameterRange(DIM,1,10));  //kappa
  grid_params.push_back(ParameterRange(DIM,2,4));    //eta
  grid_params.push_back(ParameterRange(DIM,1,20)); //lambda
  
  param_list optimized_list = simple_grid_search(grid_params,"prices",VGSA_error); 
 
  quote_list quotes = ReadQuotes("prices");
  cout<<"Error: "<<VGSA_error(quotes,optimized_list)<<"\n";
  
  for(param_list_iterator it = optimized_list.begin(); it!= optimized_list.end(); ++it)
    cout<<*it<<"\n";

  return 0;
}
