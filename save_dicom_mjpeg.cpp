//Depends on independent jpeg group libjpeg (http://www.ijg.org/ tested with version 9 on 18th of April 2013)
//	COMPILE STATIC:
// g++ -o save_dicom_mjpeg.exe save_dicom_mjpeg.cpp -static -O3 -march=i686 -ljpeg
// to build (assuming libavformat, libavcodec, libavutil, and swscale are correctly installed on
// your system).

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector> //DICOMIA VARTEN
#include <math.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <vector>



class mjpegWriter{
	private:
		char* text;
		unsigned int numero;
		std::vector<long int> framet;
		std::vector<int> fsizes;
		long int movi,frameP[2],kohta[3]; //Tiedoston sijaintipointterit
		int colors;
		int quality;
		FILE* video;
		unsigned int height,width,frames;
	public:
		
		//Functions
		mjpegWriter(char* tnimi,unsigned int image_width,unsigned int image_height,int varit, int laatu); //Constructor
		void write_frame (unsigned char* kuva);
		void finalize_mjpeg(unsigned int ruutuja);
		void write_JPEG(unsigned char* data);
};

void mjpegWriter::finalize_mjpeg(unsigned int ruutuja){
	frames = ruutuja;
	long int nyk;
	nyk = ftell(video);
	fseek(video,movi-4,SEEK_SET);
	numero = nyk-movi;
	fwrite(&numero,4,1,video);		//Movi chunk size
	fseek(video,nyk,SEEK_SET);
	//Write index
	text = (char*) "idx1";		//idx1 chunk
	fwrite(text,4,1,video);		//idx1 chunk
	numero = frames*4*4;
	fwrite(&numero,4,1,video);	//index size!!
	for (int k = 0;k<frames;k++){
		text = (char*) "00dc";		//idx1 chunk
		fwrite(text,4,1,video);		//idx1 chunk
		numero = 16;
		fwrite(&numero,4,1,video);	//Key frame flag!!
		numero = framet[k];
		fwrite(&numero,4,1,video);	//offset from BEGINNING of (thus the +4) movi!!
		numero = fsizes[k];
		fwrite(&numero,4,1,video);		//chunk size
	}
	nyk = ftell(video);
	fseek(video,4,SEEK_SET);
	numero = nyk-8;
	fwrite(&numero,4,1,video);		//File size
	fseek(video,frameP[0],SEEK_SET);
	numero = frames;
	fwrite(&numero,4,1,video);		//frames1
	fseek(video,frameP[1],SEEK_SET);
	numero = frames;
	fwrite(&numero,4,1,video);		//frames2
	//Index written...
	//Close video
	fclose(video);
	
}


void mjpegWriter::write_JPEG(unsigned char* data)
{
   struct jpeg_compress_struct cinfo;
   struct jpeg_error_mgr jerr;
 
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
   jpeg_stdio_dest(&cinfo, video);
  cinfo.image_width = width; 	/* image width and height, in pixels */
  cinfo.image_height = height;
  cinfo.input_components = colors;		/* # of color components per pixel */
  if (colors ==3){
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  }
  if (colors ==1){
	cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
  }
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
  jpeg_start_compress(&cinfo, TRUE);
  row_stride = width * colors;	/* JSAMPLEs per row in image_buffer */
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = & data[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
}
//MJPEG FUCNTIONS
void mjpegWriter::write_frame (unsigned char* kuva){
	text = (char*) "00dc";		//MOVI chunk
	numero = width*height*3;
	kohta[0] =ftell(video);
	fwrite(text,4,1,video);		//Movi chunk
	fwrite(&numero,4,1,video);		//Movi chunk size place holder....
	kohta[1] =ftell(video);
	write_JPEG(kuva);
	if ((ftell(video)-kohta[0])%4 > 0){ //Zero padding
		numero = 0;
		fwrite(&numero,4-((ftell(video)-kohta[0])%4),1,video);
	}
	kohta[2] =ftell(video);
	fseek(video,kohta[0]+4,SEEK_SET);
	numero = kohta[2]-kohta[1];
	fwrite(&numero,4,1,video);		//Movi chunk size
	fseek(video,kohta[2],SEEK_SET);
	framet.push_back(kohta[0]-movi);
	fsizes.push_back(numero);
}

mjpegWriter::mjpegWriter(char* tnimi,unsigned int image_width,unsigned int image_height,int varit, int laatu){
	width = image_width;
	height = image_height;
	colors = varit;
	quality = laatu;
	video = fopen(tnimi,"wb");
	text = (char*) "RIFF";
	fwrite(text,4,1,video);
	numero = 12+192+width*height*3*frames+frames*8+8+frames*4*4+8;	//T‰h‰n file size -8
	fwrite(&numero,4,1,video);	//file size in bytes
	//AVI
	text = (char*) "AVI ";
	fwrite(text,4,1,video);
	text = (char*) "LIST";
	fwrite(text,4,1,video);
	numero = 192;	//T‰h‰n header size-12
	fwrite(&numero,4,1,video);	//list size in bytes
	text = (char*) "hdrl";		//main header
	fwrite(text,4,1,video);
	text = (char*) "avih";		//avi header
	fwrite(text,4,1,video);
	numero = 14*4;
	fwrite(&numero,4,1,video);	//avih size in bytes
	//AVI header data
	numero = 40000;		//usec per frame
	fwrite(&numero,4,1,video);
	numero = 25*3*width*height;		//bytes per sec
	fwrite(&numero,4,1,video);
	numero = 0;		//reserved
	fwrite(&numero,4,1,video);
	numero = 2320;		//flags (none required for raw)
	fwrite(&numero,4,1,video);
	frameP[0] = ftell(video);
	numero = frames;		//total frames (none required for raw)
	fwrite(&numero,4,1,video);
	numero = 0;		//initial frames (0 for non-interleaved)
	fwrite(&numero,4,1,video);
	numero = 1;		//No. of streams
	fwrite(&numero,4,1,video);
	numero = width*height*3;		//Buffer size
	fwrite(&numero,4,1,video);
	numero = width;		//Width
	fwrite(&numero,4,1,video);
	numero = height;		//Height
	fwrite(&numero,4,1,video);
	numero = 0;		//Reserved[4] set to zero
	fwrite(&numero,4,1,video);
	fwrite(&numero,4,1,video);
	fwrite(&numero,4,1,video);
	fwrite(&numero,4,1,video);
	//AVI header written
	text = (char*) "LIST";
	fwrite(text,4,1,video);
	numero =116;
	fwrite(&numero,4,1,video);	//list size in bytes
	text = (char*) "strl";		//Stream list
	fwrite(text,4,1,video);		//Stream list
	text = (char*) "strh";		//Stream header
	fwrite(text,4,1,video);		//VideoStream header
	numero = 56;
	fwrite(&numero,4,1,video);	//VideoStream header size in bytes
	//Stream header data
	text = (char*) "vids";		//Stream header
	fwrite(text,4,1,video);		//VideoStream header
	text = (char*) "MJPG";		//Stream handler
	fwrite(text,4,1,video);		//Stream handler
	numero = 0;
	fwrite(&numero,4,1,video);	//Flags
	fwrite(&numero,4,1,video);	//Priority none needed for raw..
	fwrite(&numero,4,1,video);	//Language needed for raw..
	//InitialFrames not used -> no audio...
	numero = 1;
	fwrite(&numero,4,1,video);	//dwScale
	numero = 25;
	fwrite(&numero,4,1,video);	//dwRate
	numero = 0;
	fwrite(&numero,4,1,video);	//dwStart
	frameP[1] = ftell(video);
	numero = frames;
	fwrite(&numero,4,1,video);	//dwLength
	numero = 0;
	fwrite(&numero,4,1,video);	//dwBuffer
	numero = -1;
	fwrite(&numero,4,1,video);	//dwQuality
	numero = 0;
	fwrite(&numero,4,1,video);	//dwSampleSize
	numero = 0;
	fwrite(&numero,4,1,video);	//rcFrame
	fwrite(&numero,4,1,video);	//rcFrame
	//Stream header data written
	text = (char*) "strf";		//Stream format
	fwrite(text,4,1,video);		//VideoStream format
	numero = 40;
	fwrite(&numero,4,1,video);	//VideoStream fomat size in bytes
	numero = 40;
	fwrite(&numero,4,1,video);	//Format header length = 40
	//format data
	numero = width;
	fwrite(&numero,4,1,video);	//width
	numero = height;
	fwrite(&numero,4,1,video);	//height
	unsigned short numerro = 1;
	fwrite(&numerro,2,1,video);	//planes = must be 1
	numerro = 0;
	fwrite(&numerro,2,1,video);	//bitCount (0 =jpeg tai png)
	//numero = 0;
	//fwrite(&numero,4,1,video);	//Compression
	text = (char*) "MJPG";		//Compression
	fwrite(text,4,1,video);		//compression
	numero = width*height*3;
	fwrite(&numero,4,1,video);	//size image in bytes
	numero = 0;
	fwrite(&numero,4,1,video);	//xpixels per meter
	numero = 0;
	fwrite(&numero,4,1,video);	//ypixels per meter
	numero = 0;
	fwrite(&numero,4,1,video);	//ClrUsed
	numero = 0;
	fwrite(&numero,4,1,video);	//ClrImportant
	//format data written...
	
	//DATA chunk!!
	text = (char*) "LIST";		//MOVI chunk
	fwrite(text,4,1,video);		//Movi chunk
	
	numero = width*height*3*frames+4+frames*8;
	fwrite(&numero,4,1,video);	//Data size!!
	
	movi = ftell(video);
	text = (char*) "movi";		//MOVI list name
	fwrite(text,4,1,video);		//Movi list name
}
//MJPEG FUCNTIONS loppuu

//using namespace std; //DICOMIA VARTEN

/**************************************************************/
int lleveys = 760,lkorkeus = 500;


int main(int argc, char **argv)
{
    const char *filename,*filein;
	unsigned short riveja, kolumneja,videon_korkeus,videon_leveys;
	videon_korkeus = 496;
	videon_leveys = 752;
    if (argc != 3) {
        printf("usage: save_dicom_mjpeg video.dcm output.avi\n");
        exit(1);
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
	
	
	unsigned char* kuva;
	
	kuva = new unsigned char[videon_leveys*videon_korkeus*3];
	memset(kuva,0,videon_leveys*videon_korkeus*3*sizeof(unsigned char));
	vali = new unsigned char[kolumneja*riveja*3];
	//Etsit‰‰n ensimm‰inen  item (FFFE,E000) 
		while (( hexat[0] != 15*16+14 || hexat[1] != 15*16+15 || hexat[2] != 0 || hexat[3] != 14*16) && ftell(tie) < loppu) { //7FE0,0010  PIXEL DATA
			fread(buffer,1,1,tie);
			hexat.erase(hexat.begin()); //Poistetaan ensimm‰inen
			hexat.push_back(*buffer);		//Lis‰t‰‰n viimeiseksi
		}
		
	//DICOM VALMISTELTU
	//MJPEG valmistelu
	mjpegWriter videoksi(argv[2],videon_leveys,videon_korkeus,3, 99);
	
	//Lue seuraava DICOM-kuva
	//T‰h‰n silmukka kuvan hakemiselle
	
	
		//Silmukka, jolla luetaan kuvat l‰pi
	unsigned int kuvia_naytetty = 0;
	while (ftell(tie) < loppu){// && kuvia_naytetty < 1){
		kuvia_naytetty++;
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
			for (int jjj = 0;jjj<videon_korkeus;jjj++){
				for (int iii = 0;iii<videon_leveys;iii++){
					kuva[iii*3+jjj*videon_leveys*3]=vali[iii+jjj*kolumneja];	//R
					kuva[iii*3+1+jjj*videon_leveys*3]=vali[iii+jjj*kolumneja+riveja*kolumneja];	//G
					kuva[iii*3+2+jjj*videon_leveys*3]=vali[iii+jjj*kolumneja+2*riveja*kolumneja];;	//B
					}
			}
			//Kuva valmis
			videoksi.write_frame(kuva);	//Lis‰t‰‰n kuva videoon
			printf("Kuva %u\r",kuvia_naytetty);
		}

	} //seuraavalle kierrokselle
			
	//T‰ss‰ on valmista, nollaillaan kaikki
	videoksi.finalize_mjpeg(kuvia_naytetty);
	delete kuva;
	delete vali;
	fclose(tie);
	printf("Valmista tuli\n");
 
    return 0;
}

