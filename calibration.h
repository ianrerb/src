#ifndef CALIBRATION_H
#define CALIBRATION_H
#include<vector>
#include<fstream>
#include<sstream>
#include<string>
#include<iostream>

//BEGIN NAMESPACE
namespace calibration 
{  
 //struct to hold quotes for stl containers (vector, list, set, etc...)
  struct quote 
  { 
    double strike;
    std::string type;
    double maturity;
    double bid;
    double ask;
    double rate;
    double q;
    double fwd;  
  };

  //typedefs for working with option quotes and pricing functions
  typedef std::vector<quote> quote_list;
  typedef std::vector<quote>::iterator quote_list_iterator;
  
  typedef std::vector<double> param_list;
  typedef std::vector<double>::iterator param_list_iterator;

} // <===== END OF NAMESPACE

//function to fill param_list with array of values

calibration::param_list ParameterRange(unsigned int size, double start, double end)
{  
  calibration::param_list params;
  double step = (end - start)/static_cast<double>(size);

  for(unsigned int i = 0; i!= size; ++i)
  {  
    params.push_back(start + static_cast<double>(i)*step);
  }
  
  return params;
}

//function pointer to be passed as objective function to optimization routines
typedef double (*ErrorFunction)(calibration::quote_list &, calibration::param_list &);
  
//subroutine for reading quotes from textfiles (see prices for formatting)
calibration::quote_list ReadQuotes(const char* file)
{  
  calibration::quote_list retvec;
  std::ifstream infile(file);
  std::string s;
  
  std::getline(infile,s); //ignore titles
  
  while( std::getline(infile, s) ) 
  {  
    std::stringstream ss;
    calibration::quote temp;
    ss << s;

    ss >> temp.strike;
    ss >> temp.type;
    ss >> temp.maturity;
    ss >> temp.bid;
    ss >> temp.ask;
    ss >> temp.rate;
    ss >> temp.q;
    ss >> temp.fwd;  
    temp.rate /= 100.0;
    temp.q /= 100.0;
    retvec.push_back( temp );
  }

  return retvec;
}
//////////////////////////////////////////////
// GRID SEARCH OPTIMIZATION ROUTINE  /////////
//////////////////////////////////////////////
calibration::param_list simple_grid_search(std::vector<calibration::param_list> &parameters, const char* file, ErrorFunction TheFunction)
{  
  calibration::quote_list quotes = ReadQuotes(file);
  unsigned int count = 0; //counts number of evaluations so far
  unsigned int param_count = parameters.size();
  unsigned int scenario_count = 1;
  
  calibration::param_list return_list(param_count);
  calibration::param_list test_list(param_count);  
  double min;

  for(unsigned int i = 0; i!= param_count; ++i) //compute total scenarios
  {  
    scenario_count *= parameters[i].size();
  }

  std::vector<unsigned int> index(param_count); //each value corresponds to index for respective parameter
  
  
  while(count < scenario_count) //check if max has been reached
  {  
    unsigned int rest = count;
    unsigned int base = scenario_count; //used to get number for most significant digit
    
    for(unsigned int level = 0; level != param_count; ++level)
    {  
      base /=parameters[level].size();
      index[level] = rest < base ? 0 : (rest / base);
      rest -= index[level]*base;
      test_list[level] = parameters[level][index[level]]; 
      
    }
      
    if(count == 0)
    { 
      return_list = test_list; 
      min = TheFunction(quotes,test_list); 
      std::cout<<"Error: "<<min<<"\t Parameters: ";
      
      for(calibration::param_list_iterator it = return_list.begin(); it!=return_list.end(); ++it)
	std::cout<<*it<<"  ";
      std::cout<<"\n\n"; 
    }
    else
    {  
      double temp = TheFunction(quotes,test_list);
      if(min > temp)
      { 
	return_list = test_list; 
	min = temp; 
	std::cout<<"Error: "<<min<<"\t Parameters: ";
      
	for(calibration::param_list_iterator it = return_list.begin(); it!=return_list.end(); ++it)
	  std::cout<<*it<<"  ";
	std::cout<<"\n\n"; 

      }
    }
    
    ++count; //increment and re-run while loop
  }
  
  return return_list;

} 

#endif
