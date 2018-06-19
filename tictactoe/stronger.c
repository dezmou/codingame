#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

# define CBLACK  "\33[30m"
# define CRED    "\33[31m"
# define CGREEN  "\33[32m"
# define CWHITE  "\33[37m"

typedef struct s_field Field;
typedef struct s_map Map;

struct s_map{
    unsigned short maps[3];
    char result;
    int totalHits;
    int posX;
    int posY;
    Field *field;
};

typedef struct s_place{
    int score;
    int computed;
    int x;
    int y;
    Map *map;
}Place;

struct s_field{
    Map maps[3][3];
    Map globalMap;
    Place places[81];
    int totalHits;
    int nbrPlaces;
    int nbrCovers[3];
};

int random_number(int min_num, int max_num){
    int result = (rand() % (max_num - min_num)) + min_num;
    return (char)result;
}

long long tim() {
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}


void copyField(Field *dest, Field *base){
    memcpy(dest, base, sizeof(Field));
    for (int y=0; y<3;y++)
        for (int x=0; x<3;x++){
            dest->maps[y][x].field = dest;
        }
    dest->globalMap.field = dest;
    for (int i=0; i< dest->nbrPlaces ; i++){
        dest->places[i].map = &dest->maps[base->places[i].map->posY][base->places[i].map->posX];
    }
}

void printField(Field *field){
    static char printable[9][9];
    for (int yy=0 ; yy<3; yy++){
        for (int xx=0 ; xx<3; xx++){
            for (int y=0 ; y<3; y++){
                for (int x=0 ; x<3; x++){
                    if (field->maps[yy][xx].maps[1] >> 8 - (x + (y * 3)) & 1 == 1){
                        printable[yy * 3 + y][xx * 3 + x] = 'X';
                    }else if (field->maps[yy][xx].maps[2] >> 8 - (x + (y * 3)) & 1 == 1){
                        printable[yy * 3 + y][xx * 3 + x] = '0';
                    }else{
                        printable[yy * 3 + y][xx * 3 + x] = '.';
                    }
                }
            }
        }
    }
    for (int yy=0 ; yy<9; yy++){
        if (yy % 3 == 0) printf(" -----------------------------\n");
        for (int xx=0; xx<9; xx++){
            if (xx % 3 == 0) printf("%s|",CWHITE);
            if (printable[yy][xx] == 'X') printf(" %s%c ",CGREEN ,printable[yy][xx]);
            else if (printable[yy][xx] == '0') printf(" %s%c ", CRED,printable[yy][xx]);
            else if (printable[yy][xx] == '.') printf(" %s%c ",CWHITE,printable[yy][xx]);
        }
        printf("%s|\n", CWHITE);
    }
    printf(" -----------------------------\n");
    for (int y = 0 ; y < 3 ; y ++){
        printf("          |");
        for (int x = 0 ; x < 3 ; x ++){
            if (field->maps[y][x].result == -1){
                printf(" %s. ", CWHITE);
            }else if (field->maps[y][x].result == 1){
                printf(" %sX ", CGREEN);
            }else if (field->maps[y][x].result == 2){
                printf(" %s0 ", CRED);
            }else if (field->maps[y][x].result == 0){
                printf(" %s# ", CBLACK);
            }
        }
        printf("%s|\n",CWHITE);
    }
    printf("           ---------\n");
}

void printMap(Map *map){
    unsigned char *act = (unsigned char*)&map->maps[0];
    unsigned char chien;
    int total = 0;
    int actual = 0;
    for (int i=2-1;i>=0;i--){
        for (int j=7 ; j>=0; j--){
            total++;
            if (total < 8)
                continue;
            actual++;
            chien = (act[i] >> j) & 1;
            printf("%u  ", chien);
            if (actual % 3 == 0){
                printf("\n");
            }
        }
    }
    printf("\n");
}

void printBits(size_t const size, void const * const ptr){
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    for (i=size-1;i>=0;i--){
        for (j=7;j>=0;j--){
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("\n");
}

void resetMap(Map* map){
    map->maps[0]    = 0;
    map->maps[1]    = 0;
    map->maps[2]    = 0;
    map->result     = -1;
    map->totalHits  = 0;
}

void resetField(Field* field){
    field->nbrCovers[0] = 0;
    field->nbrCovers[1] = 0;
    field->nbrCovers[2] = 0;
    for (int y=0; y<3;y++)
        for (int x=0; x<3;x++)
            resetMap(&field->maps[y][x]);
    field->totalHits = 0;
    resetMap(&field->globalMap);
    field->nbrPlaces = 0;
    for (int yy = 0 ; yy < 3 ; yy++){
        for (int xx = 0 ; xx < 3 ; xx++){
            for (int y = 0 ; y < 3 ; y++){
                for (int x = 0 ; x < 3 ; x++){
                    field->places[field->nbrPlaces].x = x;                 
                    field->places[field->nbrPlaces].y = y;
                    field->places[field->nbrPlaces].score = 0;
                    field->places[field->nbrPlaces].computed = 0;
                    field->places[field->nbrPlaces].map = &field->maps[yy][xx];
                    field->nbrPlaces += 1;
                }
            }
        }
    }
}

Field *newField(void){
    Field *field = malloc(sizeof(Field));
    for (int y=0; y<3;y++)
        for (int x=0; x<3;x++){
            resetMap(&field->maps[y][x]);
            field->maps[y][x].field = field;
            field->maps[y][x].posX = x;
            field->maps[y][x].posY = y;
        }
    field->globalMap.field = field;
    resetField(field);
    return field;
}

int putMap(Map *map, int player,int y, int x){
    int yy = y * 3;
    int pos = x + yy;
    map->maps[player] = map->maps[player] | 0b0000000100000000 >> pos;
    map->maps[0] = map->maps[0] | 0b0000000100000000 >> pos;
    map->totalHits += 1;
    if (map->result == -1 && map->totalHits >= 3){
        if (player != 0){
            
            if ((map->maps[player] & 0b0000000111000000 >> yy ) == 0b0000000111000000 >> yy){
                map->result = player;
                map->field->nbrCovers[player] += 1;
                return player;
            } 
            if ((map->maps[player] & 0b0000000100100100 >>  x ) == 0b0000000100100100 >>  x){
                map->result = player;
                map->field->nbrCovers[player] += 1;
                return player;
            }
            if ((map->maps[player] & 0b0000000100010001) == 0b0000000100010001){
                map->result = player;
                map->field->nbrCovers[player] += 1;
                return player;
            }
            if ((map->maps[player] & 0b0000000001010100) == 0b0000000001010100){
                map->result = player;
                map->field->nbrCovers[player] += 1;
                return player;
            }
        }
        if (map->totalHits == 9){
            map->result = 0;
            return 0;
        }
    }
    return -1;
}

void addMappInPlaces(Map *map){
    for (int yy = 0; yy < 3 ;yy++){
        for (int xx = 0; xx < 3; xx++){
            if ((map->maps[0] >> 8 - (xx + (yy * 3)) & 1) != 1){
                map->field->places[map->field->nbrPlaces].x = xx;
                map->field->places[map->field->nbrPlaces].y = yy;
                map->field->places[map->field->nbrPlaces].map = map;
                map->field->nbrPlaces += 1;
            }
        }
    }
}

int place(Map *map, int player , int y, int x){
    int res = putMap(map, player, y, x);
    if (res != -1){
        int ress = putMap(&map->field->globalMap, res, map->posY, map->posX);

        if (ress != -1){
            return ress;
        }
    }

    map->field->nbrPlaces = 0;
    if (map->field->maps[y][x].result == -1){ // NOT FREE
        addMappInPlaces(&map->field->maps[y][x]);
    }else{ // FREE
        for (int yy = 0 ; yy < 3 ; yy++){
            for (int xx = 0 ; xx < 3 ; xx++){
                if (map->field->maps[yy][xx].result == -1){
                    addMappInPlaces(&map->field->maps[yy][xx]);
                }
            }
        }
    }

    return -1;
}

void printScores(Field *field){
    // printf("NBRPLACES : %d\n", field->nbrPlaces);
    for (int i=0 ; i < field->nbrPlaces; i++){
        // printf("Places %d %d - %d %d  ~~> %5d  %5d ~~> %5.1lf%%\n", field->places[i].map->posY, field->places[i].map->posX, field->places[i].y, field->places[i].x , field->places[i].score, field->places[i].computed , (double)field->places[i].score / (double)field->places[i].computed * 100.  );
    }
}

typedef struct s_engine{
    Field *field;
    Field *testField;
    int player;
    int op;
    long timeout;
}   Engine;

int weakEngine(Field *field, int player){
    return random_number(0, field->nbrPlaces);
}

int chien = 0;

int startEngine(Engine *e){
    for (int i=0 ; i < e->field->nbrPlaces; i++){
        copyField(e->testField, e->field);
        int chien = place(e->testField->places[i].map, e->player, e->testField->places[i].y, e->testField->places[i].x);
        if (chien != -1){
            e->field->places[i].computed ++;
            if (chien == e->player) e->field->places[i].score += 1;
            else if (chien == 0) e->field->places[i].score += -1;
            else e->field->places[i].score += -1;
            chien++;
            break ;
        }

        int player = e->player;
        int op ;
        while (1){
            op = player;
            player = player == 1 ? 2 : 1;
            int weak = weakEngine(e->testField, player);
            int res = place(e->testField->places[weak].map, player, e->testField->places[weak].y, e->testField->places[weak].x);
            if ( res == e->player){
                e->field->places[i].computed ++;
                e->field->places[i].score += 1;
                chien++;
                break;
            }
            else if (res == e->op){
                e->field->places[i].computed ++;
                e->field->places[i].score += -1;
                chien++;
                break;
            }
            else if (res == 0){
                e->field->places[i].computed ++;
                e->field->places[i].score += -1;
                chien++;
                break;
            }
            // else if (e->testField->nbrCovers[e->player] > e->field->nbrCovers[e->player] + 2){
            //     e->field->places[i].computed ++;
            //     e->field->places[i].score += 1;
            //     chien++;
            //     break;
            // }
            // else if (e->testField->nbrCovers[e->op] > e->field->nbrCovers[e->op] + 1){
            //     e->field->places[i].computed ++;
            //     chien++;
            //     e->field->places[i].score += -1;
            //     break;
            // }
        }
    }

}

int prepareField(Field *field, int nbr){
    int player = 2;
    for (int i = 0; i < nbr; i++){
        player = player == 1 ? 2 : 1;
        int weak = weakEngine(field, player);
        place(field->places[weak].map, player, field->places[weak].y, field->places[weak].x);
    }
    return player = player == 1 ? 2 : 1;
}

int mainEngine(Engine *e){
    long startTime = tim();
    long mustEndTime = startTime + e->timeout;

    while (1){

        long long startLoopTime = tim();

        int res = startEngine(e);

        long long endTime = tim();
        long long timeCompute = endTime - startLoopTime;
        if (timeCompute + endTime >= mustEndTime) break;

    }
    int max = -9999999;
    int maxPos = 0;
    for (int i=0; i< e->field->nbrPlaces; i++){
        if (e->field->places[i].score > max){
            max = e->field->places[i].score;
            maxPos = i;
        }
    }
    return maxPos;
}

int main(int argc, char *argv[]){
    srand(time(0));
    if (argc >= 2){
        Engine e;
        e.field = newField();
        e.testField = newField();
        // e.player = prepareField(e.field, 2);
        e.player = 1;
        e.timeout = 100;     // LOCAL
        // e.timeout = 100;    //CODING GAME
        // e.timeout = 3000;   //TEST
        int res;
        int fi;
        while (1){
            e.op = e.player;
            e.player = e.player == 1 ? 2 : 1;
            if (e.player == 1){
                res = mainEngine(&e);
            }else{
                res = mainEngine(&e);
                // res = weakEngine(e.field, e.player);
            }
            fi = place(e.field->places[res].map, e.player, e.field->places[res].y, e.field->places[res].x);
            printField(e.field);
            printf("--> %c\n",  e.player == 1 ? 'X' : '0');
            printScores(e.field);
            printf("Carlo : %d\n", chien);
            chien = 0;
            getchar();
            if (fi != -1){
                break;
            }
        }

        // printf("froijgr  %d\n", res);
    }else{
        int hero    = 1;
        int op      = 2;
        Engine e;
        Field *field = newField();
        e.field = newField();
        e.testField = newField();
        e.op = op;
        e.player = hero;
        e.timeout = 100;  
        while (1){
            int opponentRow;
            int opponentCol;
            scanf("%d%d", &opponentRow, &opponentCol);
            int validActionCount;
            scanf("%d", &validActionCount);
            for (int i = 0; i < validActionCount; i++) {
                int row;
                int col;
                scanf("%d%d", &row, &col);
            }
            if (opponentRow != -1){
                place(&e.field->maps[opponentRow / 3][opponentCol / 3], e.op, opponentRow % 3, opponentCol % 3);
            }else{
                place(&e.field->maps[1][1], e.player, 1,  1);
                printf("4 4\n");
                continue;
            }
            int res = mainEngine(&e);
            // int res = weakEngine(e.field, 1);
            int x = e.field->places[res].x + (e.field->places[res].map->posX * 3);
            int y = e.field->places[res].y + (e.field->places[res].map->posY * 3);
            printf("%d %d\n", y, x);
            place(e.field->places[res].map, e.player, e.field->places[res].y, e.field->places[res].x);
        }     
    }
    return (0);
}