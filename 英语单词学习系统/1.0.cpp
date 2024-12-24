#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h> // ���ڴ���Ŀ¼

#ifdef _WIN32
#include <direct.h>   // ���ڴ���Ŀ¼
#include <windows.h>  // ����Ŀ¼����
#define MKDIR(path) _mkdir(path)
#else
#include <sys/types.h>
#include <dirent.h>    // �����ڷ�Windowsϵͳ
#define MKDIR(path) mkdir(path, 0755)
#endif

// �����û��ṹ��
typedef struct User {
    char username[50];
    char password[50];
    char name[100];
    int checkin_days;
    int total_words;
    int wrong_words;
    float correct_rate; // ��ȷ��
    char last_checkin_time[20]; // ��ʽ: YYYY-MM-DD HH:MM:SS
    int isAdmin; // �Ƿ�Ϊ����Ա
    struct User *next;
} User;

// ���嵥�ʽṹ��
typedef struct Word {
    char english[50];
    char chinese[100];
} Word;

// �������а���Ŀ�ṹ��
typedef struct RankEntry {
    char name[100];
    int score;
} RankEntry;

// ȫ��ͷָ��
User *head = NULL;

// ȫ�ֵ������鼰����
Word *wordList = NULL;
int wordCount = 0;

// ��������
void loadUsers();
void loadAdmins();
int saveAllUsers();
int saveAllAdmins();
int saveAll(); // ͬʱ�����û��͹���Ա
void registerUser(int isAdmin);
User* loginUser(int isAdmin);
void displayUserInfo(User *user);
void checkIn(User *user);
void reciteWords(User *user); // ���е��ʹ���
void dictation(User *user); // Ĭд����
void freeUsers();
void clearInputBuffer();
void appendCheckInLog(User *user);
int loadWords();
void freeWords();
void viewWrongWords();
void viewAlreadyLearnedWords();
void wrongWordTraining(User *user);
void searchWord();
void challengeMode(User *user);
void displayRanking();
void adminMenu(User *adminUser);
// ������������
void addToShengciBen(const char *english, const char *chinese);
void viewShengciBen(); // ������ʱ�����

// ������������
int compareRankEntries(const void *a, const void *b);
void trimNewline(char *str);

// �ļ�������
void browseUsers();
void deleteUser(); // ɾ���û�

// ����������
void manageWordLibraries();
void listWordLibraries();
void addWordLibrary();
void deleteWordLibrary();
void manageSingleWordLibrary();

// �������

int main() {
    int choice;
    User *currentUser = NULL;
    srand(time(NULL)); // ��ʼ�����������

    // �������Ŀ¼���������
    struct stat st = {0};
    if (stat("word_libraries", &st) == -1) {
        if (MKDIR("word_libraries") != 0) {
            perror("�޷����� word_libraries Ŀ¼");
            return 1;
        }
    }

    loadUsers();    // ������ͨ�û�
    loadAdmins();   // ���ع���Ա

    // ���ص���
    if (loadWords() != 0) {
        printf("���ص���ʧ�ܣ���ȷ������Ч�Ĵʿ��ļ� 'word_libraries\\cet.txt' ������ word_libraries Ŀ¼�С�\n");
    }

    while (1) {
        if (currentUser == NULL) {
            printf("\n=== ������ ===\n");
            printf("1. ע�����û�\n");
            printf("2. �û���¼\n");
            printf("3. ��̨����\n");
            printf("4. �˳�\n");
            printf("������ѡ��: ");

            if (scanf("%d", &choice) != 1) {
                clearInputBuffer();
                printf("��Ч�����룬���������֡�\n");
                continue;
            }
            clearInputBuffer(); // ������뻺����

            switch (choice) {
            case 1:
                registerUser(0); // ��ͨ�û�ע��
                break;
            case 2:
                currentUser = loginUser(0); // ��ͨ�û���¼
                break;
            case 3: {
                // ��̨����˵�
                printf("\n=== ��̨���� ===\n");
                printf("1. ����Աע��\n");
                printf("2. ����Ա��¼\n");
                printf("3. �������˵�\n");
                printf("������ѡ��: ");
                int adminChoice;
                if (scanf("%d", &adminChoice) != 1) {
                    clearInputBuffer();
                    printf("��Ч�����룬���������֡�\n");
                    continue;
                }
                clearInputBuffer(); // ������뻺����
                switch (adminChoice) {
                case 1:
                    registerUser(1); // ����Աע��
                    break;
                case 2: {
                    User *adminUser = loginUser(1); // ����Ա��¼
                    if (adminUser != NULL && adminUser->isAdmin) {
                        adminMenu(adminUser);
                        currentUser = NULL; // �˳�����Ա�˵���ǳ�
                    }
                    break;
                }
                case 3:
                    // �������˵�
                    break;
                default:
                    printf("��Ч��ѡ�����������롣\n");
                }
                break;
            }
            case 4:
                printf("�˳�����\n");
                freeUsers();
                freeWords();
                exit(0);
            default:
                printf("��Ч��ѡ�����������롣\n");
            }
        } else {
            if (currentUser->isAdmin) {
                // ����Ա�Ѿ��� adminMenu �д���������⴦��
                currentUser = NULL;
                continue;
            }

            // ��ͨ�û��˵�
            printf("\n=== �û��˵� ===\n");
            printf("1. ��ʾ������Ϣ\n");
            printf("2. ��\n");
            printf("3. ���е���\n");
            printf("4. ���Ȿ\n");
            printf("5. �鿴��ѧ���ĵ���\n");
            printf("6. ���⸴ϰ\n");
            printf("7. ��������\n");
            printf("8. Ȥζ��ս\n");
            printf("9. ���а�\n");
            printf("10. ���ʱ�\n"); 
            printf("11. ����������\n");
            printf("12. �˳�����\n");
            printf("������ѡ��: ");

            if (scanf("%d", &choice) != 1) {
                clearInputBuffer();
                printf("��Ч�����룬���������֡�\n");
                continue;
            }
            clearInputBuffer(); // ������뻺����

            switch (choice) {
            case 1:
                displayUserInfo(currentUser);
                break;
            case 2:
                checkIn(currentUser);
                break;
            case 3:
                reciteWords(currentUser);
                break;
            case 4:
                viewWrongWords();
                break;
            case 5:
                viewAlreadyLearnedWords();
                break;
            case 6:
                wrongWordTraining(currentUser);
                break;
            case 7:
                searchWord();
                break;
            case 8:
                challengeMode(currentUser);
                break;
            case 9:
                displayRanking();
                break;
            case 10:
                viewShengciBen(); // ����������ʱ�����
                break;
            case 11:
                printf("�ǳ��ɹ���\n");
                currentUser = NULL;
                break;
            case 12:
                printf("�˳�����\n");
                freeUsers();
                freeWords();
                exit(0);
            default:
                printf("��Ч��ѡ�����������롣\n");
            }
        }
    }

    return 0;
}


// ������뻺����
void clearInputBuffer() {
    while (getchar() != '\n');
}

// ȥ���ַ���ĩβ�Ļ��з�
void trimNewline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

// ���ļ�������ͨ�û�������
void loadUsers() {
    FILE *file = fopen("users.txt", "r");
    if (file == NULL) {
        // ����ļ������ڣ������ǵ�һ�����У�����Ҫ����
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        User *newUser = (User *)malloc(sizeof(User));
        if (newUser == NULL) {
            perror("�ڴ����ʧ��");
            fclose(file);
            return;
        }

        trimNewline(line);

        // ʹ�ö��ŷָ��ֶ�
        char *token = strtok(line, ",");
        if (token != NULL) {
            strcpy(newUser->username, token);
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            strcpy(newUser->password, token);
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            strcpy(newUser->name, token);
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->checkin_days = atoi(token);
        } else {
            newUser->checkin_days = 0;
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->total_words = atoi(token);
        } else {
            newUser->total_words = 0;
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->wrong_words = atoi(token);
        } else {
            newUser->wrong_words = 0;
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            strcpy(newUser->last_checkin_time, token);
        } else {
            strcpy(newUser->last_checkin_time, "δ��");
        }

        // ���Զ�ȡ correct_rate��������������ʼ��Ϊ 0.0
        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->correct_rate = atof(token);
        } else {
            newUser->correct_rate = 0.0f;
        }

        newUser->isAdmin = 0; // ��ͨ�û�

        newUser->next = head;
        head = newUser;
    }

    fclose(file);
}

// ���ļ����ع���Ա������
void loadAdmins() {
    FILE *file = fopen("monitor.txt", "r");
    if (file == NULL) {
        // ����ļ������ڣ������ǵ�һ�����У�����Ҫ����
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        User *newUser = (User *)malloc(sizeof(User));
        if (newUser == NULL) {
            perror("�ڴ����ʧ��");
            fclose(file);
            return;
        }

        trimNewline(line);

        // ʹ�ö��ŷָ��ֶ�
        char *token = strtok(line, ",");
        if (token != NULL) {
            strcpy(newUser->username, token);
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            strcpy(newUser->password, token);
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            strcpy(newUser->name, token);
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->checkin_days = atoi(token);
        } else {
            newUser->checkin_days = 0;
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->total_words = atoi(token);
        } else {
            newUser->total_words = 0;
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->wrong_words = atoi(token);
        } else {
            newUser->wrong_words = 0;
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            strcpy(newUser->last_checkin_time, token);
        } else {
            strcpy(newUser->last_checkin_time, "δ��");
        }

        // ���Զ�ȡ correct_rate��������������ʼ��Ϊ 0.0
        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->correct_rate = atof(token);
        } else {
            newUser->correct_rate = 0.0f;
        }

        newUser->isAdmin = 1; // ����Ա

        newUser->next = head;
        head = newUser;
    }

    fclose(file);
}

// ��������ͨ�û����浽 users.txt
int saveAllUsers() {
    FILE *file = fopen("users.txt", "w");
    if (file == NULL) {
        perror("�޷��� users.txt �ļ����б���");
        return -1;
    }

    User *current = head;
    while (current != NULL) {
        if (current->isAdmin == 0) { // ��������ͨ�û�
            fprintf(file, "%s,%s,%s,%d,%d,%d,%.2f,%s\n",
                current->username,
                current->password,
                current->name,
                current->checkin_days,
                current->total_words,
                current->wrong_words,
                current->correct_rate,
                current->last_checkin_time);
        }
        current = current->next;
    }

    fclose(file);
    return 0;
}

// �����й���Ա���浽 monitor.txt
int saveAllAdmins() {
    FILE *file = fopen("monitor.txt", "w");
    if (file == NULL) {
        perror("�޷��� monitor.txt �ļ����б���");
        return -1;
    }

    User *current = head;
    while (current != NULL) {
        if (current->isAdmin == 1) { // ���������Ա
            fprintf(file, "%s,%s,%s,%d,%d,%d,%.2f,%s\n",
                current->username,
                current->password,
                current->name,
                current->checkin_days,
                current->total_words,
                current->wrong_words,
                current->correct_rate,
                current->last_checkin_time);
        }
        current = current->next;
    }

    fclose(file);
    return 0;
}

// ͬʱ���������û��͹���Ա
int saveAll() {
    if (saveAllUsers() != 0) {
        return -1;
    }
    if (saveAllAdmins() != 0) {
        return -1;
    }
    return 0;
}

// ע�����û������Ա
void registerUser(int isAdmin) {
    User *newUser = (User *)malloc(sizeof(User));
    if (newUser == NULL) {
        perror("�ڴ����ʧ��");
        return;
    }

    // ����Ҫ������뻺��������Ϊ���ú���ǰ�Ѿ����

    if (isAdmin) {
        printf("\n=== ����Աע�� ===\n");
    } else {
        printf("\n=== �û�ע�� ===\n");
    }

    // �����û���
    printf("�������û���: ");
    fgets(newUser->username, sizeof(newUser->username), stdin);
    trimNewline(newUser->username);

    // ����û����Ƿ��Ѵ���
    User *current = head;
    while (current != NULL) {
        if (strcmp(current->username, newUser->username) == 0) {
            printf("�û����Ѵ��ڣ���ѡ�������û�����\n");
            free(newUser);
            return;
        }
        current = current->next;
    }

    // ��������
    printf("����������: ");
    fgets(newUser->password, sizeof(newUser->password), stdin);
    trimNewline(newUser->password);

    // ��������
    printf("����������: ");
    fgets(newUser->name, sizeof(newUser->name), stdin);
    trimNewline(newUser->name);

    // ��ʼ�����û��������ֶ�
    newUser->checkin_days = 0;
    newUser->total_words = 0;
    newUser->wrong_words = 0;
    newUser->correct_rate = 0.0f; // ��ʼ����ȷ��
    strcpy(newUser->last_checkin_time, "δ��");
    newUser->isAdmin = isAdmin;

    // ��ӵ�����ͷ��
    newUser->next = head;
    head = newUser;

    // �����û�����Ӧ���ļ�
    if (isAdmin) {
        if (saveAllAdmins() != 0) {
            printf("ע��ʧ�ܣ��޷��������Ա���ݡ�\n");
            return;
        }
        printf("����Աע��ɹ���\n");
    } else {
        if (saveAllUsers() != 0) {
            printf("ע��ʧ�ܣ��޷������û����ݡ�\n");
            return;
        }
        printf("ע��ɹ���\n");
    }
}

// ��¼����
User* loginUser(int isAdmin) {
    char username[50];
    char password[50];

    if (isAdmin) {
        printf("\n=== ����Ա��¼ ===\n");
    } else {
        printf("\n=== �û���¼ ===\n");
    }

    // �����û���
    printf("�������û���: ");
    fgets(username, sizeof(username), stdin);
    trimNewline(username);

    // ��������
    printf("����������: ");
    fgets(password, sizeof(password), stdin);
    trimNewline(password);

    // �������в����û�
    User *current = head;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 &&
            strcmp(current->password, password) == 0 &&
            current->isAdmin == isAdmin) {
            if (isAdmin) {
                printf("����Ա��¼�ɹ�����ӭ, %s��\n", current->name);
            } else {
                printf("��¼�ɹ�����ӭ, %s��\n", current->name);
            }
            return current;
        }
        current = current->next;
    }

    if (isAdmin) {
        printf("����Ա�û������������\n");
    } else {
        printf("�û������������\n");
    }
    return NULL;
}

// ��ʾ������Ϣ
void displayUserInfo(User *user) {
    printf("\n=== ������Ϣ ===\n");
    printf("����: %s\n", user->name);
    printf("�û���: %s\n", user->username);
    printf("������: %d\n", user->checkin_days);
    printf("�ܼƱ��˶��ٵ���: %d\n", user->total_words);
    printf("������: %d\n", user->wrong_words);
    printf("��ȷ��: %.2f%%\n", user->correct_rate); // ��ʾ��ȷ��
    printf("��һ�δ�ʱ��: %s\n", user->last_checkin_time);
}

// �򿨹���
void checkIn(User *user) {
    // ��ȡ��ǰʱ��
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char current_date[11]; // ��ʽ: YYYY-MM-DD
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", t);

    // ��ȡ��һ�δ򿨵�����
    char last_date[11];
    if (strcmp(user->last_checkin_time, "δ��") != 0) {
        strncpy(last_date, user->last_checkin_time, 10);
        last_date[10] = '\0';
    } else {
        strcpy(last_date, "0000-00-00");
    }

    // �Ƚϵ�ǰ���ں���һ�δ�����
    if (strcmp(current_date, last_date) == 0) {
        printf("�����Ѿ�������ˣ�������������\n");
        return;
    }

    // �����û���Ϣ
    user->checkin_days += 1;

    // ���´�ʱ��
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);
    strcpy(user->last_checkin_time, time_str);

    // ͳ����ѧ���ĵ�����
    int already_count = 0;
    FILE *alreadyFile = fopen("already.txt", "r");
    if (alreadyFile != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), alreadyFile)) {
            if (strlen(line) > 1) { // �ų�����
                already_count++;
            }
        }
        fclose(alreadyFile);
    }

    // ͳ�ƴ���ĵ�����
    int wrong_count = 0;
    FILE *wrongFile = fopen("wrong.txt", "r");
    if (wrongFile != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), wrongFile)) {
            if (strlen(line) > 1) { // �ų�����
                wrong_count++;
            }
        }
        fclose(wrongFile);
    }

    // ������ѧϰ������
    int total_learned = already_count + wrong_count;
    user->total_words = total_learned;

    // ������ȷ��
    if (total_learned > 0) {
        user->correct_rate = ((float)(total_learned - wrong_count) / total_learned) * 100.0f;
    } else {
        user->correct_rate = 0.0f;
    }

    // ���������û��͹���Ա���ļ�
    if (saveAll() != 0) {
        printf("��ʧ�ܣ��޷������û����ݡ�\n");
        return;
    }

    // ��¼����־
    appendCheckInLog(user);

    printf("�򿨳ɹ���\n");
    printf("��ѧϰ������: %d\n", user->total_words);
    printf("��ȷ��: %.2f%%\n", user->correct_rate);
}

// ������Ϣ׷�ӵ�����־�ļ�
void appendCheckInLog(User *user) {
    FILE *file = fopen("checkin.txt", "a");
    if (file == NULL) {
        perror("�޷��򿪴���־�ļ�");
        return;
    }

    fprintf(file, "����: %s, �û���: %s, ������: %d, ��һ�δ�ʱ��: %s\n",
        user->name,
        user->username,
        user->checkin_days,
        user->last_checkin_time);
    fclose(file);
}

// �ͷ������ڴ�
void freeUsers() {
    User *current = head;
    while (current != NULL) {
        User *temp = current;
        current = current->next;
        free(temp);
    }
}

// ��ָ���ʿ��ļ����ص��ʵ����飨�޸ĺ�� loadWords��
int loadWords() {
#ifdef _WIN32
    const char *wordPath = "word_libraries\\cet.txt";
#else
    const char *wordPath = "word_libraries/cet.txt";
#endif

    FILE *file = fopen(wordPath, "r"); // ���� 'word_libraries/cet.txt' �ļ�
    if (file == NULL) {
        perror("�޷��� 'word_libraries/cet.txt' �ļ�");
        return -1;
    }

    // ͳ�Ƶ�������
    wordCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) { // �ų�����
            wordCount++;
        }
    }

    if (wordCount == 0) {
        printf("'word_libraries\\cet.txt' �ļ���û����Ч�ĵ��ʡ�\n");
        fclose(file);
        return -1;
    }

    // �����ڴ�
    wordList = (Word *)malloc(sizeof(Word) * wordCount);
    if (wordList == NULL) {
        perror("�ڴ����ʧ��");
        fclose(file);
        return -1;
    }

    // ��ȡ���е���
    rewind(file);
    int index = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) <= 1) continue; // �ų�����

        trimNewline(line);

        // ʹ�õ�һ���ո�ָ�Ӣ�ĺ�����
        char *space = strchr(line, ' ');
        if (space == NULL) {
            strcpy(wordList[index].english, line);
            strcpy(wordList[index].chinese, "δ֪");
        } else {
            *space = '\0';
            strcpy(wordList[index].english, line);
            strcpy(wordList[index].chinese, space + 1);
        }

        index++;
    }

    fclose(file);
    return 0;
}

// �ͷŵ����ڴ�
void freeWords() {
    if (wordList != NULL) {
        free(wordList);
        wordList = NULL;
    }
}

// ������ʱ�����
void viewShengciBen() {
    FILE *file = fopen("shengciben.txt", "r");
    if (file == NULL) {
        printf("���ʱ��ļ������ڻ�Ϊ�ա�\n");
        return;
    }

    char line[256];
    int count = 0;
    printf("\n=== ���ʱ� ===\n");
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) > 0) {
            printf("%d. %s\n", ++count, line);
        }
    }

    if (count == 0) {
        printf("���ʱ�Ϊ�ա�\n");
    }

    fclose(file);
}

// ���е��ʹ��ܣ���Ϊ����ģʽ
void reciteWords(User *user) {
    if (wordCount == 0) {
        printf("�ʿ�Ϊ�գ��޷����б��С�\n");
        return;
    }

    printf("\n=== ���е��� ===\n");
    printf("��ѡ����ģʽ:\n");
    printf("1. ����Ӣ��д����\n");
    printf("2. ��������дӢ��\n");
    printf("������ѡ��: ");

    int mode;
    if (scanf("%d", &mode) != 1) {
        clearInputBuffer();
        printf("��Ч�����룬���������֡�\n");
        return;
    }
    clearInputBuffer(); // ������뻺����

    if (mode < 1 || mode > 2) {
        printf("��Ч��ѡ��������ѡ��\n");
        return;
    }

    // ����һ��������������ѡ�񵥴�
    int *indices = (int *)malloc(sizeof(int) * wordCount);
    if (indices == NULL) {
        perror("�ڴ����ʧ��");
        return;
    }

    for (int i = 0; i < wordCount; i++) {
        indices[i] = i;
    }

    // ����ģʽ�����Ƿ����˳������ͳһ���ң�
    for (int i = wordCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    printf("\n=== ���п�ʼ ===\n");

    int totalQuestions = 10; // ���ñ��е�������
    if (wordCount < 10) {
        totalQuestions = wordCount;
    }

    for (int i = 0; i < totalQuestions; i++) {
        int idx = indices[i];
        char question[100];
        char correctAnswer[100];
        char prompt[100];

        if (mode == 1) {
            // ����Ӣ��д����
            strcpy(question, wordList[idx].english);
            strcpy(correctAnswer, wordList[idx].chinese);
            snprintf(prompt, sizeof(prompt), "��д������Ӣ�ĵ��ʵ�����:\n%s: ", question);
        } else {
            // ��������дӢ��
            strcpy(question, wordList[idx].chinese);
            strcpy(correctAnswer, wordList[idx].english);
            snprintf(prompt, sizeof(prompt), "��д���������ĵ��ʵ�Ӣ��:\n%s: ", question);
        }

        printf("%s", prompt);

        char answer[100];
        fgets(answer, sizeof(answer), stdin);
        trimNewline(answer);

        // ���𰸣����Դ�Сд��
        if (strcasecmp(answer, correctAnswer) == 0) {
            printf("��ȷ��\n");
            // ��¼��ѧ���ĵ���
            FILE *alreadyFile = fopen("already.txt", "a");
            if (alreadyFile != NULL) {
                fprintf(alreadyFile, "%s %s\n", wordList[idx].english, wordList[idx].chinese);
                fclose(alreadyFile);
            } else {
                perror("�޷���already.txt");
            }
        } else {
            printf("������ȷ����: %s\n", correctAnswer);
            // ���Ӵ��󵥴ʼ���
            user->wrong_words += 1;
            // ��¼���󵥴�
            FILE *wrongFile = fopen("wrong.txt", "a");
            if (wrongFile != NULL) {
                fprintf(wrongFile, "%s %s\n", wordList[idx].english, wordList[idx].chinese);
                fclose(wrongFile);
            } else {
                perror("�޷���wrong.txt");
            }
        }

        // ѯ�ʲ���ѡ��
        printf("��ѡ�����:\n");
        printf("1. ��һ��\n");
        printf("2. �������ʱ�\n");
        printf("3. �˳�����\n");
        printf("������ѡ��: ");

        int op;
        if (scanf("%d", &op) != 1) {
            clearInputBuffer();
            printf("��Ч�����룬Ĭ�ϼ�����һ�⡣\n");
            continue;
        }
        clearInputBuffer(); // ������뻺����

        switch (op) {
        case 1:
            // ������һ��
            break;
        case 2:
            addToShengciBen(wordList[idx].english, wordList[idx].chinese);
            printf("�ѽ����� '%s' ��ӵ����ʱ���\n", wordList[idx].english);
            // ������ѡ����һ������
            while (1) {
                printf("��ѡ�����:\n");
                printf("1. ��һ��\n");
                printf("2. �˳�����\n");
                printf("������ѡ��: ");
                int subOp;
                if (scanf("%d", &subOp) != 1) {
                    clearInputBuffer();
                    printf("��Ч�����룬���������֡�\n");
                    continue;
                }
                clearInputBuffer(); // ������뻺����

                if (subOp == 1) {
                    // ������һ��
                    break;
                } else if (subOp == 2) {
                    printf("���˳����С�\n");
                    free(indices);
                    // �����û��͹���Ա����
                    if (saveAll() != 0) {
                        printf("�����û�����ʧ�ܡ�\n");
                    } else {
                        printf("�û������Ѹ��¡�\n");
                    }
                    return;
                } else {
                    printf("��Ч��ѡ�����������롣\n");
                }
            }
            break;
        case 3:
            printf("���˳����С�\n");
            free(indices);
            // �����û��͹���Ա����
            if (saveAll() != 0) {
                printf("�����û�����ʧ�ܡ�\n");
            } else {
                printf("�û������Ѹ��¡�\n");
            }
            return; // �˳�����
        default:
            printf("��Ч��ѡ�񣬼�����һ�⡣\n");
        }
    }

    free(indices);

    // �����û��͹���Ա����
    if (saveAll() != 0) {
        printf("�����û�����ʧ�ܡ�\n");
    } else {
        printf("������ɣ��û������Ѹ��¡�\n");
    }
}

// Ĭд����
void dictation(User *user) {
    if (wordCount < 20) {
        printf("������������20������ǰ������: %d\n", wordCount);
        return;
    }

    // ����һ�����������������ѡ��
    int *indices = (int *)malloc(sizeof(int) * wordCount);
    if (indices == NULL) {
        perror("�ڴ����ʧ��");
        return;
    }

    for (int i = 0; i < wordCount; i++) {
        indices[i] = i;
    }

    // ���������������
    for (int i = wordCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // ����
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // ѡ��ǰ20������
    printf("\n=== Ĭд��ʼ ===\n");
    printf("�����뵥�ʵ�Ӣ�ķ��룬���� 'exit' �� 'quit' ������ǰ����Ĭд��\n");
    for (int i = 0; i < 20; i++) {
        int idx = indices[i];
        printf("��д���������ĵ��ʵ�Ӣ��:\n%s: ", wordList[idx].chinese);

        char answer[50];
        fgets(answer, sizeof(answer), stdin);
        trimNewline(answer);

        // ����Ƿ���������ֹ����
        if (strcasecmp(answer, "exit") == 0 || strcasecmp(answer, "quit") == 0) {
            printf("����ǰ����Ĭд��\n");
            break;
        }

        // ���𰸣����Դ�Сд��
        if (strcasecmp(answer, wordList[idx].english) == 0) {
            printf("��ȷ��\n");
            // ׷�ӵ� already.txt
            FILE *alreadyFile = fopen("already.txt", "a");
            if (alreadyFile != NULL) {
                fprintf(alreadyFile, "%s %s\n", wordList[idx].english, wordList[idx].chinese);
                fclose(alreadyFile);
            } else {
                perror("�޷���already.txt");
            }
        } else {
            printf("������ȷ����: %s\n", wordList[idx].english);
            // ���Ӵ��󵥴ʼ���
            user->wrong_words += 1;
            // ׷�ӵ� wrong.txt
            FILE *wrongFile = fopen("wrong.txt", "a");
            if (wrongFile != NULL) {
                fprintf(wrongFile, "%s %s\n", wordList[idx].english, wordList[idx].chinese);
                fclose(wrongFile);
            } else {
                perror("�޷���wrong.txt");
            }
        }

        // �������֣�ѯ���Ƿ񽫵��ʼ������ʱ�
        printf("�Ƿ񽫸õ��ʼ������ʱ���(y/n): ");
        char choice;
        scanf(" %c", &choice);
        clearInputBuffer(); // ������뻺����
        if (choice == 'y' || choice == 'Y') {
            addToShengciBen(wordList[idx].english, wordList[idx].chinese);
            printf("�ѽ����� '%s' ��ӵ����ʱ���\n", wordList[idx].english);
        }
    }

    free(indices);

    // �����û��͹���Ա����
    if (saveAll() != 0) {
        printf("�����û�����ʧ�ܡ�\n");
    } else {
        printf("Ĭд��ɣ��û������Ѹ��¡�\n");
    }
}

// ������������������ӵ����ʱ�
void addToShengciBen(const char *english, const char *chinese) {
    FILE *file = fopen("shengciben.txt", "a");
    if (file == NULL) {
        perror("�޷���shengciben.txt");
        return;
    }
    fprintf(file, "%s %s\n", english, chinese);
    fclose(file);
}

// �鿴���󵥴�
void viewWrongWords() {
    FILE *file = fopen("wrong.txt", "r");
    if (file == NULL) {
        printf("���󵥴��б����ڡ�\n");
        return;
    }

    char line[256];
    int count = 0;
    printf("\n=== ���󵥴��б� ===\n");
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) > 0) {
            printf("%d. %s\n", ++count, line);
        }
    }

    if (count == 0) {
        printf("���󵥴��б�Ϊ�ա�\n");
    }

    fclose(file);
}

// �鿴��ѧ���ĵ���
void viewAlreadyLearnedWords() {
    FILE *file = fopen("already.txt", "r");
    if (file == NULL) {
        printf("��ѧ���ĵ����б����ڡ�\n");
        return;
    }

    char line[256];
    int count = 0;
    printf("\n=== ��ѧ���ĵ����б� ===\n");
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) > 0) {
            printf("%d. %s\n", ++count, line);
        }
    }

    if (count == 0) {
        printf("��ѧ���ĵ����б�Ϊ�ա�\n");
    }

    fclose(file);
}

// ���ⵥ��ѵ������
void wrongWordTraining(User *user) {
    FILE *file = fopen("wrong.txt", "r");
    if (file == NULL) {
        printf("���Ȿ�����ڻ�Ϊ�ա�\n");
        return;
    }

    // ��ȡ���д��⵽����
    char **wrongWords = NULL;
    int wrongCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) > 0) {
            wrongWords = (char **)realloc(wrongWords, sizeof(char *) * (wrongCount + 1));
            if (wrongWords == NULL) {
                perror("�ڴ����ʧ��");
                fclose(file);
                return;
            }
            wrongWords[wrongCount] = strdup(line);
            if (wrongWords[wrongCount] == NULL) {
                perror("�ڴ����ʧ��");
                fclose(file);
                return;
            }
            wrongCount++;
        }
    }
    fclose(file);

    if (wrongCount == 0) {
        printf("���ⱾΪ�գ�����ѵ����\n");
        free(wrongWords);
        return;
    }

    // ����ѵ���ĵ�������
    int trainCount = wrongCount < 20 ? wrongCount : 20;

    // ����һ�����������������ѡ��
    int *indices = (int *)malloc(sizeof(int) * wrongCount);
    if (indices == NULL) {
        perror("�ڴ����ʧ��");
        // �ͷ�wrongWords
        for (int i = 0; i < wrongCount; i++) {
            free(wrongWords[i]);
        }
        free(wrongWords);
        return;
    }

    for (int i = 0; i < wrongCount; i++) {
        indices[i] = i;
    }

    // ���������������
    for (int i = wrongCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // ����
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // ѡ��ǰtrainCount����������ѵ��
    printf("\n=== ���ⵥ��ѵ����ʼ ===\n");
    int *correctIndices = (int *)malloc(sizeof(int) * trainCount);
    if (correctIndices == NULL) {
        perror("�ڴ����ʧ��");
        // �ͷ���Դ
        free(indices);
        for (int i = 0; i < wrongCount; i++) {
            free(wrongWords[i]);
        }
        free(wrongWords);
        return;
    }
    int correctCount = 0;

    for (int i = 0; i < trainCount; i++) {
        int idx = indices[i];
        // �ָ�Ӣ�ĺ�����
        char *english = strtok(wrongWords[idx], " ");
        char *chinese = strtok(NULL, "");

        if (english == NULL || chinese == NULL) {
            printf("��ʽ����: %s\n", wrongWords[idx]);
            continue;
        }

        printf("��д���������ĵ��ʵ�Ӣ��:\n%s: ", chinese);

        char answer[50];
        fgets(answer, sizeof(answer), stdin);
        trimNewline(answer);

        // ���𰸣����Դ�Сд��
        if (strcasecmp(answer, english) == 0) {
            printf("��ȷ��\n");
            correctIndices[correctCount++] = idx;
            // ��¼��ѧ���ĵ���
            FILE *alreadyFile = fopen("already.txt", "a");
            if (alreadyFile != NULL) {
                fprintf(alreadyFile, "%s %s\n", english, chinese);
                fclose(alreadyFile);
            } else {
                perror("�޷���already.txt");
            }
        } else {
            printf("������ȷ����: %s\n", english);
            // ���Ӵ��󵥴ʼ���
            user->wrong_words += 1;
        }

        // ѯ���û���һ������
        while (1) {
            printf("\n��ѡ�����:\n");
            printf("1. ��һ��\n");
            printf("2. �˳���ϰ\n");
            printf("������ѡ��: ");
            int op;
            if (scanf("%d", &op) != 1) {
                clearInputBuffer();
                printf("��Ч�����룬���������֡�\n");
                continue;
            }
            clearInputBuffer(); // ������뻺����

            if (op == 1) {
                // ������һ��
                break;
            } else if (op == 2) {
                printf("���˳����⸴ϰ��\n");
                // �����û��͹���Ա����
                if (saveAll() != 0) {
                    printf("�����û�����ʧ�ܡ�\n");
                } else {
                    printf("�û������Ѹ��¡�\n");
                }
                // �Ƴ���ȷ�ش�ĵ���
                if (correctCount > 0) {
                    // ����һ���µĴ������飬�ų���ȷ�ĵ���
                    char **newWrongWords = NULL;
                    int newWrongCount = 0;
                    for (int j = 0; j < wrongCount; j++) {
                        int toRemove = 0;
                        for (int k = 0; k < correctCount; k++) {
                            if (j == correctIndices[k]) {
                                toRemove = 1;
                                break;
                            }
                        }
                        if (!toRemove) {
                            newWrongWords = (char **)realloc(newWrongWords, sizeof(char *) * (newWrongCount + 1));
                            if (newWrongWords == NULL) {
                                perror("�ڴ����ʧ��");
                                break;
                            }
                            newWrongWords[newWrongCount] = strdup(wrongWords[j]);
                            if (newWrongWords[newWrongCount] == NULL) {
                                perror("�ڴ����ʧ��");
                                break;
                            }
                            newWrongCount++;
                        }
                    }

                    // ����д��wrong.txt
                    FILE *newWrongFile = fopen("wrong.txt", "w");
                    if (newWrongFile == NULL) {
                        perror("�޷���wrong.txt����д��");
                    } else {
                        for (int j = 0; j < newWrongCount; j++) {
                            fprintf(newWrongFile, "%s\n", newWrongWords[j]);
                            free(newWrongWords[j]);
                        }
                        fclose(newWrongFile);
                        free(newWrongWords);
                        printf("�Ѹ��´��Ȿ��\n");
                    }
                }

                // �ͷ���Դ
                free(indices);
                free(correctIndices);
                for (int j = 0; j < wrongCount; j++) {
                    free(wrongWords[j]);
                }
                free(wrongWords);
                return;
            } else {
                printf("��Ч��ѡ�����������롣\n");
            }
        }
    }

    // �Ƴ���ȷ�ش�ĵ���
    if (correctCount > 0) {
        // ����һ���µĴ������飬�ų���ȷ�ĵ���
        char **newWrongWords = NULL;
        int newWrongCount = 0;
        for (int i = 0; i < wrongCount; i++) {
            int toRemove = 0;
            for (int j = 0; j < correctCount; j++) {
                if (i == correctIndices[j]) {
                    toRemove = 1;
                    break;
                }
            }
            if (!toRemove) {
                newWrongWords = (char **)realloc(newWrongWords, sizeof(char *) * (newWrongCount + 1));
                if (newWrongWords == NULL) {
                    perror("�ڴ����ʧ��");
                    break;
                }
                newWrongWords[newWrongCount] = strdup(wrongWords[i]);
                if (newWrongWords[newWrongCount] == NULL) {
                    perror("�ڴ����ʧ��");
                    break;
                }
                newWrongCount++;
            }
        }

        // ����д��wrong.txt
        FILE *newWrongFile = fopen("wrong.txt", "w");
        if (newWrongFile == NULL) {
            perror("�޷���wrong.txt����д��");
        } else {
            for (int i = 0; i < newWrongCount; i++) {
                fprintf(newWrongFile, "%s\n", newWrongWords[i]);
                free(newWrongWords[i]);
            }
            fclose(newWrongFile);
            free(newWrongWords);
            printf("ѵ����ɣ��Ѹ��´��Ȿ��\n");
        }
    } else {
        printf("û����ȷ�ش�ĵ��ʣ�������´��Ȿ��\n");
    }

    // �ͷ���Դ
    free(indices);
    free(correctIndices);
    for (int i = 0; i < wrongCount; i++) {
        free(wrongWords[i]);
    }
    free(wrongWords);

    // �����û��͹���Ա����
    if (saveAll() != 0) {
        printf("�����û�����ʧ�ܡ�\n");
    } else {
        printf("���ⵥ��ѵ����ɣ��û������Ѹ��¡�\n");
    }
}

// ��������
void searchWord() {
    clearInputBuffer(); // ������뻺����

    printf("\n=== �������� ===\n");
    printf("������Ҫ������Ӣ�ĵ���: ");
    char search[50];
    fgets(search, sizeof(search), stdin);
    trimNewline(search);

    // ����wordList����
    int found = 0;
    for (int i = 0; i < wordCount; i++) {
        if (strcasecmp(wordList[i].english, search) == 0) {
            printf("����: %s\n������˼: %s\n", wordList[i].english, wordList[i].chinese);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("δ�ҵ����� '%s' ��������˼��\n", search);
    }
}

// Ȥζ��սģʽ���ܣ�ʹ������ģʽ
void challengeMode(User *user) {
    if (wordCount == 0) {
        printf("�����б�Ϊ�գ��޷�������սģʽ��\n");
        return;
    }

    printf("\n=== Ȥζ��սģʽ ===\n");
    printf("��ѡ����սģʽ:\n");
    printf("1. ����Ӣ��д����\n");
    printf("2. ��������дӢ��\n");
    printf("������ѡ��: ");

    int mode;
    if (scanf("%d", &mode) != 1) {
        clearInputBuffer();
        printf("��Ч�����룬���������֡�\n");
        return;
    }
    clearInputBuffer(); // ������뻺����

    if (mode < 1 || mode > 2) {
        printf("��Ч��ѡ��������ѡ��\n");
        return;
    }

    printf("����60���ʱ����Ĭд�����ܶ�ĵ��ʡ�\n");
    printf("ÿ���һ�����ʼ�10�֣�����5�֡�\n");
    printf("��ս������ʼ��׼�������𣿰��س�����ʼ...");
    getchar(); // �ȴ��û����س�

    int score = 0;
    time_t start_time = time(NULL);
    time_t current_time;

    // ����һ����������Ա����ظ�
    int *sequence = (int *)malloc(sizeof(int) * wordCount);
    if (sequence == NULL) {
        perror("�ڴ����ʧ��");
        return;
    }
    for (int i = 0; i < wordCount; i++) {
        sequence[i] = i;
    }
    // ��������
    for (int i = wordCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = sequence[i];
        sequence[i] = sequence[j];
        sequence[j] = temp;
    }

    int index = 0;
    while (1) {
        current_time = time(NULL);
        double elapsed = difftime(current_time, start_time);
        if (elapsed >= 60.0) {
            printf("\nʱ�䵽����ս������\n");
            break;
        }

        if (index >= wordCount) {
            // ������е��ʶ��Ѿ���ս�������´���
            for (int i = 0; i < wordCount; i++) {
                sequence[i] = i;
            }
            // ��������
            for (int i = wordCount - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                int temp = sequence[i];
                sequence[i] = sequence[j];
                sequence[j] = temp;
            }
            index = 0;
        }

        int word_idx = sequence[index++];
        char question[100];
        char correctAnswer[100];
        char prompt[100];

        if (mode == 1) {
            // ����Ӣ��д����
            strcpy(question, wordList[word_idx].english);
            strcpy(correctAnswer, wordList[word_idx].chinese);
            snprintf(prompt, sizeof(prompt), "��д������Ӣ�ĵ��ʵ�����:\n%s: ", question);
        } else {
            // ��������дӢ��
            strcpy(question, wordList[word_idx].chinese);
            strcpy(correctAnswer, wordList[word_idx].english);
            snprintf(prompt, sizeof(prompt), "��д���������ĵ��ʵ�Ӣ��:\n%s: ", question);
        }

        printf("\n%s", prompt);

        char answer[50];
        fgets(answer, sizeof(answer), stdin);
        trimNewline(answer);

        // ���𰸣����Դ�Сд��
        if (strcasecmp(answer, correctAnswer) == 0) {
            printf("��ȷ��+10��\n");
            score += 10;
            // ��¼��ѧ���ĵ���
            FILE *alreadyFile = fopen("already.txt", "a");
            if (alreadyFile != NULL) {
                fprintf(alreadyFile, "%s %s\n", wordList[word_idx].english, wordList[word_idx].chinese);
                fclose(alreadyFile);
            } else {
                perror("�޷���already.txt");
            }
        } else {
            printf("������ȷ����: %s��-5��\n", correctAnswer);
            score -= 5;
            if (score < 0) score = 0; // ��ֹ����Ϊ��
            // ��¼����ĵ���
            user->wrong_words += 1;
            FILE *wrongFile = fopen("wrong.txt", "a");
            if (wrongFile != NULL) {
                fprintf(wrongFile, "%s %s\n", wordList[word_idx].english, wordList[word_idx].chinese);
                fclose(wrongFile);
            } else {
                perror("�޷���wrong.txt");
            }
        }
    }

    free(sequence);

    // �����û��͹���Ա����
    if (saveAll() != 0) {
        printf("�����û�����ʧ�ܡ�\n");
    }

    // �������ͷ�������rank.txt
    FILE *rankFile = fopen("rank.txt", "a");
    if (rankFile == NULL) {
        perror("�޷���rank.txt");
    } else {
        fprintf(rankFile, "%s,%d\n", user->name, score);
        fclose(rankFile);
    }

    printf("���ĵ÷�: %d\n", score);
}

// �ȽϺ�������qsort
int compareRankEntries(const void *a, const void *b) {
    RankEntry *entryA = (RankEntry *)a;
    RankEntry *entryB = (RankEntry *)b;
    return entryB->score - entryA->score; // �Ӹߵ�������
}

// ���а���
void displayRanking() {
    FILE *file = fopen("rank.txt", "r");
    if (file == NULL) {
        printf("���а��ļ������ڡ�\n");
        return;
    }

    // ��ȡ���а�����
    RankEntry *entries = NULL;
    int entryCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) == 0) continue;

        // �ָ������ͷ���
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        char *name = token;

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        int score = atoi(token);

        // ��ӵ����а�����
        entries = (RankEntry *)realloc(entries, sizeof(RankEntry) * (entryCount + 1));
        if (entries == NULL) {
            perror("�ڴ����ʧ��");
            fclose(file);
            return;
        }
        strcpy(entries[entryCount].name, name);
        entries[entryCount].score = score;
        entryCount++;
    }
    fclose(file);

    if (entryCount == 0) {
        printf("���а�Ϊ�ա�\n");
        free(entries);
        return;
    }

    // �������Ӹߵ�������
    qsort(entries, entryCount, sizeof(RankEntry), compareRankEntries);

    // ��ʾ���а�
    printf("\n=== ���а� ===\n");
    printf("%-5s %-20s %-10s\n", "����", "����", "����");
    for (int i = 0; i < entryCount && i < 10; i++) { // ��ʾǰ10��
        printf("%-5d %-20s %-10d\n", i + 1, entries[i].name, entries[i].score);
    }

    free(entries);
}

// ����û���Ϣ������Ա���ܣ�
void browseUsers() {
    printf("\n=== �û���Ϣ�б� ===\n");
    User *current = head;
    int count = 0;
    while (current != NULL) {
        if (!current->isAdmin) { // ����ʾ��ͨ�û�
            printf("\n--- �û� %d ---\n", ++count);
            printf("����: %s\n", current->name);
            printf("�û���: %s\n", current->username);
            printf("������: %d\n", current->checkin_days);
            printf("�ܼƱ��˶��ٵ���: %d\n", current->total_words);
            printf("������: %d\n", current->wrong_words);
            printf("��ȷ��: %.2f%%\n", current->correct_rate);
            printf("��һ�δ�ʱ��: %s\n", current->last_checkin_time);
        }
        current = current->next;
    }

    if (count == 0) {
        printf("��ǰû����ͨ�û���\n");
    }
}

// ɾ���û����ܣ�����Ա��
void deleteUser() {
    char usernameToDelete[50];
    printf("\n������Ҫɾ�����û���: ");
    fgets(usernameToDelete, sizeof(usernameToDelete), stdin);
    trimNewline(usernameToDelete);

    if (strlen(usernameToDelete) == 0) {
        printf("�û�������Ϊ�ա�\n");
        return;
    }

    User *current = head;
    User *previous = NULL;

    while (current != NULL) {
        if (strcmp(current->username, usernameToDelete) == 0) {
            if (current->isAdmin) {
                printf("�޷�ɾ������Ա�û���\n");
                return;
            }
            // �ҵ�Ҫɾ�����û�
            if (previous == NULL) {
                // ɾ��ͷ�ڵ�
                head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            printf("�û� '%s' �ѳɹ�ɾ����\n", usernameToDelete);
            // ���������û�
            if (saveAllUsers() != 0) {
                printf("ɾ���û��󣬱����û�����ʧ�ܡ�\n");
            }
            return;
        }
        previous = current;
        current = current->next;
    }

    printf("δ�ҵ��û���Ϊ '%s' ���û���\n", usernameToDelete);
}

// ����Ա�˵�
void adminMenu(User *adminUser) {
    int choice;
    while (1) {
        printf("\n=== ����Ա�˵� ===\n");
        printf("1. �û�����\n");
        printf("2. �������\n");
        printf("3. �������˵�\n");
        printf("������ѡ��: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("��Ч�����룬���������֡�\n");
            continue;
        }
        clearInputBuffer(); // ������뻺����

        switch (choice) {
        case 1:
            // �û������Ӳ˵�
            {
                int userChoice;
                while (1) {
                    printf("\n=== �û����� ===\n");
                    printf("1. ��������û�\n");
                    printf("2. ɾ���û�\n");
                    printf("3. ���ع���Ա�˵�\n");
                    printf("������ѡ��: ");

                    if (scanf("%d", &userChoice) != 1) {
                        clearInputBuffer();
                        printf("��Ч�����룬���������֡�\n");
                        continue;
                    }
                    clearInputBuffer(); // ������뻺����

                    switch (userChoice) {
                    case 1:
                        browseUsers();
                        break;
                    case 2:
                        // ����ɾ���û�����
                        deleteUser();
                        break;
                    case 3:
                        // ���ع���Ա�˵�
                        goto admin_menu_end;
                    default:
                        printf("��Ч��ѡ�����������롣\n");
                    }
                }
            }
        admin_menu_end:
            break;
        case 2:
            manageWordLibraries();
            break;
        case 3:
            printf("�������˵���\n");
            return;
        default:
            printf("��Ч��ѡ�����������롣\n");
        }
    }
}

// ������⹦��
void manageWordLibraries() {
    int choice;
    while (1) {
        printf("\n=== ������˵� ===\n");
        printf("1. �г��������\n");
        printf("2. ����µ����\n");
        printf("3. ɾ���������\n");
        printf("4. ����������еĵ���\n");
        printf("5. ���ع���Ա�˵�\n");
        printf("������ѡ��: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("��Ч�����룬���������֡�\n");
            continue;
        }
        clearInputBuffer(); // ������뻺����

        switch (choice) {
        case 1:
            listWordLibraries();
            break;
        case 2:
            addWordLibrary();
            break;
        case 3:
            deleteWordLibrary();
            break;
        case 4:
            manageSingleWordLibrary();
            break;
        case 5:
            return;
        default:
            printf("��Ч��ѡ�����������롣\n");
        }
    }
}

// �г��������
void listWordLibraries() {
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char searchPath[150];
    snprintf(searchPath, sizeof(searchPath), "word_libraries\\*");

    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("�޷��� word_libraries Ŀ¼��Ŀ¼Ϊ�ա�\n");
        return;
    } else {
        printf("\n=== ��ǰ����б� ===\n");
        int count = 0;
        do {
            const char *name = findFileData.cFileName;
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
                continue;
            printf("%d. %s\n", ++count, name);
        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
        if (count == 0) {
            printf("��ǰû���κ���⡣\n");
        }
    }
#else
    DIR *d;
    struct dirent *dir;
    d = opendir("word_libraries");
    if (d) {
        printf("\n=== ��ǰ����б� ===\n");
        int count = 0;
        while ((dir = readdir(d)) != NULL) {
            // �ų� "." �� ".."
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                continue;
            printf("%d. %s\n", ++count, dir->d_name);
        }
        closedir(d);
        if (count == 0) {
            printf("��ǰû���κ���⡣\n");
        }
    } else {
        perror("�޷��� word_libraries Ŀ¼");
    }
#endif
}

// ����µ����
void addWordLibrary() {
    clearInputBuffer(); // ������뻺����
    char libraryName[100];
    printf("\n���������������� (����: new_library.txt): ");
    fgets(libraryName, sizeof(libraryName), stdin);
    trimNewline(libraryName);

    // ����ļ����Ƿ�Ϸ�
    if (strlen(libraryName) == 0) {
        printf("������Ʋ���Ϊ�ա�\n");
        return;
    }

    // ����ļ��Ƿ��Ѿ�����
    char filepath[150];
#ifdef _WIN32
    snprintf(filepath, sizeof(filepath), "word_libraries\\%s", libraryName);
#else
    snprintf(filepath, sizeof(filepath), "word_libraries/%s", libraryName);
#endif
    FILE *file = fopen(filepath, "r");
    if (file != NULL) {
        printf("��� '%s' �Ѵ��ڡ�\n", libraryName);
        fclose(file);
        return;
    }

    // �����µ�����ļ�
    file = fopen(filepath, "w");
    if (file == NULL) {
        perror("�޷������µ�����ļ�");
        return;
    }
    fclose(file);
    printf("��� '%s' �����ɹ���\n", libraryName);
}

// ɾ���������
void deleteWordLibrary() {
    clearInputBuffer(); // ������뻺����
    char libraryName[100];
    printf("\n������Ҫɾ����������� (����: new_library.txt): ");
    fgets(libraryName, sizeof(libraryName), stdin);
    trimNewline(libraryName);

    // ����ļ��Ƿ����
    char filepath[150];
#ifdef _WIN32
    snprintf(filepath, sizeof(filepath), "word_libraries\\%s", libraryName);
#else
    snprintf(filepath, sizeof(filepath), "word_libraries/%s", libraryName);
#endif
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        printf("��� '%s' �����ڡ�\n", libraryName);
        return;
    }
    fclose(file);

    // ȷ��ɾ��
    printf("ȷ��Ҫɾ����� '%s' �� (y/n): ", libraryName);
    char confirm;
    scanf(" %c", &confirm);
    clearInputBuffer(); // ������뻺����
    if (confirm == 'y' || confirm == 'Y') {
#ifdef _WIN32
        if (remove(filepath) == 0) {
            printf("��� '%s' ɾ���ɹ���\n", libraryName);
        } else {
            perror("ɾ�����ʧ��");
        }
#else
        if (remove(filepath) == 0) {
            printf("��� '%s' ɾ���ɹ���\n", libraryName);
        } else {
            perror("ɾ�����ʧ��");
        }
#endif
    } else {
        printf("ȡ��ɾ����\n");
    }
}

// ����������еĵ���
void manageSingleWordLibrary() {
    clearInputBuffer(); // ������뻺����
    char libraryName[100];
    printf("\n������Ҫ������������ (����: new_library.txt): ");
    fgets(libraryName, sizeof(libraryName), stdin);
    trimNewline(libraryName);

    // ����ļ��Ƿ����
    char filepath[150];
#ifdef _WIN32
    snprintf(filepath, sizeof(filepath), "word_libraries\\%s", libraryName);
#else
    snprintf(filepath, sizeof(filepath), "word_libraries/%s", libraryName);
#endif
    FILE *file = fopen(filepath, "r+");
    if (file == NULL) {
        printf("��� '%s' �����ڡ�\n", libraryName);
        return;
    }
    fclose(file);

    int choice;
    while (1) {
        printf("\n=== ������� '%s' ===\n", libraryName);
        printf("1. �г����е���\n");
        printf("2. ����µ���\n");
        printf("3. ɾ������\n");
        printf("4. ����������˵�\n");
        printf("������ѡ��: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("��Ч�����룬���������֡�\n");
            continue;
        }
        clearInputBuffer(); // ������뻺����

        switch (choice) {
        case 1:
            // �г����е���
            printf("\n=== �����б� ===\n");
            file = fopen(filepath, "r");
            if (file == NULL) {
                perror("�޷�������ļ�");
                break;
            }
            {
                char line[256];
                int count = 0;
                while (fgets(line, sizeof(line), file)) {
                    trimNewline(line);
                    if (strlen(line) > 0) {
                        printf("%d. %s\n", ++count, line);
                    }
                }
                if (count == 0) {
                    printf("���Ϊ�ա�\n");
                }
            }
            fclose(file);
            break;
        case 2:
            // ����µ���
            {
                char english[50], chinese[100];
                printf("\n������Ӣ�ĵ���: ");
                fgets(english, sizeof(english), stdin);
                trimNewline(english);
                printf("������������˼: ");
                fgets(chinese, sizeof(chinese), stdin);
                trimNewline(chinese);

                // ����Ƿ��Ѵ���
                file = fopen(filepath, "r");
                if (file != NULL) {
                    char line[256];
                    int exists = 0;
                    while (fgets(line, sizeof(line), file)) {
                        trimNewline(line);
                        if (strlen(line) == 0) continue;
                        char *existingEnglish = strtok(line, " ");
                        if (existingEnglish && strcmp(existingEnglish, english) == 0) {
                            exists = 1;
                            break;
                        }
                    }
                    fclose(file);
                    if (exists) {
                        printf("���� '%s' �Ѵ���������С�\n", english);
                        break;
                    }
                }

                // ׷���µ���
                file = fopen(filepath, "a");
                if (file == NULL) {
                    perror("�޷�������ļ�����д��");
                    break;
                }
                fprintf(file, "%s %s\n", english, chinese);
                fclose(file);
                printf("���� '%s' ��ӳɹ���\n", english);
            }
            break;
        case 3:
            // ɾ������
            {
                char english[50];
                printf("\n������Ҫɾ����Ӣ�ĵ���: ");
                fgets(english, sizeof(english), stdin);
                trimNewline(english);

                // ��ȡ���е��ʲ���д�ļ����ų�Ҫɾ���ĵ���
                FILE *readFile = fopen(filepath, "r");
                if (readFile == NULL) {
                    perror("�޷�������ļ�");
                    break;
                }

                // ��ʱ�ļ�
                char tempPath[160];
#ifdef _WIN32
                snprintf(tempPath, sizeof(tempPath), "word_libraries\\temp_%s", libraryName);
#else
                snprintf(tempPath, sizeof(tempPath), "word_libraries/temp_%s", libraryName);
#endif
                FILE *tempFile = fopen(tempPath, "w");
                if (tempFile == NULL) {
                    perror("�޷�������ʱ�ļ�");
                    fclose(readFile);
                    break;
                }

                char line[256];
                int found = 0;
                while (fgets(line, sizeof(line), readFile)) {
                    trimNewline(line);
                    if (strlen(line) == 0) continue;
                    char *existingEnglish = strtok(line, " ");
                    char *existingChinese = strtok(NULL, "");
                    if (existingEnglish && strcmp(existingEnglish, english) == 0) {
                        found = 1;
                        continue; // ����Ҫɾ���ĵ���
                    }
                    fprintf(tempFile, "%s %s\n", existingEnglish, existingChinese ? existingChinese : "");
                }

                fclose(readFile);
                fclose(tempFile);

                if (found) {
                    // �滻ԭ�ļ�
                    if (remove(filepath) != 0) {
                        perror("�޷�ɾ��ԭ����ļ�");
                        remove(tempPath);
                        break;
                    }
                    if (rename(tempPath, filepath) != 0) {
                        perror("�޷���������ʱ�ļ�");
                        break;
                    }
                    printf("���� '%s' ɾ���ɹ���\n", english);
                } else {
                    printf("δ�ҵ����� '%s'��\n", english);
                    remove(tempPath);
                }
            }
            break;
        case 4:
            // ����������˵�
            return;
        default:
            printf("��Ч��ѡ�����������롣\n");
        }
    }
}
