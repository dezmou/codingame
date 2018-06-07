import json
import datetime
import random
import sys

CBLACK  = '\33[30m'
CRED    = '\33[31m'
CGREEN  = '\33[32m'
CWHITE  = '\33[37m'

cols = [
    CWHITE,
    CRED,
    CGREEN
]

Y = 0
X = 1

DEAD    = -1
FREE    = 0
OP      = 1
HERO    = 2


MAX_TIMEOUT = 98

LOCAL = len(sys.argv) > 1

def copyObj(obj):
    return json.loads(json.dumps(obj))

def tim():
    return int((datetime.datetime.utcnow() - datetime.datetime(1970, 1, 1)).total_seconds() * 1000)

def prf(msg):
    print(msg, file=sys.stderr)

class Field:
    def __init__(self, copy = None):
        if copy is None:
            self.bigMap = [[[[FREE for x in range(3)] for t in range(3)] for u in range(3)] for b in range(3)]
            self.gmap = [[FREE for x in range(3)] for y in range(3)]
            self.lastPlay = None
            self.lastMap = None
            self.isFree = True
            self.playableMap = None
            self.winner = None
            self.total = 0
        else:
            self.bigMap = copyObj(copy.bigMap)
            self.gmap = copyObj(copy.gmap)
            self.lastPlay = copyObj(copy.lastPlay)
            lastMapCord = copy.getMapCoord(copy.lastMap) if copy.lastMap is not None else None
            self.lastMap = self.bigMap[lastMapCord[Y]][lastMapCord[X]] if copy.lastMap is not None else None
            self.isFree = copy.isFree
            playableMapCord = copy.getMapCoord(copy.playableMap) if copy.playableMap is not None else None
            self.playableMap = self.bigMap[playableMapCord[Y]][playableMapCord[X]] if copy.playableMap is not None else None
            self.winner = None            
            self.total = copy.total

    def getMapCoord(self, map):
        for y in range(3):
            for x in range(3):
                if self.bigMap[y][x] is map:
                    return [y, x]
        raise Exception("Wrong map")

    def getPosInField(self, yf, xf, ym, xm):
        y = yf * 3 + ym
        x = xf * 3 + xm
        return [y,x]

    def getMapPosByAbsolute(self, y,x):
        map = self.bigMap[int(y/3)][int(x/3)]
        pos = [y % 3, x % 3]
        return [map, pos]
    
    def getPosByMap(self, smap, ym, xm):
        for y,line in enumerate(self.bigMap):
            for x,m in enumerate(line):
                if m is smap:
                    return [y * 3 + ym, x * 3 + xm]

    def printMap(self):
        field = self.bigMap
        final = [[0 for x in range(9)] for z in range(9)]
        for yf in range(3):
            for xf in range(3):
                for ym in range(3):
                    for xm,c in enumerate(field[yf][xf][ym]):
                        cs = self.getPosInField(yf, xf, ym, xm)
                        final[cs[Y]][cs[X]] = c
        pri = ""
        for j,line in enumerate(final):
            if j % 3 == 0:
                pri += '\n'
            for i, char in enumerate(line):
                if i % 3 == 0:
                    pri += ' '
                pri += " {}{} ".format("",char) if len(sys.argv) == 1 else " {}{} ".format(cols[char],char)
            pri += "\n"
        pri += "\n"
        for y in range(3):
            pri += "           "
            for x in range(3): pri += " {} ".format(self.gmap[y][x])
            pri += '\n'
        prf(pri)

    def computeMap(self, smap):
        for i in range(1,3):
            for yx in range(3):
                countx = 0
                county = 0
                for xy in range(3):
                    if smap[xy][yx] == i: countx += 1
                    if smap[yx][xy] == i: county += 1
                if countx == 3 or county == 3: return i
            cou = 0
            coua = 0
            for j in range(3):
                if smap[j][j] == i: cou += 1
                if smap[2 - j][j] == i: coua += 1
            if cou == 3 or coua == 3: return i
        for y in range(3):
            for x in range(3):
                if smap[y][x] == FREE: return FREE
        return DEAD

    def isWinner(self):
        tmp = self.computeMap(self.gmap)
        if tmp != FREE:
            self.winner = tmp
            return True
        return False

    def place(self, id,map, y, x):
        map[y][x] = id
        mp = self.getMapCoord(map)
        if self.gmap[mp[Y]][mp[X]] == FREE:
            self.gmap[mp[Y]][mp[X]] = self.computeMap(map)
            if self.gmap[mp[Y]][mp[X]] != FREE:
                if self.isWinner():
                    return self.winner
        self.lastMap = map
        self.lastPlay = [y,x]
        self.isFree = self.gmap[y][x] != 0
        self.playableMap = None if self.isFree else self.bigMap[y][x]
        self.total += 1
        return None

    def getRandomPlace(self, map):
        while True:
            x = random.randint(0,2)
            y = random.randint(0,2)
            if map[y][x] == FREE:
                return [y, x]

    def getAllPlayeablePos(self):
        final = []
        if not self.isFree:
            for y in range(3):
                for x in range(3):
                    if self.playableMap[y][x] == FREE:
                        final.append({"map" : self.playableMap, "pos" : [y, x], "score" : 0})
        else:
            for y in range(3):
                for x in range(3):
                    if self.gmap[y][x] == FREE:
                        for yy in range(3):
                            for xx in range(3):
                                if self.bigMap[y][x][yy][xx] == FREE:
                                    final.append({"map" : self.bigMap[y][x], "pos" : [yy, xx], "score" : 0 })
        return final

class Game:
    def weakPlaceSelector(self, map, id):
        pass
    
    def weakEngine(self, id, field):
        if field.isFree:
            co = field.getRandomPlace(field.gmap)
            map = field.bigMap[co[Y]][co[X]]
        else:
            map = field.playableMap
        return [map, field.getRandomPlace(map)]

    def strongEngine(self, id, field):
        if field.isFree and field.bigMap[1][1][1][1] == FREE and field.total == 0:
            return [field.bigMap[1][1], [1, 1]]
        choices = field.getAllPlayeablePos()
        startTime = tim()
        mustEndTime = startTime + MAX_TIMEOUT
        max = 0
        min = 0
        baseLenChoices = len(choices)
        while True:
            startLoopTime = tim()
            if id == HERO:
                if baseLenChoices > 7 and max >= 3 and min <= -3 and len(choices) > 4:
                    for i, choice in enumerate(choices):
                        if choice['score'] <= -3: 
                            del(choices[i])
            for choice in choices:
                testField = Field(copy = field)
                mapPos = field.getMapCoord(choice['map'])
                finished = False
                r = testField.place(id, testField.bigMap[mapPos[Y]][mapPos[X]], choice['pos'][Y], choice['pos'][X])
                if r is not None:
                    if r == id: choice['score'] += 1
                    elif r != DEAD : choice['score'] += -1
                    finished = True
                players = [HERO, OP] if id == OP else [OP, HERO]
                while not finished:
                    for player in players:
                        cho = self.weakEngine(player, testField)
                        res = testField.place(player, cho[0], cho[1][Y], cho[1][X])
                        if res is not None:
                            if res == id: choice['score'] += 1
                            elif res == OP : choice['score'] += -1
                            elif res == DEAD : choice['score'] += 0
                            finished = True
                            break
                if choice['score'] > max: max = choice['score']
                if choice['score'] < min: min = choice['score']

            endTime = tim()
            timeCompute = endTime - startLoopTime
            if timeCompute + endTime > mustEndTime:
                break
        choices = sorted(choices, key = lambda x: x['score'], reverse = True)

        final = choices[0]
        return [final['map'], [ final['pos'][Y], final['pos'][X] ] ]

    def startLocal(self, players):
        while True:
            for player in players:
                if player == HERO:
                    ac = self.strongEngine(player, self.field)
                    res = self.field.place(player, ac[0], ac[1][Y], ac[1][X])
                else:
                    ac = self.strongEngine(player, self.field)
                    res = self.field.place(player, ac[0], ac[1][Y], ac[1][X])
                # input()
                # self.field.printMap()           
                if res is not None:
                    return res

    def startOnline(self):
        while True:
            opponent_row, opponent_col = [int(i) for i in input().split()]
            valid_action_count = int(input())
            for i in range(valid_action_count):
                row, col = [int(j) for j in input().split()]
            if opponent_row != -1:
                res = self.field.getMapPosByAbsolute(opponent_row, opponent_col)
                self.field.place(OP, res[0], res[1][Y], res[1][X])
            ac = self.strongEngine(HERO, self.field)
            self.field.place(HERO, ac[0], ac[1][Y], ac[1][X])
            # abs = self.field.getPosByMap(ac[0], ac[1][Y], ac[1][X])
            c = self.field.getMapCoord(ac[0])
            finalY = c[Y] * 3 + ac[1][Y]
            finalX = c[X] * 3 + ac[1][X]
            # prf(" {} {} -- {} {}    --  {} {}".format(c[Y], c[X], ac[1][Y], ac[1][X], finalY, finalX))
            print("{} {}".format(finalY, finalX))
            # self.field.printMap()


    def __init__(self):
        self.field = Field()
        # self.field.printMap()

class Main:
    def __init__(self):
        if LOCAL:
            scores = {
                "pourcent" : 0.0,
                "total" : 0,
                "Hero" : 0,
                "Bandit" : 0,
                "Tie" : 0
            }
            playerChoices = [[HERO, OP], [OP, HERO]]
            while True:
                for players in playerChoices:
                    g = Game()
                    winner = g.startLocal(players) 
                    if winner == HERO : scores['Hero'] += 1
                    if winner == OP : scores['Bandit'] += 1
                    if winner == DEAD : scores['Tie'] += 1
                    scores['total'] += 1 if winner != DEAD else 0
                    if scores['total'] > 0:
                        scores["pourcent"] = scores['Hero'] / scores['total']  * 100
                    print(scores)
        else:
            g = Game()
            g.startOnline()            


Main()
