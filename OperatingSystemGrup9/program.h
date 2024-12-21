// Boolean değerler tanımlanıyor. TRUE 1'e, FALSE ise TRUE'nun tersi (0) olarak ayarlanıyor.
#define TRUE 1
#define FALSE !TRUE

// LIMIT, SATIR, ve SPLIT için sabit değerler tanımlanıyor.
#define LIMIT 256 
#define SATIR 1024 
#define SPLIT 256

// Konsol renkleri için ANSI escape kodları tanımlanıyor.
// Bu kodlar terminalde renkli metin göstermek için kullanılır.
#define KNRM  "\x1B[0m"  // Normal metin rengi
#define KRED  "\x1B[31m" // Kırmızı renk
#define KGRN  "\x1B[32m" // Yeşil renk
#define KYEL  "\x1B[33m" // Sarı renk
#define KBLU  "\x1B[34m" // Mavi renk
#define KMAG  "\x1B[35m" // Mor renk
#define KCYN  "\x1B[36m" // Camgöbeği renk
#define KWHT  "\x1B[37m" // Beyaz renk

// currentDirectory: Geçerli çalışma dizini için bir gösterici
// environ: Çevresel değişkenlerin bir dizisi (environment variables)
static char* currentDirectory;
extern char** environ;

// Sigara (child) prosesinin sonlanmasını işlemek için sinyal handler'ı
void sig_chld(int);

// Komutların yardımcı fonksiyonları
int cd(char **args); // cd komutunun işlevini yerine getiren fonksiyon
int help(char **args); // Yardım komutunun işlevi
int quit(char **args); // Çıkış komutunun işlevi
int increment(char *inputFile); // inputFile parametresi ile bir işlem yapacak fonksiyon
int calistir(char **args, int background); // args ile verilen komutu çalıştıracak fonksiyon, background parametresi arka planda çalışıp çalışmayacağını belirler
int arkaPlandaCalistir(char **args); // Arka planda bir komut çalıştırmak için kullanılan fonksiyon
void dosyaInput(char args[], char inputFile); // Dosyadan giriş yapmak için kullanılan fonksiyon
void dosyaOutput(char args[], char inputFile); // Dosyaya çıkış yapmak için kullanılan fonksiyon
void pipeFonk(char *args[], char *komut, char *parametre); // Pipe komutlarını işlemek için kullanılan fonksiyon
void Prompt(); // Kullanıcıdan giriş almak için komut istemi fonksiyonu
int builtin_sayisi(); // Sistem içinde yerleşik komut sayısını döndüren fonksiyon
int komutYorumla(char * args[]); // Komutu yorumlayıp, uygun işlem yapmak için kullanılan fonksiyon
