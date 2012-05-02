#ifndef SURFACES_H
#define SURFACES_H
#include<sstream>
#include<fstream>
#include<iostream>
#include<vector>
#include<cmath>
#include "../../transform_model/include/transform.h"
#define STRIKE_DELTA 10.0
#define TIME_DELTA .1


namespace surface 
{  
  struct rate_data
  {  
    double maturity;
    double rate;
    double div_rate;  
  }; 
  
  typedef std::vector<rate_data> rate_model;
  typedef std::vector<rate_data>::iterator rate_model_iterator;
}

surface::rate_model ReadRates(const char* file)
{  
  surface::rate_model rates;
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
    temp.rate = .005;
    temp.div_rate = .01;
    rates.push_back(temp);
  }
  
  return rates;
}

double CalendarSpread(double spot, double strike, transform_base &TheEngine, pricemodel &TheModel)
{  
  double t = TheModel.TimeToExpiry(); 
  double C1 = exp(t*(TheModel.DividendRate() - TheModel.RiskFreeRate()));
  double C2 = C1*exp(TIME_DELTA*(TheModel.DividendRate() - TheModel.RiskFreeRate()));
  
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
  double der1 = ButterflySpread(spot,strike,C,TheEngine, TheModel);
  double der2 = ButterflySpread(spot,strike+STRIKE_DELTA, C, TheEngine, TheModel);
  return( (der2 - der1)/STRIKE_DELTA );
}

double ComputeLocalVolatility(double spot, double strike, transform_base &TheEngine, pricemodel &TheModel)
{ 
  double C = exp(TheModel.TimeToExpiry()*(TheModel.DividendRate()-TheModel.RiskFreeRate()));
  double price = TheEngine.Price(spot,strike, C, TheModel);
  double cal_spread = CalendarSpread(spot,strike,TheEngine,TheModel);
  double butterfly_spread = ButterflySpread(spot,strike,C,TheEngine,TheModel);
      double second_derivative = SecondDer(spot,strike,C,TheEngine,TheModel);
      
      double vol = cal_spread + (TheModel.RiskFreeRate() - TheModel.DividendRate())*strike*butterfly_spread;
	     //std::cout<<"vol1: "<<vol<<" ";
	     vol += TheModel.DividendRate()*price;
	     //std::cout<<"vol2: "<<vol<<" ";
	     vol /= (.5*strike*strike)*second_derivative;
	     // std::cout<<"vol3: "<<vol<<" ";
	     vol = pow(vol,.5);
	     //std::cout<<"vol4: "<<vol<<" ";
  /*
  std::cout<<"=== "
	   <<"ca_spr: "<< cal_spread <<" "
	   <<"but spr: "<< butterfly_spread <<" "
	   <<"2nd der: "<< second_derivative <<" ";
*/
  return vol;
}

void VolSurface(double spot, std::vector<double> strikes, transform_base &TheEngine, pricemodel &TheModel, const char* file)
{  
  surface::rate_model rates = ReadRates(file);
  const unsigned int T_count = rates.size();
  const unsigned int K_count = strikes.size();
 
  
  std::cout<<"Strike \t TTM \t Vol\n";
  
  for(unsigned int i = 0; i!= T_count; ++i)
  {  
    TheModel.TimeToExpiry(rates[i].maturity);
    TheModel.RiskFreeRate(rates[i].rate);
    TheModel.DividendRate(rates[i].div_rate);
    
    for(unsigned int j = 0; j!=K_count; ++j)
    {       
      	          
      std::cout<<strikes[j]<<"\t"
	       <<rates[i].maturity<<"\t"
	       <<ComputeLocalVolatility(spot,strikes[j],TheEngine,TheModel)<<"\n";	  
    }
    if(K_count > 1){  std::cout<<"\n"; }
  }
}



void PremiumSurface(double spot, std::vector<double> strikes, transform_base &TheEngine, pricemodel &TheModel, const char* file)
{  
  surface::rate_model rates = ReadRates(file);
  
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
