#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"


/* definition of storage cell structure -----------------------
 * members:
 * int building: building number of the destination
 * int room: room number of the destination
 * int cnt: number of packages in the cell
 * char passwd[]: password setting (4 characters)
 * char *contents: package context (message string)
 * ------------------------------------------------------------ */
typedef struct {
    int building;
    int room;
    int cnt;
    char passwd[PASSWD_LEN+1];
    char *context;
} storage_t;


static storage_t** deliverySystem;          /* deliverySystem */
static int storedCnt = 0;                   /* number of cells occupied */
static int systemSize[2] = {0, 0};          /* row/column of the delivery system */
static char masterPassword[PASSWD_LEN+1];   /* master password */


/* inner functions ============================================ */

/* ------------------------------------------------------------
 * print the inside context of a specific cell
 * int x, int y: cell to print the context
 * ------------------------------------------------------------ */
static void printStorageInside(int x, int y)
{
    printf("\n------------------------------------------------------------------------\n");
    printf("------------------------------------------------------------------------\n");
    if (deliverySystem[x][y].cnt > 0)
        printf("<<<<<<<<<<<<<<<<<<<<<<<< : %s >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", deliverySystem[x][y].context);
    else
        printf("<<<<<<<<<<<<<<<<<<<<<<<< empty >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    printf("------------------------------------------------------------------------\n");
    printf("------------------------------------------------------------------------\n\n");
}


/* ------------------------------------------------------------
 * initialize the storage
 * set all the member variable as an initial value
 * and allocate memory to the context pointer
 * int x, int y: cell coordinate to be initialized
 * ------------------------------------------------------------ */
static void initStorage(int x, int y)
{
    /* set all the member variable as an initial value */
    deliverySystem[x][y].building = 0;
    deliverySystem[x][y].room = 0;
    deliverySystem[x][y].cnt = 0;
    strcpy(deliverySystem[x][y].passwd, masterPassword);

    /* allocate memory to the context pointer */
    deliverySystem[x][y].context = malloc(sizeof(char) * (MAX_MSG_SIZE+1));
    strcpy(deliverySystem[x][y].context, "\0");
}


/* ------------------------------------------------------------
 * get password input and check if it is correct for the cell (x,y)
 * int x, int y: cell for password check
 * return: 0 - password is matching, -1 - password is not matching
 * ------------------------------------------------------------ */
static int inputPasswd(int x, int y)
{
    char input[MAX_INPUT_SIZE+1];

    /* get password input */
    printf(" - input password for (%d, %d) storage : ", x, y);
    scanf("%s", input);

    /* check if input equals password (or master password) */
    if (strcmp(input, deliverySystem[x][y].passwd) &&
        strcmp(input, masterPassword)) {
        printf(" -----------> password is wrong!!\n");
        return -1;
    }

    return 0;
}


/* API function for main.c file =============================== */

/* ------------------------------------------------------------
 * create delivery system on the double pointer deliverySystem
 * char* filepath: filepath and name to read config parameters
 * (row, column, master password, past contexts of the delivery system)
 * return: 0 - successfully created, -1 - failed to create the system
 * ------------------------------------------------------------ */
int str_createSystem(char* filepath)
{
    int row, col;
    int x, y, nBuilding, nRoom;
    char passwd[PASSWD_LEN+1], msg[MAX_MSG_SIZE+1];
    FILE *fp;

    /* open the file */
    fp = fopen(filepath, "r");
    if (fp == NULL)
        return -1;

    /* read row, column, and master password */
    fscanf(fp, "%d %d", &systemSize[0], &systemSize[1]);
    fscanf(fp, "%s", masterPassword);

    /* allocate memory to the double pointer deliverySystem */
    deliverySystem = malloc(sizeof(storage_t *) * systemSize[0]);
    for (row = 0; row < systemSize[0]; row++)
        deliverySystem[row] = malloc(sizeof(storage_t) * systemSize[1]);

    /* initialize the storage */
    for (row = 0; row < systemSize[0]; row++)
        for (col = 0; col < systemSize[1]; col++)
            initStorage(row, col);

    /* read config parameters and put a package to the cell */
    while (1) {
        fscanf(fp, "%d %d %d %d %s %s", &x, &y, &nBuilding, &nRoom, passwd, msg);
        if (fgetc(fp) == EOF) break;
        str_pushToStorage(x, y, nBuilding, nRoom, msg, passwd);
    }

    /* close the file */
    fclose(fp);

    return 0;
}


/* ------------------------------------------------------------
 * free the memory of the deliverySystem
 * ------------------------------------------------------------ */
void str_freeSystem(void)
{
    int row, col;

    /* free the memory of the context */
    for (row = 0; row < systemSize[0]; row++)
        for (col = 0; col < systemSize[1]; col++)
            free(deliverySystem[row][col].context);

    /* free the memory of the deliverySystem */
    for (row = 0; row < systemSize[0]; row++)
        free(deliverySystem[row]);
    free(deliverySystem);
}


/* ------------------------------------------------------------
 * print the current state of the whole delivery system
 * (which cells are occupied and the destination of the each occupied cells)
 * ------------------------------------------------------------ */
void str_printStorageStatus(void)
{
    int row, col;
    printf("----------------------------- Delivery Storage System Status (%i occupied out of %i )-----------------------------\n\n", storedCnt, systemSize[0]*systemSize[1]);

    printf("\t");
    for (col = 0; col < systemSize[1]; col++)
        printf(" %i\t\t", col);
    printf("\n-----------------------------------------------------------------------------------------------------------------\n");

    for (row = 0; row < systemSize[0]; row++) {
        printf("%i|\t", row);
        for (col = 0; col < systemSize[1]; col++)
            if (deliverySystem[row][col].cnt > 0)
                printf("%i,%i\t|\t", deliverySystem[row][col].building, deliverySystem[row][col].room);
            else
                printf(" -  \t|\t");
        printf("\n");
    }
    printf("--------------------------------------- Delivery Storage System Status --------------------------------------------\n\n");
}


/* ------------------------------------------------------------
 * check if the input cell (x,y) is valid and whether it is occupied or not
 * ------------------------------------------------------------ */
int str_checkStorage(int x, int y)
{
    if (x < 0 || x >= systemSize[0])
        return -1;

    if (y < 0 || y >= systemSize[1])
        return -1;

    return deliverySystem[x][y].cnt;
}


/* ------------------------------------------------------------
 * put a package (msg) to the cell
 * input parameters
 * int x, int y: coordinate of the cell to put the package
 * int nBuilding, int nRoom: building and room numbers of the destination
 * char msg[]: package context (message string)
 * char passwd[]: password string (4 characters)
 * return: 0 - successfully put the package, -1 - failed to put
 * ------------------------------------------------------------ */
int str_pushToStorage(int x, int y, int nBuilding, int nRoom, char msg[MAX_MSG_SIZE+1], char passwd[PASSWD_LEN+1])
{
    /* check length of msg and password */
    if (strlen(msg) > 100 || strlen(passwd) > 4)
        return -1;

    /* put a package to the cell */
    deliverySystem[x][y].building = nBuilding;
    deliverySystem[x][y].room = nRoom;
    deliverySystem[x][y].cnt = 1;
    strcpy(deliverySystem[x][y].passwd, passwd);
    strcpy(deliverySystem[x][y].context, msg);

    storedCnt++;

    return 0;
}


/* ------------------------------------------------------------
 * extract the package context with password checking
 * after password checking, then put the msg string on the screen and re-initialize the storage
 * int x, int y: coordinate of the cell to extract
 * return: 0 - successfully extracted, -1 - failed to extract
 * ------------------------------------------------------------ */
int str_extractStorage(int x, int y)
{
    /* password checking */
    if (inputPasswd(x, y) != 0)
        return -1;

    /* put the msg string on the screen */
    printStorageInside(x, y);

    /* re-initialize the storage */
    initStorage(x, y);

    storedCnt--;

    return 0;
}


/* ------------------------------------------------------------
 * find my package from the storage
 * print all the cells (x,y) which has my package
 * int nBuilding, int nRoom: my building/room numbers
 * return: number of packages that the storage system has
 * ------------------------------------------------------------ */
int str_findStorage(int nBuilding, int nRoom)
{
    int row, col, cnt = 0;

    /* print the cells (x,y) */
    for (row = 0; row < systemSize[0]; row++)
        for (col = 0; col < systemSize[1]; col++)
            if (deliverySystem[row][col].building == nBuilding &&
                deliverySystem[row][col].room == nRoom) {
                printf(" -----------> Found a package in (%d, %d)\n", row, col);
                cnt++;
            }

    return cnt;
}


/* ------------------------------------------------------------
 * backup the delivery system context to the file system
 * char* filepath: filepath and name to write
 * return: 0 - backup was successfully done, -1 - failed to backup
 * ------------------------------------------------------------ */
int str_backupSystem(char* filepath)
{
    int row, col;
    FILE *fp;

    /* open the file */
    fp = fopen(filepath, "w");
    if (fp == NULL)
        return -1;

    /* write row, column, and master password */
    fprintf(fp, "%d %d\n", systemSize[0], systemSize[1]);
    fprintf(fp, "%s\n", masterPassword);

    /* backup the delivery system context */
    for (row = 0; row < systemSize[0]; row++)
        for (col = 0; col < systemSize[1]; col++)
            if (deliverySystem[row][col].cnt > 0)
                fprintf(fp, "%d %d %d %d %s %s\n", row, col,
                    deliverySystem[row][col].building,
                    deliverySystem[row][col].room,
                    deliverySystem[row][col].passwd,
                    deliverySystem[row][col].context);

    /* close the file */
    fclose(fp);

    return 0;
}
