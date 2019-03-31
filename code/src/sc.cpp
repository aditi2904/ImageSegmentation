#include "sc.h"
#include <math.h>
#include <vector>
#include <memory>
using namespace cv;
using namespace std;


bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image){

    // some sanity checks
    // Check 1 -> new_width <= in_image.cols
    if(new_width>in_image.cols){
        cout<<"Invalid request!!! new_width has to be smaller than the current size!"<<endl;
        return false;
    }
    if(new_height>in_image.rows){
        cout<<"Invalid request!!! ne_height has to be smaller than the current size!"<<endl;
        return false;
    }
    
    if(new_width<=0){
        cout<<"Invalid request!!! new_width has to be positive!"<<endl;
        return false;
    }
    
    if(new_height<=0){
        cout<<"Invalid request!!! new_height has to be positive!"<<endl;
        return false;    
    }

    Mat iimage = in_image.clone();
    Mat oimage = in_image.clone();
    while(iimage.rows!=new_height || iimage.cols!=new_width){
        
        // horizontal seam if needed
        if(iimage.rows>new_height){  
            reduce_horizontal_seam_trivial(iimage, oimage);
            iimage = oimage.clone();
        }
        if(iimage.cols>new_width){
            reduce_vertical_seam_trivial(iimage, oimage);
            iimage = oimage.clone();
        }
    }
    out_image = oimage.clone();
    
    return true;
}

double CalculateEnergy(Mat& in_image,int i,int j)
{    Vec3b hx, hy , vx, vy;
     int rows = in_image.rows;
     int cols = in_image.cols;

    (i!=rows-1)? hx  = in_image.at<Vec3b>(i+1, j) : hx  = in_image.at<Vec3b>(0, j);
	(j!=cols-1)? hy  = in_image.at<Vec3b>(i, j+1) : hy  = in_image.at<Vec3b>(i, 0);
	(i!=0)? vx  = in_image.at<Vec3b>(i-1, j) : vx  = in_image.at<Vec3b>(rows-1, j);
    (j!=0)? vy  = in_image.at<Vec3b>(i, j-1) :  vy  = in_image.at<Vec3b>(i, cols-1);
	  
	double xGradient = pow(hx[0] - vx[0],2) + pow(hx[1] - vx[1],2) + pow(hx[2] - vx[2],2);
	double yGradient = pow(hy[0] - vy[0],2) + pow(hy[1] - vy[1],2) + pow(hy[2] - vy[2],2);

	return xGradient + yGradient;
} 

// vertical trivial seam is a seam through the center of the image
bool reduce_vertical_seam_trivial(Mat& in_image, Mat& out_image){
      Mat src;
	src = in_image.clone();
    // retrieve the dimensions of the new image
    int rows = in_image.rows;
    int cols = in_image.cols;
     GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
  double  PE = 0;
   int  temp = 0,i,j;
    vector<vector<int> > TotalEnergy;
	TotalEnergy.resize(rows,vector<int>(cols));

    //filling up the  energy matrix
    for(i = 0; i < rows; i++){
        for( j = 0; j < cols; j++){
			PE = CalculateEnergy(src,i,j);
			  if(i==0)
            temp = 0;
           else  if (j == 0)
                temp = min(TotalEnergy[i-1][j+1], TotalEnergy[i-1][j]);
            else if (j == cols-1)
                temp = min(TotalEnergy[i-1][j-1], TotalEnergy[i-1][j]);
            else
                temp = min(TotalEnergy[i-1][j-1], min(TotalEnergy[i-1][j], TotalEnergy[i-1][j+1]));
            
            TotalEnergy[i][j] = PE + temp;
        }
    }
 int minEnergyPixel = TotalEnergy[rows-1][0], seam = 0;
    
 //Find optimal Vertical Seam
	for(j=1;j<cols;j++)
		  {  if (minEnergyPixel > TotalEnergy[rows-1][j])
			  {  minEnergyPixel = TotalEnergy[rows-1][j];
			     seam = j;
			  }
	  }
  
vector<int> verticalValues(rows);
	verticalValues[rows-1] = seam;
    //Backtracking and  storing that seam in the seam array
    for(i = rows-1; i>0; i--){
            if(seam ==0){
               if (TotalEnergy[i-1][seam] > TotalEnergy[i-1][seam+1]) 
				   seam++;
            }
            else if(seam  == rows-1)
			{
                (TotalEnergy[i-1][seam-1] < TotalEnergy[i-1][seam]) ? seam ++ : seam --;
					
				 }
            else if (TotalEnergy[i-1][seam-1] < TotalEnergy[i-1][seam] )
			{(TotalEnergy[i-1][seam-1] < TotalEnergy[i-1][seam+1]) ?  seam-- : seam ++;
				 }
                else {
					 (TotalEnergy[i-1][seam]> TotalEnergy[i-1][seam+1]) ? seam ++ : seam --;
					
				}
				
                   verticalValues[i-1] = seam; 
            }
       
out_image.create(rows, cols-1, CV_8UC3);
int  u, v;
//taking pixels from input image and copying to output image
	    for(u = 0; u < rows; ++u){
		int x= verticalValues[u];
        for(v=0;v<x;++v)
		{ 
	       out_image.at<Vec3b>(u,v) = in_image.at<Vec3b>(u, v);  
		}
		 for( v=x+1;v<cols;++v){
                   
                out_image.at<Vec3b>(u,v-1) = in_image.at<Vec3b>(u, v);
            }
	}

    return true;
}
 

bool reduce_horizontal_seam_trivial(Mat& in_image, Mat& out_image){
     Mat src;
	src = in_image.clone();
	  GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
    // retrieve the dimensions of the new image
    int rows = in_image.rows;
    int cols = in_image.cols;
    double  pixel = 0;
	
   int  temp = 0,i,j;
    vector<vector<int> > TotalEnergy;
	TotalEnergy.resize(rows,vector<int>(cols));
	
	    //Cumulative energy
    for(j=0;j<cols;++j)
     for(i=0;i<rows;++i){
       pixel = CalculateEnergy(src,i,j);
    	if(j==0)
			temp = 0;
		else if (i==0)
		    temp = min(TotalEnergy[i][j-1],TotalEnergy[i+1][j-1]);
		else if(i==rows-1)
            temp =  min(TotalEnergy[i-1][j-1],TotalEnergy[i][j-1]);
        else
            temp = min(min(TotalEnergy[i-1][j-1],TotalEnergy[i][j-1]),TotalEnergy[i+1][j-1]);

	    TotalEnergy[i][j] = pixel + temp;
	}
int minEnergyPixel = TotalEnergy[0][cols-1], seam = 0;
    //finding the column with the least cumulative energy and storing it as the last value of the seam array
   
    for(j=1;j<rows;j++)
		  {  if (minEnergyPixel > TotalEnergy[j][cols-1])
			  { minEnergyPixel = TotalEnergy[j][cols-1];
			    seam = j;
			  }
	  }
    
	vector<int> horizontalValues(cols); //X values for each Y
	horizontalValues[cols-1] = seam;
  //Backtracking and  storing that seam in the seam array
    for(int j = cols-1; j>0; j--){
        
  
            if(seam ==0){
                if(TotalEnergy[seam][j-1] > TotalEnergy[seam+1][j-1]) 
					seam ++;
            }
            else if(seam ==(rows-1)){
               (TotalEnergy[seam-1][j-1] < TotalEnergy[seam][j-1])  ? seam -- : seam ++;
				   
            }
               else  if(TotalEnergy[seam-1][j-1] < TotalEnergy[seam][j-1])
             {       
                 (TotalEnergy[seam-1][j-1] < TotalEnergy[seam+1][j-1]) ? seam -- :  seam ++;
            }
			else
		{   (TotalEnergy[seam][j-1] > TotalEnergy[seam+1][j-1]) ? seam++ : seam --;
           
		}
		horizontalValues[j-1] = seam;
			
    }
	
  
 // create an image slighly smaller
    out_image.create(rows-1, cols, CV_8UC3);
	int u,v;

	 //taking pixels from input image and copying to output image
	    for(v = 0; v < cols; ++v){
		int x= horizontalValues[v];
        for(u=0;u<x;++u)
		{ 
	       out_image.at<Vec3b>(u,v) = in_image.at<Vec3b>(u, v);  
		}
		 for( u=x+1;u<rows;++u){
                   
                out_image.at<Vec3b>(u-1,v) = in_image.at<Vec3b>(u, v);
            }
	}

    return true;
}