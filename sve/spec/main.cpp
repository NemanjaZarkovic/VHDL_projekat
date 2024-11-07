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
#include <cstdint>
#include <cstring>
#include <sstream>

#include "imload.h"
#include "image.h"
#include "ipoint.h"
#include "fasthessian.h"
#define DEVICE_PATH "/dev/surf_driver"

#define OriHistTh 0.8
#define window M_PI/3
#define IndexSigma 1.0

#define get_sum(I, x1, y1, x2, y2) (I[y1+1][x1+1] + I[y2][x2] - I[y2][x1+1] - I[y1+1][x2])
#define get_wavelet1(IPatch, x, y, size) (get_sum(IPatch, x + size, y, x - size, y - size) - get_sum(IPatch, x + size, y + size, x - size, y))
#define get_wavelet2(IPatch, x, y, size) (get_sum(IPatch, x + size, y + size, x, y - size) - get_sum(IPatch, x, y + size, x - size, y - size))

using namespace std;
using namespace surf;

Image *_iimage = nullptr;
Ipoint *_current = nullptr;
std::vector<std::vector<std::vector<double>>> _index;
bool _doubleImage = false;
int _VecLength = 0;
int _IndexSize = 4;
int _MagFactor = 0;
int _OriSize = 0;
int _width = 0, _height = 0;

double _sine = 0.0, _cose = 1.0;
double SINE, COSE;
std::vector<std::vector<double>> _Pixels;

double _lookup1[83], _lookup2[40];

double scale;
double row;
double col;
int x;
int y;

void createVector(double scale, double row, double col);
void normalise();
void createLookups();
void initializeGlobals(Image *im, bool dbl, int insi);
int getVectLength();
void setIpoint(Ipoint* ipt);
void assignOrientation();
void makeDescriptor();
void doubleToUchar(unsigned char *buf, double val);
//void communicateWithDriver(unsigned char* scaleBuf, unsigned char* rowBuf, unsigned char* colBuf, unsigned char* sineBuf, unsigned char* coseBuf);
void communicateWithDriver(double scale, double row, double col) ;
    
int VLength; // Length of the descriptor vector

// Forward declaration of the functions to load/save the SURF points
void saveIpoints(string fn, const vector< Ipoint >& keys);

int main (int argc, char **argv)
{
    int samplingStep = 2; // Initial sampling step (default 2)
    int octaves = 4; // Number of analysed octaves (default 4)
    double thres = 4.0; // Blob response threshold
    bool doubleImageSize = false; // Set this flag "true" to double the image size
    int initLobe = 3; // Initial lobe size, default 3 and 5 (with double image size)
    int indexSize = 4; // Spatial size of the descriptor window (default 4)
    struct timezone tz; struct timeval tim1, tim2; // Variables for the timing measure

    // Read the arguments
    ImLoad ImageLoader;
    int arg = 0;
    string fn = "../data/out.surf";
    Image *im = NULL;
    while (++arg < argc) {
        if (!strcmp(argv[arg], "-i"))
            im = ImageLoader.readImage(argv[++arg]);
        if (!strcmp(argv[arg], "-o"))
            fn = argv[++arg];
    }

    // Start measuring the time
    gettimeofday(&tim1, &tz);

    // Create the integral image
    Image iimage(im, doubleImageSize);

    // Start finding the SURF points
    cout << "Finding SURFs...\n";

    // These are the interest points
    vector<Ipoint> ipts;
    ipts.reserve(300);

    // Extract interest points with Fast-Hessian
    FastHessian fh(&iimage, ipts, thres, doubleImageSize, initLobe * 3, samplingStep, octaves);
    fh.getInterestPoints();

    // Initialise the SURF descriptor
    initializeGlobals(&iimage, doubleImageSize, indexSize);

    // Get the length of the descriptor vector resulting from the parameters
    VLength = getVectLength();

    // Compute the orientation and the descriptor for every interest point
    for (unsigned n = 0; n < ipts.size(); n++) {
        setIpoint(&ipts[n]); // Set the current interest point
        assignOrientation(); // Assign reproducible orientation
        makeDescriptor(); // Make the SURF descriptor
    }

    // Stop measuring the time
    gettimeofday(&tim2, &tz);

    // Save the interest points in the output file
    saveIpoints(fn, ipts);

    // Print some nice information on the command prompt
    cout << "Detection time: " <<
        (double)tim2.tv_sec + ((double)tim2.tv_usec)*1e-6 -
        (double)tim1.tv_sec - ((double)tim1.tv_usec)*1e-6 << endl;

    delete im;

    return 0;
}

// Save the interest points to a regular ASCII file
void saveIpoints(string sFileName, const vector<Ipoint>& ipts)
{
    ofstream ipfile(sFileName.c_str());
    if (!ipfile) {
        cerr << "ERROR in loadIpoints(): Couldn't open file '" << sFileName << "'!" << endl;
        return;
    }

    double sc;
    unsigned count = ipts.size();

    // Write the file header
    ipfile << VLength + 1 << endl << count << endl;

    for (unsigned n = 0; n < ipts.size(); n++) {
        sc = 2.5 * ipts[n].scale;
        sc *= sc;
        ipfile << ipts[n].x << " " << ipts[n].y << " " << 1.0 / sc
               << " " << 0.0 << " " << 1.0 / sc;
        ipfile << " " << 0.0; // Placeholder for the Laplacian sign

        for (int i = 0; i < VLength; i++) {
            ipfile << " " << ipts[n].ivec[i];
        }
        ipfile << endl;
    }

    cout << count << " interest points found" << endl;
}

double *pixels1D;

void initializePixels1D(int width, int height, vector<vector<double>>& Pixels) {
    pixels1D = new double[width * height];
    int pixels1D_index = 0;
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            pixels1D[pixels1D_index++] = Pixels[w][h];
        }
    }
}

double *_index1D;

void initializeIndex1D(int IndexSize, int OriSize) {
    _index1D = new double[IndexSize * IndexSize * OriSize]();
}

// Initialize global variables
void initializeGlobals(Image *im, bool dbl = false, int insi = 4) {
    _iimage = im;
    _doubleImage = dbl;
    _IndexSize = insi;
    _MagFactor = 12 / insi;
    _OriSize = 4;
    _VecLength = _IndexSize * _IndexSize * _OriSize;
    _width = im->getWidth();
    _height = im->getHeight();

    createLookups(); // Populate lookup tables
    
    double **tempPixels = _iimage->getPixels();
    _Pixels.resize(_height);
    for (int i = 0; i < _height; ++i) {
        _Pixels[i].resize(_width);
        for (int j = 0; j < _width; ++j) {
            _Pixels[i][j] = tempPixels[i][j];
        }
    }

    initializePixels1D(_width, _height, _Pixels);
    initializeIndex1D(_IndexSize, _OriSize);

    _index.resize(_IndexSize, std::vector<std::vector<double>>(_IndexSize, std::vector<double>(_OriSize, 0.0)));

    _sine = 0.0;
    _cose = 1.0;
}

int getVectLength() {
    return _VecLength;
}

void setIpoint(Ipoint *ipt) {
    _current = ipt;
}

void assignOrientation() {
    scale = (1.0 + _doubleImage) * _current->scale;
    x = (int)((1.0 + _doubleImage) * _current->x + 0.5);
    y = (int)((1.0 + _doubleImage) * _current->y + 0.5);

    int pixSi = (int)(2 * scale + 1.6);
    const int pixSi_2 = (int)(scale + 0.8);
    double weight;
    const int radius = 9;
    double dx = 0, dy = 0, magnitude, angle, distsq;
    const double radiussq = 81.5;
    int y1, x1;
    int yy, xx;

    vector<pair<double, double>> values;
    for (yy = y - pixSi_2 * radius, y1 = -radius; y1 <= radius; y1++, yy += pixSi_2) {
        for (xx = x - pixSi_2 * radius, x1 = -radius; x1 <= radius; x1++, xx += pixSi_2) {
            if (yy + pixSi + 2 < _height && xx + pixSi + 2 < _width && yy - pixSi > -1 && xx - pixSi > -1) {
                distsq = (y1 * y1 + x1 * x1);

                if (distsq < radiussq) {
                    weight = _lookup1[(int)distsq];
                    dx = get_wavelet2(_iimage->getPixels(), xx, yy, pixSi);
                    dy = get_wavelet1(_iimage->getPixels(), xx, yy, pixSi);

                    magnitude = sqrt(dx * dx + dy * dy);
                    if (magnitude > 0.0) {
                        angle = atan2(dy, dx);
                        values.push_back(make_pair(angle, weight * magnitude));
                    }
                }
            }
        }
    }

    double best_angle = 0;

    if (!values.empty()) {
        sort(values.begin(), values.end());
        int N = values.size();

        float d2Pi = 2.0 * M_PI;

        for (int i = 0; i < N; i++) {
            values.push_back(values[i]);
            values.back().first += d2Pi;
        }

        double part_sum = values[0].second;
        double best_sum = 0;
        double part_angle_sum = values[0].first * values[0].second;

        for (int i = 0, j = 0; i < N && j < 2 * N;) {
            if (values[j].first - values[i].first < window) {
                if (part_sum > best_sum) {
                    best_angle = part_angle_sum / part_sum;
                    best_sum = part_sum;
                }
                j++;
                part_sum += values[j].second;
                part_angle_sum += values[j].second * values[j].first;
            } else {
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
    normalise();
}

/*void doubleToUchar(unsigned char *buf, double val)
{
    // Convert double to integer using fixed-point scaling
    // Assuming you want to transform a double to an integer-like format
    int64_t intVal = static_cast<int64_t>(val * (1LL << 30)); // Scale by 2^30 to preserve precision

    static_assert(sizeof(intVal) == sizeof(val), "Size mismatch between double and uint64_t");
    std::memcpy(&intVal, &val, sizeof(val));

    // Kopiramo svaki bajt uint64_t-a u unsigned char niz
    for (int i = 0; i < sizeof(uint64_t); ++i)
    {
        buf[i] = static_cast<unsigned char>((intVal >> (i * 8)) & 0xFF);
    }
}

unsigned char scaleBuffer[sizeof(double)];
unsigned char rowBuffer[sizeof(double)];
unsigned char colBuffer[sizeof(double)];
unsigned char sineBuffer[sizeof(double)];
unsigned char coseBuffer[sizeof(double)];

// Pretvaranje `double` vrednosti u `unsigned char` korišćenjem funkcije `doubleToUchar`
doubleToUchar(scaleBuffer, scale);
doubleToUchar(rowBuffer, row);
doubleToUchar(colBuffer, col);
doubleToUchar(sineBuffer, SINE);
doubleToUchar(coseBuffer, COSE);



void communicateWithDriver(unsigned char* scaleBuf, unsigned char* rowBuf, unsigned char* colBuf, unsigned char* sineBuf, unsigned char* coseBuf) {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        std::cerr << "Neuspešno otvaranje uređaja\n";
        return;
    }

    // Veličina bafera koji uključuje scale, row, col, SINE, COSE, pixels1D i _lookup2
    size_t pixelsSize = _width * _height * sizeof(unsigned char) * sizeof(double);
    size_t lookup2Size = 40 * sizeof(unsigned char) * sizeof(double); // Veličina niza `_lookup2`
    size_t totalSize = sizeof(unsigned char) * sizeof(double) * 5 + pixelsSize + lookup2Size; // scale, row, col, SINE, COSE + pixels1D + _lookup2
    unsigned char* buffer = new unsigned char[totalSize];

    // Kopiraj podatke za scale, row, col, SINE, COSE u unsigned char bafer
    memcpy(buffer, scaleBuf, sizeof(double));
    memcpy(buffer + sizeof(double), rowBuf, sizeof(double));
    memcpy(buffer + 2 * sizeof(double), colBuf, sizeof(double));
    memcpy(buffer + 3 * sizeof(double), sineBuf, sizeof(double));
    memcpy(buffer + 4 * sizeof(double), coseBuf, sizeof(double));

    // Kopiraj niz `pixels1D` u bafer
    for (size_t i = 0; i < _width * _height; ++i) {
        doubleToUchar(buffer + sizeof(unsigned char) * sizeof(double) * 5 + i * sizeof(double), pixels1D[i]);
    }

    // Kopiraj niz `_lookup2` u bafer
    for (size_t i = 0; i < 40; ++i) {
        doubleToUchar(buffer + sizeof(unsigned char) * sizeof(double) * 5 + pixelsSize + i * sizeof(double), _lookup2[i]);
    }

    // Pošalji bafer kernelu
    ssize_t bytesSent = write(fd, buffer, totalSize);
    if (bytesSent < 0) {
        std::cerr << "Greška prilikom slanja bafera drajveru\n";
    } 
    else if ((size_t)bytesSent != totalSize) {
        std::cerr << "Neispravan broj poslatih bajtova drajveru\n";
        delete[] buffer;
        close(fd);
        return;
    }

    // Sada ćemo čitati podatke iz drajvera (_index1D)
    int IndexSize = _IndexSize;  
    int OriSize = _OriSize;      
    size_t totalReadSize = IndexSize * IndexSize * OriSize * sizeof(double);
    double* readBuffer = new double[IndexSize * IndexSize * OriSize];

    // Čitaj podatke iz drajvera
    ssize_t bytesRead = read(fd, readBuffer, totalReadSize);
    if (bytesRead < 0) {
        perror("Greška prilikom čitanja podataka iz drajvera");
        delete[] readBuffer;
        close(fd);
        return;
    }

    std::cout << "Pročitano " << bytesRead << " bajtova iz drajvera\n";

    // Obrada podataka (prikaz kao primer)
    for (size_t i = 0; i < bytesRead / sizeof(double); ++i) {
        std::cout << "readBuffer[" << i << "] = " << readBuffer[i] << std::endl;
    }

    // Oslobodi resurse
    delete[] readBuffer;
    delete[] buffer;
    close(fd);
}*/


void communicateWithDriver(double scale, double row, double col) {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        cerr << "Neuspešno otvaranje uređaja\n";
        return;
    }

    // Ukupna veličina bafera sada uključuje pixels1D i _lookup2
    size_t pixelsSize = _width * _height * sizeof(double);
    size_t lookup2Size = 40 * sizeof(double); // Veličina niza _lookup2
    size_t totalSize = sizeof(double) * 5 + pixelsSize + lookup2Size; // scale, row, col, SINE, COSE + pixels1D + _lookup2
    unsigned char* buffer = new unsigned char[totalSize];

    // Kopiraj podatke za scale, row, col, SINE, COSE
    memcpy(buffer, &scale, sizeof(double));
    memcpy(buffer + sizeof(double), &row, sizeof(double));
    memcpy(buffer + 2 * sizeof(double), &col, sizeof(double));
    memcpy(buffer + 3 * sizeof(double), &SINE, sizeof(double));
    memcpy(buffer + 4 * sizeof(double), &COSE, sizeof(double));

    // Kopiraj niz pixels1D u bafer
    memcpy(buffer + sizeof(double) * 5, pixels1D, pixelsSize);

    // Kopiraj niz _lookup2 u bafer
    memcpy(buffer + sizeof(double) * 5 + pixelsSize, _lookup2, lookup2Size);

    // Pošalji bafer kernelu
    ssize_t bytesSent = write(fd, buffer, totalSize);
    if (bytesSent < 0) {
        cerr << "Greška prilikom slanja bafera drajveru\n";
    } 
    else if ((size_t)bytesSent != totalSize) {
        cerr << "Neispravan broj poslatih bajtova drajveru\n";
        delete[] buffer;
        close(fd);
        return;
    }

    // Sada ćemo čitati podatke iz drajvera (_index1D)
    int IndexSize = _IndexSize;  
    int OriSize = _OriSize;      
    // Pripremi bafer za čitanje podataka
    size_t totalReadSize = IndexSize * IndexSize * OriSize * sizeof(double);
    double* readBuffer = new double[IndexSize * IndexSize * OriSize];

    // Čitaj podatke iz drajvera
    ssize_t bytesRead = read(fd, readBuffer, totalReadSize);
    if (bytesRead < 0) {
        perror("Greška prilikom čitanja podataka iz drajvera");
        delete[] readBuffer;
        close(fd);
        return;
    }

    std::cout << "Pročitano " << bytesRead << " bajtova iz drajvera\n";

    // Obrada podataka (prikaz kao primer)
    for (size_t i = 0; i < bytesRead / sizeof(double); ++i) {
        std::cout << "readBuffer[" << i << "] = " << readBuffer[i] << std::endl;
    }

    // Oslobodi resurse
    delete[] readBuffer;
    close(fd);
}
    

// Normalise descriptor vector for illumination invariance for
// Lambertian surfaces
void normalise() {
    double val, sqlen = 0.0, fac;
    for (int i = 0; i < _VecLength; i++) {
        val = _current->ivec[i];
        sqlen += val * val;
    }
    fac = 1.0 / sqrt(sqlen);
    for (int i = 0; i < _VecLength; i++)
        _current->ivec[i] *= fac;
}

// Create lookup tables
void createLookups() {
    for (int n = 0; n < 83; n++)
        _lookup1[n] = exp(-((double)(n + 0.5)) / 12.5);

    for (int n = 0; n < 40; n++)
        _lookup2[n] = exp(-((double)(n + 0.5)) / 8.0);
}

