
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int test_empty_read(int fd)
{
    char receive[256];
    int ret;

    ret = read(fd, receive, 256);
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    if (strlen(receive))
    {
        return 1;
    }

    return 0;
}

int test_overwrite(int fd)
{
    char stringToSend[1024];
    int ret;

    // Send max number of x's
    for (int i = 0; i < 1024; i++)
    {
        stringToSend[i] = 'x';
    }
    ret = write(fd, stringToSend, strlen(stringToSend));
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    // Send max number of y's
    for (int i = 0; i < 1024; i++)
    {
        stringToSend[i] = 'y';
    }
    ret = write(fd, stringToSend, strlen(stringToSend));
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    // Read
    char receive[1024];
    ret = read(fd, receive, 1024); // Read the response from the LKM
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    for (int i = 0; i < 1024 - 1; i++)
    {
        if (receive[i] != 'x')
        {
            return 1;
        }
    }
    if (receive[1024 - 1] != '\0')
    {
        return 1;
    }
}

int test(char *description, int (*func)(int), int fd)
{
    printf("%-30s: ", description);
    if ((*func)(fd))
    {
        printf("Failed\n");
        return 1;
    }

    printf("───==≡≡ΣΣ((( つºل͜º)つ\n");
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: test <path to device>\n");
        exit(0);
    }
    char *devicepath = argv[1];

    int fd = open(devicepath, O_RDWR); // Open the device with read/write access
    if (fd < 0)
    {
        perror("Failed to open the device...");
        return errno;
    }

    test("Test reading from empty buffer", test_empty_read, fd);
    test("Overwriting buffer", test_empty_read, fd);

    return 0;
}
