#ifndef __ranvar_h
#define __ranvar_h

#include <tr1/random>
#include <sys/time.h>

#define INTER_DISCRETE 0	// no interpolation (discrete)
#define INTER_CONTINUOUS 1	// linear interpolation
#define INTER_INTEGRAL 2	// linear interpolation and round up

struct CDFentry {
	double cdf_;
	double val_;
};

class EmpiricalRandomVariable {
public:
	virtual double value();
	virtual double interpolate(double u, double x1, double y1, double x2, double y2);
	virtual double avg();
	EmpiricalRandomVariable(int interp, int seed);
	~EmpiricalRandomVariable();
	double& minCDF() { return minCDF_; }
	double& maxCDF() { return maxCDF_; }
	int loadCDF(const char* filename);

protected:
	int lookup(double u);

	double minCDF_;		// min value of the CDF (default to 0)
	double maxCDF_;		// max value of the CDF (default to 1)
	int interpolation_;	// how to interpolate data (INTER_DISCRETE...)
	int numEntry_;		// number of entries in the CDF table
	int maxEntry_;		// size of the CDF table (mem allocation)
	CDFentry* table_;	// CDF table of (val_, cdf_)
	std::tr1::uniform_real<double> uni;
	std::tr1::ranlux64_base_01 gen;
};

class ExponentialRandomVariable {
 public:
	virtual double value();
	ExponentialRandomVariable(double, int);
	virtual inline double avg() { return avg_; };
	void setavg(double d) { avg_ = d; };
 private:
	double avg_;
	std::tr1::exponential_distribution<double> exponential;
	std::tr1::ranlux64_base_01 gen;
};


#endif
