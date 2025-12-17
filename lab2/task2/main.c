#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_GROUPS 50
#define MAX_PROCESSES 100
#define MAX_NAME_LENGTH 50
#define MAX_CMD_LENGTH 256

typedef struct {
    char name[MAX_NAME_LENGTH];
    pid_t processes[MAX_PROCESSES];
    int count;
} ProcessGroup;

ProcessGroup groups[MAX_GROUPS];
int group_count = 0;

void execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("Помилка виконання команди");
        output[0] = '\0';
        return;
    }
    
    size_t total = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL && total < output_size - 1) {
        size_t len = strlen(buffer);
        if (total + len < output_size - 1) {
            strcpy(output + total, buffer);
            total += len;
        }
    }
    output[total] = '\0';
    pclose(fp);
}

void list_all_processes() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Помилка створення дочірнього процесу");
        return;
    } else if (pid == 0) {
        execlp("ps", "ps", "-e", "-o", "pid,ppid,comm", NULL);
        perror("Помилка виконання ps");
        exit(1);
    } else {
        wait(NULL);
    }
}

int process_exists(pid_t pid) {
    char cmd[100];
    char output[1024];
    
    snprintf(cmd, sizeof(cmd), "ps -p %d -o pid=", pid);
    execute_command(cmd, output, sizeof(output));
    
    return (strlen(output) > 0);
}

void kill_process(pid_t pid) {
    pid_t child_pid = fork();
    
    if (child_pid < 0) {
        perror("Помилка створення дочірнього процесу");
        return;
    } else if (child_pid == 0) {
        char pid_str[20];
        snprintf(pid_str, sizeof(pid_str), "%d", pid);
        execlp("kill", "kill", "-9", pid_str, NULL);
        perror("Помилка виконання kill");
        exit(1);
    } else {
        int status;
        waitpid(child_pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Процес %d успішно зупинено\n", pid);
        } else {
            printf("Помилка при зупинці процесу %d\n", pid);
        }
    }
}

void create_group(const char *name) {
    if (group_count >= MAX_GROUPS) {
        printf("Досягнуто максимальну кількість груп\n");
        return;
    }
    
    for (int i = 0; i < group_count; i++) {
        if (strcmp(groups[i].name, name) == 0) {
            printf("Група '%s' вже існує\n", name);
            return;
        }
    }
    
    strncpy(groups[group_count].name, name, MAX_NAME_LENGTH - 1);
    groups[group_count].name[MAX_NAME_LENGTH - 1] = '\0';
    groups[group_count].count = 0;
    group_count++;
    
    printf("Групу '%s' створено успішно\n", name);
}

void add_process_to_group(const char *group_name, pid_t pid) {
    int group_index = -1;
    for (int i = 0; i < group_count; i++) {
        if (strcmp(groups[i].name, group_name) == 0) {
            group_index = i;
            break;
        }
    }
    
    if (group_index == -1) {
        printf("Групу '%s' не знайдено\n", group_name);
        return;
    }
    
    if (!process_exists(pid)) {
        printf("Процес %d не існує або недоступний\n", pid);
        return;
    }
    
    for (int i = 0; i < groups[group_index].count; i++) {
        if (groups[group_index].processes[i] == pid) {
            printf("Процес %d вже в групі '%s'\n", pid, group_name);
            return;
        }
    }
    
    if (groups[group_index].count >= MAX_PROCESSES) {
        printf("Група досягла максимальної кількості процесів\n");
        return;
    }
    
    groups[group_index].processes[groups[group_index].count++] = pid;
    printf("Процес %d додано до групи '%s'\n", pid, group_name);
}

void list_groups() {
    if (group_count == 0) {
        printf("Немає створених груп\n");
        return;
    }
    
    printf("\n=== СПИСОК ГРУП ===\n");
    for (int i = 0; i < group_count; i++) {
        printf("\nГрупа: %s\n", groups[i].name);
        printf("Кількість процесів: %d\n", groups[i].count);
        
        if (groups[i].count > 0) 
        {
            printf("Процеси: ");
            for (int j = 0; j < groups[i].count; j++) 
            {
                printf("%d", groups[i].processes[j]);
                if (j < groups[i].count - 1) printf(", ");
            }
            printf("\n");
        }
    }
    printf("\n");
}

void kill_group(const char *group_name) 
{
    int group_index = -1;
    for (int i = 0; i < group_count; i++) 
    {
        if (strcmp(groups[i].name, group_name) == 0) 
        {
            group_index = i;
            break;
        }
    }
    
    if (group_index == -1) 
    {
        printf("Групу '%s' не знайдено\n", group_name);
        return;
    }
    
    if (groups[group_index].count == 0) 
    {
        printf("Група '%s' не містить процесів\n", group_name);
        return;
    }
    
    printf("Зупинка процесів у групі '%s'...\n", group_name);
    for (int i = 0; i < groups[group_index].count; i++) 
    {
        pid_t pid = groups[group_index].processes[i];
        printf("Зупинка процесу %d...\n", pid);
        kill_process(pid);
    }
    
    groups[group_index].count = 0;
    printf("Всі процеси групи '%s' зупинено\n", group_name);
}

void delete_group(const char *group_name) 
{
    int group_index = -1;
    for (int i = 0; i < group_count; i++) 
    {
        if (strcmp(groups[i].name, group_name) == 0) 
        {
            group_index = i;
            break;
        }
    }
    
    if (group_index == -1) 
    {
        printf("Групу '%s' не знайдено\n", group_name);
        return;
    }
    
    for (int i = group_index; i < group_count - 1; i++)groups[i] = groups[i + 1];
  
    group_count--;
    
    printf("Групу '%s' видалено\n", group_name);
}

void show_help() {
    printf("\n=== МЕНЕДЖЕР ГРУП ПРОЦЕСІВ ===\n\n");
    printf("Доступні команди:\n");
    printf("  create <назва>          - Створити нову групу\n");
    printf("  add <група> <pid>       - Додати процес до групи\n");
    printf("  list                    - Показати всі групи та їх процеси\n");
    printf("  listproc                - Показати всі процеси в системі\n");
    printf("  kill <група>            - Зупинити всі процеси в групі\n");
    printf("  delete <група>          - Видалити групу (без зупинки процесів)\n");
    printf("  help                    - Показати цю довідку\n");
    printf("  exit                    - Вийти з програми\n\n");
}

int main() {
    char command[MAX_CMD_LENGTH];
    char action[50], arg1[MAX_NAME_LENGTH], arg2[50];
    
    printf("Введіть 'help' для отримання списку команд.\n\n");
    
    while (1) {
        printf("pgm> ");
        fflush(stdout);
        
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        command[strcspn(command, "\n")] = 0;
        
        if (strlen(command) == 0) {
            continue;
        }
        
        int args = sscanf(command, "%s %s %s", action, arg1, arg2);
        
        if (strcmp(action, "exit") == 0 || strcmp(action, "quit") == 0)
        {
            break;
        } 
        else if (strcmp(action, "help") == 0) 
        {
            show_help();
        } 
        else if (strcmp(action, "create") == 0) 
        {
            if (args < 2) 
            {
                printf("Використання: create <назва_групи>\n");
            } 
            else 
            {
                create_group(arg1);
            }
        } 
        else if (strcmp(action, "add") == 0) 
        {
            if (args < 3) 
            {
                printf("Використання: add <назва_групи> <pid>\n");
            } 
            else 
            {
                pid_t pid = atoi(arg2);
                ((pid <= 0) ? printf("Невірний PID\n") : add_process_to_group(arg1, pid));
            }
        } 
        else if (strcmp(action, "list") == 0) 
        {
            list_groups();
        } 
        else if (strcmp(action, "listproc") == 0) 
        {
            printf("\n=== СПИСОК ПРОЦЕСІВ ===\n");
            list_all_processes();
            printf("\n");
        } 
        else if (strcmp(action, "kill") == 0) 
        {
            if (args < 2) 
            {
                printf("Використання: kill <назва_групи>\n");
            } 
            else 
            {
                kill_group(arg1);
            }
        } 
        else if (strcmp(action, "delete") == 0) 
        {
            if (args < 2) 
            {
                printf("Використання: delete <назва_групи>\n");
            }
            else 
            {
                delete_group(arg1);
            }
        } 
        else 
        {
            printf("Невідома команда: %s\n", action);
            printf("Введіть 'help' для отримання списку команд.\n");
        }
    }
    
    return 0;
}
