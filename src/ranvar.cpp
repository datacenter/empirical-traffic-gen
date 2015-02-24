#include <stdio.h>
#include "ranvar.h"
#include <iostream>

/*
// Empirical Random Variable:
//  CDF input from file with the following column
//   1.  Possible values in a distrubutions
//   2.  Number of occurances for those values
//   3.  The CDF for those value
//  code provided by Giao Nguyen
*/

EmpiricalRandomVariable::EmpiricalRandomVariable(int interp, int s) : minCDF_(0), maxCDF_(1), maxEntry_(32), table_(0)
{
    interpolation_ = interp;
    // initialize random seed
    if (s)
      gen.seed(s);
    else {
      struct timeval time;
      gettimeofday(&time, NULL);
      gen.seed((time.tv_sec*1000000) + time.tv_usec);
    }    
    uni = std::tr1::uniform_real<double>(minCDF_, maxCDF_);
}

int EmpiricalRandomVariable::loadCDF(const char* filename)
{
	FILE* fp;
	char line[256];
	CDFentry* e;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
	    printf("could not open empirical distribution %s\n", filename);
	    exit(1);
	}

	if (table_ == 0)
		table_ = new CDFentry[maxEntry_];

	for (numEntry_= 0; fgets(line, 256, fp); numEntry_++)
	{
		if (numEntry_ >= maxEntry_)
		{	// resize the CDF table
			maxEntry_ *= 2;
			e = new CDFentry[maxEntry_];
			for (int i=numEntry_-1; i >= 0; i--)
				e[i] = table_[i];
			delete [] table_;
			table_ = e;
		}
		e = &table_[numEntry_];
		// Use * and l together raises a warning
		sscanf(line, "%lf %lf", &e->val_, &e->cdf_);
	}

	fclose(fp);
	return numEntry_;
}

double EmpiricalRandomVariable::value()
{
	if (numEntry_ <= 0)
		return 0;
	double u = uni(gen); // uniform(minCDF_, maxCDF_, 1);
	int mid = lookup(u);
	if (mid && interpolation_ && u < table_[mid].cdf_)
		return interpolate(u, table_[mid-1].cdf_, table_[mid-1].val_,
				   table_[mid].cdf_, table_[mid].val_);
	return table_[mid].val_;
}

double EmpiricalRandomVariable::interpolate(double x, double x1, double y1, double x2, double y2)
{
	double value = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
	if (interpolation_ == INTER_INTEGRAL)	// round up
		return ceil(value);
	return value;
}

int EmpiricalRandomVariable::lookup(double u)
{
	// always return an index whose value is >= u
	int lo, hi, mid;
	if (u <= table_[0].cdf_)
		return 0;
	for (lo=1, hi=numEntry_-1;  lo < hi; ) {
		mid = (lo + hi) / 2;
		if (u > table_[mid].cdf_)
			lo = mid + 1;
		else
			hi = mid;
	}
	return lo;
}

double EmpiricalRandomVariable::avg() 
{
        double avg = 0.0;

	for (int i = 0; i < numEntry_; i++) {
	      double value, prob;	       
	      if (i == 0) {
		     value = table_[0].val_/2;
		     prob = table_[0].cdf_;
	      }
	      else {
		     value = (table_[i-1].val_ + table_[i].val_)/2;
		     prob = table_[i].cdf_ - table_[i-1].cdf_;
	      }
	      avg += (value * prob);
	}
	return avg;
}

EmpiricalRandomVariable::~EmpiricalRandomVariable()
{
    delete [] table_;
}


/*
 * Exponential random variable
 */
ExponentialRandomVariable::ExponentialRandomVariable(double avg, int s)
{
	avg_ = avg;
	// initialize random seed
	if (s)
	  gen.seed(s);
	else {
	  struct timeval time;
	  gettimeofday(&time, NULL);
	  gen.seed((time.tv_sec*1000000) + time.tv_usec);
	}
	exponential = std::tr1::exponential_distribution<double>(1 / avg_);
}

double ExponentialRandomVariable::value()
{
	return exponential(gen);
}
