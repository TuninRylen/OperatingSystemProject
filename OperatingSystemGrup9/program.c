/*
(GRUP NO : 9)
-	B221210063 Uğur Can Çelik  
-	B221210039 Alperen İsmet Esen
-	B211210091 Bedirhan Baltık
-	B221210006 İbrahim Talha Durna
-	B241210900 Furkan Türel
*/

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
	gethostname(hostn, sizeof(hostn));
	printf(KNRM "\n%s@%s:"KWHT KBLU "%s > " KWHT, getenv("LOGNAME"), hostn, getcwd(currentDirectory, 1024));
}

// Yerleşik komutların dizisini tanımlar
char *builtin_komutlar[]={
	"cd",
	"help",
	"quit"
};

// Yerleşik komutların işlevlerini tutan bir dizi
int (*builtin_func[])(char**) = {
	&cd,
	&help,
	&quit
};

// Yerleşik komut sayısını döndüren fonksiyon
int builtin_sayisi()
{
	return sizeof(builtin_komutlar)/sizeof(char *);
}

// Bir dosyadaki sayıyı bir artıran fonksiyon
int increment(char *inputFile) {
    if (inputFile == NULL) {
        fprintf(stderr, "Dosya belirtilmedi.\n");
        return 1;
    }

    char *fileName = inputFile;
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Dosya açılamadı");
        return 1;
    }

    int number;
    if (fscanf(file, "%d", &number) != 1) {
        fprintf(stderr, "Dosyada geçerli bir sayı yok.\n");
        fclose(file);
        return 1;
    }
    fclose(file);

    number++; // Sayıyı bir artır

    printf("%d", number);
    return 1;
}
// cd komutunu işler ve dizini değiştirir
int cd(char **args)
{
	if (args[1]==NULL)
	{
		fprintf(stderr,": expected argument to \"cd\"\n");
	}
	else
	{
		if (chdir(args[1])!=0)
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
	for (i=0;i<builtin_sayisi();i++)
	{
		printf("  %s\n",builtin_komutlar[i]);
	}
	printf("Diger programlar icin 'man' komudunu kullanarak yardım alabilirsiniz.\n");
	return 1;
}

// Programdan çıkışı gerçekleştirir
int quit(char **args)
{
	int status;
	while (!waitpid(-1,&status,WNOHANG)){}
	exit(0);
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

        close(pipefd[0]); // Pipe'ın okuma ucu kapatılır
        close(pipefd[1]); // Pipe'ın yazma ucu kopyalandıktan sonra kapatılır

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

        close(pipefd[1]); // Pipe'ın yazma ucu kapatılır
        close(pipefd[0]); // Pipe'ın okuma ucu kopyalandıktan sonra kapatılır

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

        if (tokens[0] == NULL) continue; // Geçersiz komut

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
                tokens[j] = NULL; // '<' işaretini kaldır
                dosyaInput(tokens, tokens[j + 1]);
                goto next_command;
            } else if (strcmp(tokens[j], ">") == 0) {
                if (tokens[j + 1] == NULL) {
                    printf("Yeterli Arguman Yok\n");
                    break;
                }
                tokens[j] = NULL; // '>' işaretini kaldır
                dosyaOutput(tokens, tokens[j + 1]);
                goto next_command;
            } else if (strcmp(tokens[j], "|") == 0) {
                if (tokens[j + 1] == NULL) {
                    printf("Pipe Hatası\n");
                    break;
                }
                tokens[j] = NULL; // '|' işaretini kaldır
                pipeFonk(tokens, tokens[j + 1], tokens[j + 2]);
                goto next_command;
            } else if (strcmp(tokens[j], "&") == 0) {
                tokens[j] = NULL; // '&' işaretini kaldır
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
void dosyaInput(char *args[],char* inputFile)
{
	if (strcmp(args[0], "increment") == 0) {
    	increment(inputFile);
    	return;
	}

	pid_t pid;
	if (!(access (inputFile,F_OK) != -1))
	{	
		printf("Hata: %s adinda bir dosya bulunamadi\n",inputFile);
		return;
	}
	int err=-1;
	int dosya;
	if((pid=fork()) == -1)
	{
		printf("Child olusturulamadi\n");
		return;
	}
	if (pid==0)
	{
		dosya=open(inputFile, O_RDONLY, 0600);
		dup2(dosya,STDIN_FILENO);
		close(dosya);

		if (execvp(args[0],args)==err)	
		{
			printf("err");
			kill(getpid(),SIGTERM);
		} 
	}
	waitpid(pid,NULL,0);
}

// Dosyaya çıkış yazmak için kullanılan fonksiyon
void dosyaOutput(char *args[],char* outputFile)
{
	pid_t pid;
	int err=-1;
	int dosya;
	
	if((pid=fork()) == -1)
	{
		printf("Child olusturulamadi\n");
		return;
	}
	if (pid==0)
	{
		dosya=open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
		dup2(dosya,STDOUT_FILENO);
		close(dosya);
		if (execvp(args[0],args)==err)	
		{
			printf("err");
			kill(getpid(),SIGTERM);
		} 
	}
	else {
		waitpid(pid,NULL,0);
	}
}

// Arka planda bir komut çalıştıran fonksiyon
int arkaPlandaCalistir(char **args)
{
	pid_t pid;
	int status;

	struct sigaction act;
	act.sa_handler = sig_chld;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NOCLDSTOP;

	if (sigaction(SIGCHLD,&act,NULL)<0)
	{
		fprintf(stderr,"sigaction failed\n");
		return 1;
	}

	pid=fork();
	if (pid == 0)
	{
		if (execvp(args[0],args) == -1)
		{
			printf("Komut bulunamadi");
			kill(getpid(),SIGTERM);
		}
	}
	else if (pid < 0)
	{
		perror("Hata:");		
	}
	else
	{
		printf("Proses PID:%d Degeriyle Olusturuldu",pid);
	}																																												
	return 1; 
}																													

// Komutları çalıştıran fonksiyon
int calistir(char **args,int arkaplandaCalisiyorMu)
{
	if (arkaplandaCalisiyorMu==0)
	{
		pid_t pid;
		int status;
		pid=fork();
		if (pid == 0)
		{
			if (execvp(args[0],args) == -1)
			{
				printf("Komut Bulunamadi");
				kill(getpid(),SIGTERM);
			}
		}
		else if (pid < 0)
		{
			perror("Hata:");
		}
		else
		{
			waitpid(pid,NULL,0);
		}
	}
	else
	{
		arkaPlandaCalistir(args);
	}
	return 1; 
}

// Child proseslerin bitişini işleyen sinyal işleyici
void sig_chld(int signo) 
{
    int status, child_val,chid;
	chid = waitpid(-1, &status, WNOHANG);
	if (chid > 0)
	{
		if (WIFEXITED(status))
	    {
	        child_val = WEXITSTATUS(status);
	        printf("[%d] retval : %d",chid, child_val);
	    }
	}
}

// Ana fonksiyon, programın çalışmasını başlatır
int main (int argc, char **argv, char **envp)
{
	char line[SATIR];
	char *tokens[LIMIT];
	int tokenSayisi;
	int status=1;
	environ=envp;

	system("clear");
	printf("\n************************* HOSGELDİNİZ *************************\n");

	while (status)
	{
    	Prompt();
    	memset(line, '\0', SATIR);
    	fgets(line, SATIR, stdin);
    	char *trimmedLine = strtok(line, "\n");
    	if (trimmedLine == NULL) continue;

    	char *commands[1];
    	commands[0] = trimmedLine;

    	komutYorumla(commands);
	}
}
