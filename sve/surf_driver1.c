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
#define FIXED_POINT_SHIFT 16
#define FIXED_POINT_MULT (1 << FIXED_POINT_SHIFT)


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


// Prototipi funkcija
int surf_driver_open(struct inode *inode, struct file *file);
int surf_driver_release(struct inode *inode, struct file *file);
ssize_t surf_driver_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);
ssize_t surf_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos);
int init_module(void);
void cleanup_module(void);
void initializePixels1D(int width, int height, double **Pixels);
void createVector(int scale, int row, int col);
void initializeIndex1D(int IndexSize, int OriSize);
int to_fixed(long value);
int to_fixed_int(int value);
int fixed_mul(int a, int b);
int fixed_div(int a, int b);

//static long ioctl_data(struct file *file, unsigned int cmd, unsigned long arg);

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

/*static long ioctl_data(struct file *file, unsigned int cmd, unsigned long arg) {
    struct ipoint_data data;

    switch(cmd) {
        case IOCTL_SET_IPOINT:
            if (copy_from_user(&data, (struct ipoint_data *)arg, sizeof(struct ipoint_data))) {
                return -EFAULT;
            }
            printk("Received x: %lf, y: %lf, scale: %lf\n", data.ipoint_row, data.ipoint_col, data.ipoint_scale);
            break;
        default:
            return -EINVAL;
    }
    return 0;
}
*/

// Pomoćne funkcije za konverziju iz i u fiksnu tačku
int to_fixed(long value) {
    return (int)(value * FIXED_POINT_MULT);
}

int to_fixed_int(int value) {
    return value * FIXED_POINT_MULT;
}

int fixed_mul(int a, int b) {
    return (a * b) >> FIXED_POINT_SHIFT;
}

int fixed_div(int a, int b) {
    return (a << FIXED_POINT_SHIFT) / b;
}

// Funkcija za inicijalizaciju 1D niza iz 2D niza
void initializePixels1D(int width, int height, double **Pixels) {
    // Alociraj memoriju za 1D niz u kernelu
    pixels1D = kmalloc(width * height * sizeof(double), GFP_KERNEL);
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

ssize_t surf_driver_write(struct file *file, const char __user *user_buff, size_t size, loff_t *offset) {
				double scale, row, col; 
				double sine, cose;
					   
				
    /*if (size != sizeof(transfer_data_t)) {
        return -EINVAL; // Neispravna veličina podataka
    }*/

    /* Prenos podataka iz korisničkog prostora u kernel prostor
    if (copy_from_user(&kernel_data, user_buff, sizeof(transfer_data_t))) {
        return -EFAULT; // Došlo je do greške prilikom kopiranja
    }*/

    if (copy_from_user(&scale, user_buff, sizeof(double)))
    {
        printk(KERN_ALERT "Failed to copy image scale from user space\n");
        return -EFAULT;
    }

    if (copy_from_user(&row, user_buff + sizeof(double), sizeof(double)))
    {
        printk(KERN_ALERT "Failed to copy image rows from user space\n");
        return -EFAULT;
    }
    
    if (copy_from_user(&col, user_buff + 2 * sizeof(double), sizeof(double)))
    {
        printk(KERN_ALERT "Failed to copy image columns from user space\n");
        return -EFAULT;
    }
    
     if (copy_from_user(&sine, user_buff + 3 * sizeof(double), sizeof(double))) {
		       printk(KERN_ALERT "Failed to copy image sinus from user space\n");
		       return -EFAULT;
		   }
		   
		   if (copy_from_user(&cose, user_buff + 4 * sizeof(double), sizeof(double))) {
		       printk(KERN_ALERT "Failed to copy image cosinus from user space\n");
		       return -EFAULT;
		   }

    // Proveri da li veličina bafera odgovara očekivanom broju piksela
    if (size != width * height * sizeof(double)) {
        printk(KERN_ALERT "Neispravna veličina bafera za pixels1D\n");
        return -EINVAL;
    }

    // Oslobodi prethodno alociran pixels1D ako postoji
    if (pixels1D != NULL) {
        kfree(pixels1D);
        printk(KERN_INFO "Prethodni pixels1D je oslobođen\n");
    }

    // Alociraj memoriju za novi pixels1D i kopiraj podatke iz korisničkog prostora
    pixels1D = kmalloc(size, GFP_KERNEL);
    if (!pixels1D) {
        printk(KERN_ALERT "Neuspešna alokacija memorije za pixels1D\n");
        return -ENOMEM;
    }

    // Kopiraj niz `pixels1D` iz korisničkog prostora u kernel prostor
    if (copy_from_user(pixels1D, user_buff, size)) {
        printk(KERN_ALERT "Greška prilikom kopiranja niza pixels1D iz korisničkog prostora\n");
        kfree(pixels1D); // Oslobodi memoriju u slučaju greške
        return -EFAULT;
    }

    printk(KERN_INFO "pixels1D uspešno kopiran iz aplikacije u kernel\n");
		  
		  if (size != sizeof(_lookup2)) {
        printk(KERN_ALERT "Neispravna veličina bafera za _lookup2\n");
        return -EINVAL;
    }

    // Kopiraj niz _lookup2 iz korisničkog prostora u kernel
    if (copy_from_user(_lookup2, user_buff, sizeof(_lookup2))) {
        printk(KERN_ALERT "Greška prilikom kopiranja niza _lookup2 iz korisničkog prostora\n");
        return -EFAULT;
    }

    printk(KERN_INFO "Niz _lookup2 uspešno inicijalizovan u kernelu\n"); 
    
    data.ipoint_scale = scale;
    data.ipoint_row =	row;
    data.ipoint_col = col;
    SINE = sine;
		  COSE = cose;
		  printk(KERN_INFO "Received data: scale=%lf, row=%lf, col=%lf, sine=%lf, cose=%lf\n", scale, row, col, sine, cose);
				
				
				// Definišemo pomoćne promenljive za izračunavanje vrednosti
// Prilagođavamo tvoje izraze koristeći fiksne tačke
int ipoint_scale_calc = (1 + to_fixed(_doubleImage)) * to_fixed(data.ipoint_scale);
int ipoint_col_calc = (1 + to_fixed(_doubleImage)) * to_fixed(data.ipoint_col);
int ipoint_row_calc = (1 + to_fixed(_doubleImage)) * to_fixed(data.ipoint_row);

// Kreiramo pomoćnu promenljivu za prvu komponentu izraza koristeći fiksne tačke
int first_component = fixed_mul(to_fixed(1.65), ipoint_scale_calc);

// Pozivamo funkciju createVector sa izračunatim vrednostima u fiksnim tačkama
createVector(first_component, ipoint_col_calc, ipoint_row_calc);

				/*createVector(1.65*(1+_doubleImage)*_current->scale,
                 (1+_doubleImage)*_current->y,
                 (1+_doubleImage)*_current->x);
				createVector(1.65*(1+_doubleImage)*data.ipoint_scale,
                 (1+_doubleImage)*data.ipoint_col,
                 (1+_doubleImage)*data.ipoint_row);*/
				
    return size;
}

ssize_t surf_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
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


/*ssize_t surf_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    	printk(KERN_INFO "Reading from surf_driver\n");

    	size_t bytes_to_read;
    	size_t remaining_bytes;

    	bytes_to_read = count;
    	remaining_bytes = (ROWS * COLS * sizeof(Point3i)) - *ppos;

    	printk(KERN_INFO "ROWS: %d, COLS: %d, sizeof(Point3i): %zu, *ppos: %lld\n", ROWS, COLS, sizeof(Point3i), *ppos);
    	printk(KERN_INFO "Remaining bytes to read: %zu\n", remaining_bytes);

    	if (remaining_bytes == 0)
    	{
        	printk(KERN_INFO "No more data to read (EOF)\n");
        	return 0; // EOF
    	}

    	if (bytes_to_read > remaining_bytes)
        	bytes_to_read = remaining_bytes;

    	printk(KERN_INFO "Copying %zu bytes to user space\n", bytes_to_read);

    	if (copy_to_user(user_buf, ptInsurf + (*ppos / sizeof(Point3i)), bytes_to_read))
    	{
        	printk(KERN_ALERT "Failed to copy ptInsurf to user space\n");
       		return -EFAULT;
    	}

    	*ppos += bytes_to_read;

    	return bytes_to_read;
}*/


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

    return 0; // Uspešna inicijalizacija
}


/*int init_module(void) {
					printk(KERN_INFO "\n");
					printk(KERN_INFO "surf driver starting insmod.\n");
    	major_num = register_chrdev(0, DEVICE_NAME, &surf_driver_fops);

    	if (major_num < 0)
    	{
        	printk(KERN_ALERT "Failed to register a major number\n");
        	return major_num;
    	}

    surfCenters = kmalloc(3 * sizeof(double), GFP_KERNEL);
    	if (!surfCenters)
    	{
        	printk(KERN_ALERT "Failed to allocate memory for surfCenters\n");
        	unregister_chrdev(major_num, DEVICE_NAME);
        	return -ENOMEM;
    	}

    	ptInsurf = kmalloc(64 * sizeof(double), GFP_KERNEL);
    	
    	if (!ptInsurf)
    	{
        	printk(KERN_ALERT "Failed to allocate memory for ptInsurf\n");
        	kfree(surfCenters);
        	unregister_chrdev(major_num, DEVICE_NAME);
        	return -ENOMEM;
    	}

    	
    	printk(KERN_INFO "Registered correctly with major number %d\n", major_num);

    	return 0;
}*/

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

/*static int my_max(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}*/

void createVector(int scale, int row, int col) {
    int iradius, iy, ix;
    int spacing, radius, rpos, cpos, rx, cx;
    int step = max(fixed_div(scale, to_fixed(2)) + (FIXED_POINT_MULT / 2), to_fixed_int(1));
  
    iy = (int)(fixed_div(row + FIXED_POINT_MULT / 2, FIXED_POINT_MULT));
    ix = (int)(fixed_div(col + FIXED_POINT_MULT / 2, FIXED_POINT_MULT));

    int fracy = row - (iy * FIXED_POINT_MULT);
    int fracx = col - (ix * FIXED_POINT_MULT);
    int fracr = fixed_mul(to_fixed(COSE), fracy) + fixed_mul(to_fixed(SINE), fracx);
    int fracc = -fixed_mul(to_fixed(SINE), fracy) + fixed_mul(to_fixed(COSE), fracx);
  
    // Računanje spacing koristeći fiksne tačke
    spacing = fixed_mul(scale, to_fixed(_MagFactor));

    // Računanje radius koristeći fiksne tačke
    radius = fixed_mul(to_fixed(1.4), fixed_mul(spacing, to_fixed(_IndexSize + 1)) / 2);
    iradius = fixed_div(radius, step) + (FIXED_POINT_MULT / 2);
  		
    for (int i = 0; i <= 2 * iradius; i++) {
        for (int j = 0; j <= 2 * iradius; j++) {
            int temp1_rpos, temp2_rpos, temp3_rpos, temp4_rpos;
            int temp1_cpos, temp2_cpos, temp3_cpos, temp4_cpos;
            
            // Rotiraj uzorke koristeći fiksne tačke
            temp1_rpos = fixed_mul(to_fixed(COSE), (i - iradius) * FIXED_POINT_MULT);
            temp2_rpos = fixed_mul(to_fixed(SINE), (j - iradius) * FIXED_POINT_MULT);
            temp3_rpos = temp1_rpos + temp2_rpos; 
            temp4_rpos = fixed_mul(step, temp3_rpos) - fracr;
            rpos = fixed_div(temp4_rpos, spacing);

            temp1_cpos = -fixed_mul(to_fixed(SINE), (i - iradius) * FIXED_POINT_MULT);
            temp2_cpos = fixed_mul(to_fixed(COSE), (j - iradius) * FIXED_POINT_MULT);
            temp3_cpos = temp1_cpos + temp2_cpos; 
            temp4_cpos = fixed_mul(step, temp3_cpos) - fracc;
            cpos = fixed_div(temp4_cpos, spacing);
                
            // Konvertujemo rpos i cpos u odgovarajuće vrednosti u fiksnoj tački
            rx = rpos + 2 * FIXED_POINT_MULT - (FIXED_POINT_MULT / 2);
            cx = cpos + 2 * FIXED_POINT_MULT - (FIXED_POINT_MULT / 2);

            // Provera da li uzorak leži unutar granica _index patch-a
            if (rx > -1 * FIXED_POINT_MULT && rx < _IndexSize * FIXED_POINT_MULT  &&
                cx > -1 * FIXED_POINT_MULT && cx < _IndexSize * FIXED_POINT_MULT) {
          
                int r = iy + (i - iradius) * step;
                int c = ix + (j - iradius) * step;
                int ori1, ori2;
                int ri, ci;
                
                int addSampleStep = scale;

                // Računanje težine i gradijenata koristeći fiksne tačke
                int weight = _lookup2[(int)(fixed_mul(rpos, rpos) + fixed_mul(cpos, cpos))];

                int dxx1 = pixels1D[(r + addSampleStep + 1) * width + (c + addSampleStep + 1)] 
                            + pixels1D[(r - addSampleStep) * width + c]
                            - pixels1D[(r - addSampleStep) * width + (c + addSampleStep + 1)]
                            - pixels1D[(r + addSampleStep + 1) * width + c];
                int dxx2 = pixels1D[(r + addSampleStep + 1) * width + (c + 1)]
                            + pixels1D[(r - addSampleStep) * width + (c - addSampleStep)]
                            - pixels1D[(r - addSampleStep) * width + (c + 1)]
                            - pixels1D[(r + addSampleStep + 1) * width + (c - addSampleStep)];
                int dyy1 = pixels1D[(r + 1) * width + (c + addSampleStep + 1)]
                            + pixels1D[(r - addSampleStep) * width + (c - addSampleStep)]
                            - pixels1D[(r - addSampleStep) * width + (c + addSampleStep + 1)]
                            - pixels1D[(r + 1) * width + (c - addSampleStep)];
                int dyy2 = pixels1D[(r + addSampleStep + 1) * width + (c + addSampleStep + 1)]
                            + pixels1D[r * width + (c - addSampleStep)]
                            - pixels1D[r * width + (c + addSampleStep + 1)]
                            - pixels1D[(r + addSampleStep + 1) * width + (c - addSampleStep)];
 
                int dxx = fixed_mul(weight, (dxx1 - dxx2));
                int dyy = fixed_mul(weight, (dyy1 - dyy2));
				
				// Računanje dx i dy sa fiksnom tačkom
                int dx1 = fixed_mul(to_fixed(COSE), dxx);
                int dx2 = fixed_mul(to_fixed(SINE), dyy);
                int dx = dx1 + dx2;
                
                int dy1 = fixed_mul(to_fixed(SINE), dxx);
                int dy2 = fixed_mul(to_fixed(COSE), dyy);
                int dy = dy1 - dy2;

                ori1 = (dx < 0) ? 0 : 1;
                ori2 = (dy < 0) ? 2 : 3;

                ri = (rx < 0) ? 0 : ((rx >= _IndexSize * FIXED_POINT_MULT) ? _IndexSize - 1 : (rx / FIXED_POINT_MULT));
                ci = (cx < 0) ? 0 : ((cx >= _IndexSize * FIXED_POINT_MULT) ? _IndexSize - 1 : (cx / FIXED_POINT_MULT));

                int rfrac = rx - ri * FIXED_POINT_MULT;
                int cfrac = cx - ci * FIXED_POINT_MULT;

                int rweight1 = fixed_mul(dx, (FIXED_POINT_MULT - rfrac));
                int rweight2 = fixed_mul(dy, (FIXED_POINT_MULT - rfrac));
                int cweight1 = fixed_mul(rweight1, (FIXED_POINT_MULT - cfrac));
                int cweight2 = fixed_mul(rweight2, (FIXED_POINT_MULT - cfrac));

                if (ri >= 0 && ri < _IndexSize && ci >= 0 && ci < _IndexSize) {
                    _index1D[ri * (_IndexSize * _OriSize) + ci * _OriSize + ori1] = cweight1;
                    _index1D[ri * (_IndexSize * _OriSize) + ci * _OriSize + ori2] = cweight2;
                }
            }  
        }             
    }
}

