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
// cd komutunu işler ve dizini değiştirir
int cd(char **args)
{
        if (args[1] == NULL)  // cd komutu için parametre eksikse hata mesajı verir
        {
                fprintf(stderr, ": expected argument to \"cd\"\n");
        }
        else
        {
                if (chdir(args[1]) != 0)  // chdir ile dizini değiştirir
                {
                        perror(":");
                }
        }
        return 1;
}

// Yardım komutunu işler ve kullanıcıya bilgi verir
int help(char **args)
{
        int i;
        printf("Isletim Sistemleri Odevi\n");
        printf("%s\n", "Built_in Komutlar");
        for (i = 0; i < builtin_sayisi(); i++)  // Yerleşik komutları listeler
        {
                printf("  %s\n", builtin_komutlar[i]);
        }
        printf("Diger programlar icin 'man' komudunu kullanarak yardım alabilirsiniz.\n");
        return 1;
}

// Programdan çıkışı gerçekleştirir
int quit(char **args)
{
        int status;
        while (!waitpid(-1, &status, WNOHANG)) {}  // Arka planda çalışan çocuk proseslerin bitmesini bekler
        exit(0);  // Programı sonlandırır
}

// Pipe komutunu işler ve iki komut arasında veri iletimi sağlar
void pipeFonk(char *args[], char *komut, char *parametre) {
    pid_t pid1, pid2;
    int pipefd[2];

    // Pipe oluşturma
    if (pipe(pipefd) == -1) {
        perror("Pipe oluşturulamadı");
        exit(EXIT_FAILURE);
    }

    // İlk komutun argümanları
    char *argv1[] = {args[0], args[1], NULL};

    // İkinci komutun argümanları
    char *argv2[] = {komut, parametre, NULL};

    // İlk çocuk proses: args komutunu çalıştırır
    pid1 = fork();
    if (pid1 == -1) {
        perror("Fork (1. proses) başarısız");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        // Pipe'ın yazma ucunu standart çıkışa yönlendir
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2 (1. proses) başarısız");
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);  // Pipe'ın okuma ucu kapatılır
        close(pipefd[1]);  // Pipe'ın yazma ucu kopyalandıktan sonra kapatılır

        // İlk komutu çalıştır
        execvp(argv1[0], argv1);
        perror("Birinci komut çalıştırılamadı");
        exit(EXIT_FAILURE);
    }

    // İkinci çocuk proses: argv2 komutunu çalıştırır
    pid2 = fork();
    if (pid2 == -1) {
        perror("Fork (2. proses) başarısız");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        // Pipe'ın okuma ucunu standart girişe yönlendir
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2 (2. proses) başarısız");
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]);  // Pipe'ın yazma ucu kapatılır
        close(pipefd[0]);  // Pipe'ın okuma ucu kopyalandıktan sonra kapatılır

        // İkinci komutu çalıştır
        execvp(argv2[0], argv2);
        perror("İkinci komut çalıştırılamadı");
        exit(EXIT_FAILURE);
    }

    // Pipe uçlarını kapat
    close(pipefd[0]);
    close(pipefd[1]);

    // Çocuk proseslerin tamamlanmasını bekle
    if (waitpid(pid1, NULL, 0) == -1) {
        perror("Birinci proses beklenirken hata oluştu");
    }

    if (waitpid(pid2, NULL, 0) == -1) {
        perror("İkinci proses beklenirken hata oluştu");
    }
}

// Komutları ayırır ve uygun işlemi gerçekleştirir
int komutYorumla(char *args[])
{
    char *commands[SPLIT];
    int commandCount = 0;

    // Noktalı virgül ile ayrılmış komutları ayır
    char *command = strtok(args[0], ";");
    while (command != NULL) {
        commands[commandCount++] = command;
        command = strtok(NULL, ";");
    }

    for (int i = 0; i < commandCount; i++) {
        char *tokens[LIMIT];
        int tokenCount = 0;

        // Komutları boşluklarla ayır
        tokens[tokenCount++] = strtok(commands[i], " \n\t");
        while ((tokens[tokenCount] = strtok(NULL, " \n\t")) != NULL) {
            tokenCount++;
        }

        if (tokens[0] == NULL) continue;  // Geçersiz komut

        // Yerleşik komutları kontrol et
        for (int m = 0; m < builtin_sayisi(); m++) {
            if (strcmp(tokens[0], builtin_komutlar[m]) == 0) {
                (*builtin_func[m])(tokens);
                goto next_command;
            }
        }

        // Operatörleri kontrol et (<, >, |, &)
        for (int j = 0; tokens[j] != NULL; j++) {
            if (strcmp(tokens[j], "<") == 0) {
                if (tokens[j + 1] == NULL) {
                    printf("Yeterli Arguman Yok\n");
                    break;
                }
                tokens[j] = NULL;  // '<' işaretini kaldır
                dosyaInput(tokens, tokens[j + 1]);
                goto next_command;
            } else if (strcmp(tokens[j], ">") == 0) {
                if (tokens[j + 1] == NULL) {
                    printf("Yeterli Arguman Yok\n");
                    break;
                }
                tokens[j] = NULL;  // '>' işaretini kaldır
                dosyaOutput(tokens, tokens[j + 1]);
                goto next_command;
            } else if (strcmp(tokens[j], "|") == 0) {
                if (tokens[j + 1] == NULL) {
                    printf("Pipe Hatası\n");
                    break;
                }
                tokens[j] = NULL;  // '|' işaretini kaldır
                pipeFonk(tokens, tokens[j + 1], tokens[j + 2]);
                goto next_command;
            } else if (strcmp(tokens[j], "&") == 0) {
                tokens[j] = NULL;  // '&' işaretini kaldır
                arkaPlandaCalistir(tokens);
                goto next_command;
            }
        }

        // Diğer komutları çalıştır
        calistir(tokens, 0);

    next_command:
        continue;
    }

    return 1;
}

// Dosyadan giriş almak için kullanılan fonksiyon
void dosyaInput(char args[], char inputFile)
{
	if (strcmp(args[0], "increment") == 0) {
    	increment(inputFile);
    	return;
	}

	pid_t pid;
	if (!(access (inputFile, F_OK) != -1))  // Dosya mevcut mu kontrol eder
	{	
		printf("Hata: %s adinda bir dosya bulunamadi\n", inputFile);
		return;
	}
	int err = -1;
	int dosya;
	if ((pid = fork()) == -1)  // Çocuk proses oluşturur
	{
		printf("Child olusturulamadi\n");
		return;
	}
	if (pid == 0)
	{
		dosya = open(inputFile, O_RDONLY, 0600);  // Dosyayı okuma modunda açar
		dup2(dosya, STDIN_FILENO);  // Dosya içeriğini stdin'e yönlendirir
		close(dosya);

		if (execvp(args[0], args) == err)  // Komutu çalıştırır
		{
			printf("err");
			kill(getpid(), SIGTERM);  // Hata durumunda proses sonlandırılır
		} 
	}
	waitpid(pid, NULL, 0);  // Çocuk prosesin bitmesini bekler
}

// Dosyaya çıkış yazmak için kullanılan fonksiyon
void dosyaOutput(char args[], char outputFile)
{
	pid_t pid;
	int err = -1;
	int dosya;
	
	if ((pid = fork()) == -1)  // Çocuk proses oluşturur
	{
		printf("Child olusturulamadi\n");
		return;
	}
	if (pid == 0)
	{
		dosya = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);  // Dosyayı yazma modunda açar
		dup2(dosya, STDOUT_FILENO);  // Standart çıkışı dosyaya yönlendirir
		close(dosya);
		if (execvp(args[0], args) == err)  // Komutu çalıştırır
		{
			printf("err");
			kill(getpid(), SIGTERM);  // Hata durumunda proses sonlandırılır
		} 
	}
	else {
		waitpid(pid, NULL, 0);  // Çocuk prosesin bitmesini bekler
	}
}
