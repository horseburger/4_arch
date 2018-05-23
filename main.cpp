#include <iostream.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef unsigned long LONG;

BYTE far *Screen;

#define GRAPHICAL 0x13
#define TEXT 0x03

struct BITMAPFILEHEADER{
    UINT bfType; // Opis formatu pliku. Musi być 'BM'
    DWORD bfSize; // Rozmiar pliku BM w bajtach
    UINT bfReserved1; // Zarezerwowane. Musi być 0
    UINT bfReserved2; // Zarezerwowane. Musi być 0
    DWORD bfOffBits; // Przesunięcie w bajtach początku danych obrazu liczone od końca struktury
};

struct BITMAPINFOHEADER{
    DWORD biSize; // Rozmiar struktury BITMAPINFOHEADER
    LONG biWidth; // Szerokośc bitmapy w pikselach
    LONG biHeight; // Wyskość bitmapy w pikselach
    WORD biPlanes; // Ilość płaszczyzn. Musi być 1 
    WORD biBitCount; // Głębia kolorów w bitach na piksel
    DWORD biCompression; // Rodzaj kompresji (0 - brak)
    DWORD biSizeImage; // Rozmiar obrazu w bajtach. Uwaga, może być 0
    LONG biXPelsPerMeter; // Rozdzielczość pozioma w pikselach na metr
    LONG biYPelsPerMeter; // Rozdzielczość pionowa w pikselach na metr
    DWORD biClrUsed; // Ilośc używanych kolorów z palety
    DWORD biClrImportant; // Ilość kolorów z palety niezbędnych do wyświetlenia obrazu
};

struct RGBQUAD{
    BYTE Blue;
    BYTE Green;
    BYTE Red;
    BYTE rgbReserved;
};

void setMode(int mode){
    REGPACK regs;
    regs.r_ax = mode;
    intr(0x10, &regs);
}

void setDAC(unsigned char DAC, RGBQUAD colour){
    outportb(0x03C8, DAC);
    outportb(0x03C9, colour.Red >> 2);
    outportb(0x03C9, colour.Green >> 2);
    outportb(0x03C9, colour.Blue >> 2);
}

void toScreen(UINT size, char* picture){
    memcpy((void*) Screen, picture, size);
}

void Original(UINT size, RGBQUAD *colour, char* col){
    for (int i=0; i<256; i++) {
        setDAC(i, colour[i]);
    }

    toScreen(size, col);

}

void Negative(UINT size, RGBQUAD* colour, char* col){
    for (int i = 0; i < 256; i++) {
        RGBQUAD tmp = colour[i];
        tmp.Red = ~colour[i].Red;
        tmp.Green = ~colour[i].Green;
        tmp.Blue = ~colour[i].Blue;
        setDAC(i, tmp);
    }

    toScreen(size, col);
}

BYTE Brighten(int colour, int mod) {
    int var = colour + mod;
    if (var > 255) return 255;
    if (var < 0) return 0;
    return var;
}


void Brightness(UINT size, RGBQUAD* colour, char* col){
    for (int i = 0; i<256; i++) {
        RGBQUAD tmp = colour[i];
        tmp.Red = Brighten(tmp.Red,100);
        tmp.Green = Brighten(tmp.Green,100);
        tmp.Blue = Brighten(tmp.Blue,100);
        tmp.rgbReserved = 0;
        setDAC(i, tmp);
    }

    toScreen(size, col);
}

double getBrightness(RGBQUAD color) {
    return 0.2126 * color.Red + 0.7152 * color.Green + 0.0722 * color.Blue;
}

void Threshold (UINT size, RGBQUAD* colour, char* col){
    RGBQUAD black = {0,0,0,0};
    RGBQUAD white = {255,255,255,0};
    for ( int i = 0; i < 256; i++ ) {
        RGBQUAD tmp;
        if(getBrightness(colour[i]) >= 100) tmp = black;
        else tmp = white;
        setDAC(i, tmp);
    }

    toScreen(size, col);
}



void main(){

    Screen = (char far *) 0xA0000000L;

    FILE *file;
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    RGBQUAD colour[256];

    file = fopen("lena.bmp", "rb");

    fread(&bmfh, sizeof(BITMAPFILEHEADER), 1, file);
    fread(&bmih, sizeof(BITMAPINFOHEADER), 1, file);
    fread(&colour, sizeof(RGBQUAD)*256, 1, file);

    UINT size = bmih.biHeight*bmih.biWidth;

    char* picture = (char*) malloc(size);

    for (int i = bmih.biHeight - 1; i>=0; i--) {
         for (int j = 0; j<bmih.biWidth; j++) {
            picture[(i*bmih.biWidth)+j] = fgetc(file);             
        }
    }


    int wybor;
    do{
        cout << "1. Oryginal\n" << "2. Negatyw\n" << "3. Zmiana jasnosci\n" << "4. Progowanie\n" << "5. Exit\n" << "Twoj wybor: ";
        cin >> wybor;
        
        setMode(GRAPHICAL);

        switch(wybor){
           
            case 1:{
                Original(size, colour, picture);
                getchar();
                break;
            }
            case 2:{
                Negative(size, colour, picture);
                getchar();
                break;
            }
            case 3:{
                Brightness(size, colour, picture);
                getchar();
                break;
            }
            case 4:{
                Threshold(size, colour, picture);
                getchar();
                break;
            }
            case 5:{
                break;
            }
        }

        setMode(TEXT);

    }while(wybor!=5);

    fclose(file);
    
}