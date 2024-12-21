#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "program.h"

// Kullanıcıya komut istemini (prompt) gösterir
void Prompt()
{
	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));  // Sistemin ana bilgisayar adını alır
	printf(KNRM "\n%s@%s:"KWHT KBLU "%s > " KWHT, getenv("LOGNAME"), hostn, getcwd(currentDirectory, 1024));  // Kullanıcı adı, host adı ve mevcut dizini gösterir
}

// Yerleşik komutların dizisini tanımlar
char *builtin_komutlar[] = {
	"cd",  // cd komutu
	"help",  // help komutu
	"quit"  // quit komutu
};

// Yerleşik komutların işlevlerini tutan bir dizi
int (builtin_func[])(char*) = {
	&cd,  // cd komutunun işlevi
	&help,  // help komutunun işlevi
	&quit  // quit komutunun işlevi
};

// Yerleşik komut sayısını döndüren fonksiyon
int builtin_sayisi()
{
	return sizeof(builtin_komutlar) / sizeof(char*);
}

// Bir dosyadaki sayıyı bir artıran fonksiyon
int increment(char *inputFile) {
    if (inputFile == NULL) {
        fprintf(stderr, "Dosya belirtilmedi.\n");
        return 1;
    }

    char *fileName = inputFile;
    FILE *file = fopen(fileName, "r");  // Dosyayı okuma modunda açar
    if (file == NULL) {
        perror("Dosya açılamadı");
        return 1;
    }

    int number;
    if (fscanf(file, "%d", &number) != 1) {  // Dosyadan bir sayı okur
        fprintf(stderr, "Dosyada geçerli bir sayı yok.\n");
        fclose(file);
        return 1;
    }
    fclose(file);

    number++;  // Sayıyı bir artır
    printf("%d", number);  // Yeni sayıyı ekrana yazdır
    return 1;
}