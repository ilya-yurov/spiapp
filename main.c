#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0])) // конструкция вида "sizeof(a) / sizeof((a)[0])"
                                                   // вычисляет колличество элементов в массиве

static uint8_t mode = SPI_CS_HIGH; // переменная устанавливает режим spi
static uint8_t bits = 8;           // переменная устанавливает колличество бит
static uint32_t speed = 500000;    // переменная устанавливает максимальную скорость передачи данных
static void pabort(const char *s)  // функция pabort выполняет печать ошибки и выход из программы
{
	perror(s);
	abort();
}
static void transfer(int fd) // реализация функции полнодюплексной передачи данных
{
        int ret;
        uint8_t tx[] = {              // массив данных, которые необходимо передать
        0x48, 0x45, 0x4C, 0x4C, 0x4F, // ASCII chars "HELLO"
        0x20,                         // ASCII char " "
        0x57, 0x4F, 0x52, 0x4C, 0x44, // ASCII chars "WORLD"
        0x0A                          // ASCII char "\n"
        };
        uint8_t rx[ARRAY_SIZE(tx)] = {0, };  // массив данных, куда записываем данные
        struct spi_ioc_transfer tr = {       //реализация transfer-структуры
                .tx_buf = (unsigned long)tx, // указатель на буфер данных, которые необходимо передать
                .rx_buf = (unsigned long)rx, // указатель на буфер данных, куда нужно писать
                .len = ARRAY_SIZE(tx),       // размер буферов rx и tx в байтах
                .delay_usecs = 0,            // врем ожидания в микросекундах
                .speed_hz = 0,               // скорость передачи данных
                .bits_per_word = bits,       // колличество бит в слове
        };

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr); // передача сообщения через системный вызов ioctl(); &tr - указатель на массив данных
        if (ret < 1)
                pabort("can't send spi message");
}

int main()
{
	static const char *device = "/dev/spidev0.0";
        int ret = 0;
        int fd;
        fd = open(device, O_RDWR);
        if (fd < 0)
                pabort("can't open device");
        /*
         * spi mode
         */
        ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1)
                pabort("can't set spi mode");

        ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
        if (ret == -1)
                pabort("can't get spi mode");
        /*
         * бит в слове
         */
        ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits); // передает указатель на переменную, определяющую колличество бит для записи
        if (ret == -1)
                pabort("can't set bits per word");

        ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits); // передает указатель на переменную, определяющую колличество бит для чтения
        if (ret == -1)
                pabort("can't get bits per word");
	/*
         * максимальная скорость передачи hz
         */
        ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed); // указатель на переменную u32, которая определяет максимальную скорость передачи данных (запись)
        if (ret == -1)
                pabort("can't set max speed hz");

        ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed); // указатель на переменную u32, которая определяет максимальную скорость передачи данных (чтение)
        if (ret == -1)
                pabort("can't get max speed hz");

        printf("spi mode: %d\n", mode);
        printf("bits per word: %d\n", bits);
        printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

        transfer(fd);

        close(fd);

        return 0;
}
