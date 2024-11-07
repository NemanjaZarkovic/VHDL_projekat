#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/version.h>


#define SURF_NUMBER 6
#define MAX_IMAGE_DATA_SIZE 129*129
#define DEVICE_NAME "surf_driver"
#define BUF_LEN 200000
#define IOCTL_SET_IPOINT _IOW('i', 1, struct ipoint_data)


MODULE_LICENSE("GPL");
MODULE_AUTHOR("y23-g03");
MODULE_DESCRIPTION("surf driver");

int major_num;
//double _index[x][y][z];
bool _doubleImage = false;
int _IndexSize = 4;
int _OriSize = 0;
int _MagFactor = 0;

struct ipoint_data {
    int ipoint_row;//x
    int ipoint_col;//y
    int ipoint_scale;
} data;
    //double _lookup2[40];




//double SINE = 0.0;
/*double COSE = 1.0;
int *pixels1D; // Globalni pokazivač za pixels1D
double *_index1D;  // Globalni pokazivač za _index1D
int width = 129;    // Globalna promenljiva za širinu
int height = 129;   // Globalna promenljiva za visinu
*/

// Prototipi funkcija
int surf_driver_open(struct inode *inode, struct file *file);
int surf_driver_release(struct inode *inode, struct file *file);
ssize_t surf_driver_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);
ssize_t surf_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos);
int init_module(void);
void cleanup_module(void);
//void initializePixels1D(int width, int height, double **Pixels);
//void createVector(int scale, int row, int col);
//void initializeIndex1D(int IndexSize, int OriSize);

//static long ioctl_data(struct file *file, unsigned int cmd, unsigned long arg);

static const struct file_operations surf_driver_fops = {
    	.owner = THIS_MODULE,
    	.open = surf_driver_open,
    	.release = surf_driver_release,
    	//.read = surf_driver_read,
    	.write = surf_driver_write,
};

int surf_driver_open(struct inode *inode, struct file *file) {
	return 0;
}

int surf_driver_release(struct inode *inode, struct file *file) {
	return 0;
}


// Funkcija za inicijalizaciju 1D niza iz 2D niza
/*void initializePixels1D(int width, int height, int **Pixels) {
    // Alociraj memoriju za 1D niz u kernelu
    pixels1D = kmalloc(width * height * sizeof(int), GFP_KERNEL);
    if (!pixels1D) {
        printk(KERN_ALERT "Greška prilikom alokacije memorije za pixels1D\n");
        return;
    }

    // Kopiraj podatke iz 2D niza u 1D niz
    int pixels1D_index = 0;
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            pixels1D[pixels1D_index++] = Pixels[w][h];
        }
    }

    printk(KERN_INFO "pixels1D uspešno inicijalizovan\n");
}

void initializeIndex1D(int IndexSize, int OriSize) {
    // Provera da li je već alociran, ako jeste, oslobađamo ga
    if (_index1D != NULL) {
        kfree(_index1D);
        _index1D = NULL;
    }

    // Alociraj memoriju za _index1D i inicijalizuj vrednosti na 0
    _index1D = kmalloc(IndexSize * IndexSize * OriSize * sizeof(double), GFP_KERNEL);
    if (!_index1D) {
        printk(KERN_ALERT "Neuspešna alokacija memorije za _index1D\n");
    } else {
        // Inicijalizacija svih vrednosti na 0
        memset(_index1D, 0, IndexSize * IndexSize * OriSize * sizeof(double));
        printk(KERN_INFO "_index1D uspešno alociran i inicijalizovan\n");
    }
}
					*/   
ssize_t surf_driver_write(struct file *file, const char __user *user_buff, size_t size, loff_t *offset) {
    int scale, row, col; 
    //double sine, cose;

    size_t header_size = 3*sizeof(int);  // scale, row, col, sine, cose
    //size_t pixels_size = width * height * sizeof(int);
    //size_t lookup2_size = sizeof(_lookup2);
    //size_t expected_size = header_size + pixels_size + lookup2_size;
				size_t expected_size = header_size;
    // Proveri da li je veličina podataka u skladu sa očekivanjem
    if (size != expected_size) {
        printk(KERN_ALERT "Neispravna veličina bafera. Očekivano: %zu, Primljeno: %zu\n", expected_size, size);
        return -EINVAL;
    }

    // Kopiraj osnovne podatke iz korisničkog prostora (scale, row, col, sine, cose)
    if (copy_from_user(&scale, user_buff, sizeof(int))) {
        printk(KERN_ALERT "Greška prilikom kopiranja scale iz korisničkog prostora\n");
        return -EFAULT;
    }

    if (copy_from_user(&row, user_buff + sizeof(int), sizeof(int))) {
        printk(KERN_ALERT "Greška prilikom kopiranja row iz korisničkog prostora\n");
        return -EFAULT;
    }

    if (copy_from_user(&col, user_buff + 2 * sizeof(int), sizeof(int))) {
        printk(KERN_ALERT "Greška prilikom kopiranja col iz korisničkog prostora\n");
        return -EFAULT;
    }

    /*if (copy_from_user(&sine, user_buff + 3 * sizeof(double), sizeof(double))) {
        printk(KERN_ALERT "Greška prilikom kopiranja sine iz korisničkog prostora\n");
        return -EFAULT;
    }

    if (copy_from_user(&cose, user_buff + 4 * sizeof(double), sizeof(double))) {
        printk(KERN_ALERT "Greška prilikom kopiranja cose iz korisničkog prostora\n");
        return -EFAULT;
    }

    // Oslobodi prethodno alociran pixels1D ako postoji
    if (pixels1D != NULL) {
        kfree(pixels1D);
        printk(KERN_INFO "Prethodni pixels1D je oslobođen\n");
    }

    // Alociraj memoriju za novi pixels1D
    pixels1D = kmalloc(pixels_size, GFP_KERNEL);
    if (!pixels1D) {
        printk(KERN_ALERT "Neuspešna alokacija memorije za pixels1D\n");
        return -ENOMEM;
    }

    // Kopiraj niz `pixels1D` iz korisničkog prostora
    if (copy_from_user(pixels1D, user_buff + header_size, pixels_size)) {
        printk(KERN_ALERT "Greška prilikom kopiranja niza pixels1D iz korisničkog prostora\n");
        kfree(pixels1D); // Oslobodi memoriju u slučaju greške
        return -EFAULT;
    }
    printk(KERN_INFO "pixels1D uspešno kopiran iz aplikacije u kernel\n");

    // Kopiraj niz `_lookup2` iz korisničkog prostora
    if (copy_from_user(_lookup2, user_buff + header_size + pixels_size, lookup2_size)) {
        printk(KERN_ALERT "Greška prilikom kopiranja niza _lookup2 iz korisničkog prostora\n");
        return -EFAULT;
    }

    printk(KERN_INFO "_lookup2 uspešno kopiran iz aplikacije u kernel\n");

    // Skladišti podatke u globalne promenljive*/
    data.ipoint_scale = scale;
    data.ipoint_row = row;
    data.ipoint_col = col;
    //SINE = sine;
    //COSE = cose;
				printk(KERN_INFO "Received data: scale=%d, row=%d, col=%d", scale, row, col);
    //printk(KERN_INFO "Received data: scale=%lf, row=%lf, col=%lf, sine=%lf, cose=%lf\n", scale, row, col, sine, cose);

    // Pozivaj createVector sa dobijenim vrednostima
    /*createVector(2 * (1 + _doubleImage) * data.ipoint_scale,
                 (1 + _doubleImage) * data.ipoint_col,
                 (1 + _doubleImage) * data.ipoint_row);*/

    return size;
}


/*ssize_t surf_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    int IndexSize = _IndexSize;  // Vrednosti preuzete iz vašeg koda
    int OriSize = _OriSize;
    size_t total_size = IndexSize * IndexSize * OriSize * sizeof(double); // Ukupna veličina _index1D niza
    size_t bytes_to_read;
    size_t remaining_bytes;

    printk(KERN_INFO "Reading from surf_driver\n");

    // Provera koliko bajtova je preostalo za čitanje
    remaining_bytes = total_size - *ppos;
    if (remaining_bytes == 0) {
        printk(KERN_INFO "No more data to read (EOF)\n");
        return 0;  // Kraj fajla
    }

    // Odredi koliko bajtova treba pročitati
    bytes_to_read = count;
    if (bytes_to_read > remaining_bytes) {
        bytes_to_read = remaining_bytes;
    }

    // Kopiraj podatke iz _index1D niza u korisnički prostor
    if (copy_to_user(user_buf, _index1D + (*ppos / sizeof(double)), bytes_to_read)) {
        printk(KERN_ALERT "Failed to copy _index1D to user space\n");
        return -EFAULT;
    }

    // Ažuriraj ppos
    *ppos += bytes_to_read;

    printk(KERN_INFO "Copied %zu bytes to user space\n", bytes_to_read);
    return bytes_to_read;
}
*/




int init_module(void) {
    printk(KERN_INFO "\n");
    printk(KERN_INFO "surf driver starting insmod.\n");
    
    // Registracija uređaja
    major_num = register_chrdev(0, DEVICE_NAME, &surf_driver_fops);
    if (major_num < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        return major_num;
    }

    // Alokacija memorije za pixels1D (npr. 1D niz piksela)
    /*pixels1D = kmalloc(MAX_IMAGE_DATA_SIZE * sizeof(double), GFP_KERNEL);
    if (!pixels1D) {
        printk(KERN_ALERT "Failed to allocate memory for pixels1D\n");
        unregister_chrdev(major_num, DEVICE_NAME);
        return -ENOMEM;
    }

    // Alokacija memorije za _index1D (ako je potrebna)
    _index1D = kmalloc(_IndexSize * _OriSize * sizeof(double), GFP_KERNEL);
    if (!_index1D) {
        printk(KERN_ALERT "Failed to allocate memory for _index1D\n");
        kfree(pixels1D);  // Oslobodi prethodno alociranu memoriju
        unregister_chrdev(major_num, DEVICE_NAME);
        return -ENOMEM;
    }*/

    printk(KERN_INFO "Registered correctly with major number %d\n", major_num);

    return 0; // Uspešna inicijalizacija
}



void cleanup_module(void) {	
				 /*Oslobađanje pixels1D ako je alociran 
    if (pixels1D != NULL) {
        kfree(pixels1D);
        printk(KERN_INFO "pixels1D je oslobođen\n");
    }


    Oslobađanje ptInsurf ako je alociran
    if (ptInsurf)
        kfree(ptInsurf);

    // Oslobađanje surfCenters ako je alociran
    if (surfCenters)
        kfree(surfCenters);
    // Provera da li je niz _index1D alociran, ako jeste, oslobađamo ga
    if (_index1D != NULL) {
        kfree(_index1D); // Oslobađanje memorije za _index1D
        _index1D = NULL;  // Postavljanje pokazivača na NULL da spreči dalje korišćenje
        printk(KERN_INFO "_index1D je oslobođen\n");
    } else {
        printk(KERN_INFO "_index1D nije alociran ili je već oslobođen\n");
    }
*/
    // Unregistering the device
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Unregistered %s device\n", DEVICE_NAME);
}


/*void createVector(double scale, double row, double col) {
    int iradius, iy, ix;
    double spacing, radius, rpos, cpos, rx, cx;
    int step = my_max((int)(scale/2 + 0.5),1);
  
    iy = (int) (row + 0.5);
    ix = (int) (col + 0.5);

    double fracy = row-iy;
    double fracx = col-ix;
    double fracr =   COSE * fracy + SINE * fracx;
    double fracc = - SINE * fracy + COSE * fracx;
  
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
				
																double temp1_rpos, temp2_rpos, temp3_rpos, temp4_rpos;
																double temp1_cpos, temp2_cpos, temp3_cpos, temp4_cpos;
				
																temp1_rpos = COSE * (i - iradius);
																temp2_rpos = SINE * (j - iradius);
																temp3_rpos = temp1_rpos + temp2_rpos; 
																temp4_rpos = step * temp3_rpos - fracr;
																rpos = temp4_rpos / spacing;

																temp1_cpos = - SINE * (i - iradius);
																temp2_cpos = COSE * (j - iradius);
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
          
                    int r = iy + (i - iradius) * step;
                    int c = ix + (j - iradius) * step;
                    int ori1, ori2;
                    int ri, ci;
                    
                    int addSampleStep = (int)scale;
                    
                    double weight;
                    double dxx1, dxx2, dyy1, dyy2;
                    double dx, dy;
                    double dxx, dyy;
                    
                    double rfrac, cfrac;
                    double rweight1, rweight2, cweight1, cweight2;
                    
					double dx1, dx2, dy1, dy2;
                    
                    if (r >= 1 + addSampleStep && r < height - 1 - addSampleStep && c >= 1 + addSampleStep && c < width - 1 - addSampleStep) {
                        weight = _lookup2[(int)(rpos * rpos + cpos * cpos)];

 
                       dxx1 = pixels1D[(r + addSampleStep + 1) * width + (c + addSampleStep + 1)] 
																												+ pixels1D[(r - addSampleStep) * width + c]
																												- pixels1D[(r - addSampleStep) * width + (c + addSampleStep + 1)]
																												- pixels1D[(r + addSampleStep + 1) * width + c];
																												
																							dxx2 = pixels1D[(r + addSampleStep + 1) * width + (c + 1)]
																												+ pixels1D[(r - addSampleStep) * width + (c - addSampleStep)]
																												- pixels1D[(r - addSampleStep) * width + (c + 1)]
																												- pixels1D[(r + addSampleStep + 1) * width + (c - addSampleStep)];
																												
																							dyy1 = pixels1D[(r + 1) * width + (c + addSampleStep + 1)]
																												+ pixels1D[(r - addSampleStep) * width + (c - addSampleStep)]
																												- pixels1D[(r - addSampleStep) * width + (c + addSampleStep + 1)]
																												- pixels1D[(r + 1) * width + (c - addSampleStep)];
																												
																							dyy2 = pixels1D[(r + addSampleStep + 1) * width + (c + addSampleStep + 1)]
																												+ pixels1D[r * width + (c - addSampleStep)]
																												- pixels1D[r * width + (c + addSampleStep + 1)]
																												- pixels1D[(r + addSampleStep + 1) * width + (c - addSampleStep)];
		 
	
																				
                        dxx = weight * (dxx1 - dxx2);
                        dyy = weight * (dyy1 - dyy2);
																								
																								dx1 = COSE * dxx;
																								dx2 = SINE * dyy;
																								dx = dx1 + dx2;
																								
																								dy1 = SINE * dxx;
																								dy2 = COSE * dyy;
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
    }*/
