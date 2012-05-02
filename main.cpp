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

double VGSA_error( quote_list &quotes, param_list &params );
void OptimizationSub(const char*);
void PriceSub(const double, const double, const double, const double, const double, const double);
void PrintPremiumSurface(const double, const double, const unsigned int);
void PrintVolSurface(const double, const double, const unsigned int);
int main()
{ 
 // PriceSub(.0374,.215,-.0946,7.06,2.48,9.456);
 // OptimizationSub("prices_no_short_mat");
  PrintPremiumSurface(1000,2000,1);
  PrintVolSurface(1000,2000,1);
  return 0;
}
////////////////////////////////
// Vol Surface Sub
////////////////////////////////
void PrintVolSurface(const double min, const double max, const unsigned int dim)
{  
  vector<double> strikes;
  for(unsigned int i = 0; i!=dim; ++i)
    strikes.push_back(min+(max-min)*static_cast<double>(i)/static_cast<double>(dim));

  VGSA Model(.0374,.215,-.0946,7.06,2.48,9.456);
  FrFFT Engine(256,1.5,.15,.001);
  VolSurface(SPOT,strikes,Engine,Model,"rates");

}

////////////////////////////////
// Premium Surface Sub
///////////////////////////////
void PrintPremiumSurface(const double min,const double max, const unsigned int dim)
{  
  vector<double> strikes;
  for(unsigned int i = 0; i!=dim; ++i)
    strikes.push_back(min+(max-min)*static_cast<double>(i)/static_cast<double>(dim));

  VGSA Model(.0374,.215,-.0946,7.06,2.48,9.456);
  FrFFT Engine(512,1.5,.15,.01);
  PremiumSurface(SPOT,strikes,Engine,Model,"rates");
}


//////////////////////////////
// Implied Vs. Market Sub
/////////////////////////////
void PriceSub(const double sigma, const double nu, const double theta, const double kappa, const double eta, const double lambda)
{ 
  quote_list market_prices = ReadQuotes("prices");
  
  //create engine and VGSA model 
  VGSA Model(sigma,nu,theta,kappa,eta,lambda);
  FrFFT Engine(256,1.5,.15,.001);
  
  for(quote_list_iterator it = market_prices.begin(); it!= market_prices.end(); ++it)
  { 
    double mat = it->maturity/365.0; //days to years 
    Model.RiskFreeRate(it->rate);
    Model.TimeToExpiry(mat);
    Model.DividendRate(it->q); //set model parameters
    
    if(it->type == "Call"){ Engine.Alpha(1.5);} else { Engine.Alpha(-1.5); } //set alpha based on call or put
    
    double C = exp((it->q - it->rate)*mat); 
    cout<<it->type<<"  "
	<<"K: "<<it->strike<<"  "
	<<"T: "<<mat<<"  "
	<<"MP: "<<(it->ask + it-> bid)/2.0<<"  "
	<<"Imp. Price: "<<Engine.Price(SPOT,it->strike,C,Model)<<"\n";
  }

}


//////////////////////////////////////////////////////
// Subroutines and Functions for Optimization
//////////////////////////////////////////////////////
void OptimizationSub(const char* file)
{  
  vector<param_list> grid_params;
  
  grid_params.push_back(ParameterRange(DIM,.035,.038)); //sigma
  grid_params.push_back(ParameterRange(DIM,.215,.225));  //nu
  grid_params.push_back(ParameterRange(1,-.0946,-.095));  //theta
  grid_params.push_back(ParameterRange(1,7.06,7.07));  //kappa
  grid_params.push_back(ParameterRange(DIM,2.4,2.5));    //eta
  grid_params.push_back(ParameterRange(DIM,9.44,9.46)); //lambda
  
  param_list optimized_list = simple_grid_search(grid_params,file,VGSA_error); 
 
  quote_list quotes = ReadQuotes(file);
  cout<<"Error: "<<VGSA_error(quotes,optimized_list)<<"\n";
  
  for(param_list_iterator it = optimized_list.begin(); it!= optimized_list.end(); ++it)
    cout<<*it<<"\n";


}

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

