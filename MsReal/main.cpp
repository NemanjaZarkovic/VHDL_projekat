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

/*num_f* pixels1D;

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
}*/

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
  
    /*for (int i = 0; i < _height; ++i) {
        for (int j = 0; j < _width; ++j) {
            cout << "Pixels[" << i << "][" << j << "]" << " = " << _Pixels[i][j] << endl;
        }
    }*/
		
				//initializePixels1D(_width, _height, _Pixels);
    //initializeIndex1D(_IndexSize, _OriSize);
    
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

void makeDescriptor() {
    _current->allocIvec(_VecLength);
  
    // Initialize _index array
    for (int i = 0; i < _IndexSize; i++) {
        for (int j = 0; j < _IndexSize; j++) {
            for (int k = 0; k < _OriSize; k++) {
                _index[i][j][k] = 0.0;
            }
        }
    }

    // calculate _sine and co_sine once
    _sine = sin(_current->ori);
    _cose = cos(_current->ori);
    SINE = _sine;
    COSE = _cose;
    
    communicateWithDriver(scale, row, col);
                             
    int v = 0;
    for (int i = 0; i < _IndexSize; i++){
        for (int j = 0; j < _IndexSize; j++){
            for (int k = 0; k < _OriSize; k++)
            _current->ivec[v++] = _index[i][j][k];
        }
    }
  		for (int i = 0; i < _IndexSize; i++) {
    for (int j = 0; j < _IndexSize; j++) {
        cout << "Broj elemenata u _index[" << i << "][" << j << "] je: " << _OriSize << endl;
        for (int k = 0; k < _OriSize; k++) {
            cout << "_index[" << i << "][" << j << "][" << k << "] = " << _index[i][j][k] << endl;
        }
    }
}


    normalise();
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
    communicateWithDriver(scale, row, col);
                             
    int v = 0;
    for (int i = 0; i < _IndexSize * _IndexSize * _OriSize; i++) {
        _current -> ivec[v++] = _index1D[i];
    }
  
    normalise();
} */

void communicateWithDriver(double scale, double row, double col)
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