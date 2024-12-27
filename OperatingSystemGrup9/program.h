/*
(GRUP NO : 9)
-	B221210063 Uğur Can Çelik  
-	B221210039 Alperen İsmet Esen
-	B211210091 Bedirhan Baltık
-	B221210006 İbrahim Talha Durna
-	B241210900 Furkan Türel
*/

#define TRUE 1
#define FALSE !TRUE
#define LIMIT 256 
#define SATIR 1024 
#define SPLIT 256


// Konsol renkleri için ANSI escape kodları tanımlanıyor.
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

static char* currentDirectory;
extern char** environ;

// Komutların yardımcı fonksiyonları
void sig_chld(int);
int cd(char **args);
int help(char **args);
int quit(char **args);
int increment(char *inputFile);
int calistir(char **args, int background);
int arkaPlandaCalistir(char **args);
void dosyaInput(char *args[],char* inputFile);
void dosyaOutput(char *args[],char* inputFile);
void pipeFonk(char *args[], char *komut, char *parametre);
void Prompt();
int builtin_sayisi();
int komutYorumla(char * args[]);
