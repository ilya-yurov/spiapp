#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0])) // konstrukcia "sizeof(a) / sizeof((a)[0])
                                                   // vichislyaet kollichestvo elementov massiva

static uint8_t mode = SPI_CS_HIGH; // ustanavlivaem rezhim spi 0
static uint8_t bits = 8;          // ustanavlivaem kollichestvo (bit v slove)
static uint32_t speed = 500000;   // max skorost' peredachi
static void pabort(const char *s) // vyvodim oshibku i abort v otdel'nuy funkciyu
{
	perror(s);
	abort();
}
static void transfer(int fd) // funkciya polnodupleksnoy peredachi
{
        int ret;
        uint8_t tx[] = {              // eti dannye peredaem
        0x48, 0x45, 0x4C, 0x4C, 0x4F, // ASCII chars "HELLO"
        0x20,                         // ASCII char " "
        0x57, 0x4F, 0x52, 0x4C, 0x44, // ASCII chars "WORLD"
        0x0A                          // ASCII char "\n"
        };
        uint8_t rx[ARRAY_SIZE(tx)] = {0, };  // suda schityvaem
        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)tx, // ukazatel na bufer dannyh, kotorye nuzhno peredat' (libo NULL)
                .rx_buf = (unsigned long)rx, // ukazatel na bufer dannyh, kotorye nuzhno schitat' (libo NULL)
                .len = ARRAY_SIZE(tx),       // razmer buferov rx i tx v baytah
                .delay_usecs = 0,        // vremya ozhidaniya v microsekundah
                .speed_hz = 0,           // skorost' peredachi
                .bits_per_word = bits,       // kollichestvo bit na slovo
        };

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr); // peredacha. 1 v strukture. &tr - ukazatel na massiv structur
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
         * bits per word
         */
        ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits); //peredaet ukazatel na bait, opredelyaushiy k>
        if (ret == -1)
                pabort("can't set bits per word");

        ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits); //to je na chtenie
        if (ret == -1)
                pabort("can't get bits per word");
	/*
         * max speed hz
         */
        ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed); //ukazatel na peremennuy u32, kotoraya oprede>
        if (ret == -1)
                pabort("can't set max speed hz");

        ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed); //to je na chtenie
        if (ret == -1)
                pabort("can't get max speed hz");

        printf("spi mode: %d\n", mode);
        printf("bits per word: %d\n", bits);
        printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

        transfer(fd);

        close(fd);

        return 0;
}
