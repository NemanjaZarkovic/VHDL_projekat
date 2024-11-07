#include <iostream>
#include <ctime>
#include <cmath>
#include "opencv2/highgui/highgui.hpp"
#include <fstream>
#include <fcntl.h>
Expand
user_app.cpp
7 KB
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

//#define ROWS 512
//#define COLS 512
#define MAX_ROWS_COLS 512*512
#define CLUSTERS_NUMBER 6
#define MAX_IMAGE_DATA_SIZE 1024 * 1024
#define DEVICE_NAME "cluster_driver"
#define BUF_LEN 200000

int major_num;
char msg[BUF_LEN];
char *msg_ptr;
unsigned char *dummyImage;
int ROWS = 0,COLS=0;
typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Pixel;

typedef struct {
    int x;
    int y;
    int z;
} Point3i;

Pixel *clustersCenters;
Point3i *ptInClusters;
Pixel startClusters[6];

void findAssociatedCluster(Pixel *clustersCenters, Point3i *ptInClusters, unsigned char* imageData)
{
for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            int minDistance = INT_MAX;
            int closestClusterIndex = 0;

            Pixel pixel;
            int index = ((r * COLS) + c) * 3;
            pixel.b = imageData[index];
            pixel.g = imageData[index + 1];
            pixel.r = imageData[index + 2];

            for (int k = 0; k < CLUSTERS_NUMBER; k++)
            {
                Pixel clusterCenter = clustersCenters[k];

        int diffBlue = pixel.b - clusterCenter.b;
        int diffGreen = pixel.g - clusterCenter.g;
        int diffRed = pixel.r - clusterCenter.r;
        int distance = abs(diffBlue) + abs(diffGreen) + abs(diffRed);

                if (distance < minDistance)
        {
            minDistance = distance;
            closestClusterIndex = k;
        }
            }

    ptInClusters[(r * COLS) + c].x = c;
    ptInClusters[(r * COLS) + c].y = r;
    ptInClusters[(r * COLS) + c].z = closestClusterIndex;
        }
        
}

for(int i = 0; i < ROWS*COLS; i++)
{
       printk(KERN_INFO "Point %d: (%d, %d, %d)\n", i, ptInClusters[i].x, ptInClusters[i].y, ptInClusters[i].z);
    }
    
    printk(KERN_INFO "Found associated clusters.\n");
}

int cluster_driver_open(struct inode *inode, struct file *file)
{
return 0;
}

int cluster_driver_release(struct inode *inode, struct file *file)
{
return 0;
}

ssize_t cluster_driver_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    // Define variables for image rows and columns
    int img_rows, img_cols;

    // Read the image rows and columns from the user buffer
    if (copy_from_user(&img_rows, user_buf, sizeof(int)))
    {
        printk(KERN_ALERT "Failed to copy image rows from user space\n");
        return -EFAULT;
    }
... (149 lines left)
Collapse
cluster_driver.c
8 KB
#!/bin/bash

sudo rm /dev/cluster_driver

sudo rmmod cluster_driver
Expand
delete.sh
1 KB
Attachment file type: unknown
Makefile
173 bytes
#!/bin/bash

make

sudo insmod cluster_driver.ko
Expand
run.sh
1 KB
Attachment file type: document
MSREAL.docx
714.91 KB
You missed a call from 
stefanrb
 that lasted 2 minutes.
 — Today at 4:08 PM
stefanrb — Today at 4:10 PM
moze?
nemanja00 — Today at 4:10 PM
само минут
nemanja00
 started a call.
 — Today at 4:10 PM
nemanja00 — Today at 4:21 PM
#define SC_INCLUDE_FX
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/time.h>
#include <string.h>
#include <cmath>
#include <limits>

#include <systemc>
#include <vector>

#include "imload.h"
#include "image.h"
#include "ipoint.h"
#include "fasthessian.h"

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
std::vector<std::vector<num_f>> _Pixels;

num_f _lookup1[83], _lookup2[40];

double scale;
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

... (449 lines left)
Collapse
main.cpp
21 KB
﻿
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

//#define ROWS 512
//#define COLS 512
#define MAX_ROWS_COLS 512*512
#define CLUSTERS_NUMBER 6
#define MAX_IMAGE_DATA_SIZE 1024 * 1024
#define DEVICE_NAME "cluster_driver"
#define BUF_LEN 200000

int major_num;
char msg[BUF_LEN];
char *msg_ptr;
unsigned char *dummyImage;
int ROWS = 0,COLS=0;
typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Pixel;

typedef struct {
    int x;
    int y;
    int z;
} Point3i;

Pixel *clustersCenters;
Point3i *ptInClusters;
Pixel startClusters[6];

void findAssociatedCluster(Pixel *clustersCenters, Point3i *ptInClusters, unsigned char* imageData)
{
	for (int r = 0; r < ROWS; r++)
    	{
        	for (int c = 0; c < COLS; c++)
        	{
            		int minDistance = INT_MAX;
            		int closestClusterIndex = 0;

            		Pixel pixel;
            		int index = ((r * COLS) + c) * 3;
            		pixel.b = imageData[index];
            		pixel.g = imageData[index + 1];
            		pixel.r = imageData[index + 2];

            	for (int k = 0; k < CLUSTERS_NUMBER; k++)
            	{
                	Pixel clusterCenter = clustersCenters[k];

		        int diffBlue = pixel.b - clusterCenter.b;
		        int diffGreen = pixel.g - clusterCenter.g;
		        int diffRed = pixel.r - clusterCenter.r;
		        int distance = abs(diffBlue) + abs(diffGreen) + abs(diffRed);

                	if (distance < minDistance)
		        {
		            minDistance = distance;
		            closestClusterIndex = k;
		        }
            	}

		    ptInClusters[(r * COLS) + c].x = c;
		    ptInClusters[(r * COLS) + c].y = r;
		    ptInClusters[(r * COLS) + c].z = closestClusterIndex;
        	}
        
	}
	
	for(int i = 0; i < ROWS*COLS; i++)
	{
       		printk(KERN_INFO "Point %d: (%d, %d, %d)\n", i, ptInClusters[i].x, ptInClusters[i].y, ptInClusters[i].z);
    	}
    	
    	printk(KERN_INFO "Found associated clusters.\n");
}

int cluster_driver_open(struct inode *inode, struct file *file)
{
	return 0;
}

int cluster_driver_release(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t cluster_driver_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    // Define variables for image rows and columns
    int img_rows, img_cols;

    // Read the image rows and columns from the user buffer
    if (copy_from_user(&img_rows, user_buf, sizeof(int)))
    {
        printk(KERN_ALERT "Failed to copy image rows from user space\n");
        return -EFAULT;
    }

    if (copy_from_user(&img_cols, user_buf + sizeof(int), sizeof(int)))
    {
        printk(KERN_ALERT "Failed to copy image columns from user space\n");
        return -EFAULT;
    }
    
    ROWS = img_rows;
    COLS = img_cols;

    // Calculate the size of the image data
    size_t imageBytes = img_rows * img_cols * 3;
    // Calculate the size of the cluster centers
    size_t clustersBytes = CLUSTERS_NUMBER * sizeof(Pixel);

    // Check if the user buffer contains both image data and cluster centers
    if (count < 2 * sizeof(int) + imageBytes + clustersBytes)
    {
        printk(KERN_ALERT "Insufficient data in user buffer\n");
        return -EFAULT;
    }

    // Copy image data from user buffer
    if (copy_from_user(dummyImage, user_buf + 2 * sizeof(int), imageBytes))
    {
        printk(KERN_ALERT "Failed to copy image data from user space\n");
        return -EFAULT;
    }

    // Copy cluster centers from user buffer
    if (copy_from_user(startClusters, user_buf + 2 * sizeof(int) + imageBytes, clustersBytes))
    {
        printk(KERN_ALERT "Failed to copy clustersCenters data from user space\n");
        return -EFAULT;
    }

    // Call findAssociatedCluster to populate ptInClusters
    findAssociatedCluster(startClusters, ptInClusters, dummyImage);

    return count;
}



ssize_t cluster_driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    	printk(KERN_INFO "Reading from cluster_driver\n");

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

    	if (copy_to_user(user_buf, ptInClusters + (*ppos / sizeof(Point3i)), bytes_to_read))
    	{
        	printk(KERN_ALERT "Failed to copy ptInClusters to user space\n");
       		return -EFAULT;
    	}

    	*ppos += bytes_to_read;

    	return bytes_to_read;
}

static const struct file_operations cluster_driver_fops = {
    	.owner = THIS_MODULE,
    	.open = cluster_driver_open,
    	.release = cluster_driver_release,
    	.read = cluster_driver_read,
    	.write = cluster_driver_write,
};

int init_module(void)
{
    	major_num = register_chrdev(0, DEVICE_NAME, &cluster_driver_fops);

    	if (major_num < 0)
    	{
        	printk(KERN_ALERT "Failed to register a major number\n");
        	return major_num;
    	}
	ROWS = kmalloc(sizeof(int),GFP_KERNEL);
	COLS = kmalloc(sizeof(int),GFP_KERNEL);
	printk(KERN_INFO "ROWS: %d", ROWS);
	//printk("COLS:\n %d" COLS);
    	clustersCenters = kmalloc(CLUSTERS_NUMBER * 3 * sizeof(int), GFP_KERNEL);
    	if (!clustersCenters)
    	{
        	printk(KERN_ALERT "Failed to allocate memory for clustersCenters\n");
        	unregister_chrdev(major_num, DEVICE_NAME);
        	return -ENOMEM;
    	}

    	ptInClusters = kmalloc(MAX_ROWS_COLS * 3 * sizeof(int), GFP_KERNEL);
    	
    	if (!ptInClusters)
    	{
        	printk(KERN_ALERT "Failed to allocate memory for ptInClusters\n");
        	kfree(clustersCenters);
        	unregister_chrdev(major_num, DEVICE_NAME);
        	return -ENOMEM;
    	}

    	dummyImage = kmalloc(MAX_ROWS_COLS * 3 * sizeof(int), GFP_KERNEL);
    	if (!dummyImage)
    	{
        	printk(KERN_ALERT "Failed to allocate memory for dummyImage\n");
        	kfree(ptInClusters);
        	kfree(clustersCenters);
        	unregister_chrdev(major_num, DEVICE_NAME);
        	return -ENOMEM;
    	}

    	printk(KERN_INFO "Registered correctly with major number %d\n", major_num);

    	return 0;
}

void cleanup_module(void)
{
    	if (dummyImage)
        	kfree(dummyImage);
    	if (ptInClusters)
        	kfree(ptInClusters);
    	if (clustersCenters)
        	kfree(clustersCenters);
    	unregister_chrdev(major_num, DEVICE_NAME);
    	printk(KERN_INFO "Unregistered %s device\n", DEVICE_NAME);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Cluster driver");
cluster_driver.c
8 KB


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
    										if (i == 5 && j == 6) {
                /* static int counterfor;
                counterfor++;
                cout << "Uslo u for petlju " << counterfor << " puta" << endl;*/
        
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
                            _index[ri][ci][ori1] += cweight1;
                            _index[ri][ci][ori2] += cweight2;
                        }
                    }  
                }  
                cout << "i: " << i << ", j: " << j << endl;
                cout << "temp1_rpos: " << temp1_rpos << ", temp2_rpos: " << temp2_rpos << ", temp3_rpos: " << temp3_rpos << ", temp4_rpos: " << temp4_rpos << endl;             
           	}
      		}
    }
