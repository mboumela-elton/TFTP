#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


#include "utils.c"
#include "constant.c"

int main()
{
    char command[256];

    printMessage(WELCOME_MESSAGE);

    while (1)
    {

        // Read the command entered by the user
        int bytes_read = readCommandLine(command);

        // Remove the newline character
        command[bytes_read - 1] = '\0';

        if (bytes_read <= 1)
        {
            printMessage(ENSEASH);
            continue;
        }

        // Check for the 'exit' command
        if (strcmp(command, EXIT) == 0)
        {
            break;
        }
        execSingleCommand(command);
    }

    printMessage(GOODBYE_MESSAGE);
    return 0;
}