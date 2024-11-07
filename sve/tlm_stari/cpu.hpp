#ifndef CPU_H
#define CPU_H
#define SC_INCLUDE_FX

#include "../spec/image.h"
#include "../spec/ipoint.h"
#include "../spec/imload.h"
#include "../spec/fasthessian.h"
#include "utils.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <cmath>
#include <systemc>
#include <string>
#include <fstream>
#include <deque>
#include <vector>
#include <array>
#include <algorithm>
#include "types.hpp"
#include "addr.hpp"
#include "tlm_utils/tlm_quantumkeeper.h"


using namespace std;
using namespace sc_core;

SC_MODULE(Cpu)
{
    public:
        SC_HAS_PROCESS(Cpu);
        Cpu(sc_module_name name, const std::string& image_name, int argc, char **argv);
        tlm_utils::simple_initiator_socket<Cpu> interconnect_socket;
        
        surf::Image *_iimage; //= nullptr;
        surf::Ipoint *_current; //= nullptr;
        std::vector<std::vector<std::vector<num_f>>> _index;
        bool _doubleImage = false;
        num_i _VecLength = 0;
        num_i _MagFactor = 0;
        num_i _OriSize = 0;
        
        num_f _sine = 0.0, _cose = 1.0;
        std::vector<std::vector<num_f>> _Pixels;
        
        num_f _lookup1[83], _lookup2[40];
        
        int VLength; // Length of the descriptor vector
        
        double scale;
        int x;
        int y;
    
    protected:
        sc_core::sc_time offset;
        
        vector <num_f> mem;
        
        void software();
        void createVector(double scale, double row, double col);
        void normalise();
        void createLookups();
        void initializeGlobals(surf::Image *im, bool dbl, int insi);
        int getVectLength();
        void setIpoint(surf::Ipoint* ipt);
        void assignOrientation();
        void makeDescriptor();
        void saveIpoints(string sFileName, const vector< surf::Ipoint >& ipts);
        
        num_f read_mem(sc_dt::sc_uint<64> addr);
        int read_hard_int(sc_uint<64> addr);
        double read_hard_double(sc_uint<64> addr);
        void write_hard_int(sc_uint<64> addr, int val);
        void write_hard_double(sc_uint<64> addr, double val);
    
    private:
	surf::Image *_im;
	string fn; 
	
};


#endif
