
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int clearBuffer(int fd)
{
    char dump[1024];
    if (read(fd, dump, 1024) < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }

    return 0;
}

int test_simple_read(int fd)
{
    char receive[256];
    int ret;

    ret = read(fd, receive, 256);
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }

    return 0;
}

int test_simple_write(int fd)
{
    char *stringToSend = "This is a string to send";
    int ret;

    ret = write(fd, stringToSend, strlen(stringToSend));
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    return 0;
}

int test_empty_read(int fd)
{
    char receive[1024];
    int ret;

    // Read in nothing
    ret = read(fd, receive, 1024);
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }

    // Nothing should have been read
    if (strlen(receive) > 0)
    {
        return 1;
    }

    return 0;
}

int test_overwrite(int fd)
{
    char stringToSend[1024 + 100];
    int ret;

    // Fill the buffer with x's
    for (int i = 0; i < 1024 + 100; i++)
    {
        stringToSend[i] = 'x';
    }

    ret = write(fd, stringToSend, strlen(stringToSend));
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    // Despite trying to write 1124, only 1024 should have been successfully written.
    if (ret != 1024)
    {
        return 1;
    }

    return 0;
}

int test_write_and_read(int fd)
{
    char *stringToSend = "This is a string to send";
    char receive[256];
    int ret;

    // Write to the buffer
    ret = write(fd, stringToSend, strlen(stringToSend));
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    // Immediately read from the buffer
    ret = read(fd, receive, 256);
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }

    // The read in value should be identical to what was written
    if (strcmp(receive, stringToSend))
    {
        return 1;
    }

    return 0;
}

int test(char *description, int (*func)(int), int fd)
{

    if (clearBuffer(fd))
    {
        printf("Failed to Clear Buffer\n");
        return 1;
    }

    printf("%-40s: ", description);
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

    int fd = open(devicepath, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open the device...");
        return errno;
    }

    test("Reading gives no error", test_simple_read, fd);
    test("Writing gives no error", test_simple_write, fd);

    test("Read gets the same value that is written", test_write_and_read, fd);

    test("Reading empty buffer", test_empty_read, fd);
    test("Overwriting buffer", test_overwrite, fd);

    return 0;
}
