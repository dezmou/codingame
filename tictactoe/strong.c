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


# define FORX(x) for (int x = 0; x < 3; x++)
# define FORY(y) for (int y = 0; y < 3; y++)

# define FOY for (int y = 0; y < 3; y++)
# define FOX for (int x = 0; x < 3; x++)
# define FOYY for (int yy = 0; yy < 3; yy++)
# define FOXX for (int xx = 0; xx < 3; xx++)
# define PM printField(&env.field);
# define PMM printField(&env.testField);
# define PME printFieldErr(&env.field);

# define FREE 0
# define HERO 2
# define OP 1
# define DEAD -1

# define MAX_TIMEOUT 100

char random_number(int min_num, int max_num);



typedef struct s_coord {
    char x;
    char y;
}  t_coord;

typedef struct placeRes {
    t_coord coord;
    char **map;
} t_placeRes;

typedef struct s_field {
    char    ****bigMap;
    char    **gmap;
    char    winner;
    char    **lastMap ;
    char    isFree ;
    char    **playableMap;
    int     total ;
} t_field;

typedef struct playable{
    t_placeRes place;
    int score;
    char allowed;
    char end;
}       t_playable;


typedef struct t_env {
    t_playable *playables;
    t_field field;
    t_field testField;
    char printable[9][9];
} t_env;

t_env env;

long long tim() {
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

void copyField(t_field *field, t_field *source){
    FORY(yy){
        FORX(xx){
            field->gmap[yy][xx] = source->gmap[yy][xx];
            if (source->bigMap[yy][xx] == source->lastMap) field->lastMap = field->bigMap[yy][xx];
            if (source->bigMap[yy][xx] == source->playableMap) field->playableMap = field->bigMap[yy][xx];
            FORY(y){
                FORX(x){
                    field->bigMap[yy][xx][y][x] = source->bigMap[yy][xx][y][x];
                }
            }
        }
    }
    field->isFree = source->isFree;
    field->winner = 0;
    field->total = source->total;
}

t_coord getMapCoord(t_field *field, char **map){
    t_coord final;
    FOY{
        FOX{
            if (field->bigMap[y][x] == map){
                final.x = x;
                final.y = y;
                return final;
            }
        }
    }
    printf("WRONG MAP\n");
    exit(1);
    return final;
}

char computeMap(char **map){
    int county;
    int countx;
    int cou;
    int coua;
    for (char player = 1 ; player <= 2 ; player++){
        FORX(yx){
            countx = 0;
            county = 0;
            FORY(xy){
                if (map[xy][yx] == player) countx += 1;
                if (map[yx][xy] == player) county += 1;
            }
            if (countx == 3 || county == 3){
                return player;
            } 
        }
        cou = 0;
        coua = 0;
        FORX(j){
            if (map[j][j] == player) cou += 1;
            if (map[2 - j][j] == player) coua += 1;
        }
        if (cou == 3 || coua == 3) return player;
    }
    FORX(t){
        FORX(g){
            if (map[t][g] == FREE) return FREE;
        }
    }
    return DEAD;
}

char isWinner(t_field *field){
    char tmp = computeMap(field->gmap);
    
    if (tmp != FREE){
        field->winner = tmp;
        return tmp;
    }
    return 0;
}

char place(t_field *field, char id, char **map, char y, char x){
    char res;
    if (map[y][x] != FREE){
        printf("ERREUR\n");
        exit(0);
    }
    map[y][x] = id;
    t_coord mp = getMapCoord(field, map);
    if (field->gmap[mp.y][mp.x] == FREE){
        field->gmap[mp.y][mp.x] = computeMap(map);
        if (field->gmap[mp.y][mp.x] != FREE){
            res = isWinner(field) ;
            if (res ){
                return res;
            }
        }
    }
    field->lastMap = map;
    field->isFree = field->gmap[y][x] != 0;
    field->playableMap = field->isFree ? NULL : (char **)(field->bigMap[y][x]);
    field->total += 1;
    return FREE;
}

void resetField(t_field *field){
    field->lastMap = NULL;
    field->isFree = 1;
    field->playableMap = field->bigMap[1][1];
    field->winner = 0;
    field->total = 0;
    FORY(yy){
        FORX(xx){
            field->gmap[yy][xx] = FREE;
            FORY(y){
                FORX(x){
                    field->bigMap[yy][xx][y][x] = 0;
                }
            }
        }
    }
}

void initField(t_field *field){
    field->gmap = malloc(sizeof(char*) * 3);
    field->bigMap = malloc(sizeof(char*) * 3);
    FOY{
        field->gmap[y] = malloc(sizeof(char) * 3);
        field->bigMap[y] = malloc(sizeof(char*) * 3);
        FOX{
            field->bigMap[y][x] = malloc(sizeof(char*) * 3);
            FORY(yy) field->bigMap[y][x][yy] = malloc(sizeof(char) * 3);
        }
    }
    resetField(field);
}

t_coord getRandomPlace(char **map){
    t_coord corde;
    while (1){
        char x = random_number(0,3);
        char y = random_number(0,3);
        if (map[y][x] == FREE) {
            corde.x = x;
            corde.y = y;
            return corde;
        }
    }
    return corde;
}

t_coord getPosInField(int yf, int xf, int ym, int xm){
    t_coord final;
    final.y = yf * 3 + ym;
    final.x = xf * 3 + xm;
    return final;
}

void printField(t_field *field){
    FORY(yf){
        FORX(xf){
            FORY(ym){
                FORX(xm){
                    t_coord c = getPosInField(yf, xf, ym, xm);
                    env.printable[c.y][c.x] = field->bigMap[yf][xf][ym][xm];
                }
            }
        }
    }
    for (int y = 0 ; y < 9 ; y++){
        if (y % 3 == 0) printf("\n");
        for (int x = 0 ; x < 9 ; x++){
            if (x % 3 == 0) printf(" ");
            if (env.printable[y][x] == HERO ) printf(" %s%d ", CGREEN ,env.printable[y][x]);
            else if (env.printable[y][x] == OP )printf(" %s%d ", CRED ,env.printable[y][x]);
            else printf(" %s%d ", CWHITE ,env.printable[y][x]);
        }
        printf("\n");
    }
    printf("\n");
    FOY{
        printf("           ");
        FOX {
            if (field->gmap[y][x] == HERO ) printf(" %s%d ", CGREEN ,field->gmap[y][x]);
            else if (field->gmap[y][x] == OP )printf(" %s%d ", CRED ,field->gmap[y][x]);
            else if (field->gmap[y][x] == DEAD )printf(" %s# ", CBLACK);
            else printf(" %s%d ", CWHITE ,field->gmap[y][x]);
        }
        printf("\n");
    }
    printf("\n");
    
}

void printFieldErr(t_field *field){
    FORY(yf){
        FORX(xf){
            FORY(ym){
                FORX(xm){
                    t_coord c = getPosInField(yf, xf, ym, xm);
                    env.printable[c.y][c.x] = field->bigMap[yf][xf][ym][xm];
                }
            }
        }
    }
    for (int y = 0 ; y < 9 ; y++){
        if (y % 3 == 0) fprintf(stderr,"\n");
        for (int x = 0 ; x < 9 ; x++){
            if (x % 3 == 0) fprintf(stderr," ");
            fprintf(stderr," %d ",env.printable[y][x]);
        }
        fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
    FOY{
        fprintf(stderr,"           ");
        FOX {
            fprintf(stderr," %d ",field->gmap[y][x]);
        }
        fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
}

char random_number(int min_num, int max_num)
{
    int result = (rand() % (max_num - min_num)) + min_num;
    return (char)result;
}


t_placeRes weakEngine(char id, t_field *field){
    t_placeRes res;
    t_coord tmp;
    if (field->isFree){
        tmp = getRandomPlace(field->gmap);
        res.map = field->bigMap[tmp.y][tmp.x];
    }else{
        res.map = field->playableMap;
    }
    res.coord = getRandomPlace(res.map);
    return res;
}

void resetPlayables(){
    for (int i = 0; i < 90; i++){
        env.playables[i].allowed = 0;
        env.playables[i].score = 0;
        env.playables[i].end = 1;
    }
}


int getAllPlayeablePos(t_field *field){
    resetPlayables();
    int total = 0;
    if (!field->isFree){
        // fprintf(stderr, "NOT FREE\n");
        FOY{
            FOX{
                if (field->playableMap[y][x] == FREE){
                    env.playables[total].allowed = 1;
                    env.playables[total].score = 0;
                    env.playables[total].end = 0;
                    env.playables[total].place.coord.x = x;
                    env.playables[total].place.coord.y = y;
                    env.playables[total].place.map = field->playableMap;
                    total += 1;
                }
            }
        }
    }else{
        // fprintf(stderr, "FREE\n");
        FOY{
            FOX{
                if (field->gmap[y][x] == FREE){
                    FORY(yy){
                        FORX(xx){
                            if (field->bigMap[y][x][yy][xx] == FREE){
                                env.playables[total].allowed = 1;
                                env.playables[total].score = 0;
                                env.playables[total].end = 0;
                                env.playables[total].place.coord.x = xx;
                                env.playables[total].place.coord.y = yy;
                                env.playables[total].place.map = field->bigMap[y][x];
                                total += 1;
                            }
                        }
                    }
                }
            }
        }
    }
    env.playables[total].end = 1;
    return (total);
}

t_placeRes getMapPosByAbsolute(t_field *field, char y, char x){
    t_placeRes res;
    res.map = field->bigMap[y/3][x/3];
    res.coord.x = x % 3;
    res.coord.y = y % 3;
    return res;
}


# define PLI env.playables[i]
# define TFIELD env.testField

t_placeRes strongEngine(char id, t_field *field){
    int max = 0;
    int min = 0;
    long long startTime = tim();
    long long mustEndTime = startTime + MAX_TIMEOUT;
    int total = getAllPlayeablePos(&env.field);
    while (1){
        long long startLoopTime = tim();
        for (int i=0; i < 90; i++){
            if (PLI.end) break;
            copyField(&env.testField, field);
            t_coord mapPos = getMapCoord(field, PLI.place.map);
            int finished = 0;
            char r = place(&TFIELD, HERO, TFIELD.bigMap[mapPos.y][mapPos.x] , PLI.place.coord.y, PLI.place.coord.x);
            if (r != FREE){
                if (r == HERO) PLI.score += 1;
                if (r == OP) PLI.score += -1;
                if (r == DEAD) PLI.score += 0;
                finished = 1;
            }
            int actPlayer = id == HERO ? OP : HERO;
            while (!finished){
                t_placeRes zouze = weakEngine(actPlayer, &TFIELD);
                int rus = place(&TFIELD, actPlayer, zouze.map, zouze.coord.y, zouze.coord.x);
                if (rus != FREE){
                    if (rus == HERO) PLI.score += 1;
                    if (rus == OP) PLI.score += -1;
                    if (rus == DEAD) PLI.score += 0;
                    finished = 1;
                } 
                actPlayer = actPlayer == HERO ? OP : HERO;
            }
            if (env.playables[i].end) break;
        }
        long long endTime = tim();
        long long timeCompute = endTime - startLoopTime;
        if (timeCompute + endTime > mustEndTime) break;
    }
    t_placeRes finalRes;
    for (int i=0; i < 90; i++){
        if (PLI.score > max) max = PLI.score;
        if (PLI.score < min) min = PLI.score;
        if (PLI.end) break;
    }    
    for (int i=0; i < 90; i++){
        if (PLI.end) break;
        
        if (PLI.score == max){
            finalRes.coord.y = PLI.place.coord.y;
            finalRes.coord.x = PLI.place.coord.x;
            finalRes.map = PLI.place.map;
            return finalRes;
        }
    }
    return weakEngine(id, field);
}

int main(int argc, char *argv[]){
    srand(time(0));
    initField(&env.field);
    initField(&env.testField);
    env.playables = malloc(sizeof(t_playable) * 100);
    
    if (argc >= 2){
        t_placeRes res ;
        // strongEngine(HERO, &env.field);
        // exit(0);
        char out;
        for (int i=0; i < 8 ; i++){
            while (1){
                res = strongEngine(HERO, &env.field);
                out = place(&env.field, HERO, res.map , res.coord.y, res.coord.x);
                // PM 
                if (out != FREE) break;
                // getchar();
                res = weakEngine(OP, &env.field);
                out = place(&env.field, OP, res.map , res.coord.y, res.coord.x);
                // PM
                if (out != FREE) break;
                // getchar();
            }
            PM
            // getchar();
            resetField(&env.field);
        }
    }else{
        while (1) {
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
                t_placeRes res = getMapPosByAbsolute(&env.field, (char)opponentRow, (char)opponentCol);
                place(&env.field, OP, res.map, res.coord.y, res.coord.x);
            }
            t_placeRes me = strongEngine(HERO, &env.field);
            // PME
            place(&env.field, HERO, me.map, me.coord.y, me.coord.x);
            t_coord mapCord = getMapCoord(&env.field, me.map);
            int finalX = mapCord.x * 3 + me.coord.x;
            int finalY = mapCord.y * 3 + me.coord.y;
            printf("%d %d\n", finalY, finalX);

        }        
    }
    

    return 0;
}
