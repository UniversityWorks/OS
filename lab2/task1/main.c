#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(const char* message);

int main(void)
{
    char buffer[256];
    int pos_only_flag = 0;

    if (scanf("%255s", buffer) != 1) 
    {
        error("there are no arguments in stdin stream.\n");
    }

    if (strcmp("--positive-only", buffer) == 0) 
    {
        pos_only_flag = 1;
    }

    const char *res_message = ((pos_only_flag) ? "only positive numbers" : "all numbers");
    printf("%s: ", res_message);

    if (!pos_only_flag) 
    {
        printf("%s ", buffer);
    }

    while (scanf("%255s", buffer) == 1)
    {
        if (pos_only_flag && buffer[0] == '-') continue;
        
        printf("%s ", buffer);
    }
    printf("\n");

    return 0;
}

void error(const char* message)
{
    printf("stdin error: %s", message);
    exit(1);
}
