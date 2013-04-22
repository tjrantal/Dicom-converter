

//g++ VideoReader.cpp -o videoReader -static -O3 -march=i686 -static -lavformat -lavcodec -lavutil -lswscale -lwsock32 -lgdi32 -lmingw32 -lx264 -lpthread -lfaac -liconv

//modet.exe 0104valo.wmv

#include <iostream>
#include <stdio.h>
#include <vector>
#include <CImg.h>
#include "videot.h"
//MAIN BEGINS
int main(int argc, char **argv){
	int maxframe = 1000; //Tähän voisi hakea cvGetCaptureProperty( video, CV_CAP_PROP_FRAME_WIDTH );
	unsigned long frameja = 0;
	int lleveys, lkorkeus;
	if (argc < 2){
		printf("Usage convertDicom.exe dicom result.avi\n");
		return 1;
	}
	videoReader lahde(argv[1],maxframe);	//Create video reader object
	printf("Opened file %s\n",argv[1]);
	//lahde.avaa_video();
	lleveys = lahde.leveys;
	lkorkeus = lahde.korkeus;
	printf("Video luettu leveys %d korkeus %d\n",lleveys,lkorkeus);
	videoWriter ulosVideo(argv[2],lleveys,lkorkeus);
	

	while (lahde.frames == lahde.varattu){ //Lukee videon loppuun saakka
		//printf("Videota lukemaan %d\n",lahde.frames);
		lahde.lue_videota();
		//printf("Saatiin luettua\n");
		for (long k = 0; k<lahde.frames;k++){ //Jätän alusta turhaa pois MUUTA TÄMÄ
			++frameja;
			//Tallennetaan video
			memcpy(ulosVideo.tmp_picture2->data[0],lahde.video[k],lleveys*lkorkeus*sizeof(unsigned char));
			ulosVideo.write_video_frame();
			//printf("Transcoded frame %d\r", frameja);
		}	//Maxframe for
	}	//while
	printf("Got to the end\n");
	
	lahde.sulje_video();
	printf("Closed the source video\n");
	ulosVideo.write_trailer();
	printf("All done\n");
}




