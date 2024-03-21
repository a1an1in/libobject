#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <libobject/mockery/mockery.h>

static int test_reg(TEST_ENTRY *entry, void *argc, void *argv)
{
    int status ,i;  
    int cflags = REG_EXTENDED;  
    regmatch_t pmatch[5];  
    const size_t nmatch = 5;  
    char matched[1024];
    regex_t reg;  
    const char * pattern = "([0-9A-Za-z\\-_\\.]+)@([0-9a-z]+\\.[a-z]{2,3})";  
    char * buf = "chenjiayi@126.com";
    int x;

    regcomp(&reg,pattern,cflags);//编译正则模式  
    status = regexec(&reg,buf,nmatch, pmatch, 0);//执行正则表达式和缓存的比较  
    if(status == REG_NOMATCH) { 
        printf("No match\n");  
    }  

    for (x = 0; x < 5 && pmatch[x].rm_so != -1; ++x) {
        int len = pmatch[x].rm_eo - pmatch[x].rm_so;
        memcpy(matched, buf + pmatch[x].rm_so, len);
        matched[len] = '\0';
        printf("matched:%s, len=%d \n", matched, len);
    }
    regfree(&reg);  
    return 0;
}
REGISTER_TEST_CMD(test_reg);

static int test_reg3(TEST_ENTRY *entry, void *argc, void *argv)
{
    regex_t re;
    regmatch_t subs[1024];
    char matched[1024];
    char src[1024]="This order was placed for QT3000, OK?";
    char pattern[1024] = ",";
    int x;

    int err = regcomp(&re, pattern, REG_EXTENDED);
    if (err) {
        printf("regex error");
        return 1;
    }

    const char *ptr = src;
    // 匹配模式字串以及子正则
    err = regexec(&re, ptr, 4, subs, 0);
    for (x = 0; x < 4 && subs[x].rm_so != -1; ++x) {
        int len = subs[x].rm_eo - subs[x].rm_so;
        memcpy(matched, ptr + subs[x].rm_so, len);
        matched[len] = '\0';
        printf("matched:%s, len=%d \n", matched, len);
    }

    regfree(&re);

    return 0;
}
REGISTER_TEST_CMD(test_reg3);

static int test_reg4(TEST_ENTRY *entry, void *argc, void *argv)
{
    char *bematch = "hhhericchd@gmail.com";
    char *pattern = "h{3,10}(.*)@.{5}.(.*)";
    char errbuf[1024];
    char match[100];
    regex_t reg;
    int err,nm = 10;
    regmatch_t pmatch[nm];
    int i;

    if(regcomp(&reg,pattern,REG_EXTENDED) < 0){
        regerror(err,&reg,errbuf,sizeof(errbuf));
        printf("err:%s\n",errbuf);
    }

    err = regexec(&reg,bematch,nm,pmatch,0);

    if(err == REG_NOMATCH){
        printf("no match\n");
        exit(-1);

    }else if(err){
        regerror(err,&reg,errbuf,sizeof(errbuf));
        printf("err:%s\n",errbuf);
        exit(-1);
    }

    for(i=0;i<10 && pmatch[i].rm_so!=-1;i++){
        int len = pmatch[i].rm_eo-pmatch[i].rm_so;
        if(len){
            memset(match,'\0',sizeof(match));
            memcpy(match,bematch+pmatch[i].rm_so,len);
            printf("%s\n",match);
        }
    }
    return 0;
}
REGISTER_TEST_CMD(test_reg4);
