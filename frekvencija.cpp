#include <math.h>
#include <stdio.h>

double _lookup2[40] = { .939411163330078125, .829029083251953125, .7316131591796875, .6456451416015625, .569782257080078125, .50283050537109375, .443744659423828125, .391605377197265625, .34558868408203125, .304981231689453125, .269145965576171875, .237518310546875, .2096099853515625, .184978485107421875, .163242340087890625, .144062042236328125, .127132415771484375, .112194061279296875, .099010467529296875, .087375640869140625, .07711029052734375, .068050384521484375, .06005096435546875, .052997589111328125, .0467681884765625, .041271209716796875, .0364227294921875, .03214263916015625, .0283660888671875, .02503204345703125, .022090911865234375, .01949310302734375, .01720428466796875, .0151824951171875, .013397216796875, .011821746826171875, .010433197021484375, .00920867919921875, .00812530517578125, .007171630859375};
int _IndexSize = 4;
int iradius = 23;
double _cose = 0.93941116330078125, _sine = .829029083251953125;
int step = 4;
double fracr = 4.29283, fracc = 3.28324;
double spacing = 3.50605;
int iy = 2; ix = 3;
double _Pixels[4] = { 4797.886272430419921875, 8336.73332977294921875, 10852.898036956787109375, 1873.266666412353515625};

void hard(double _index[4][4][4]) {
    double rpos = (step*(_cose + _sine) - fracr) / spacing;
    double cpos = (step*(- _sine + _cose) - fracc) / spacing;
 
    double rx = rpos + _IndexSize / 2.0 - 0.5;
    double cx = cpos + _IndexSize / 2.0 - 0.5;

    if (rx > -1.0 && rx < (double) _IndexSize &&
        cx > -1.0 && cx < (double) _IndexSize) {
        
        int r = iy + step;
        int c = ix + step;
        int ori1 = 1, ori2 = 2;
        int ri = 2, ci = 3;
        int scale = 2;
        int _height = 129, _width = 129;

        int addSampleStep = scale;

        double weight;
        double dxx1, dxx2, dyy1, dyy2;
        double dx, dy;
        double dxx, dyy;
        double rfrac, cfrac;
        double rweight1, rweight2, cweight1, cweight2;

        if (r >= 1 + addSampleStep && r < _height - 1 - addSampleStep && c >= 1 + addSampleStep && c < _width - 1 - addSampleStep) {

            weight = _lookup2[(int)(rpos * rpos + cpos * cpos)];

            dxx1 = _Pixels[0];
            dxx2 = _Pixels[1];
            dyy1 = _Pixels[2];
            dyy2 = _Pixels[3];

            dxx = weight * (dxx1 - dxx2);
            dyy = weight * (dyy1 - dyy2);
            dx = _cose * dxx + _sine * dyy;
            dy = _sine * dxx - _cose * dyy;

            if (dx < 0) ori1 = 0;
            else ori1 = 1;

            if (dy < 0) ori2 = 2;
            else ori2 = 3;

            if (rx < 0) ri = 0;
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
                _index[ri][ci][ori1] = cweight1;
                _index[ri][ci][ori2] = cweight2;
            }
        }
    }
}

int main() {
    double _index[4][4][4];

    hard(_index);
    
    return 0;
}