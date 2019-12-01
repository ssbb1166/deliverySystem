#define MAX_MSG_SIZE			100
#define PASSWD_LEN				4

int str_createSystem(char* filepath);
void str_freeSystem(void);

void str_printStorageStatus(void);
int str_checkStorage(int x,int y);
int str_pushToStorage(int x, int y, int nBuilding, int nRoom, char msg[MAX_MSG_SIZE+1], char passwd[PASSWD_LEN+1]);
int str_extractStorage(int x, int y);
int str_findStorage(int nBuilding, int nRoom);
int str_backupSystem(char* filepath);
