#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include "addr.hpp"
#include "types.hpp"
#include <systemc>
#include <iostream>
#include <string.h>
#include <bitset>
#include <math.h>
#include <string>
#include <tlm>
#include <vector>

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace sc_dt;

int toInt(unsigned char *buf);
void intToUchar(unsigned char *buf,int val);
double toDouble(unsigned char *buf);
void doubleToUchar(unsigned char *buf, double val);
num_f toNum_f(unsigned char *buf);
bool is_negative(num_f val);

#endif
