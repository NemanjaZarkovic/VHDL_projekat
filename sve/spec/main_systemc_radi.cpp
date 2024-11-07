#define SC_INCLUDE_FX
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/time.h>
#include <string.h>
#include <cmath>
#include <limits>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <systemc>
#include <vector>

#include "imload.h"
#include "image.h"
#include "ipoint.h"
#include "fasthessian.h"
#define DEVICE_PATH "/dev/fasthessian_driver"

#define OriHistTh 0.8
#define window M_PI/3
#define IndexSigma 1.0

#define get_sum(I, x1, y1, x2, y2) (I[y1+1][x1+1] + I[y2][x2] - I[y2][x1+1] - I[y1+1][x2])
#define get_wavelet1(IPatch, x, y, size) (get_sum(IPatch, x + size, y, x - size, y - size) - get_sum(IPatch, x + size, y + size, x - size, y))
#define get_wavelet2(IPatch, x, y, size) (get_sum(IPatch, x + size, y + size, x, y - size) - get_sum(IPatch, x, y + size, x - size, y - size))

using namespace std;
using namespace surf;

typedef sc_dt::sc_int<11> num_i;
typedef sc_dt::sc_fixed<48, 30, sc_dt::SC_TRN, sc_dt::SC_SAT> num_f;

Image *_iimage = nullptr;
Ipoint *_current = nullptr;
std::vector<std::vector<std::vector<num_f>>> _index;
bool _doubleImage = false;
num_i _VecLength = 0;
num_i _IndexSize = 4;
num_i _MagFactor = 0;
num_i _OriSize = 0;
num_i _width = 0, _height = 0;

num_f _sine = 0.0, _cose = 1.0;
double SINE, COSE;
std::vector<std::vector<num_f>> _Pixels;

num_f _lookup1[83], _lookup2[40];

double scale;
double row;
double col;
int x;
int y;

void createVector(double scale, double row, double col);
//void AddSample(num_i r, num_i c, num_f rpos, num_f cpos, num_f rx, num_f cx, num_i step);
//void PlaceInIndex(num_f dx, num_i ori1, num_f dy, num_i ori2, num_f rx, num_f cx);
void normalise();
void createLookups();
void initializeGlobals(Image *im, bool dbl, int insi);
int getVectLength();
void setIpoint(Ipoint* ipt);
void assignOrientation();
void makeDescriptor();
void communicateWithDriver(double scale, double row, double col);
    
int VLength; // Length of the descriptor vector

// Forward declaration of the functions to load/save the SURF points
void saveIpoints(string fn, const vector< Ipoint >& keys);

int sc_main (int argc, char **argv)
{
    int samplingStep = 2; // Initial sampling step (default 2)
    int octaves = 4; // Number of analysed octaves (default 4)
    double thres = 4.0; // Blob response treshold
    bool doubleImageSize = false; // Set this flag "true" to double the image size
    int initLobe = 3; // Initial lobe size, default 3 and 5 (with double image size)
    int indexSize = 4; // Spatial size of the descriptor window (default 4)
    struct timezone tz; struct timeval tim1, tim2; // Variables for the timing measure

    // Read the arguments
    ImLoad ImageLoader;
    int arg = 0;
    string fn = "../data/out.surf";
    Image *im=NULL;
    while (++arg < argc) {
        if (! strcmp(argv[arg], "-i"))
            im = ImageLoader.readImage(argv[++arg]);
        if (! strcmp(argv[arg], "-o"))
            fn = argv[++arg];
    }

    // Start measuring the time
    gettimeofday(&tim1, &tz);

    // Create the integral image
    Image iimage(im, doubleImageSize);
  
    //inicijalizacija
    //initializeGlobals(image, false, 4);

    // Start finding the SURF points
    cout << "Finding SURFs...\n";

    // These are the interest points
    vector< Ipoint > ipts;
    ipts.reserve(300);

    // Extract interest points with Fast-Hessian
    FastHessian fh(&iimage, /* pointer to integral image */
                   ipts,
                   thres, /* blob response threshold */
                   doubleImageSize, /* double image size flag */
                   initLobe * 3 /* 3 times lobe size equals the mask size */,
                   samplingStep, /* subsample the blob response map */
                   octaves /* number of octaves to be analysed */);


    fh.getInterestPoints();

    // Initialise the SURF descriptor
    initializeGlobals(&iimage, doubleImageSize, indexSize);
    // Get the length of the descriptor vector resulting from the parameters
    VLength = getVectLength();

    // Compute the orientation and the descriptor for every interest point
    for (unsigned n=0; n<ipts.size(); n++){
        setIpoint(&ipts[n]); // set the current interest point
        assignOrientation(); // assign reproducible orientation
        makeDescriptor(); // make the SURF descriptor
    }
    // stop measuring the time, we're all done
    gettimeofday(&tim2, &tz);

    // save the interest points in the output file
    saveIpoints(fn, ipts);

    // print some nice information on the command prompt
    cout << "Detection time: " <<
        (double)tim2.tv_sec + ((double)tim2.tv_usec)*1e-6 -
        (double)tim1.tv_sec - ((double)tim1.tv_usec)*1e-6 << endl;

    delete im;

    return 0;
}

// Save the interest points to a regular ASCII file
void saveIpoints(string sFileName, const vector< Ipoint >& ipts)
{
    ofstream ipfile(sFileName.c_str());
    if( !ipfile ) {
        cerr << "ERROR in loadIpoints(): "
             << "Couldn't open file '" << sFileName << "'!" << endl;
        return;
    }
  
  
    double sc;
    unsigned count = ipts.size();

    // Write the file header
    ipfile << VLength + 1 << endl << count << endl;

    for (unsigned n=0; n<ipts.size(); n++){
        // circular regions with diameter 5 x scale
        sc = 2.5 * ipts[n].scale; sc*=sc;
        ipfile  << ipts[n].x /* x-location of the interest point */
                << " " << ipts[n].y /* y-location of the interest point */
                << " " << 1.0/sc /* 1/r^2 */
                << " " << 0.0     //(*ipts)[n]->strength /* 0.0 */
                << " " << 1.0/sc; /* 1/r^2 */

        // Here should come the sign of the Laplacian. This is still an open issue
        // that will be fixed in the next version. For the matching, just ignore it
        // at the moment.
        ipfile << " " << 0.0; //(*ipts)[n]->laplace;

        // Here comes the descriptor
        for (int i = 0; i < VLength; i++) {
            ipfile << " " << ipts[n].ivec[i];
        }
        ipfile << endl;
    }

    // Write message to terminal.
    cout << count << " interest points found" << endl;
}

num_f* pixels1D;

void initializePixels1D(int width, int height, vector<vector<num_f>>& Pixels) {
    pixels1D = new num_f[width * height];
    int pixels1D_index = 0;
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            pixels1D[pixels1D_index++] = static_cast<num_f>(Pixels[w][h]);
        }
    }
}

num_f* _index1D;

void initializeIndex1D(int IndexSize, int OriSize) {
    _index1D = new num_f[IndexSize * IndexSize * OriSize]();
}

//Inicijalizacija globalnih promenljivih
void initializeGlobals(Image *im, bool dbl = false, int insi = 4) {
    _iimage = im;
    _doubleImage = dbl;
    _IndexSize = insi;
    _MagFactor = 12 / insi; // Pretpostavka na osnovu prvobitne logike
    _OriSize = 4; // Pretpostavljena vrednost
    _VecLength = _IndexSize * _IndexSize * _OriSize; // Izračunavanje na osnovu datih vrednosti
    _width = im->getWidth();
    _height = im->getHeight();
    // Inicijalizacija _Pixels, _lookup1, _lookup2...
    createLookups(); // Popunjava _lookup1 i _lookup2 tabele
    
    
    double** tempPixels = _iimage->getPixels();
    _Pixels.resize(_height);
    for (int i = 0; i < _height; ++i) {
        _Pixels[i].resize(_width);
        for (int j = 0; j < _width; ++j) {
            _Pixels[i][j] = static_cast<num_f>(tempPixels[i][j]);
        }
    }
  
				initializePixels1D(_width, _height, _Pixels);
    initializeIndex1D(_IndexSize, _OriSize);
    
    // allocate _index
    _index.resize(_IndexSize, std::vector<std::vector<num_f>>(_IndexSize, std::vector<num_f>(_OriSize, 0.0f)));

    // initial sine and cosine
    _sine = 0.0;
    _cose = 1.0;
}

int getVectLength() {
    return _VecLength;
}

void setIpoint(Ipoint* ipt) {
    _current = ipt;
}

void assignOrientation() {
    scale = (1.0+_doubleImage) * _current->scale;
    x = (int)((1.0+_doubleImage) * _current->x + 0.5);
    y = (int)((1.0+_doubleImage) * _current->y + 0.5);
  
    int pixSi = (int)(2*scale + 1.6);
    const int pixSi_2 = (int)(scale + 0.8);
    double weight;
    const int radius=9;
    double dx=0, dy=0, magnitude, angle, distsq;
    const double radiussq = 81.5;
    int y1, x1;
    int yy, xx;

    vector< pair< double, double > > values;
    for (yy = y - pixSi_2*radius, y1= -radius; y1 <= radius; y1++, yy+=pixSi_2){
        for (xx = x - pixSi_2*radius, x1 = -radius; x1 <= radius; x1++, xx+=pixSi_2) {
            // Do not use last row or column, which are not valid
            if (yy + pixSi + 2 < _height && xx + pixSi + 2 < _width && yy - pixSi > -1 && xx - pixSi > -1) {
                distsq = (y1 * y1 + x1 * x1);
        
                if (distsq < radiussq) {
                    weight = _lookup1[(int)distsq];
                    dx = get_wavelet2(_iimage->getPixels(), xx, yy, pixSi);
                    dy = get_wavelet1(_iimage->getPixels(), xx, yy, pixSi);

                    magnitude = sqrt(dx * dx + dy * dy);
                    if (magnitude > 0.0){
                        angle = atan2(dy, dx);
                        values.push_back( make_pair( angle, weight*magnitude ) );
                    }
                }
            }
        }
    }
  
    double best_angle = 0;

    if (values.size()) {
        sort( values.begin(), values.end() );
        int N = values.size();

        float d2Pi = 2.0*M_PI;
    
        for( int i = 0; i < N; i++ ) {
            values.push_back( values[i] );
            values.back().first += d2Pi;
        }

        double part_sum = values[0].second;
        double best_sum = 0;
        double part_angle_sum = values[0].first * values[0].second;

        for( int i = 0, j = 0; i < N && j<2*N; ) {
            if( values[j].first - values[i].first < window ) {
                if( part_sum > best_sum ) {
                    best_angle  = part_angle_sum / part_sum;
                    best_sum = part_sum;
                }
                j++;
                part_sum += values[j].second;
                part_angle_sum += values[j].second * values[j].first;
            }
        else {
            part_sum -= values[i].second;
            part_angle_sum -= values[i].second * values[i].first;
            i++;
        }
        }
    }
    _current->ori = best_angle;
}

/*void makeDescriptor() {
    _current->allocIvec(_VecLength);
  
    // Initialize _index array
    for (int i = 0; i < _IndexSize * _IndexSize * _OriSize; i++) {
        _index1D[i] = 0.0;
    }

    // calculate _sine and co_sine once
    _sine = sin(_current->ori);
    _cose = cos(_current->ori);
    SINE = _sine;
    COSE = _cose;
    
    communicateWithDriver(scale, row, col);
                             
    int v = 0;
    for (int i = 0; i < _IndexSize * _IndexSize * _OriSize; i++) {
        _current -> ivec[v++] = _index1D[i];
    }
  
    }
}


    normalise();
} 
*/


/*void communicateWithDriver(double scale, double row, double col)
{
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        cerr << "Failed to open the device file" << endl;
        return;
    }


    // Allocate memory for the buffer
    unsigned char* buffer = new unsigned char[totalSize];

    // Copy image rows and columns to the buffer
    memcpy(buffer, &scale, sizeof(double));
    memcpy(buffer + sizeof(double), &row, sizeof(double));
				memcpy(buffer + 2 * sizeof(double), &col, sizeof(double));
				memcpy(buffer + 3 * sizeof(double), &SINE, sizeof(double));
				memcpy(buffer + 4 * sizeof(double), &COSE, sizeof(double));

    // Send the buffer to the driver
    ssize_t bytesSent = write(fd, buffer, totalSize);
    if (bytesSent != totalSize)
    {
        cerr << "Failed to send data to driver" << endl;
        delete[] buffer;
        close(fd);
        return;
    }

    // Read data from the driver directly into ptInClusters
    for (int i = 0; i < 64 * sizeof(double); ++i)
    {
        Point3i receivedPoint;
        ssize_t bytesRead = read(fd, &receivedPoint, sizeof(Point3i));
        if (bytesRead != sizeof(Point3i))
        {
            cerr << "Failed to read data from driver" << endl;
            delete[] buffer;
            close(fd);
            return;
        }

        // Add the received Point3i to ptInClusters
        ptInClusters.push_back(receivedPoint);
    }

    cout << "Reading " << ptInClusters.size() << " points from the driver" << endl;

    // Free the buffer and close the file
    delete[] buffer;
    close(fd);
}*/
void printIndex1D(int indexSize, int oriSize) {
    cout << "Vrednosti u index1D:" << endl;
    for (int i = 0; i < indexSize * indexSize * oriSize; i++) {
        cout << "index1D[" << i << "] = " << _index1D[i] << endl;
    }
}

void printPixels1D(int width, int height) {
    cout << "Vrednosti u pixels1D:" << endl;
    for (int i = 0; i < width * height; i++) {
        cout << "pixels1D[" << i << "] = " << pixels1D[i] << endl;
    }
}


void makeDescriptor() {
    _current->allocIvec(_VecLength);
    
    for (int i = 0; i < _IndexSize * _IndexSize * _OriSize; i++) {
        _index1D[i] = 0.0;
    }

    // calculate _sine and co_sine once
    _sine = sin(_current->ori);
    _cose = cos(_current->ori);
    
    // Produce _upright sample vector
    createVector(1.65*(1+_doubleImage)*_current->scale,
                 (1+_doubleImage)*_current->y,
                 (1+_doubleImage)*_current->x);
                             
    int v = 0;
    for (int i = 0; i < _IndexSize * _IndexSize * _OriSize; i++) {
        _current -> ivec[v++] = _index1D[i];
        printIndex1D(_IndexSize, _OriSize);
    }
  printPixels1D(_width, _height);
    normalise();
} 

void createVector(double scale, double row, double col) {
    int iradius, iy, ix;
    double spacing, radius, rpos, cpos, rx, cx;
    int step = MAX((int)(scale/2 + 0.5),1);
  
    iy = (int) (row + 0.5);
    ix = (int) (col + 0.5);

    double fracy = row-iy;
    double fracx = col-ix;
    double fracr =   _cose * fracy + _sine * fracx;
    double fracc = - _sine * fracy + _cose * fracx;
  
    // The spacing of _index samples in terms of pixels at this scale
    spacing = scale * _MagFactor;

    // Radius of _index sample region must extend to diagonal corner of
    // _index patch plus half sample for interpolation.
    radius = 1.4 * spacing * (_IndexSize + 1) / 2.0;
    iradius = (int) (radius/step + 0.5);
  		
    // Examine all points from the gradient image that could lie within the
    // _index square.
    for (int i = 0; i <= 2*iradius; i++) {
            for (int j = 0; j <= 2*iradius; j++) {
               
                // Rotate sample offset to make it relative to key orientation.
                // Uses (x,y) coords.  Also, make subpixel correction as later image
                // offset must be an integer.  Divide by spacing to put in _index units.
				
																num_f temp1_rpos, temp2_rpos, temp3_rpos, temp4_rpos;
																num_f temp1_cpos, temp2_cpos, temp3_cpos, temp4_cpos;
				
																temp1_rpos = _cose * (i - iradius);
																temp2_rpos = _sine * (j - iradius);
																temp3_rpos = temp1_rpos + temp2_rpos; 
																temp4_rpos = step * temp3_rpos - fracr;
																rpos = temp4_rpos / spacing;

																temp1_cpos = - _sine * (i - iradius);
																temp2_cpos = _cose * (j - iradius);
																temp3_cpos = temp1_cpos + temp2_cpos; 
																temp4_cpos = step * temp3_cpos - fracc;
																cpos = temp4_cpos / spacing;
                
                // Compute location of sample in terms of real-valued _index array
                // coordinates.  Subtract 0.5 so that rx of 1.0 means to put full
                // weight on _index[1]
                rx = rpos + 2.0 - 0.5;
                cx = cpos + 2.0 - 0.5;

                // Test whether this sample falls within boundary of _index patch
                if (rx > -1.0 && rx < (double) _IndexSize  &&
                    cx > -1.0 && cx < (double) _IndexSize) {
          
                     num_i r = iy + (i - iradius) * step;
                    num_i c = ix + (j - iradius) * step;
                    num_i ori1, ori2;
                    num_i ri, ci;
                    
                    num_i addSampleStep = int(scale);
                    
                    num_f weight;
                    num_f dxx1, dxx2, dyy1, dyy2;
                    num_f dx, dy;
                    num_f dxx, dyy;
                    
                    num_f rfrac, cfrac;
                    num_f rweight1, rweight2, cweight1, cweight2;
                    
					num_f dx1, dx2, dy1, dy2;
                    
                    if (r >= 1 + addSampleStep && r < _height - 1 - addSampleStep && c >= 1 + addSampleStep && c < _width - 1 - addSampleStep) {
                        weight = _lookup2[num_i(rpos * rpos + cpos * cpos)];
 
                       dxx1 = pixels1D[(r + addSampleStep + 1) * _width + (c + addSampleStep + 1)] 
																												+ pixels1D[(r - addSampleStep) * _width + c]
																												- pixels1D[(r - addSampleStep) * _width + (c + addSampleStep + 1)]
																												- pixels1D[(r + addSampleStep + 1) * _width + c];
																												
																							dxx2 = pixels1D[(r + addSampleStep + 1) * _width + (c + 1)]
																												+ pixels1D[(r - addSampleStep) * _width + (c - addSampleStep)]
																												- pixels1D[(r - addSampleStep) * _width + (c + 1)]
																												- pixels1D[(r + addSampleStep + 1) * _width + (c - addSampleStep)];
																												
																							dyy1 = pixels1D[(r + 1) * _width + (c + addSampleStep + 1)]
																												+ pixels1D[(r - addSampleStep) * _width + (c - addSampleStep)]
																												- pixels1D[(r - addSampleStep) * _width + (c + addSampleStep + 1)]
																												- pixels1D[(r + 1) * _width + (c - addSampleStep)];
																												
																							dyy2 = pixels1D[(r + addSampleStep + 1) * _width + (c + addSampleStep + 1)]
																												+ pixels1D[r * _width + (c - addSampleStep)]
																												- pixels1D[r * _width + (c + addSampleStep + 1)]
																												- pixels1D[(r + addSampleStep + 1) * _width + (c - addSampleStep)];
		 
	
																				
                        dxx = weight * (dxx1 - dxx2);
                        dyy = weight * (dyy1 - dyy2);
						
						dx1 = _cose * dxx;
						dx2 = _sine * dyy;
						dx = dx1 + dx2;
						
						dy1 = _sine * dxx;
						dy2 = _cose * dyy;
						dy = dy1 - dy2;

                        if (dx < 0) ori1 = 0;
                        else ori1 = 1;

                        if (dy < 0) ori2 = 2;
                        else ori2 = 3;

                        if (rx < 0)  ri = 0;
                        else if (rx >= _IndexSize) ri = _IndexSize - 1;
                        else ri = rx;

                        if (cx < 0) ci = 0;
                        else if (cx >= _IndexSize) ci = _IndexSize - 1;
                        else ci = cx;

                        rfrac = rx - ri;
                        cfrac = cx - ci;

                        if (rfrac < 0.0) rfrac = 0.0;
                        else if (rfrac > 1.0) rfrac = 1.0;
                        
                        if (cfrac < 0.0) cfrac = 0.0;
                        else if (cfrac > 1.0) cfrac = 1.0;

                        rweight1 = dx * (1.0 - rfrac);
                        rweight2 = dy * (1.0 - rfrac);
                        cweight1 = rweight1 * (1.0 - cfrac);
                        cweight2 = rweight2 * (1.0 - cfrac);

                        if (ri >= 0 && ri < _IndexSize && ci >= 0 && ci < _IndexSize) {
																												_index1D[ri * (_IndexSize * _OriSize) + ci * _OriSize + ori1] = cweight1;
																												_index1D[ri * (_IndexSize * _OriSize) + ci * _OriSize + ori2] = cweight2;
																								}
                    }  
                }             
           	}
      		}
    }



// Normalise descriptor vector for illumination invariance for
// Lambertian surfaces
void normalise() {
    num_f val, sqlen = 0.0, fac;
    for (num_i i = 0; i < _VecLength; i++){
        val = _current->ivec[i];
        sqlen += val * val;
    }
    fac = 1.0/sqrt(sqlen);
    for (num_i i = 0; i < _VecLength; i++)
        _current->ivec[i] *= fac;
}

// Create _lookup tables
void createLookups(){
    for (int n=0;n<83;n++)
        _lookup1[n]=exp(-((double)(n+0.5))/12.5);

    for (int n=0;n<40;n++)
        _lookup2[n]=exp(-((double)(n+0.5))/8.0);   
}
