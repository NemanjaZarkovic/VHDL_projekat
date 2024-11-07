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

#define MAX_ROWS_COLS 129*129
#define FASTHESSIAN_NUMBER 6
#define MAX_IMAGE_DATA_SIZE 129*129
#define DEVICE_NAME "fasthessian_driver"
#define BUF_LEN 200000
#define IOCTL_SET_IPOINT _IOW('i', 1, struct ipoint_data)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("y23-g03");
MODULE_DESCRIPTION("Fasthessian driver");

int major_num;
//double _index[x][y][z];
bool _doubleImage = false;
int _IndexSize = 4;

struct ipoint_data {
    double ipoint_row;//x
    double ipoint_col;//y
    double ipoint_scale;
} data;
typedef struct {
    double lookup1[83];
    double lookup2[40];
} transfer_data_t;


double _index [10][10][10];
double _Pixels [130][130];

double SINE,COSE;


// Prototipi funkcija
int fasthessian_driver_open(struct inode *inode, struct file *file);
int fasthessian_driver_release(struct inode *inode, struct file *file);
ssize_t fasthessian_driver_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);
//static long ioctl_data(struct file *file, unsigned int cmd, unsigned long arg);

static const struct file_operations fasthessian_driver_fops = {
    	.owner = THIS_MODULE,
    	.open = fasthessian_driver_open,
    	.release = fasthessian_driver_release,
    	//.read = fasthessian_driver_read,
    	.write = fasthessian_driver_write,
};

int fasthessian_driver_open(struct inode *inode, struct file *file)
{
	return 0;
}

int fasthessian_driver_release(struct inode *inode, struct file *file)
{
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
ssize_t fasthessian_driver_write(struct file *file, const char __user *user_buff, size_t size, loff_t *offset) 
{
    transfer_data_t kernel_data;
				double scale, row, col; 
				
    if (size != sizeof(transfer_data_t)) {
        return -EINVAL; // Neispravna veličina podataka
    }

    // Prenos podataka iz korisničkog prostora u kernel prostor
    if (copy_from_user(&kernel_data, user_buff, sizeof(transfer_data_t))) {
        return -EFAULT; // Došlo je do greške prilikom kopiranja
    }

    if (copy_from_user(&scale, user_buff, sizeof(double)))
    {
        printk(KERN_ALERT "Failed to copy image scale from user space\n");
        return -EFAULT;
    }

    if (copy_from_user(&row, user_buff + sizeof(double), sizeof(double)))
    {
        printk(KERN_ALERT "Failed to copy image columns from user space\n");
        return -EFAULT;
    }
    
    if (copy_from_user(&col, user_buff + 2 * sizeof(double), sizeof(double)))
    {
        printk(KERN_ALERT "Failed to copy image columns from user space\n");
        return -EFAULT;
    }
    
    data.ipoint_scale = scale;
    data.ipoint_row =	row;
    data.ipoint_col = col;

				createVector(1.65*(1+_doubleImage)*_current->scale,
                 (1+_doubleImage)*_current->y,
                 (1+_doubleImage)*_current->x);
				/*createVector(1.65*(1+_doubleImage)*ipoint_scale,
                 (1+_doubleImage)*ipoint_col,
                 (1+_doubleImage)*ipoint_row);*/
				
    return size;
}


double ***allocate_index(int x_size, int y_size, int z_size) {
    int i, j;
    double ***_index;

    _index = kmalloc(x_size * sizeof(double **), GFP_KERNEL);
    if (!_index) {
        printk(KERN_ALERT "Failed to allocate memory for 3D _index array\n");
        return NULL;
    }

    for (i = 0; i < x_size; i++) {
        _index[i] = kmalloc(y_size * sizeof(double *), GFP_KERNEL);
        if (!_index[i]) {
            printk(KERN_ALERT "Failed to allocate memory for 3D _index row\n");
            return NULL;
        }
        for (j = 0; j < y_size; j++) {
            _index[i][j] = kmalloc(z_size * sizeof(double), GFP_KERNEL);
            if (!_index[i][j]) {
                printk(KERN_ALERT "Failed to allocate memory for 3D _index column\n");
                return NULL;
            }
        }
    }

    return _index;
}



/*ssize_t fasthessian_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    	printk(KERN_INFO "Reading from fasthessian_driver\n");

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

    	if (copy_to_user(user_buf, ptInFasthessian + (*ppos / sizeof(Point3i)), bytes_to_read))
    	{
        	printk(KERN_ALERT "Failed to copy ptInFasthessian to user space\n");
       		return -EFAULT;
    	}

    	*ppos += bytes_to_read;

    	return bytes_to_read;
}*/


int init_module(void)
{
					printk(KERN_INFO "\n");
					printk(KERN_INFO "fasthessian driver starting insmod.\n");
    	major_num = register_chrdev(0, DEVICE_NAME, &fasthessian_driver_fops);

    	if (major_num < 0)
    	{
        	printk(KERN_ALERT "Failed to register a major number\n");
        	return major_num;
    	}

    /*	fasthessianCenters = kmalloc(3 * sizeof(double), GFP_KERNEL);
    	if (!fasthessianCenters)
    	{
        	printk(KERN_ALERT "Failed to allocate memory for fasthessianCenters\n");
        	unregister_chrdev(major_num, DEVICE_NAME);
        	return -ENOMEM;
    	}

    	ptInFasthessian = kmalloc(64 * sizeof(double), GFP_KERNEL);
    	
    	if (!ptInFasthessian)
    	{
        	printk(KERN_ALERT "Failed to allocate memory for ptInFasthessian\n");
        	kfree(fasthessianCenters);
        	unregister_chrdev(major_num, DEVICE_NAME);
        	return -ENOMEM;
    	}*/

    	
    	printk(KERN_INFO "Registered correctly with major number %d\n", major_num);

    	return 0;
}

void cleanup_module(void)
{
    // Oslobađanje memorije za trodimenzionalni niz _index
    if (_index) {
        for (int i = 0; i < _IndexSize; i++) {
            for (int j = 0; j < _IndexSize; j++) {
                kfree(_index[i][j]);
            }
            kfree(_index[i]);
        }
        kfree(_index);
        _index = NULL;  // Postavljanje pokazivača na NULL nakon oslobađanja
    }

    /* Oslobađanje ptInFasthessian ako je alociran
    if (ptInFasthessian)
        kfree(ptInFasthessian);

    // Oslobađanje fasthessianCenters ako je alociran
    if (fasthessianCenters)
        kfree(fasthessianCenters);*/

    // Unregistering the device
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Unregistered %s device\n", DEVICE_NAME);
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
				
																double temp1_rpos, temp2_rpos, temp3_rpos, temp4_rpos;
																double temp1_cpos, temp2_cpos, temp3_cpos, temp4_cpos;
				
																temp1_rpos = _cose * (i - iradius);
																temp2_rpos = _sine * (j - iradius);
																temp3_rpos = temp1_rpos + temp2_rpos; 
																temp4_rpos = step * temp3_rpos - fracr;
																rpos = temp4_rpos / spacing;

																temp1_cpos = - _sine * (i - iradius);
																temp2_cpos = _cose * (j - iradius);
																temp3_cpos = temp1_cpos + temp2_cpos; 
																temp4_cpos = step * temp3_rpos - fracc;
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
                    
                    int addSampleStep = int(scale);
                    
                    double weight;
                    double dxx1, dxx2, dyy1, dyy2;
                    double dx, dy;
                    double dxx, dyy;
                    
                    double rfrac, cfrac;
                    double rweight1, rweight2, cweight1, cweight2;
                    
					double dx1, dx2, dy1, dy2;
                    
                    if (r >= 1 + addSampleStep && r < _height - 1 - addSampleStep && c >= 1 + addSampleStep && c < _width - 1 - addSampleStep) {
                        weight = _lookup2[int(rpos * rpos + cpos * cpos)];
 
                        dxx1 = _Pixels[r + addSampleStep + 1][c + addSampleStep + 1] + _Pixels[r - addSampleStep][c] - _Pixels[r - addSampleStep][c + addSampleStep + 1] - _Pixels[r + addSampleStep + 1][c];
                        dxx2 = _Pixels[r + addSampleStep + 1][c + 1] + _Pixels[r - addSampleStep][c - addSampleStep] - _Pixels[r - addSampleStep][c + 1] - _Pixels[r + addSampleStep + 1][c - addSampleStep];
                        dyy1 = _Pixels[r + 1][c + addSampleStep + 1] + _Pixels[r - addSampleStep][c - addSampleStep] - _Pixels[r - addSampleStep][c + addSampleStep + 1] - _Pixels[r + 1][c - addSampleStep];
                        dyy2 = _Pixels[r + addSampleStep + 1][c + addSampleStep + 1] + _Pixels[r][c - addSampleStep] - _Pixels[r][c + addSampleStep + 1] - _Pixels[r + addSampleStep + 1][c - addSampleStep];

																				
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
                            _index[ri][ci][ori1] = cweight1;
                            _index[ri][ci][ori2] = cweight2;
                        }
                    }  
                }  
                           
           	}
      		}
    }