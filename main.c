#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define ENCRYPT_MODULE_FILE "/dev/manh0"

int hextostring(char *in, int len, char *out)
{
    int i;

    memset(out, 0, sizeof(out));
    for (i = 0; i < len; i++)
    {
        sprintf(out, "%s%02hhx", out, in[i]);
    }
    return 0;
}

int stringtohex(char *in, int len, char *out)
{
    int i;
    int converter[105];
    converter['0'] = 0;
    converter['1'] = 1;
    converter['2'] = 2;
    converter['3'] = 3;
    converter['4'] = 4;
    converter['5'] = 5;
    converter['6'] = 6;
    converter['7'] = 7;
    converter['8'] = 8;
    converter['9'] = 9;
    converter['a'] = 10;
    converter['b'] = 11;
    converter['c'] = 12;
    converter['d'] = 13;
    converter['e'] = 14;
    converter['f'] = 15;

    memset(out, 0, sizeof(out));

    for (i = 0; i < len; i = i + 2)
    {
        char byte = converter[in[i]] << 4 | converter[in[i + 1]];
        out[i / 2] = byte;
    }
}


int encrypt(char *file_path)
{
    char buffer[200], buffer_hex[200], send_buffer[400], encrypt_buffer_hex[400], encrypt_buffer[400],  encrypt_file_path[200];
    int aes_fd = open(ENCRYPT_MODULE_FILE, O_RDWR);
    int encrypt_fd, file_fd = open(file_path, O_RDWR);

    strcpy(encrypt_file_path, file_path);
    strcat(encrypt_file_path, ".enc");

    encrypt_fd = open(encrypt_file_path, O_RDWR | O_CREAT, S_IROTH | S_IWUSR | S_IRUSR);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(buffer_hex, 0, sizeof(buffer_hex));
        memset(send_buffer, 0, sizeof(send_buffer));
        memset(encrypt_buffer_hex, 0, sizeof(encrypt_buffer_hex));
        memset(encrypt_buffer, 0, sizeof(encrypt_buffer));

        if (read(file_fd, buffer, 16) == 0)
            break;    

        hextostring(buffer, 16, buffer_hex);
        sprintf(send_buffer, "encrypt\n%s", buffer_hex);

        write(aes_fd, send_buffer, strlen(send_buffer));
        read(aes_fd, encrypt_buffer_hex, sizeof(encrypt_buffer_hex));

        stringtohex(encrypt_buffer_hex, strlen(encrypt_buffer_hex), encrypt_buffer);
        write(encrypt_fd, encrypt_buffer, 16);
    }

    close(encrypt_fd);
    rename(encrypt_file_path, file_path);
    return 0;
}

int decrypt(char *file_path)
{
    char buffer[200], buffer_hex[200], send_buffer[400], decrypt_buffer_hex[400], decrypt_buffer[400],  decrypt_file_path[200];
    int aes_fd = open(ENCRYPT_MODULE_FILE, O_RDWR);
    int decrypt_fd, file_fd = open(file_path, O_RDWR);

    strcpy(decrypt_file_path, file_path);
    strcat(decrypt_file_path, ".de");

    decrypt_fd = open(decrypt_file_path, O_RDWR | O_CREAT, S_IROTH | S_IWUSR | S_IRUSR);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(buffer_hex, 0, sizeof(buffer_hex));
        memset(send_buffer, 0, sizeof(send_buffer));
        memset(decrypt_buffer_hex, 0, sizeof(decrypt_buffer_hex));
        memset(decrypt_buffer, 0, sizeof(decrypt_buffer));

        if (read(file_fd, buffer, 16) == 0)
            break;    

        hextostring(buffer, 16, buffer_hex);
        sprintf(send_buffer, "decrypt\n%s", buffer_hex);

        write(aes_fd, send_buffer, strlen(send_buffer));
        read(aes_fd, decrypt_buffer_hex, sizeof(decrypt_buffer_hex));

        stringtohex(decrypt_buffer_hex, strlen(decrypt_buffer_hex), decrypt_buffer);
        write(decrypt_fd, decrypt_buffer, 16);
    }

    close(decrypt_fd);
    rename(decrypt_file_path, file_path);
    return 0;
}

void checkparam(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("nhap qua it tham so\n");
        exit(-1);
    }

    if (strcmp(argv[1], "encrypt") != 0 && strcmp(argv[1], "decrypt") != 0)
    {
        printf("nhap sai thong tin lenh\n");
        exit(-1);
    }
}

int main(int argc, char *argv[]) 
{
    checkparam(argc, argv);

    if (strcmp(argv[1], "encrypt") == 0)
        encrypt(argv[2]);

    if (strcmp(argv[1], "decrypt") == 0)
        decrypt(argv[2]);

    return 0;
}
