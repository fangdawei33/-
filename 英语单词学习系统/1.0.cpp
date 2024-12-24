#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h> // 用于创建目录

#ifdef _WIN32
#include <direct.h>   // 用于创建目录
#include <windows.h>  // 用于目录操作
#define MKDIR(path) _mkdir(path)
#else
#include <sys/types.h>
#include <dirent.h>    // 仅用于非Windows系统
#define MKDIR(path) mkdir(path, 0755)
#endif

// 定义用户结构体
typedef struct User {
    char username[50];
    char password[50];
    char name[100];
    int checkin_days;
    int total_words;
    int wrong_words;
    float correct_rate; // 正确率
    char last_checkin_time[20]; // 格式: YYYY-MM-DD HH:MM:SS
    int isAdmin; // 是否为管理员
    struct User *next;
} User;

// 定义单词结构体
typedef struct Word {
    char english[50];
    char chinese[100];
} Word;

// 定义排行榜条目结构体
typedef struct RankEntry {
    char name[100];
    int score;
} RankEntry;

// 全局头指针
User *head = NULL;

// 全局单词数组及计数
Word *wordList = NULL;
int wordCount = 0;

// 函数声明
void loadUsers();
void loadAdmins();
int saveAllUsers();
int saveAllAdmins();
int saveAll(); // 同时保存用户和管理员
void registerUser(int isAdmin);
User* loginUser(int isAdmin);
void displayUserInfo(User *user);
void checkIn(User *user);
void reciteWords(User *user); // 背诵单词功能
void dictation(User *user); // 默写功能
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
// 新增函数声明
void addToShengciBen(const char *english, const char *chinese);
void viewShengciBen(); // 浏览生词本功能

// 辅助函数声明
int compareRankEntries(const void *a, const void *b);
void trimNewline(char *str);

// 文件管理功能
void browseUsers();
void deleteUser(); // 删除用户

// 用于题库管理
void manageWordLibraries();
void listWordLibraries();
void addWordLibrary();
void deleteWordLibrary();
void manageSingleWordLibrary();

// 程序入口

int main() {
    int choice;
    User *currentUser = NULL;
    srand(time(NULL)); // 初始化随机数种子

    // 创建题库目录如果不存在
    struct stat st = {0};
    if (stat("word_libraries", &st) == -1) {
        if (MKDIR("word_libraries") != 0) {
            perror("无法创建 word_libraries 目录");
            return 1;
        }
    }

    loadUsers();    // 加载普通用户
    loadAdmins();   // 加载管理员

    // 加载单词
    if (loadWords() != 0) {
        printf("加载单词失败，请确保有有效的词库文件 'word_libraries\\cet.txt' 存在于 word_libraries 目录中。\n");
    }

    while (1) {
        if (currentUser == NULL) {
            printf("\n=== 主界面 ===\n");
            printf("1. 注册新用户\n");
            printf("2. 用户登录\n");
            printf("3. 后台管理\n");
            printf("4. 退出\n");
            printf("请输入选择: ");

            if (scanf("%d", &choice) != 1) {
                clearInputBuffer();
                printf("无效的输入，请输入数字。\n");
                continue;
            }
            clearInputBuffer(); // 清除输入缓冲区

            switch (choice) {
            case 1:
                registerUser(0); // 普通用户注册
                break;
            case 2:
                currentUser = loginUser(0); // 普通用户登录
                break;
            case 3: {
                // 后台管理菜单
                printf("\n=== 后台管理 ===\n");
                printf("1. 管理员注册\n");
                printf("2. 管理员登录\n");
                printf("3. 返回主菜单\n");
                printf("请输入选择: ");
                int adminChoice;
                if (scanf("%d", &adminChoice) != 1) {
                    clearInputBuffer();
                    printf("无效的输入，请输入数字。\n");
                    continue;
                }
                clearInputBuffer(); // 清除输入缓冲区
                switch (adminChoice) {
                case 1:
                    registerUser(1); // 管理员注册
                    break;
                case 2: {
                    User *adminUser = loginUser(1); // 管理员登录
                    if (adminUser != NULL && adminUser->isAdmin) {
                        adminMenu(adminUser);
                        currentUser = NULL; // 退出管理员菜单后登出
                    }
                    break;
                }
                case 3:
                    // 返回主菜单
                    break;
                default:
                    printf("无效的选择，请重新输入。\n");
                }
                break;
            }
            case 4:
                printf("退出程序。\n");
                freeUsers();
                freeWords();
                exit(0);
            default:
                printf("无效的选择，请重新输入。\n");
            }
        } else {
            if (currentUser->isAdmin) {
                // 管理员已经在 adminMenu 中处理，无需额外处理
                currentUser = NULL;
                continue;
            }

            // 普通用户菜单
            printf("\n=== 用户菜单 ===\n");
            printf("1. 显示个人信息\n");
            printf("2. 打卡\n");
            printf("3. 背诵单词\n");
            printf("4. 错题本\n");
            printf("5. 查看已学过的单词\n");
            printf("6. 错题复习\n");
            printf("7. 搜索单词\n");
            printf("8. 趣味挑战\n");
            printf("9. 排行榜\n");
            printf("10. 生词本\n"); 
            printf("11. 返回主界面\n");
            printf("12. 退出程序\n");
            printf("请输入选择: ");

            if (scanf("%d", &choice) != 1) {
                clearInputBuffer();
                printf("无效的输入，请输入数字。\n");
                continue;
            }
            clearInputBuffer(); // 清除输入缓冲区

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
                viewShengciBen(); // 调用浏览生词本功能
                break;
            case 11:
                printf("登出成功。\n");
                currentUser = NULL;
                break;
            case 12:
                printf("退出程序。\n");
                freeUsers();
                freeWords();
                exit(0);
            default:
                printf("无效的选择，请重新输入。\n");
            }
        }
    }

    return 0;
}


// 清除输入缓冲区
void clearInputBuffer() {
    while (getchar() != '\n');
}

// 去除字符串末尾的换行符
void trimNewline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

// 从文件加载普通用户到链表
void loadUsers() {
    FILE *file = fopen("users.txt", "r");
    if (file == NULL) {
        // 如果文件不存在，可能是第一次运行，不需要处理
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        User *newUser = (User *)malloc(sizeof(User));
        if (newUser == NULL) {
            perror("内存分配失败");
            fclose(file);
            return;
        }

        trimNewline(line);

        // 使用逗号分隔字段
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
            strcpy(newUser->last_checkin_time, "未打卡");
        }

        // 尝试读取 correct_rate，如果不存在则初始化为 0.0
        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->correct_rate = atof(token);
        } else {
            newUser->correct_rate = 0.0f;
        }

        newUser->isAdmin = 0; // 普通用户

        newUser->next = head;
        head = newUser;
    }

    fclose(file);
}

// 从文件加载管理员到链表
void loadAdmins() {
    FILE *file = fopen("monitor.txt", "r");
    if (file == NULL) {
        // 如果文件不存在，可能是第一次运行，不需要处理
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        User *newUser = (User *)malloc(sizeof(User));
        if (newUser == NULL) {
            perror("内存分配失败");
            fclose(file);
            return;
        }

        trimNewline(line);

        // 使用逗号分隔字段
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
            strcpy(newUser->last_checkin_time, "未打卡");
        }

        // 尝试读取 correct_rate，如果不存在则初始化为 0.0
        token = strtok(NULL, ",");
        if (token != NULL) {
            newUser->correct_rate = atof(token);
        } else {
            newUser->correct_rate = 0.0f;
        }

        newUser->isAdmin = 1; // 管理员

        newUser->next = head;
        head = newUser;
    }

    fclose(file);
}

// 将所有普通用户保存到 users.txt
int saveAllUsers() {
    FILE *file = fopen("users.txt", "w");
    if (file == NULL) {
        perror("无法打开 users.txt 文件进行保存");
        return -1;
    }

    User *current = head;
    while (current != NULL) {
        if (current->isAdmin == 0) { // 仅保存普通用户
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

// 将所有管理员保存到 monitor.txt
int saveAllAdmins() {
    FILE *file = fopen("monitor.txt", "w");
    if (file == NULL) {
        perror("无法打开 monitor.txt 文件进行保存");
        return -1;
    }

    User *current = head;
    while (current != NULL) {
        if (current->isAdmin == 1) { // 仅保存管理员
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

// 同时保存所有用户和管理员
int saveAll() {
    if (saveAllUsers() != 0) {
        return -1;
    }
    if (saveAllAdmins() != 0) {
        return -1;
    }
    return 0;
}

// 注册新用户或管理员
void registerUser(int isAdmin) {
    User *newUser = (User *)malloc(sizeof(User));
    if (newUser == NULL) {
        perror("内存分配失败");
        return;
    }

    // 不需要清除输入缓冲区，因为调用函数前已经清除

    if (isAdmin) {
        printf("\n=== 管理员注册 ===\n");
    } else {
        printf("\n=== 用户注册 ===\n");
    }

    // 输入用户名
    printf("请输入用户名: ");
    fgets(newUser->username, sizeof(newUser->username), stdin);
    trimNewline(newUser->username);

    // 检查用户名是否已存在
    User *current = head;
    while (current != NULL) {
        if (strcmp(current->username, newUser->username) == 0) {
            printf("用户名已存在，请选择其他用户名。\n");
            free(newUser);
            return;
        }
        current = current->next;
    }

    // 输入密码
    printf("请输入密码: ");
    fgets(newUser->password, sizeof(newUser->password), stdin);
    trimNewline(newUser->password);

    // 输入姓名
    printf("请输入姓名: ");
    fgets(newUser->name, sizeof(newUser->name), stdin);
    trimNewline(newUser->name);

    // 初始化新用户的其他字段
    newUser->checkin_days = 0;
    newUser->total_words = 0;
    newUser->wrong_words = 0;
    newUser->correct_rate = 0.0f; // 初始化正确率
    strcpy(newUser->last_checkin_time, "未打卡");
    newUser->isAdmin = isAdmin;

    // 添加到链表头部
    newUser->next = head;
    head = newUser;

    // 保存用户到相应的文件
    if (isAdmin) {
        if (saveAllAdmins() != 0) {
            printf("注册失败，无法保存管理员数据。\n");
            return;
        }
        printf("管理员注册成功！\n");
    } else {
        if (saveAllUsers() != 0) {
            printf("注册失败，无法保存用户数据。\n");
            return;
        }
        printf("注册成功！\n");
    }
}

// 登录功能
User* loginUser(int isAdmin) {
    char username[50];
    char password[50];

    if (isAdmin) {
        printf("\n=== 管理员登录 ===\n");
    } else {
        printf("\n=== 用户登录 ===\n");
    }

    // 输入用户名
    printf("请输入用户名: ");
    fgets(username, sizeof(username), stdin);
    trimNewline(username);

    // 输入密码
    printf("请输入密码: ");
    fgets(password, sizeof(password), stdin);
    trimNewline(password);

    // 在链表中查找用户
    User *current = head;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 &&
            strcmp(current->password, password) == 0 &&
            current->isAdmin == isAdmin) {
            if (isAdmin) {
                printf("管理员登录成功！欢迎, %s。\n", current->name);
            } else {
                printf("登录成功！欢迎, %s。\n", current->name);
            }
            return current;
        }
        current = current->next;
    }

    if (isAdmin) {
        printf("管理员用户名或密码错误。\n");
    } else {
        printf("用户名或密码错误。\n");
    }
    return NULL;
}

// 显示个人信息
void displayUserInfo(User *user) {
    printf("\n=== 个人信息 ===\n");
    printf("姓名: %s\n", user->name);
    printf("用户名: %s\n", user->username);
    printf("打卡天数: %d\n", user->checkin_days);
    printf("总计背了多少单词: %d\n", user->total_words);
    printf("错题数: %d\n", user->wrong_words);
    printf("正确率: %.2f%%\n", user->correct_rate); // 显示正确率
    printf("上一次打卡时间: %s\n", user->last_checkin_time);
}

// 打卡功能
void checkIn(User *user) {
    // 获取当前时间
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char current_date[11]; // 格式: YYYY-MM-DD
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", t);

    // 提取上一次打卡的日期
    char last_date[11];
    if (strcmp(user->last_checkin_time, "未打卡") != 0) {
        strncpy(last_date, user->last_checkin_time, 10);
        last_date[10] = '\0';
    } else {
        strcpy(last_date, "0000-00-00");
    }

    // 比较当前日期和上一次打卡日期
    if (strcmp(current_date, last_date) == 0) {
        printf("今天已经打过卡了，请明天再来。\n");
        return;
    }

    // 更新用户信息
    user->checkin_days += 1;

    // 更新打卡时间
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);
    strcpy(user->last_checkin_time, time_str);

    // 统计已学过的单词数
    int already_count = 0;
    FILE *alreadyFile = fopen("already.txt", "r");
    if (alreadyFile != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), alreadyFile)) {
            if (strlen(line) > 1) { // 排除空行
                already_count++;
            }
        }
        fclose(alreadyFile);
    }

    // 统计错误的单词数
    int wrong_count = 0;
    FILE *wrongFile = fopen("wrong.txt", "r");
    if (wrongFile != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), wrongFile)) {
            if (strlen(line) > 1) { // 排除空行
                wrong_count++;
            }
        }
        fclose(wrongFile);
    }

    // 计算总学习单词数
    int total_learned = already_count + wrong_count;
    user->total_words = total_learned;

    // 计算正确率
    if (total_learned > 0) {
        user->correct_rate = ((float)(total_learned - wrong_count) / total_learned) * 100.0f;
    } else {
        user->correct_rate = 0.0f;
    }

    // 保存所有用户和管理员到文件
    if (saveAll() != 0) {
        printf("打卡失败，无法保存用户数据。\n");
        return;
    }

    // 记录打卡日志
    appendCheckInLog(user);

    printf("打卡成功！\n");
    printf("总学习单词数: %d\n", user->total_words);
    printf("正确率: %.2f%%\n", user->correct_rate);
}

// 将打卡信息追加到打卡日志文件
void appendCheckInLog(User *user) {
    FILE *file = fopen("checkin.txt", "a");
    if (file == NULL) {
        perror("无法打开打卡日志文件");
        return;
    }

    fprintf(file, "姓名: %s, 用户名: %s, 打卡天数: %d, 上一次打卡时间: %s\n",
        user->name,
        user->username,
        user->checkin_days,
        user->last_checkin_time);
    fclose(file);
}

// 释放链表内存
void freeUsers() {
    User *current = head;
    while (current != NULL) {
        User *temp = current;
        current = current->next;
        free(temp);
    }
}

// 从指定词库文件加载单词到数组（修改后的 loadWords）
int loadWords() {
#ifdef _WIN32
    const char *wordPath = "word_libraries\\cet.txt";
#else
    const char *wordPath = "word_libraries/cet.txt";
#endif

    FILE *file = fopen(wordPath, "r"); // 加载 'word_libraries/cet.txt' 文件
    if (file == NULL) {
        perror("无法打开 'word_libraries/cet.txt' 文件");
        return -1;
    }

    // 统计单词数量
    wordCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) { // 排除空行
            wordCount++;
        }
    }

    if (wordCount == 0) {
        printf("'word_libraries\\cet.txt' 文件中没有有效的单词。\n");
        fclose(file);
        return -1;
    }

    // 分配内存
    wordList = (Word *)malloc(sizeof(Word) * wordCount);
    if (wordList == NULL) {
        perror("内存分配失败");
        fclose(file);
        return -1;
    }

    // 读取所有单词
    rewind(file);
    int index = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) <= 1) continue; // 排除空行

        trimNewline(line);

        // 使用第一个空格分隔英文和中文
        char *space = strchr(line, ' ');
        if (space == NULL) {
            strcpy(wordList[index].english, line);
            strcpy(wordList[index].chinese, "未知");
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

// 释放单词内存
void freeWords() {
    if (wordList != NULL) {
        free(wordList);
        wordList = NULL;
    }
}

// 浏览生词本功能
void viewShengciBen() {
    FILE *file = fopen("shengciben.txt", "r");
    if (file == NULL) {
        printf("生词本文件不存在或为空。\n");
        return;
    }

    char line[256];
    int count = 0;
    printf("\n=== 生词本 ===\n");
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) > 0) {
            printf("%d. %s\n", ++count, line);
        }
    }

    if (count == 0) {
        printf("生词本为空。\n");
    }

    fclose(file);
}

// 背诵单词功能，简化为两种模式
void reciteWords(User *user) {
    if (wordCount == 0) {
        printf("词库为空，无法进行背诵。\n");
        return;
    }

    printf("\n=== 背诵单词 ===\n");
    printf("请选择背诵模式:\n");
    printf("1. 根据英文写中文\n");
    printf("2. 根据中文写英文\n");
    printf("请输入选择: ");

    int mode;
    if (scanf("%d", &mode) != 1) {
        clearInputBuffer();
        printf("无效的输入，请输入数字。\n");
        return;
    }
    clearInputBuffer(); // 清除输入缓冲区

    if (mode < 1 || mode > 2) {
        printf("无效的选择，请重新选择。\n");
        return;
    }

    // 创建一个索引数组用于选择单词
    int *indices = (int *)malloc(sizeof(int) * wordCount);
    if (indices == NULL) {
        perror("内存分配失败");
        return;
    }

    for (int i = 0; i < wordCount; i++) {
        indices[i] = i;
    }

    // 根据模式决定是否打乱顺序（这里统一打乱）
    for (int i = wordCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    printf("\n=== 背诵开始 ===\n");

    int totalQuestions = 10; // 设置背诵的总题数
    if (wordCount < 10) {
        totalQuestions = wordCount;
    }

    for (int i = 0; i < totalQuestions; i++) {
        int idx = indices[i];
        char question[100];
        char correctAnswer[100];
        char prompt[100];

        if (mode == 1) {
            // 根据英文写中文
            strcpy(question, wordList[idx].english);
            strcpy(correctAnswer, wordList[idx].chinese);
            snprintf(prompt, sizeof(prompt), "请写出以下英文单词的中文:\n%s: ", question);
        } else {
            // 根据中文写英文
            strcpy(question, wordList[idx].chinese);
            strcpy(correctAnswer, wordList[idx].english);
            snprintf(prompt, sizeof(prompt), "请写出以下中文单词的英文:\n%s: ", question);
        }

        printf("%s", prompt);

        char answer[100];
        fgets(answer, sizeof(answer), stdin);
        trimNewline(answer);

        // 检查答案（忽略大小写）
        if (strcasecmp(answer, correctAnswer) == 0) {
            printf("正确！\n");
            // 记录已学过的单词
            FILE *alreadyFile = fopen("already.txt", "a");
            if (alreadyFile != NULL) {
                fprintf(alreadyFile, "%s %s\n", wordList[idx].english, wordList[idx].chinese);
                fclose(alreadyFile);
            } else {
                perror("无法打开already.txt");
            }
        } else {
            printf("错误。正确答案是: %s\n", correctAnswer);
            // 增加错误单词计数
            user->wrong_words += 1;
            // 记录错误单词
            FILE *wrongFile = fopen("wrong.txt", "a");
            if (wrongFile != NULL) {
                fprintf(wrongFile, "%s %s\n", wordList[idx].english, wordList[idx].chinese);
                fclose(wrongFile);
            } else {
                perror("无法打开wrong.txt");
            }
        }

        // 询问操作选项
        printf("请选择操作:\n");
        printf("1. 下一题\n");
        printf("2. 加入生词本\n");
        printf("3. 退出背诵\n");
        printf("请输入选择: ");

        int op;
        if (scanf("%d", &op) != 1) {
            clearInputBuffer();
            printf("无效的输入，默认继续下一题。\n");
            continue;
        }
        clearInputBuffer(); // 清除输入缓冲区

        switch (op) {
        case 1:
            // 继续下一题
            break;
        case 2:
            addToShengciBen(wordList[idx].english, wordList[idx].chinese);
            printf("已将单词 '%s' 添加到生词本。\n", wordList[idx].english);
            // 新增：选择下一步操作
            while (1) {
                printf("请选择操作:\n");
                printf("1. 下一题\n");
                printf("2. 退出背诵\n");
                printf("请输入选择: ");
                int subOp;
                if (scanf("%d", &subOp) != 1) {
                    clearInputBuffer();
                    printf("无效的输入，请输入数字。\n");
                    continue;
                }
                clearInputBuffer(); // 清除输入缓冲区

                if (subOp == 1) {
                    // 继续下一题
                    break;
                } else if (subOp == 2) {
                    printf("已退出背诵。\n");
                    free(indices);
                    // 保存用户和管理员数据
                    if (saveAll() != 0) {
                        printf("保存用户数据失败。\n");
                    } else {
                        printf("用户数据已更新。\n");
                    }
                    return;
                } else {
                    printf("无效的选择，请重新输入。\n");
                }
            }
            break;
        case 3:
            printf("已退出背诵。\n");
            free(indices);
            // 保存用户和管理员数据
            if (saveAll() != 0) {
                printf("保存用户数据失败。\n");
            } else {
                printf("用户数据已更新。\n");
            }
            return; // 退出背诵
        default:
            printf("无效的选择，继续下一题。\n");
        }
    }

    free(indices);

    // 保存用户和管理员数据
    if (saveAll() != 0) {
        printf("保存用户数据失败。\n");
    } else {
        printf("背诵完成，用户数据已更新。\n");
    }
}

// 默写功能
void dictation(User *user) {
    if (wordCount < 20) {
        printf("单词数量不足20个，当前单词数: %d\n", wordCount);
        return;
    }

    // 创建一个索引数组用于随机选择
    int *indices = (int *)malloc(sizeof(int) * wordCount);
    if (indices == NULL) {
        perror("内存分配失败");
        return;
    }

    for (int i = 0; i < wordCount; i++) {
        indices[i] = i;
    }

    // 随机打乱索引数组
    for (int i = wordCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // 交换
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // 选择前20个索引
    printf("\n=== 默写开始 ===\n");
    printf("请输入单词的英文翻译，输入 'exit' 或 'quit' 可以提前结束默写。\n");
    for (int i = 0; i < 20; i++) {
        int idx = indices[i];
        printf("请写出以下中文单词的英文:\n%s: ", wordList[idx].chinese);

        char answer[50];
        fgets(answer, sizeof(answer), stdin);
        trimNewline(answer);

        // 检查是否输入了终止命令
        if (strcasecmp(answer, "exit") == 0 || strcasecmp(answer, "quit") == 0) {
            printf("已提前结束默写。\n");
            break;
        }

        // 检查答案（忽略大小写）
        if (strcasecmp(answer, wordList[idx].english) == 0) {
            printf("正确！\n");
            // 追加到 already.txt
            FILE *alreadyFile = fopen("already.txt", "a");
            if (alreadyFile != NULL) {
                fprintf(alreadyFile, "%s %s\n", wordList[idx].english, wordList[idx].chinese);
                fclose(alreadyFile);
            } else {
                perror("无法打开already.txt");
            }
        } else {
            printf("错误。正确答案是: %s\n", wordList[idx].english);
            // 增加错误单词计数
            user->wrong_words += 1;
            // 追加到 wrong.txt
            FILE *wrongFile = fopen("wrong.txt", "a");
            if (wrongFile != NULL) {
                fprintf(wrongFile, "%s %s\n", wordList[idx].english, wordList[idx].chinese);
                fclose(wrongFile);
            } else {
                perror("无法打开wrong.txt");
            }
        }

        // 新增部分：询问是否将单词加入生词本
        printf("是否将该单词加入生词本？(y/n): ");
        char choice;
        scanf(" %c", &choice);
        clearInputBuffer(); // 清除输入缓冲区
        if (choice == 'y' || choice == 'Y') {
            addToShengciBen(wordList[idx].english, wordList[idx].chinese);
            printf("已将单词 '%s' 添加到生词本。\n", wordList[idx].english);
        }
    }

    free(indices);

    // 保存用户和管理员数据
    if (saveAll() != 0) {
        printf("保存用户数据失败。\n");
    } else {
        printf("默写完成，用户数据已更新。\n");
    }
}

// 新增函数：将单词添加到生词本
void addToShengciBen(const char *english, const char *chinese) {
    FILE *file = fopen("shengciben.txt", "a");
    if (file == NULL) {
        perror("无法打开shengciben.txt");
        return;
    }
    fprintf(file, "%s %s\n", english, chinese);
    fclose(file);
}

// 查看错误单词
void viewWrongWords() {
    FILE *file = fopen("wrong.txt", "r");
    if (file == NULL) {
        printf("错误单词列表不存在。\n");
        return;
    }

    char line[256];
    int count = 0;
    printf("\n=== 错误单词列表 ===\n");
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) > 0) {
            printf("%d. %s\n", ++count, line);
        }
    }

    if (count == 0) {
        printf("错误单词列表为空。\n");
    }

    fclose(file);
}

// 查看已学过的单词
void viewAlreadyLearnedWords() {
    FILE *file = fopen("already.txt", "r");
    if (file == NULL) {
        printf("已学过的单词列表不存在。\n");
        return;
    }

    char line[256];
    int count = 0;
    printf("\n=== 已学过的单词列表 ===\n");
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) > 0) {
            printf("%d. %s\n", ++count, line);
        }
    }

    if (count == 0) {
        printf("已学过的单词列表为空。\n");
    }

    fclose(file);
}

// 错题单词训练功能
void wrongWordTraining(User *user) {
    FILE *file = fopen("wrong.txt", "r");
    if (file == NULL) {
        printf("错题本不存在或为空。\n");
        return;
    }

    // 读取所有错题到数组
    char **wrongWords = NULL;
    int wrongCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) > 0) {
            wrongWords = (char **)realloc(wrongWords, sizeof(char *) * (wrongCount + 1));
            if (wrongWords == NULL) {
                perror("内存分配失败");
                fclose(file);
                return;
            }
            wrongWords[wrongCount] = strdup(line);
            if (wrongWords[wrongCount] == NULL) {
                perror("内存分配失败");
                fclose(file);
                return;
            }
            wrongCount++;
        }
    }
    fclose(file);

    if (wrongCount == 0) {
        printf("错题本为空，无需训练。\n");
        free(wrongWords);
        return;
    }

    // 决定训练的单词数量
    int trainCount = wrongCount < 20 ? wrongCount : 20;

    // 创建一个索引数组用于随机选择
    int *indices = (int *)malloc(sizeof(int) * wrongCount);
    if (indices == NULL) {
        perror("内存分配失败");
        // 释放wrongWords
        for (int i = 0; i < wrongCount; i++) {
            free(wrongWords[i]);
        }
        free(wrongWords);
        return;
    }

    for (int i = 0; i < wrongCount; i++) {
        indices[i] = i;
    }

    // 随机打乱索引数组
    for (int i = wrongCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // 交换
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // 选择前trainCount个索引进行训练
    printf("\n=== 错题单词训练开始 ===\n");
    int *correctIndices = (int *)malloc(sizeof(int) * trainCount);
    if (correctIndices == NULL) {
        perror("内存分配失败");
        // 释放资源
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
        // 分割英文和中文
        char *english = strtok(wrongWords[idx], " ");
        char *chinese = strtok(NULL, "");

        if (english == NULL || chinese == NULL) {
            printf("格式错误: %s\n", wrongWords[idx]);
            continue;
        }

        printf("请写出以下中文单词的英文:\n%s: ", chinese);

        char answer[50];
        fgets(answer, sizeof(answer), stdin);
        trimNewline(answer);

        // 检查答案（忽略大小写）
        if (strcasecmp(answer, english) == 0) {
            printf("正确！\n");
            correctIndices[correctCount++] = idx;
            // 记录已学过的单词
            FILE *alreadyFile = fopen("already.txt", "a");
            if (alreadyFile != NULL) {
                fprintf(alreadyFile, "%s %s\n", english, chinese);
                fclose(alreadyFile);
            } else {
                perror("无法打开already.txt");
            }
        } else {
            printf("错误。正确答案是: %s\n", english);
            // 增加错误单词计数
            user->wrong_words += 1;
        }

        // 询问用户下一步操作
        while (1) {
            printf("\n请选择操作:\n");
            printf("1. 下一题\n");
            printf("2. 退出复习\n");
            printf("请输入选择: ");
            int op;
            if (scanf("%d", &op) != 1) {
                clearInputBuffer();
                printf("无效的输入，请输入数字。\n");
                continue;
            }
            clearInputBuffer(); // 清除输入缓冲区

            if (op == 1) {
                // 继续下一题
                break;
            } else if (op == 2) {
                printf("已退出错题复习。\n");
                // 保存用户和管理员数据
                if (saveAll() != 0) {
                    printf("保存用户数据失败。\n");
                } else {
                    printf("用户数据已更新。\n");
                }
                // 移除正确回答的单词
                if (correctCount > 0) {
                    // 创建一个新的错题数组，排除正确的单词
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
                                perror("内存分配失败");
                                break;
                            }
                            newWrongWords[newWrongCount] = strdup(wrongWords[j]);
                            if (newWrongWords[newWrongCount] == NULL) {
                                perror("内存分配失败");
                                break;
                            }
                            newWrongCount++;
                        }
                    }

                    // 重新写入wrong.txt
                    FILE *newWrongFile = fopen("wrong.txt", "w");
                    if (newWrongFile == NULL) {
                        perror("无法打开wrong.txt进行写入");
                    } else {
                        for (int j = 0; j < newWrongCount; j++) {
                            fprintf(newWrongFile, "%s\n", newWrongWords[j]);
                            free(newWrongWords[j]);
                        }
                        fclose(newWrongFile);
                        free(newWrongWords);
                        printf("已更新错题本。\n");
                    }
                }

                // 释放资源
                free(indices);
                free(correctIndices);
                for (int j = 0; j < wrongCount; j++) {
                    free(wrongWords[j]);
                }
                free(wrongWords);
                return;
            } else {
                printf("无效的选择，请重新输入。\n");
            }
        }
    }

    // 移除正确回答的单词
    if (correctCount > 0) {
        // 创建一个新的错题数组，排除正确的单词
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
                    perror("内存分配失败");
                    break;
                }
                newWrongWords[newWrongCount] = strdup(wrongWords[i]);
                if (newWrongWords[newWrongCount] == NULL) {
                    perror("内存分配失败");
                    break;
                }
                newWrongCount++;
            }
        }

        // 重新写入wrong.txt
        FILE *newWrongFile = fopen("wrong.txt", "w");
        if (newWrongFile == NULL) {
            perror("无法打开wrong.txt进行写入");
        } else {
            for (int i = 0; i < newWrongCount; i++) {
                fprintf(newWrongFile, "%s\n", newWrongWords[i]);
                free(newWrongWords[i]);
            }
            fclose(newWrongFile);
            free(newWrongWords);
            printf("训练完成，已更新错题本。\n");
        }
    } else {
        printf("没有正确回答的单词，无需更新错题本。\n");
    }

    // 释放资源
    free(indices);
    free(correctIndices);
    for (int i = 0; i < wrongCount; i++) {
        free(wrongWords[i]);
    }
    free(wrongWords);

    // 保存用户和管理员数据
    if (saveAll() != 0) {
        printf("保存用户数据失败。\n");
    } else {
        printf("错题单词训练完成，用户数据已更新。\n");
    }
}

// 搜索功能
void searchWord() {
    clearInputBuffer(); // 清除输入缓冲区

    printf("\n=== 单词搜索 ===\n");
    printf("请输入要搜索的英文单词: ");
    char search[50];
    fgets(search, sizeof(search), stdin);
    trimNewline(search);

    // 遍历wordList查找
    int found = 0;
    for (int i = 0; i < wordCount; i++) {
        if (strcasecmp(wordList[i].english, search) == 0) {
            printf("单词: %s\n中文意思: %s\n", wordList[i].english, wordList[i].chinese);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("未找到单词 '%s' 的中文意思。\n", search);
    }
}

// 趣味挑战模式功能，使用两种模式
void challengeMode(User *user) {
    if (wordCount == 0) {
        printf("单词列表为空，无法进行挑战模式。\n");
        return;
    }

    printf("\n=== 趣味挑战模式 ===\n");
    printf("请选择挑战模式:\n");
    printf("1. 根据英文写中文\n");
    printf("2. 根据中文写英文\n");
    printf("请输入选择: ");

    int mode;
    if (scanf("%d", &mode) != 1) {
        clearInputBuffer();
        printf("无效的输入，请输入数字。\n");
        return;
    }
    clearInputBuffer(); // 清除输入缓冲区

    if (mode < 1 || mode > 2) {
        printf("无效的选择，请重新选择。\n");
        return;
    }

    printf("您有60秒的时间来默写尽可能多的单词。\n");
    printf("每答对一个单词加10分，答错扣5分。\n");
    printf("挑战即将开始，准备好了吗？按回车键开始...");
    getchar(); // 等待用户按回车

    int score = 0;
    time_t start_time = time(NULL);
    time_t current_time;

    // 生成一个随机序列以避免重复
    int *sequence = (int *)malloc(sizeof(int) * wordCount);
    if (sequence == NULL) {
        perror("内存分配失败");
        return;
    }
    for (int i = 0; i < wordCount; i++) {
        sequence[i] = i;
    }
    // 打乱序列
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
            printf("\n时间到！挑战结束。\n");
            break;
        }

        if (index >= wordCount) {
            // 如果所有单词都已经挑战过，重新打乱
            for (int i = 0; i < wordCount; i++) {
                sequence[i] = i;
            }
            // 打乱序列
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
            // 根据英文写中文
            strcpy(question, wordList[word_idx].english);
            strcpy(correctAnswer, wordList[word_idx].chinese);
            snprintf(prompt, sizeof(prompt), "请写出以下英文单词的中文:\n%s: ", question);
        } else {
            // 根据中文写英文
            strcpy(question, wordList[word_idx].chinese);
            strcpy(correctAnswer, wordList[word_idx].english);
            snprintf(prompt, sizeof(prompt), "请写出以下中文单词的英文:\n%s: ", question);
        }

        printf("\n%s", prompt);

        char answer[50];
        fgets(answer, sizeof(answer), stdin);
        trimNewline(answer);

        // 检查答案（忽略大小写）
        if (strcasecmp(answer, correctAnswer) == 0) {
            printf("正确！+10分\n");
            score += 10;
            // 记录已学过的单词
            FILE *alreadyFile = fopen("already.txt", "a");
            if (alreadyFile != NULL) {
                fprintf(alreadyFile, "%s %s\n", wordList[word_idx].english, wordList[word_idx].chinese);
                fclose(alreadyFile);
            } else {
                perror("无法打开already.txt");
            }
        } else {
            printf("错误。正确答案是: %s。-5分\n", correctAnswer);
            score -= 5;
            if (score < 0) score = 0; // 防止分数为负
            // 记录错误的单词
            user->wrong_words += 1;
            FILE *wrongFile = fopen("wrong.txt", "a");
            if (wrongFile != NULL) {
                fprintf(wrongFile, "%s %s\n", wordList[word_idx].english, wordList[word_idx].chinese);
                fclose(wrongFile);
            } else {
                perror("无法打开wrong.txt");
            }
        }
    }

    free(sequence);

    // 保存用户和管理员数据
    if (saveAll() != 0) {
        printf("保存用户数据失败。\n");
    }

    // 将姓名和分数存入rank.txt
    FILE *rankFile = fopen("rank.txt", "a");
    if (rankFile == NULL) {
        perror("无法打开rank.txt");
    } else {
        fprintf(rankFile, "%s,%d\n", user->name, score);
        fclose(rankFile);
    }

    printf("您的得分: %d\n", score);
}

// 比较函数用于qsort
int compareRankEntries(const void *a, const void *b) {
    RankEntry *entryA = (RankEntry *)a;
    RankEntry *entryB = (RankEntry *)b;
    return entryB->score - entryA->score; // 从高到低排序
}

// 排行榜功能
void displayRanking() {
    FILE *file = fopen("rank.txt", "r");
    if (file == NULL) {
        printf("排行榜文件不存在。\n");
        return;
    }

    // 读取排行榜数据
    RankEntry *entries = NULL;
    int entryCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        trimNewline(line);
        if (strlen(line) == 0) continue;

        // 分割姓名和分数
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        char *name = token;

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        int score = atoi(token);

        // 添加到排行榜数组
        entries = (RankEntry *)realloc(entries, sizeof(RankEntry) * (entryCount + 1));
        if (entries == NULL) {
            perror("内存分配失败");
            fclose(file);
            return;
        }
        strcpy(entries[entryCount].name, name);
        entries[entryCount].score = score;
        entryCount++;
    }
    fclose(file);

    if (entryCount == 0) {
        printf("排行榜为空。\n");
        free(entries);
        return;
    }

    // 按分数从高到低排序
    qsort(entries, entryCount, sizeof(RankEntry), compareRankEntries);

    // 显示排行榜
    printf("\n=== 排行榜 ===\n");
    printf("%-5s %-20s %-10s\n", "排名", "姓名", "分数");
    for (int i = 0; i < entryCount && i < 10; i++) { // 显示前10名
        printf("%-5d %-20s %-10d\n", i + 1, entries[i].name, entries[i].score);
    }

    free(entries);
}

// 浏览用户信息（管理员功能）
void browseUsers() {
    printf("\n=== 用户信息列表 ===\n");
    User *current = head;
    int count = 0;
    while (current != NULL) {
        if (!current->isAdmin) { // 仅显示普通用户
            printf("\n--- 用户 %d ---\n", ++count);
            printf("姓名: %s\n", current->name);
            printf("用户名: %s\n", current->username);
            printf("打卡天数: %d\n", current->checkin_days);
            printf("总计背了多少单词: %d\n", current->total_words);
            printf("错题数: %d\n", current->wrong_words);
            printf("正确率: %.2f%%\n", current->correct_rate);
            printf("上一次打卡时间: %s\n", current->last_checkin_time);
        }
        current = current->next;
    }

    if (count == 0) {
        printf("当前没有普通用户。\n");
    }
}

// 删除用户功能（管理员）
void deleteUser() {
    char usernameToDelete[50];
    printf("\n请输入要删除的用户名: ");
    fgets(usernameToDelete, sizeof(usernameToDelete), stdin);
    trimNewline(usernameToDelete);

    if (strlen(usernameToDelete) == 0) {
        printf("用户名不能为空。\n");
        return;
    }

    User *current = head;
    User *previous = NULL;

    while (current != NULL) {
        if (strcmp(current->username, usernameToDelete) == 0) {
            if (current->isAdmin) {
                printf("无法删除管理员用户。\n");
                return;
            }
            // 找到要删除的用户
            if (previous == NULL) {
                // 删除头节点
                head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            printf("用户 '%s' 已成功删除。\n", usernameToDelete);
            // 保存所有用户
            if (saveAllUsers() != 0) {
                printf("删除用户后，保存用户数据失败。\n");
            }
            return;
        }
        previous = current;
        current = current->next;
    }

    printf("未找到用户名为 '%s' 的用户。\n", usernameToDelete);
}

// 管理员菜单
void adminMenu(User *adminUser) {
    int choice;
    while (1) {
        printf("\n=== 管理员菜单 ===\n");
        printf("1. 用户管理\n");
        printf("2. 管理题库\n");
        printf("3. 返回主菜单\n");
        printf("请输入选择: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("无效的输入，请输入数字。\n");
            continue;
        }
        clearInputBuffer(); // 清除输入缓冲区

        switch (choice) {
        case 1:
            // 用户管理子菜单
            {
                int userChoice;
                while (1) {
                    printf("\n=== 用户管理 ===\n");
                    printf("1. 浏览所有用户\n");
                    printf("2. 删除用户\n");
                    printf("3. 返回管理员菜单\n");
                    printf("请输入选择: ");

                    if (scanf("%d", &userChoice) != 1) {
                        clearInputBuffer();
                        printf("无效的输入，请输入数字。\n");
                        continue;
                    }
                    clearInputBuffer(); // 清除输入缓冲区

                    switch (userChoice) {
                    case 1:
                        browseUsers();
                        break;
                    case 2:
                        // 调用删除用户功能
                        deleteUser();
                        break;
                    case 3:
                        // 返回管理员菜单
                        goto admin_menu_end;
                    default:
                        printf("无效的选择，请重新输入。\n");
                    }
                }
            }
        admin_menu_end:
            break;
        case 2:
            manageWordLibraries();
            break;
        case 3:
            printf("返回主菜单。\n");
            return;
        default:
            printf("无效的选择，请重新输入。\n");
        }
    }
}

// 管理题库功能
void manageWordLibraries() {
    int choice;
    while (1) {
        printf("\n=== 题库管理菜单 ===\n");
        printf("1. 列出所有题库\n");
        printf("2. 添加新的题库\n");
        printf("3. 删除现有题库\n");
        printf("4. 管理单个题库中的单词\n");
        printf("5. 返回管理员菜单\n");
        printf("请输入选择: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("无效的输入，请输入数字。\n");
            continue;
        }
        clearInputBuffer(); // 清除输入缓冲区

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
            printf("无效的选择，请重新输入。\n");
        }
    }
}

// 列出所有题库
void listWordLibraries() {
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char searchPath[150];
    snprintf(searchPath, sizeof(searchPath), "word_libraries\\*");

    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("无法打开 word_libraries 目录或目录为空。\n");
        return;
    } else {
        printf("\n=== 当前题库列表 ===\n");
        int count = 0;
        do {
            const char *name = findFileData.cFileName;
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
                continue;
            printf("%d. %s\n", ++count, name);
        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
        if (count == 0) {
            printf("当前没有任何题库。\n");
        }
    }
#else
    DIR *d;
    struct dirent *dir;
    d = opendir("word_libraries");
    if (d) {
        printf("\n=== 当前题库列表 ===\n");
        int count = 0;
        while ((dir = readdir(d)) != NULL) {
            // 排除 "." 和 ".."
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                continue;
            printf("%d. %s\n", ++count, dir->d_name);
        }
        closedir(d);
        if (count == 0) {
            printf("当前没有任何题库。\n");
        }
    } else {
        perror("无法打开 word_libraries 目录");
    }
#endif
}

// 添加新的题库
void addWordLibrary() {
    clearInputBuffer(); // 清除输入缓冲区
    char libraryName[100];
    printf("\n请输入新题库的名称 (例如: new_library.txt): ");
    fgets(libraryName, sizeof(libraryName), stdin);
    trimNewline(libraryName);

    // 检查文件名是否合法
    if (strlen(libraryName) == 0) {
        printf("题库名称不能为空。\n");
        return;
    }

    // 检查文件是否已经存在
    char filepath[150];
#ifdef _WIN32
    snprintf(filepath, sizeof(filepath), "word_libraries\\%s", libraryName);
#else
    snprintf(filepath, sizeof(filepath), "word_libraries/%s", libraryName);
#endif
    FILE *file = fopen(filepath, "r");
    if (file != NULL) {
        printf("题库 '%s' 已存在。\n", libraryName);
        fclose(file);
        return;
    }

    // 创建新的题库文件
    file = fopen(filepath, "w");
    if (file == NULL) {
        perror("无法创建新的题库文件");
        return;
    }
    fclose(file);
    printf("题库 '%s' 创建成功。\n", libraryName);
}

// 删除现有题库
void deleteWordLibrary() {
    clearInputBuffer(); // 清除输入缓冲区
    char libraryName[100];
    printf("\n请输入要删除的题库名称 (例如: new_library.txt): ");
    fgets(libraryName, sizeof(libraryName), stdin);
    trimNewline(libraryName);

    // 检查文件是否存在
    char filepath[150];
#ifdef _WIN32
    snprintf(filepath, sizeof(filepath), "word_libraries\\%s", libraryName);
#else
    snprintf(filepath, sizeof(filepath), "word_libraries/%s", libraryName);
#endif
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        printf("题库 '%s' 不存在。\n", libraryName);
        return;
    }
    fclose(file);

    // 确认删除
    printf("确定要删除题库 '%s' 吗？ (y/n): ", libraryName);
    char confirm;
    scanf(" %c", &confirm);
    clearInputBuffer(); // 清除输入缓冲区
    if (confirm == 'y' || confirm == 'Y') {
#ifdef _WIN32
        if (remove(filepath) == 0) {
            printf("题库 '%s' 删除成功。\n", libraryName);
        } else {
            perror("删除题库失败");
        }
#else
        if (remove(filepath) == 0) {
            printf("题库 '%s' 删除成功。\n", libraryName);
        } else {
            perror("删除题库失败");
        }
#endif
    } else {
        printf("取消删除。\n");
    }
}

// 管理单个题库中的单词
void manageSingleWordLibrary() {
    clearInputBuffer(); // 清除输入缓冲区
    char libraryName[100];
    printf("\n请输入要管理的题库名称 (例如: new_library.txt): ");
    fgets(libraryName, sizeof(libraryName), stdin);
    trimNewline(libraryName);

    // 检查文件是否存在
    char filepath[150];
#ifdef _WIN32
    snprintf(filepath, sizeof(filepath), "word_libraries\\%s", libraryName);
#else
    snprintf(filepath, sizeof(filepath), "word_libraries/%s", libraryName);
#endif
    FILE *file = fopen(filepath, "r+");
    if (file == NULL) {
        printf("题库 '%s' 不存在。\n", libraryName);
        return;
    }
    fclose(file);

    int choice;
    while (1) {
        printf("\n=== 管理题库 '%s' ===\n", libraryName);
        printf("1. 列出所有单词\n");
        printf("2. 添加新单词\n");
        printf("3. 删除单词\n");
        printf("4. 返回题库管理菜单\n");
        printf("请输入选择: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("无效的输入，请输入数字。\n");
            continue;
        }
        clearInputBuffer(); // 清除输入缓冲区

        switch (choice) {
        case 1:
            // 列出所有单词
            printf("\n=== 单词列表 ===\n");
            file = fopen(filepath, "r");
            if (file == NULL) {
                perror("无法打开题库文件");
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
                    printf("题库为空。\n");
                }
            }
            fclose(file);
            break;
        case 2:
            // 添加新单词
            {
                char english[50], chinese[100];
                printf("\n请输入英文单词: ");
                fgets(english, sizeof(english), stdin);
                trimNewline(english);
                printf("请输入中文意思: ");
                fgets(chinese, sizeof(chinese), stdin);
                trimNewline(chinese);

                // 检查是否已存在
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
                        printf("单词 '%s' 已存在于题库中。\n", english);
                        break;
                    }
                }

                // 追加新单词
                file = fopen(filepath, "a");
                if (file == NULL) {
                    perror("无法打开题库文件进行写入");
                    break;
                }
                fprintf(file, "%s %s\n", english, chinese);
                fclose(file);
                printf("单词 '%s' 添加成功。\n", english);
            }
            break;
        case 3:
            // 删除单词
            {
                char english[50];
                printf("\n请输入要删除的英文单词: ");
                fgets(english, sizeof(english), stdin);
                trimNewline(english);

                // 读取所有单词并重写文件，排除要删除的单词
                FILE *readFile = fopen(filepath, "r");
                if (readFile == NULL) {
                    perror("无法打开题库文件");
                    break;
                }

                // 临时文件
                char tempPath[160];
#ifdef _WIN32
                snprintf(tempPath, sizeof(tempPath), "word_libraries\\temp_%s", libraryName);
#else
                snprintf(tempPath, sizeof(tempPath), "word_libraries/temp_%s", libraryName);
#endif
                FILE *tempFile = fopen(tempPath, "w");
                if (tempFile == NULL) {
                    perror("无法创建临时文件");
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
                        continue; // 跳过要删除的单词
                    }
                    fprintf(tempFile, "%s %s\n", existingEnglish, existingChinese ? existingChinese : "");
                }

                fclose(readFile);
                fclose(tempFile);

                if (found) {
                    // 替换原文件
                    if (remove(filepath) != 0) {
                        perror("无法删除原题库文件");
                        remove(tempPath);
                        break;
                    }
                    if (rename(tempPath, filepath) != 0) {
                        perror("无法重命名临时文件");
                        break;
                    }
                    printf("单词 '%s' 删除成功。\n", english);
                } else {
                    printf("未找到单词 '%s'。\n", english);
                    remove(tempPath);
                }
            }
            break;
        case 4:
            // 返回题库管理菜单
            return;
        default:
            printf("无效的选择，请重新输入。\n");
        }
    }
}
