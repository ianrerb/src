#ifndef SURFACES_H
#define SURFACES_H
#include<sstream>
#include<fstream>
#include<iostream>
#include<vector>
#include<cmath>
#include "../../transform_model/include/transform.h"
#define STRIKE_DELTA .01 
#define TIME_DELTA .0001


namespace surface 
{  
  struct rate_data
  {  
    double maturity;
    double rate;
    double div_rate;  
  }; 
  
  typedef std::vector<rate_data> rate_curve;
  typedef std::vector<rate_data>::iterator rate_model_iterator;
}

surface::rate_curve ReadRates(const char* file)
{  
  surface::rate_curve rates;
  std::ifstream infile(file);
  std::string s;

  std::getline(infile,s);
  
  while( std::getline(infile, s))
  {  
    surface::rate_data temp;
    std::stringstream ss;
    ss << s;

    ss >> temp.maturity;
    ss >> temp.rate;
    ss >> temp.div_rate;
    rates.push_back(temp);
  }
  
  return rates;
}

double CalendarSpread(double spot, double strike, transform_base &TheEngine, pricemodel &TheModel)
{  
  double t = TheModel.TimeToExpiry(); 
  double C1 = exp(-t * TheModel.RiskFreeRate());
  double C2 = C1*exp(-TIME_DELTA * TheModel.RiskFreeRate());
  
  double F1 = TheEngine.Price(spot,strike,C1,TheModel);
  
  t +=TIME_DELTA;
  TheModel.TimeToExpiry(t);
  double F2 = TheEngine.Price(spot,strike,C2,TheModel);
  
  t -= TIME_DELTA;
  TheModel.TimeToExpiry(t); //reset time on the model
  //std::cout<<"F2: "<<F2<<"    F1: "<<F1<<"  ";
  return( (F2 - F1)/TIME_DELTA );
}

double ButterflySpread(double spot, double strike, double C, transform_base &TheEngine, pricemodel &TheModel)
{  
  double price = TheEngine.Price(spot, strike, C, TheModel);
  double price_delta = TheEngine.Price(spot, strike + STRIKE_DELTA, C, TheModel);
  return( (price_delta - price)/STRIKE_DELTA);
    
}

double SecondDer(double spot, double strike, double C, transform_base &TheEngine, pricemodel &TheModel)
{  
  double val1 = TheEngine.Price(spot,strike+STRIKE_DELTA,C,TheModel);
  double val2 = TheEngine.Price(spot,strike-STRIKE_DELTA,C,TheModel);
  double val3 = TheEngine.Price(spot,strike,C,TheModel);
  
  double retval = std::abs( (val1 + val2 - 2.0*val3 )/(STRIKE_DELTA*STRIKE_DELTA) );
  return retval;
}

double ComputeLocalVolatility(double spot, double strike, double rate, double div_rate, double T, transform_base &TheEngine, pricemodel &TheModel, bool verbose = false)
{ 
  //save old values
  double old_time_val = TheModel.TimeToExpiry();
  double old_rate_val = TheModel.RiskFreeRate();
  double old_div_val = TheModel.DividendRate();

  //set model values
  TheModel.TimeToExpiry(T);
  TheModel.RiskFreeRate(rate);
  TheModel.DividendRate(div_rate);

  //compute relevant values for local vol function
  double C = exp(-TheModel.TimeToExpiry()*TheModel.RiskFreeRate());
  double price = TheEngine.Price(spot,strike, C, TheModel);
  double cal_spread = CalendarSpread(spot,strike,TheEngine,TheModel);
  double butterfly_spread = ButterflySpread(spot,strike,C,TheEngine,TheModel);
  double second_derivative = SecondDer(spot,strike,C,TheEngine,TheModel);

  //compute local vol function      
  double vol = cal_spread + (TheModel.RiskFreeRate() - TheModel.DividendRate())*strike*butterfly_spread;
  if(verbose){ std::cout<<"vol1: "<<vol<<"\n" ; }
	     
  vol += TheModel.DividendRate()*price;
  if(verbose){ std::cout<<"vol2: "<<vol<<"\n"; }
	     
  vol /= (.5*strike*strike)*second_derivative;
  if(verbose){ std::cout<<"vol3: "<<vol<<"\n"; }
  
  vol = pow(vol,.5);	     
  if(verbose){ std::cout<<"vol4: "<<vol<<"\n"; }
  
  if(verbose)
  {
    std::cout<<"=== "
	     <<"ca_spr: "<< cal_spread <<" "
	     <<"but spr: "<< butterfly_spread <<" "
	     <<"2nd der: "<< second_derivative <<" ";
  }

//reset model params
  TheModel.TimeToExpiry(old_time_val); 
  TheModel.RiskFreeRate(old_rate_val);
  TheModel.DividendRate(old_div_val);
  
  //return vol
  return vol;
}

void VolSurface(double spot, std::vector<double> strikes, surface::rate_curve rates, transform_base &TheEngine, pricemodel &TheModel, const char* file)
{  
  const unsigned int T_count = rates.size();
  const unsigned int K_count = strikes.size();
 
  
  std::cout<<"Strike \t TTM \t Vol\n";
  
  for(unsigned int i = 0; i!= T_count; ++i)
  {  
        
    for(unsigned int j = 0; j!=K_count; ++j)
    {       
      	          
      std::cout<<strikes[j]<<"\t"
	       <<rates[i].maturity<<"\t"
	       <<ComputeLocalVolatility(spot,strikes[j],rates[i].rate, rates[i].div_rate, rates[i].maturity, TheEngine, TheModel)<<"\n";	  
    }
    if(K_count > 1){  std::cout<<"\n"; }
  }
}



void PremiumSurface(double spot, std::vector<double> strikes, transform_base &TheEngine, pricemodel &TheModel, const char* file)
{  
  surface::rate_curve rates = ReadRates(file);
  
  const unsigned int T_count = rates.size();
  const unsigned int K_count = strikes.size();
  
  std::cout<<"Strike \t TTM \t Premium\n";
  
  for(unsigned int i = 0; i!= T_count; ++i)
  {  
    TheModel.TimeToExpiry(rates[i].maturity);
    TheModel.RiskFreeRate(rates[i].rate);
    TheModel.DividendRate(rates[i].div_rate);
    double C = exp(rates[i].maturity*(rates[i].div_rate - rates[i].rate));  
    
    for(unsigned int j = 0; j!=K_count; ++j)
    {       
      std::cout<<strikes[j]<<"\t"<<rates[i].maturity<<"\t"<<TheEngine.Price(spot, strikes[j], C, TheModel)<<"\n";	  
    }
    if(K_count > 1){  std::cout<<"\n"; }
  }
}

#endif
