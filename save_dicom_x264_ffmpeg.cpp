//Depends on independent jpeg group libjpeg (http://www.ijg.org/ tested with version 9 on 18th of April 2013)
//	COMPILE STATIC:
// g++ -o save_dicom_x264_ffmpeg.exe save_dicom_x264_ffmpeg.cpp -static -O3 -march=i686 -static -lavformat -lavcodec -lavutil -lswscale -lwsock32 -lgdi32 -lmingw32 -lx264 -lpthread -lfaac -liconv
// to build (assuming libavformat, libavcodec, libavutil, and swscale are correctly installed on
// your system).

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector> //DICOMIA VARTEN
#include <math.h>
#include <setjmp.h>
#include <vector>
#include "VideoReader/videoWriter.h"

/**************************************************************/
int lleveys = 760,lkorkeus = 500;


int main(int argc, char **argv)
{
    const char *filename,*filein;
	unsigned short riveja, kolumneja,videon_korkeus,videon_leveys;
	videon_korkeus = 512;//496;
	videon_leveys = 768;//752;
    if (argc < 3) {
        printf("usage: save_dicom_x264_ffmpeg video.dcm output.avi [mpeg1]\n");
        exit(1);
    }
	char* compression;
	if (argc >3){
		compression = argv[3];
	}else{
		compression = (char*) "x264";
	}
    

	//DICOMIN VALMISTELUT
	FILE* tie;
	unsigned char* buffer;
	unsigned char* vali;
	
	
	long int loppu;
		unsigned long laskuri = 0, kohta,red, green,blue;
	signed char lukuja;
	unsigned char luku;
	unsigned int i,j;
	std::vector<unsigned char> hexat(4);
	std::vector<unsigned char> resoluutio;
	std::vector<unsigned long> headeri(17,0);
	buffer = (unsigned char*) malloc(sizeof(unsigned char)*4);
	
	
	tie = fopen(argv[1],"rb");
	fseek(tie,0,SEEK_END);	//Tarkistetaan kuinka pitk‰ tiedosto on kyseess‰
	loppu = ftell(tie);
	fseek(tie,132,SEEK_SET); //128 tyhj‰‰ + DICM
	
	//Luetaan heksadesimaaleja tiedostosta
	
	//T‰ss‰ saadaan etsitty‰ headerist‰ kiinnostavia kohtia
	//E0 = 224, 7F = 127, 10 = 16 00 = 0 //7fe00010
	while (( hexat[0] != 224 || hexat[1] != 127 || hexat[2] != 16 || hexat[3] != 0) && ftell(tie) < loppu) { //7FE0,0010  PIXEL DATA
		if (ftell(tie) == loppu){
			break;	//Poistutaan silmukasta, kun data loppuu...
		}
		
		fread(buffer,1,1,tie);
		hexat.erase(hexat.begin()); //Poistetaan ensimm‰inen
		hexat.push_back(*buffer);		//Lis‰t‰‰n viimeiseksi
		//Lis‰t‰‰n t‰h‰n rivien ja kolumnien lukum‰‰r‰n tarkistukset
		
		if ( hexat[0] == 2*16+8 && hexat[1] == 0 && hexat[2] == 16 && hexat[3] == 0) { //Rows
			fread(buffer,1,4,tie); //Poistetaan kirjaimet ja luettavien lukum‰‰r‰
			fread(&riveja,1,2,tie); //Luetaan rivit			
			printf("Rivit lˆytyi %d\n",riveja);
		}
		if ( hexat[0] == 2*16+8 && hexat[1] == 0 && hexat[2] == 17 && hexat[3] == 0) { //Columns
			fread(buffer,1,4,tie); //Poistetaan kirjaimet ja luettavien lukum‰‰r‰
			fread(&kolumneja,1,2,tie); //Luetaan rivit			
			printf("Kolumnit lˆytyi %d\n",kolumneja);
		}		
		if ( hexat[0] == 2*16+8 && hexat[1] == 0 && hexat[2] == 3*16 && hexat[3] == 0) { //Rows
			fread(buffer,1,2,tie); //Poistetaan kirjaimet ja luettavien lukum‰‰r‰
			unsigned short luettavaa;
			unsigned char kirjain;
			fread(&luettavaa,1,2,tie); //Luetaan rivit
			printf("Resoluutio ");
			for (i = 0; i<luettavaa;i++){
				fread(&kirjain,1,1,tie);
				resoluutio.push_back(kirjain);
				printf("%c",kirjain);
			}
			printf("\n");
		}		
		
	}
	fread(buffer,1,2,tie);		//Data tagin j‰lkeen on viel‰ OW taiOB
	//Valmistellaan kuva ja n‰yttˆ
	
	/*Single plane*/
	/*
	uint8_t* kuva;
	kuva = (uint8_t*) malloc(sizeof(uint8_t)*videon_leveys*videon_korkeus*3);
	*/
	/*Three planes*/
	
	uint8_t * kuva[3];
	kuva[0] = new uint8_t[(videon_leveys+16)*videon_korkeus];	//+16 for the needs of libavcodec, not necessarily needed...
	kuva[1] = new uint8_t[(videon_leveys+16)*videon_korkeus];
	kuva[2] = new uint8_t[(videon_leveys+16)*videon_korkeus];
	
	
	//unsigned char* kuva;
	
	//kuva = new unsigned char[videon_leveys*videon_korkeus*3];
	//memset(kuva,0,videon_leveys*videon_korkeus*3*sizeof(unsigned char));
	vali = new unsigned char[kolumneja*riveja*3];
	//Etsit‰‰n ensimm‰inen  item (FFFE,E000) 
		while (( hexat[0] != 15*16+14 || hexat[1] != 15*16+15 || hexat[2] != 0 || hexat[3] != 14*16) && ftell(tie) < loppu) { //7FE0,0010  PIXEL DATA
			fread(buffer,1,1,tie);
			hexat.erase(hexat.begin()); //Poistetaan ensimm‰inen
			hexat.push_back(*buffer);		//Lis‰t‰‰n viimeiseksi
		}
		
	//DICOM VALMISTELTU
	//x264 valmistelu
	videoWriter ulosVideo(argv[2],videon_leveys,videon_korkeus,PIX_FMT_RGB24,compression); //
	//Lue seuraava DICOM-kuva
	//T‰h‰n silmukka kuvan hakemiselle
	
	
		//Silmukka, jolla luetaan kuvat l‰pi
	unsigned int kuvia_naytetty = 0;
	while (ftell(tie) < loppu){// && kuvia_naytetty < 1){
		++kuvia_naytetty;
		//printf("Kuvia %d",kuvia_naytetty);
		//Luetaan merkki lis‰‰, jotta skripti jatkuu..
		fread(buffer,1,1,tie);
		hexat.erase(hexat.begin()); //Poistetaan ensimm‰inen
		hexat.push_back(*buffer);		//Lis‰t‰‰n viimeiseksi
		//Etsit‰‰n seuraava  item (FFFE,E000), ensimm‰inen oli viel‰ jotain turhaa h‰im‰kk‰‰...
		while (( hexat[0] != 15*16+14 || hexat[1] != 15*16+15 || hexat[2] != 0 || hexat[3] != 14*16) && ftell(tie) < loppu) { //7FE0,0010  PIXEL DATA
			if (ftell(tie) == loppu){
				break;	//Poistutaan silmukasta, kun data loppuu...
			}
			fread(buffer,1,1,tie);
			hexat.erase(hexat.begin()); //Poistetaan ensimm‰inen
			hexat.push_back(*buffer);		//Lis‰t‰‰n viimeiseksi
		}
		if (ftell(tie) < loppu){
			//fread(buffer,1,2,tie);		//Item tagin j‰lkeen on viel‰ kaksi turhaa bitti‰
			//printf("Paikka %ld\n",ftell(tie));
			//Luetaan RLE header
			kohta = ftell(tie);
			for (j = 0;j<17;j++){
				fread(&headeri[j],1,4,tie);
				
			}
			//printf("pituus %ld\tvareja %ld\talku1 %ld\talku2 %ld\talku3 %ld\n",headeri[0],headeri[1],headeri[2],headeri[3],headeri[4]);
			//printf("Kohdat %ld\t %ld\t %ld\n",kohta+4+headeri[2],kohta+4+headeri[3],kohta+4+headeri[4]);
			
			//RLE dekoodaus

			laskuri = 0;
			//RED
			
			fseek(tie,kohta+4+headeri[2],SEEK_SET); //128 tyhj‰‰ + DICM
			//printf("RED %ld\n",ftell(tie));
			while (laskuri <riveja*kolumneja){
				fread(&lukuja,1,1,tie);
				
				if (lukuja >=0 && lukuja <=127) {
						fread(vali+laskuri,1,lukuja+1,tie);
						laskuri = laskuri+lukuja+1;
				}
				if (lukuja <0 && lukuja >=-127) {
					fread(&luku,1,1,tie);	
					for (j = 0;j<-lukuja+1;j++){
						*(vali+laskuri) = luku;	
						
						laskuri++;
					}
				}
				//printf("Laskuri %ld\r",laskuri);
			}
			//printf("Laskuri %ld\t Pit‰isi olla %d\n",laskuri,riveja*kolumneja);
			//GREEN
			fseek(tie,kohta+4+headeri[3],SEEK_SET); //128 tyhj‰‰ + DICM
			//printf("GREEN %ld\n",ftell(tie));
			while (laskuri <riveja*kolumneja*2){
				fread(&lukuja,1,1,tie);
				
				if (lukuja >=0 && lukuja <=127) {
					fread(vali+laskuri,1,lukuja+1,tie);
						laskuri = laskuri+lukuja+1;
					
				}
				if (lukuja <0 && lukuja >=-127) {
					fread(&luku,1,1,tie);	
					for (j = 0;j<-lukuja+1;j++){
						*(vali+laskuri) = luku;	
						laskuri++;
					}
				}
				//printf("Laskuri %ld\r",laskuri);
			}
			//printf("Laskuri %ld\t Pit‰isi olla %d\n",laskuri,riveja*kolumneja*2);
			//BLUE
			fseek(tie,kohta+4+headeri[4],SEEK_SET); //128 tyhj‰‰ + DICM
			//printf("BLUE %ld\n",ftell(tie));
			while (laskuri <riveja*kolumneja*3){
				fread(&lukuja,1,1,tie);
				
				if (lukuja >=0 && lukuja <=127) {
					fread(vali+laskuri,1,lukuja+1,tie);
						laskuri = laskuri+lukuja+1;
				}
				if (lukuja <0 && lukuja >=-127) {
					fread(&luku,1,1,tie);	
					for (j = 0;j<-lukuja+1;j++){
						*(vali+laskuri) = luku;	
						laskuri++;
					}
				}
			}	
			
			//TALLENNETAAN VIDEOKSI...
			//printf("Kuvaa tayttamaan\n");
			
			
			/*Single plane, packed*/
			/*
			for (int jjj = 0;jjj<videon_korkeus;jjj++){
				for (int iii = 0;iii<videon_leveys;iii++){
					if (iii < kolumneja && jjj < riveja){
						kuva[3*iii+jjj*videon_leveys*3]=vali[iii+jjj*kolumneja];	//R
						kuva[3*iii+1+jjj*videon_leveys*3]=vali[iii+jjj*kolumneja+riveja*kolumneja];	//G
						kuva[3*iii+2+jjj*videon_leveys*3]=vali[iii+jjj*kolumneja+2*riveja*kolumneja];	//B
					
					}else{
						kuva[3*iii+jjj*videon_leveys*3]	=0;	//R
						kuva[3*iii+1+jjj*videon_leveys*3]	=0;	//G
						kuva[3*iii+2+jjj*videon_leveys*3]	=0;	//B
					}
				}
			}
			*/
			/*Single plane planar*/
			/*
			for (int jjj = 0;jjj<videon_korkeus;jjj++){
				for (int iii = 0;iii<videon_leveys;iii++){
					if (iii < kolumneja && jjj < riveja){
						kuva[iii+jjj*videon_leveys]=vali[iii+jjj*kolumneja];	//R
						kuva[iii+jjj*videon_leveys+videon_leveys*videon_korkeus*1]=vali[iii+jjj*kolumneja+riveja*kolumneja];	//G
						kuva[iii+jjj*videon_leveys+videon_leveys*videon_korkeus*2]=vali[iii+jjj*kolumneja+2*riveja*kolumneja];	//B
					
					}else{
						kuva[iii+jjj*videon_leveys]=0;	//R
						kuva[iii+jjj*videon_leveys+videon_leveys*videon_korkeus*1]=0;	//G
						kuva[iii+jjj*videon_leveys+videon_leveys*videon_korkeus*2]=0;	//B
					}
				}
			}
			*/

			
			
			/*Three planes AV_PIX_FMT_GBRP*/
			
			for (int jjj = 0;jjj<videon_korkeus;jjj++){
				for (int iii = 0;iii<videon_leveys;iii++){
					kuva[2][iii+jjj*(videon_leveys+16)]=vali[iii+jjj*kolumneja];	//R = 2
					kuva[0][iii+jjj*(videon_leveys+16)]=vali[iii+jjj*kolumneja];	//G = 0
					kuva[1][iii+jjj*(videon_leveys+16)]=vali[iii+jjj*kolumneja];	//B = 1
					}
			}
			
			//Kuva valmis
			ulosVideo.write_video_frame(kuva);
			printf("Kuva %u\r",kuvia_naytetty);
		}

	} //seuraavalle kierrokselle
			
	//T‰ss‰ on valmista, nollaillaan kaikki
	ulosVideo.write_trailer();
	free(kuva);
	delete vali;
	fclose(tie);
	printf("Valmista tuli\n"); 
    return 0;
}

