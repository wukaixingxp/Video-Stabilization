// Computer Vision for Digital Post-Production
// Lecturer: Gergely Vass - vassg@vassg.hu
//
// Skeleton Code for programming assigments
// 
// Code originally from Thomas Funkhouser
// main.c
// original by Wagner Correa, 1999
// modified by Robert Osada, 2000
// modified by Renato Werneck, 2003
// modified by Jason Lawrence, 2004
// modified by Jason Lawrence, 2005
// modified by Forrester Cole, 2006
// modified by Tom Funkhouser, 2007
// modified by Chris DeCoro, 2007
//



// Include files

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <assert.h>
#include "R2/R2.h"
#include "R2Pixel.h"
#include "R2Image.h"



// Program arguments
static char options[] =
"  -help\n"
"  -svdTest\n"
"  -sobelX\n"
"  -sobelY\n"
"  -medianFilter\n"
"  -bilateralFilter\n"
"  -log\n"
"  -harris <real:sigma>\n"
"  -featureDetect \n"
"  -saturation <real:factor>\n"
"  -brightness <real:factor>\n"
"  -blur<real:sigma>\n"
"  -sharpen \n"
"  -matchTranslation <file:other_image>\n"
"  -RANSACP <file:other_image>\n"
"  -RANSACT <file:other_image>\n"
"  -RANSAC <file:other_image>\n"
"  -matchHomography <file:other_image>\n"
"  -video\n";



static void 
ShowUsage(void)
{
  // Print usage message and exit
  fprintf(stderr, "Usage: imgpro input_image output_image [  -option [arg ...] ...]\n");
  fprintf(stderr, options);
  exit(EXIT_FAILURE);
}



static void 
CheckOption(char *option, int argc, int minargc)
{
  // Check if there are enough remaining arguments for option
  if (argc < minargc)  {
    fprintf(stderr, "Too few arguments for %s\n", option);
    ShowUsage();
    exit(-1);
  }
}



static int 
ReadCorrespondences(char *filename, R2Segment *&source_segments, R2Segment *&target_segments, int& nsegments)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open correspondences file %s\n", filename);
    exit(-1);
  }

  // Read number of segments
  if (fscanf(fp, "%d", &nsegments) != 1) {
    fprintf(stderr, "Unable to read correspondences file %s\n", filename);
    exit(-1);
  }

  // Allocate arrays for segments
  source_segments = new R2Segment [ nsegments ];
  target_segments = new R2Segment [ nsegments ];
  if (!source_segments || !target_segments) {
    fprintf(stderr, "Unable to allocate correspondence segments for %s\n", filename);
    exit(-1);
  }

  // Read segments
  for (int i = 0; i <  nsegments; i++) {

    // Read source segment
    double sx1, sy1, sx2, sy2;
    if (fscanf(fp, "%lf%lf%lf%lf", &sx1, &sy1, &sx2, &sy2) != 4) { 
      fprintf(stderr, "Error reading correspondence %d out of %d\n", i, nsegments);
      exit(-1);
    }

    // Read target segment
    double tx1, ty1, tx2, ty2;
    if (fscanf(fp, "%lf%lf%lf%lf", &tx1, &ty1, &tx2, &ty2) != 4) { 
      fprintf(stderr, "Error reading correspondence %d out of %d\n", i, nsegments);
      exit(-1);
    }

    // Add segments to list
    source_segments[i] = R2Segment(sx1, sy1, sx2, sy2);
    target_segments[i] = R2Segment(tx1, ty1, tx2, ty2);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int 
main(int argc, char **argv)
{
  // Look for help
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-help")) {
      ShowUsage();
    }
    if (!strcmp(argv[i], "-svdTest")) {
      R2Image *image = new R2Image();
      image->svdTest();
      return 0;
    }
    else if (!strcmp(argv[i], "-video")) {
      printf("Video processing started\n");

      char inputName[100] = "videoinput/backyard/%07d.jpg";
      char outputName[100] = "videooutput/backyard/by%07d.jpg";
      //char tempoutputName[100] ="videooutput/backyard/test%07d.jpg";

      R2Image *mainImage = new R2Image();
      char currentFilename[100];
      char currentOutputFilename[100];
      //char tempOutputFilename[100];
     
      if (!mainImage) {
	fprintf(stderr, "Unable to allocate image\n");
	exit(-1);
      }
      
      // read very first frame
      sprintf(currentFilename, inputName, 0);
      
      if (!mainImage->Read(currentFilename)) {
	fprintf(stderr, "Unable to read first image\n");
	exit(-1);
      }
      

      // =============== VIDEO PROCESSING ===============

      //store the dx, dy motion vector frame by frame 
      double result[2];
      int end = 201;
      double dxList[300];
      double dyList[300];
      dxList[0]=0;
      dyList[0]=0;
      for (int i = 1; i <= end; i++)
	{
	  //read current frame
	  R2Image *currentImage = new R2Image();
	  if (!currentImage) {
	    fprintf(stderr, "Unable to allocate image %d\n",i);
	    exit(-1);
	  }
	  sprintf(currentFilename, inputName, i);
			
	  printf("Processing file %s\n", currentFilename);
	  if (!currentImage->Read(currentFilename)) {
	    fprintf(stderr, "Unable to read image %d\n", i);
	    exit(-1);
	  }

	  //process frames
	  mainImage->RansacT(currentImage,result);
	  dxList[i]=result[0]; //store the corresponding dx,dy 
	  dyList[i]=result[1];
	  
	  mainImage->Read(currentFilename);//replace as the previous frame
	  delete currentImage;
	}
      
      for(int i=0;i<=end;i++){
	printf("dx=%f, dy=%f",dxList[i],dyList[i]);
      }

      
      //smooth the dx,dy accumlated curve and store it in the new lists
      double dxSum = 0;
      double dySum = 0;
      /*
      double avgdx = dxSum/end;
      double avgdy = dySum/end;
      printf("avgdx=%f, avgdy=%f",avgdx,avgdy);
      */
      double dx_ws, dy_ws;
      double dx_accList[300];
      double dy_accList[300];
      double dx_nList[300];
      double dy_nList[300];
      for(int i=0;i<=end;i++){
	dxSum += dxList[i];
	dySum += dyList[i];
	dx_accList[i]=dxSum;
	dy_accList[i]=dySum;
      }

      for(int i=0;i<=end;i++){
	for(int j=-3;j<=3;j++){//average using a sliding window of length 7 
	  dx_ws =0;
	  dy_ws =0;
	  if(i+j<0){
	    dx_ws += dx_accList[-i-j];
	    dy_ws += dy_accList[-i-j];
	  }
	  else if(i+j>=0 && i+j<=end){
	    dx_ws += dx_accList[i+j];
	    dy_ws += dy_accList[i+j];
	  }
	  else if(i+j>end){
	    dx_ws += dx_accList[2*end-i-j];
	    dy_ws += dy_accList[2*end-i-j];
	     
	  }
	}
	dx_nList[i]=dx_ws/7;
	dy_nList[i]=dy_ws/7;
	
      }

      /*
      double dx_acc=0;
      double dy_acc=0;
      */
      for(int i=0;i<=end;i++){
	/*
	dx_acc+=dxList[i];
	dy_acc+=dyList[i];
	*/
	
	double difx= dx_nList[i]-dx_accList[i];
	double dify= dy_nList[i]-dy_accList[i]; //generate the new transform dx,dy;
	
        R2Image *currentImage = new R2Image();
	if (!currentImage) {
	  fprintf(stderr, "Unable to allocate image %d\n",i);
	  exit(-1);
	}

	sprintf(currentFilename, inputName, i);
	sprintf(currentOutputFilename, outputName, i);
	printf("Retransform file %s\n", currentFilename);
	  if (!currentImage->Read(currentFilename)) {
	    fprintf(stderr, "Unable to read image %d\n", i);
	    exit(-1);
	  }
	  
	  currentImage->Translate(difx,dify);
	  if (!currentImage->Write(currentOutputFilename)) {
	    fprintf(stderr, "Unable to write %s\n", currentOutputFilename);
	    exit(-1);
	  }
	  delete currentImage;
      }
      // Return success
      return EXIT_SUCCESS;
    }
  }

  // Read input and output image filenames
  if (argc < 3)  ShowUsage();
  argv++, argc--; // First argument is program name
  char *input_image_name = *argv; argv++, argc--; 
  char *output_image_name = *argv; argv++, argc--;

  // Allocate image
  R2Image *image = new R2Image();
  if (!image) {
    fprintf(stderr, "Unable to allocate image\n");
    exit(-1);
  }

  // Read input image
  if (!image->Read(input_image_name)) {
    fprintf(stderr, "Unable to read image from %s\n", input_image_name);
    exit(-1);
  }

  // Initialize sampling method
  int sampling_method = R2_IMAGE_POINT_SAMPLING;

  // Parse arguments and perform operations 
  while (argc > 0) {
    if (!strcmp(*argv, "-brightness")) {
      CheckOption(*argv, argc, 2);
      double factor = atof(argv[1]);
      argv += 2, argc -=2;
      image->Brighten(factor);
    }
	else if (!strcmp(*argv, "-sobelX")) {
      argv++, argc--;
      image->SobelX();
    }
	else if (!strcmp(*argv, "-sobelY")) {
      argv++, argc--;
      image->SobelY();
    }
	else if (!strcmp(*argv, "-log")) {
      argv++, argc--;
      image->LoG();
    }
    else if (!strcmp(*argv, "-saturation")) {
      CheckOption(*argv, argc, 2);
      double factor = atof(argv[1]);
      argv += 2, argc -= 2;
      image->ChangeSaturation(factor);
    }
    else if (!strcmp(*argv, "-harris")) {
      CheckOption(*argv, argc, 2);
      double sigma = atof(argv[1]);
      argv += 2, argc -= 2;
      image->Harris(sigma);
    }
    else if (!strcmp(*argv, "-blur")) {
      CheckOption(*argv, argc, 2);
      double sigma = atof(argv[1]);
      argv += 2, argc -= 2;
      image->Blur(sigma);
    }
    else if (!strcmp(*argv, "-sharpen")) {
      argv++, argc--;
      image->Sharpen();
    }
    else if (!strcmp(*argv, "-matchTranslation")) {
      CheckOption(*argv, argc, 2);
      R2Image *other_image = new R2Image(argv[1]);
      argv += 2, argc -= 2;
      image->blendOtherImageTranslated(other_image);
      delete other_image;
    }
    else if (!strcmp(*argv, "-RANSACP")) {
      CheckOption(*argv, argc, 2);
      R2Image *other_image = new R2Image(argv[1]);
      argv += 2, argc -= 2;
      image->RansacP(other_image);
      delete other_image;
    }
    else if (!strcmp(*argv, "-RANSAC")) {
      CheckOption(*argv, argc, 2);
      R2Image *other_image = new R2Image(argv[1]);
      argv += 2, argc -= 2;
      image->Ransac(other_image);
      delete other_image;
    }
    else if (!strcmp(*argv, "-matchHomography")) {
      CheckOption(*argv, argc, 2);
      R2Image *other_image = new R2Image(argv[1]);
      argv += 2, argc -= 2;
      image->blendOtherImageHomography(other_image);
      delete other_image;
    }
    else {
      // Unrecognized program argument
      fprintf(stderr, "image: invalid option: %s\n", *argv);
      ShowUsage();
    }
  }

  // Write output image
  if (!image->Write(output_image_name)) {
    fprintf(stderr, "Unable to read image from %s\n", output_image_name);
    exit(-1);
  }

  // Delete image
  delete image;

  // Return success
  return EXIT_SUCCESS;
}



