#include <stdio.h>

#define MAX_LEN 80
/**
 * gdb要用input在txt裡面，使用時 用Redirect run < input.txt
 * 另外a_word亂碼 display追蹤 直到fgets執行完才有正確資料   
 */
int main(int argc, char *argv[]) {
    char a_word[MAX_LEN];

    printf("What's your name?\n");
    fgets(a_word, MAX_LEN, stdin);
    printf("Hey, %sI just really wanted to say hello to you.\nI hope you have a wonderful day.", a_word);

    return 0;
}