#include "cpu.hpp"

using namespace surf;

SC_HAS_PROCESS(Cpu);


//STA URADITI SA OVIM GDE TREBA DA SE UCITA SLIKA
Cpu::Cpu(sc_module_name name,const std::string& image_name, int argc, char **argv):sc_module(name), offset(sc_core::SC_ZERO_TIME)
{

    SC_THREAD(software);
    ImLoad ImageLoader;
    std::string image_path = "../data/" + image_name + ".jpg"; // Formiranje putanje do slike
    _im = ImageLoader.readImage(image_path.c_str()); // Učitavanje slike iz datoteke
    Image image = _im;
    if (!_im) {
        std::cerr << "Nije moguće učitati sliku iz datoteke: " << image_path << std::endl;
        return;
    }
    int width = image.getWidth();
    int height = image.getHeight();
    if (width > 129 || height > 129) {
        // Print a message indicating that the image is not of the correct size
        std::cerr << "Image size is not 128x128 pixels. Please load an image with the correct size." << std::endl;
        exit(0);
    }
    // Učitavanje argumenata iz naredbenog reda
    int arg = 0;
    while (++arg < argc) {
        if (!strcmp(argv[arg], "-o"))
            fn = argv[++arg];
    }
    cout << "Cpu constructed" << endl;
}

void Cpu::software()
{
    int samplingStep = 2; // Initial sampling step (default 2)
    int octaves = 4; // Number of analysed octaves (default 4)
    double thres = 4.0; // Blob response treshold
    bool doubleImageSize = false; // Set this flag "true" to double the image size
    int initLobe = 3; // Initial lobe size, default 3 and 5 (with double image size)
    int indexSize = 4; // Spatial size of the descriptor window (default 4)
    struct timezone tz; struct timeval tim1, tim2; // Variables for the timing measure
    unsigned char *buf; //za prebacivanje podataka iz memorije u IP
    
      // Start measuring the time
    gettimeofday(&tim1, &tz);

    // Create the integral image
    Image iimage(_im, doubleImageSize);

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
    
    std::string fn = "../data/out.surf";
    // save the interest points in the output file
    saveIpoints(fn, ipts);

    // print some nice information on the command prompt
    cout << "Detection time: " <<
        (double)tim2.tv_sec + ((double)tim2.tv_usec)*1e-6 -
        (double)tim1.tv_sec - ((double)tim1.tv_usec)*1e-6 << endl;
        
    cout << "Simulacija " << offset << endl;

    delete _im;

}

void Cpu::saveIpoints(string sFileName, const vector< Ipoint >& ipts)
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


void Cpu::initializeGlobals(Image *im, bool dbl = false, int insi = 4) {
    _iimage = im;
    _doubleImage = dbl;
    //_IndexSize = insi;
    _MagFactor = 12 / insi; // Pretpostavka na osnovu prvobitne logike
    _OriSize = 4; // Pretpostavljena vrednost
    _VecLength = _IndexSize * _IndexSize * _OriSize; // Izračunavanje na osnovu datih vrednosti
    //_width = im->getWidth();
    //_height = im->getHeight();
    
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
  
    num_f* pixels1D;
  
    pixels1D = new num_f[_width * _height];
    int pixels1D_index = 0;
    for (int w = 0; w < _width; w++) {
        for (int h = 0; h < _height; h++) {
            pixels1D[pixels1D_index++] = static_cast<num_f>(_Pixels[w][h]);
        }
    }
                  
    for (long unsigned int i = 0; i < _width*_height; ++i)
    {
        mem.push_back(pixels1D[i]);
    }
                  
    for (int i = 0; i < _width * _height; i++)
    {
        pl_t pl;
        offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
        unsigned char* buf;
        buf = (unsigned char*)&mem[i];
        pl.set_address(addr_Pixels1+i);
        pl.set_data_length(1);
        pl.set_data_ptr(buf);
        pl.set_command(tlm::TLM_WRITE_COMMAND);
        pl.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        interconnect_socket->b_transport(pl, offset);
     }
                  
                  
  

    // allocate _index
    _index.resize(_IndexSize, std::vector<std::vector<num_f>>(_IndexSize, std::vector<num_f>(_OriSize, 0.0f)));

    // initial sine and cosine
    _sine = 0.0;
    _cose = 1.0;
}


int Cpu::getVectLength() {
    return _VecLength;
}


void Cpu::setIpoint(Ipoint* ipt) {  
    _current = ipt;
}


void Cpu::assignOrientation() {
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


void Cpu::makeDescriptor() {
    _current->allocIvec(_VecLength);

    // calculate _sine and co_sine once
    _sine = sin(_current->ori);
    _cose = cos(_current->ori);
    
    // Produce _upright sample vector
    createVector(1.65*(1+_doubleImage)*_current->scale,
                 (1+_doubleImage)*_current->y,
                 (1+_doubleImage)*_current->x);

    int v = 0;
    for (int i = 0; i < _IndexSize; i++){
        for (int j = 0; j < _IndexSize; j++){
            for (int k = 0; k < _OriSize; k++) {
                _current->ivec[v++] = _index[i][j][k];
            }
        }
    }
    normalise();
}

void Cpu::createVector(double scale, double row, double col) {
    int i, j, iradius, iy, ix;
    double spacing, inv_spacing, radius, rx, cx;
    double rpos,cpos;
    int step = MAX((int)(scale/2 + 0.5),1);
  
    iy = (int) (row + 0.5);
    ix = (int) (col + 0.5);

    double fracy = row-iy;
    double fracx = col-ix;
    double fracr =   _cose * fracy + _sine * fracx;
    double fracc = - _sine * fracy + _cose * fracx;
    
    //cout << "iy: " << iy << endl;
    //cout << "ix: " << ix << endl;
    //cout << "fracy: " << fracy << endl;
    //cout << "fracx: " << fracx << endl;
    //cout << "fracr: " << fracr << endl;
    //cout << "fracc: " << fracc << endl;
  
  
    //cout << "fracr: " << fracr << endl;
  
    // The spacing of _index samples in terms of pixels at this scale
    spacing = scale * _MagFactor;
    
    inv_spacing = 1/spacing;

    // Radius of _index sample region must extend to diagonal corner of
    // _index patch plus half sample for interpolation.
    radius = 1.4 * spacing * (_IndexSize + 1) / 2.0;
    iradius = (int) (radius/step + 0.5);
  
    //cout << "iradius: " << iradius << endl; 
 
    bool done = 0;
    int ready = 1;
    bool need_start = 0;
    
    write_hard_int(addr_scale, scale);
        //cout << "scale CPU: " << scale << endl;
    write_hard_int(addr_iradius, iradius);
        //cout << "iradius CPU: " << iradius << endl;
    write_hard_double(addr_fracr, fracr);
        //cout << "fracr CPU: " << fracr << endl;
    write_hard_double(addr_fracc, fracc);
        //cout << "fracc CPU: " << fracc << endl;
    write_hard_double(addr_spacing, inv_spacing);
        //cout << "spacing CPU: " << inv_spacing << endl;
    write_hard_int(addr_step, step);
        //cout << "step CPU: " << step << endl;
    write_hard_double(addr_sine, _sine);
        //cout << "_sine CPU: " << _sine << endl;
    write_hard_double(addr_cose, _cose);
        //cout << "_cose CPU: " << _cose << endl;
    write_hard_int(addr_iy, iy);
        //cout << "iy CPU: " << iy << endl;
    write_hard_int(addr_ix, ix);
        //cout << "ix CPU: " << ix << endl;
          

    vector<num_f> index1d;
          
    while(!done)
    {    
        if(ready)
        {  
            need_start = 1;
        }
              
          
        if (need_start)
        {
            write_hard_int(addr_start,1);
            need_start = 0;
        }
          
        while(ready)
        {
            ready = read_hard_int(addr_ready);
            cout<< "IP is about to start" << endl;
            if (!ready)
                write_hard_int(addr_start,0);
        }
          
        ready = read_hard_int(addr_ready);
          
        //cout << "Processing done" << endl;
              

          
        if(ready)
        { 
            index1d.clear();
            
            for (int i = 0; i < 64 ; i++) {
                offset += sc_core::sc_time(DELAY+DELAY, sc_core::SC_NS);
                index1d.push_back(read_mem(addr_index1+i));
            }
            
            int index_1d_index = 0;
            for (int i = 0; i < _IndexSize; ++i) {
                for (int j = 0; j < _IndexSize; ++j) {
                    for (int k = 0; k < 4; ++k) {
                        _index[i][j][k] = index1d[index_1d_index++];
                    }
                }
            }
                  
            /*for (int i = 0; i < _IndexSize; ++i) {
                for (int j = 0; j < _IndexSize; ++j) {
                    for (int k = 0; k < 4; ++k) {
                        cout << "IZ CPU-a _index[" << i << "]" << "[" << j << "]" << "[" << k << "]" << " = " << _index[i][j][k] << endl;
                    }
                }
            }
                  
            cout << "/////////////////////////////////////////////////////////////" << endl;*/  
                  
            ready = 0;
            need_start = 0;
            done = 1;
        }    
    }     
}

//Normalise descriptor vector for illumination invariance for Lambertian surfaces
void Cpu::normalise() {
    num_f val, sqlen = 0.0, fac;
    for (num_i i = 0; i < _VecLength; i++){
        val = _current->ivec[i];
        sqlen += val * val;
    }
    fac = 1.0/sqrt(sqlen);
    for (num_i i = 0; i < _VecLength; i++)
        _current->ivec[i] *= fac;
}

//Create _lookup tables
void Cpu::createLookups() {
    for (int n=0;n<83;n++)
        _lookup1[n]=exp(-((double)(n+0.5))/12.5);

    for (int n=0;n<40;n++)
        _lookup2[n]=exp(-((double)(n+0.5))/8.0);
}

num_f Cpu::read_mem(sc_dt::sc_uint<64> addr)
{
    pl_t pl;
    sc_dt::sc_int <64> val;
    unsigned char buf[6];
    pl.set_address(addr);
    pl.set_data_length(6);
    pl.set_data_ptr(buf);
    pl.set_command( tlm::TLM_READ_COMMAND );
    pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
    interconnect_socket->b_transport(pl,offset);
    
    num_f mega = toNum_f(buf);
    
    //cout << "buf iz cpu-a: " << mega << endl;
    
    return toNum_f(buf);
}



int Cpu::read_hard_int(sc_uint<64> addr)
{
    pl_t pl;
    unsigned char buf[8];
    pl.set_address(addr);
    pl.set_data_length(1);
    pl.set_data_ptr(buf);
    pl.set_command(tlm::TLM_READ_COMMAND);
    pl.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    sc_core::sc_time offset = sc_core::SC_ZERO_TIME;
    interconnect_socket->b_transport(pl, offset);
    return toInt(buf);
}

double Cpu::read_hard_double(sc_uint<64> addr)
{
    pl_t pl;
    unsigned char buf[8];
    pl.set_address(addr);
    pl.set_data_length(1);
    pl.set_data_ptr(buf);
    pl.set_command(tlm::TLM_READ_COMMAND);
    pl.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    sc_core::sc_time offset = sc_core::SC_ZERO_TIME;
    interconnect_socket->b_transport(pl, offset);
    return toDouble(buf);        
}


void Cpu::write_hard_int(sc_uint<64> addr, int val)
{
    pl_t pl;
    unsigned char buf[4];
    intToUchar(buf,val);
    pl.set_address(addr);
    pl.set_data_length(1);
    pl.set_data_ptr(buf);
    pl.set_command(tlm::TLM_WRITE_COMMAND);
    pl.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    interconnect_socket->b_transport(pl, offset);
}


void Cpu::write_hard_double(sc_uint<64> addr, double val)
{
    pl_t pl;
    unsigned char buf[8];
    doubleToUchar(buf,val);
    pl.set_address(addr);
    pl.set_data_length(1);
    pl.set_data_ptr(buf);
    pl.set_command(tlm::TLM_WRITE_COMMAND);
    pl.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    interconnect_socket->b_transport(pl, offset);
}
