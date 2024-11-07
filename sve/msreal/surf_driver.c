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
#include <linux/device.h>  
#include <linux/cdev.h> 
#include <linux/kdev_t.h>
#include <linux/mutex.h> 

static DEFINE_MUTEX(surf_mutex);

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
int _OriSize = 4;
int _MagFactor = 3;

struct ipoint_data {
    double ipoint_row;//x
    double ipoint_col;//y
    double ipoint_scale;
} data;
    double _lookup2[40];

double SINE = 0.0;
double COSE = 1.0;
double *pixels1D; // Globalni pokazivač za pixels1D
double *_index1D;  // Globalni pokazivač za _index1D
int width = 129;    // Globalna promenljiva za širinu
int height = 129;   // Globalna promenljiva za visinu

/*dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
*/	
// Prototipi funkcija
int surf_driver_open(struct inode *inode, struct file *file);
int surf_driver_release(struct inode *inode, struct file *file);
ssize_t surf_driver_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);
ssize_t surf_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos);
//static int __init surf_driver_init(void);
//static void __exit surf_driver_exit(void);
void createVector(double scale, double row, double col);
int init_module(void);
void cleanup_module(void);

static const struct file_operations surf_driver_fops = {
    	.owner = THIS_MODULE,
    	.open = surf_driver_open,
    	.release = surf_driver_release,
    	.read = surf_driver_read,
    	.write = surf_driver_write,
};

int surf_driver_open(struct inode *inode, struct file *file) {
	return 0;
}

int surf_driver_release(struct inode *inode, struct file *file) {
	return 0;
}


ssize_t surf_driver_write(struct file *file, const char __user *user_buff, size_t size, loff_t *offset) {
    double scale, row, col;
    double sine, cose;

				  // Zaključaj mutex pre pristupa kritičnim resursima
    if (mutex_lock_interruptible(&surf_mutex)) {
        printk(KERN_ALERT "Nije moguće zaključati mutex\n");
        return -EINTR;  // Vraća grešku ako je došlo do prekida
    }
    // Očekivana veličina podataka: scale, row, col, SINE, COSE + pixels1D + _lookup2
    size_t expected_size = sizeof(double) * 5 + width * height * sizeof(double) + 40 * sizeof(double);
    
    // Proveri da li je ukupna veličina bafera ispravna
    if (size != expected_size) {
        printk(KERN_ALERT "Neispravna veličina bafera, očekivano %zu, primljeno %zu\n", expected_size, size);
        return -EINVAL;
    }

    // Kopiraj osnovne parametre: scale, row, col, SINE, COSE
    if (copy_from_user(&scale, user_buff, sizeof(double))) {
        printk(KERN_ALERT "Failed to copy image scale from user space\n");
        return -EFAULT;
    }

    if (copy_from_user(&row, user_buff + sizeof(double), sizeof(double))) {
        printk(KERN_ALERT "Failed to copy image rows from user space\n");
        return -EFAULT;
    }

    if (copy_from_user(&col, user_buff + 2 * sizeof(double), sizeof(double))) {
        printk(KERN_ALERT "Failed to copy image columns from user space\n");
        return -EFAULT;
    }

    if (copy_from_user(&sine, user_buff + 3 * sizeof(double), sizeof(double))) {
        printk(KERN_ALERT "Failed to copy image sine from user space\n");
        return -EFAULT;
    }

    if (copy_from_user(&cose, user_buff + 4 * sizeof(double), sizeof(double))) {
        printk(KERN_ALERT "Failed to copy image cose from user space\n");
        return -EFAULT;
    }

    // Oslobodi prethodno alociran `pixels1D` ako postoji
    if (pixels1D != NULL) {
        kfree(pixels1D);
        printk(KERN_INFO "Prethodni pixels1D je oslobođen\n");
    }

    // Alociraj memoriju za novi `pixels1D`
    size_t pixels_size = width * height * sizeof(double);
    pixels1D = kmalloc(pixels_size, GFP_KERNEL);
    if (!pixels1D) {
        printk(KERN_ALERT "Neuspešna alokacija memorije za pixels1D\n");
        return -ENOMEM;
    }

    // Kopiraj niz `pixels1D` iz korisničkog prostora
    if (copy_from_user(pixels1D, user_buff + 5 * sizeof(double), pixels_size)) {
        printk(KERN_ALERT "Greška prilikom kopiranja niza pixels1D iz korisničkog prostora\n");
        kfree(pixels1D); // Oslobodi memoriju u slučaju greške
        return -EFAULT;
    }

    printk(KERN_INFO "pixels1D uspešno kopiran iz aplikacije u kernel\n");

    // Kopiraj niz `_lookup2`
    size_t lookup2_offset = 5 * sizeof(double) + pixels_size;
    if (copy_from_user(_lookup2, user_buff + lookup2_offset, 40 * sizeof(double))) {
        printk(KERN_ALERT "Greška prilikom kopiranja niza _lookup2 iz korisničkog prostora\n");
        return -EFAULT;
    }

    printk(KERN_INFO "Niz _lookup2 uspešno inicijalizovan u kernelu\n");

    // Čuvanje vrednosti u kernel promenljivama
    data.ipoint_scale = scale;
    data.ipoint_row = row;
    data.ipoint_col = col;
    SINE = sine;
    COSE = cose;
				// Izračunavanje svake promenljive posebno
				double scaledValue = 1.65 * (1 + _doubleImage) * data.ipoint_scale;
				double colValue = (1 + _doubleImage) * data.ipoint_col;
				double rowValue = (1 + _doubleImage) * data.ipoint_row;

				// Pozivanje funkcije sa prethodno izračunatim vrednostima
				createVector(scaledValue, colValue, rowValue);

    printk(KERN_INFO "Received data: scale=%lf, row=%lf, col=%lf, sine=%lf, cose=%lf\n", scale, row, col, sine, cose);
				mutex_unlock(&surf_mutex);
    return size; // Vraća uspešnu veličinu bafera
}


ssize_t surf_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    int IndexSize = _IndexSize; 
    int OriSize = _OriSize;
    size_t total_size = IndexSize * IndexSize * OriSize * sizeof(double); // Ukupna veličina _index1D niza
    size_t bytes_to_read;
    size_t remaining_bytes;
				if (mutex_lock_interruptible(&surf_mutex)) {
        printk(KERN_ALERT "Nije moguće zaključati mutex\n");
        return -EINTR;  // Vraća grešku ako je došlo do prekida
    }
    
    printk(KERN_INFO "surf_driver: Read request, count = %zu, ppos = %lld\n", count, *ppos);

    // Provera koliko bajtova je preostalo za čitanje
    remaining_bytes = total_size - *ppos;
    if (remaining_bytes == 0) {
        printk(KERN_INFO "surf_driver: No more data to read (EOF)\n");
        return 0;  // Kraj fajla
    }

    // Odredi koliko bajtova treba pročitati
    bytes_to_read = min(count, remaining_bytes); // uzimamo najmanju vrednost između tražene količine i preostalih bajtova

    // Kopiraj podatke iz _index1D niza u korisnički prostor
    if (copy_to_user(user_buf, _index1D + (*ppos / sizeof(double)), bytes_to_read)) {
        printk(KERN_ALERT "surf_driver: Failed to copy _index1D to user space\n");
        return -EFAULT;
    }

    // Ažuriraj ppos
    *ppos += bytes_to_read;

    printk(KERN_INFO "surf_driver: Copied %zu bytes to user space, new ppos = %lld\n", bytes_to_read, *ppos);
    mutex_unlock(&surf_mutex);
    return bytes_to_read;
}

int init_module(void) {
    printk(KERN_INFO "\n");
    printk(KERN_INFO "surf driver starting insmod.\n");
    if (mutex_lock_interruptible(&surf_mutex)) {
        printk(KERN_ALERT "Nije moguće zaključati mutex\n");
        return -EINTR;  // Vraća grešku ako je došlo do prekida
    }
    // Registracija uređaja
    major_num = register_chrdev(0, DEVICE_NAME, &surf_driver_fops);
    if (major_num < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        return major_num;
    }

    // Alokacija memorije za pixels1D (npr. 1D niz piksela)
    pixels1D = kmalloc(MAX_IMAGE_DATA_SIZE * sizeof(double), GFP_KERNEL);
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
    }

    printk(KERN_INFO "Registered correctly with major number %d\n", major_num);
				mutex_unlock(&surf_mutex);
    return 0; // Uspešna inicijalizacija
}
void cleanup_module(void) {	
				 /* Oslobađanje pixels1D ako je alociran */
    if (pixels1D != NULL) {
        kfree(pixels1D);
        printk(KERN_INFO "pixels1D je oslobođen\n");
    }


    /* Oslobađanje ptInsurf ako je alociran
    if (ptInsurf)
        kfree(ptInsurf);

    // Oslobađanje surfCenters ako je alociran
    if (surfCenters)
        kfree(surfCenters);*/
    // Provera da li je niz _index1D alociran, ako jeste, oslobađamo ga
    if (_index1D != NULL) {
        kfree(_index1D); // Oslobađanje memorije za _index1D
        _index1D = NULL;  // Postavljanje pokazivača na NULL da spreči dalje korišćenje
        printk(KERN_INFO "_index1D je oslobođen\n");
    } else {
        printk(KERN_INFO "_index1D nije alociran ili je već oslobođen\n");
    }

    // Unregistering the device
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Unregistered %s device\n", DEVICE_NAME);
}

/*static int __init surf_driver_init(void) {
    int ret;
    int IndexSize = _IndexSize; 
    int OriSize = _OriSize;
    printk(KERN_INFO "surf driver starting insmod.\n");

    // Register char device region
    ret = alloc_chrdev_region(&my_dev_id, 0, 1, DEVICE_NAME);
    if (ret) {
        printk(KERN_ERR "Failed to register char device\n");
        return ret;
    }
    printk(KERN_INFO "Char device region allocated\n");

    // Create device class
    my_class = class_create("surf_driver_class");
    if (IS_ERR(my_class)) {
        printk(KERN_ERR "Failed to create class\n");
        unregister_chrdev_region(my_dev_id, 1);
        return PTR_ERR(my_class);
    }
    printk(KERN_INFO "Class created\n");

    // Create device
    my_device = device_create(my_class, NULL, my_dev_id, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        printk(KERN_ERR "Failed to create device\n");
        class_destroy(my_class);
        unregister_chrdev_region(my_dev_id, 1);
        return PTR_ERR(my_device);
    }
    printk(KERN_INFO "Device created\n");

    // Register character device
    major_num = register_chrdev(0, DEVICE_NAME, &surf_driver_fops);
    if (major_num < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        device_destroy(my_class, my_dev_id);
        class_destroy(my_class);
        unregister_chrdev_region(my_dev_id, 1);
        return major_num;
    }

    printk(KERN_INFO "surf_driver registered correctly with major number %d\n", major_num);

    // Allocate memory for pixels1D
    pixels1D = kmalloc(width * height * sizeof(double), GFP_KERNEL);
    if (!pixels1D) {
        printk(KERN_ALERT "Memory allocation failed for pixels1D\n");
        cleanup_module();  // Cleanup if allocation fails
        return -ENOMEM;
    }

    _index1D = kmalloc(IndexSize * IndexSize * OriSize * sizeof(double), GFP_KERNEL);
    if (!_index1D) {
        printk(KERN_ALERT "Neuspešna alokacija memorije za _index1D\n");
    } else {
        // Inicijalizacija svih vrednosti na 0
        memset(_index1D, 0, IndexSize * IndexSize * OriSize * sizeof(double));
        printk(KERN_INFO "_index1D uspešno alociran i inicijalizovan\n");
    }


    return 0; // Uspešna inicijalizacija
}

static void __exit surf_driver_exit(void) {
    printk(KERN_INFO "surf_driver cleanup\n");

    if (pixels1D != NULL) {
        kfree(pixels1D);
        printk(KERN_INFO "pixels1D memory freed\n");
    }

    if (_index1D != NULL) {
        kfree(_index1D);
        printk(KERN_INFO "_index1D memory freed\n");
    }

    // Cleanup device
    device_destroy(my_class, my_dev_id);
    class_destroy(my_class);
    unregister_chrdev_region(my_dev_id, 1);

    // Unregister the character device
    unregister_chrdev(major_num, DEVICE_NAME);

    printk(KERN_INFO "surf_driver unregistered and cleaned up\n");
}*/
void createVector(double scale, double row, double col) {
    int iradius, iy, ix;
    double spacing, radius, rpos, cpos, rx, cx;
    int step = max((int)(scale/2 + 0.5),1);
  
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
    }
    //module_init(surf_driver_init);
    //module_exit(surf_driver_exit); 
//module_init(surf_driver_init);
//module_exit(surf_driver_exit);
