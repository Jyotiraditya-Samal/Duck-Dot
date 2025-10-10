# M5.Touch.getTouchPointRaw() 
# M5.Touch.getX()
# M5.Touch.getY()
# M5.Touch.getCount()
# image.setVisible(False)

import os, sys, io
import M5
from M5 import *
import time
import random

# symbol = {<symbolName>:<symbolPath>,...}
symbol = {'Duck': 'res/img/duck_button.png', 'Dot': 'res/img/dot_button.png', 'Blank': 'res/img/blank_button.png','duckSelect':'res/img/duck_select.png', 'dotSelect':'res/img/dot_select.png','idle':'res/img/no_select.png','background_light':'res/img/background_white.png','reset':'res/img/reset.png','duck_win':'res/img/duck_win.png','dot_win':'res/img/dot_win.png'}

# tile\button = {<buttonName>:[[pos_x,pos_y],[height,width],buttonImage,timesUpdated],state,magicSumNum...}

tiles = {0: [[33, 25], [52, 52], None, 0,-1,8], 1: [[99, 25], [52, 52], None, 0,-1,1], 2: [[165, 25], [52, 52], None, 0,-1,6],
         3: [[33, 95], [52, 52], None, 0,-1,3], 4: [[99, 95], [52, 52], None, 0,-1,5], 5: [[165, 95], [52, 52], None, 0,-1,7],
         6: [[33, 165], [52, 52], None, 0,-1,4], 7: [[99, 165], [52, 52], None, 0,-1,9], 8: [[165, 165], [52, 52], None, 0,-1,2]}

actionButtons = {'action1':[[233, 116],[71,44],None,0],'action2':[[233, 173],[71,44],None,0],'background':[[0,0],[240,360],None,1]}


actionState = random.randint(0,1)
stateList = {0:{'button':'duckSelect','tile':'Duck'},1:{'button':'dotSelect','tile':'Dot'},2:'idle'}


fillCount = -9

gameOver = False  

def isTouched(button,tiles):
    curr_X = M5.Touch.getX()
    curr_Y = M5.Touch.getY()
    
    min_X = tiles[button][0][0]
    max_X = tiles[button][0][0] + tiles[button][1][0]
    
    min_Y = tiles[button][0][1]
    max_Y = tiles[button][0][1] + tiles[button][1][1]
    
    if M5.Touch.getCount():
        if curr_X in range(min_X,max_X) and curr_Y in range(min_Y,max_Y):
            while not M5.Touch.getCount():
                continue
            print(f'isTouched :: {button}')
            return True
    return False    


def setTile(tile,sym,tiles):
    global fillCount
    if tiles[tile][2]:                  # ghost instance removal
        tiles[tile][2].setVisible(False)
        
    if tiles[tile][3] < 2:              # tile update lock
        tiles[tile][2] = Widgets.Image(symbol[sym], tiles[tile][0][0], tiles[tile][0][1])
        tiles[tile][3] = tiles[tile][3] + 1
        fillCount = fillCount + 1
        return
    print("Error :: tile update locked")

def setButton(tile,sym,tiles):    
        if tiles[tile][2]:              # ghost instance removal
            tiles[tile][2].setVisible(False)

        tiles[tile][2] = Widgets.Image(symbol[sym], tiles[tile][0][0], tiles[tile][0][1])

def initTile():
    for i in tiles:
        tiles[i][3] = 0
        tiles[i][4] = -1
        setTile(i,'Blank',tiles)
        
def showTiles():
    for j in range(3):
        for i in range(j*3,(j*3)+3):
            print(f"[{tiles[i][4]}]", end=" ")
            if i == (j*3)+3-1:
                print("",end="\n")
    print("\n")

def winSum(array,expResult,state):      # Using the "Magic Square Sum" for detection
    for i in range(0,len(array)):
        for j in range(1,len(array)):
            for k in range(2,len(array)):
                if((not i==j) and (not i==k) and (not j==k)):
                    if(array[i][4] == state) and (array[j][4] == state) and (array[k][4] == state):
                                                if array[i][5] + array[j][5] + array[k][5] == expResult:
                                                                                                return True

def whoWon():
    
    duckFlag = winSum(tiles,15,0)
    dotFlag = winSum(tiles,15,1)
    
    if duckFlag:
        print("Duck has won!!")
        setButton('background','duck_win',actionButtons)
        return True
    
    if dotFlag:
        print("Dot has won!!")
        setButton('background','dot_win',actionButtons)
        return True
    
    return False


def isEmpty():
    for i in tiles:
        if tiles[i][4] != -1:
            return False
    return True

def reset():
            global actionState,fillCount,gameOver
            actionState = random.randint(0,1)
            setButton('background','background_light',actionButtons)
            setButton('action1',stateList[actionState]['button'],actionButtons)
            setButton('action2','reset',actionButtons)
            initTile()
            gameOver = False
            fillCount = 0
            
            print("resetting()...")


def setup():
    
    M5.begin()
    
    Widgets.fillScreen(0xffffff) # white background!!
    setButton('background','background_light',actionButtons)
    setButton('action1','duckSelect',actionButtons)
    setButton('action2','reset',actionButtons)
    initTile()	# initialize blank grid :)
    
    
def loop():
    M5.update()
    global actionState,fillCount,gameOver
    
    if not gameOver:
        if fillCount == 9:                       # on gridFilled Action
            setButton('action1','idle',actionButtons)
            showTiles()
            whoWon()
            gameOver = True
            fillCount = 10
    
        if fillCount < 9:                        # on gridFree Action
            for i in tiles:                      
                if isTouched(i,tiles):           # on tile press Action
                    if tiles[i][3] < 2:
                        setTile(i,stateList[actionState]['tile'],tiles)
                        tiles[i][4] = actionState
                        actionState = int(not actionState)
                        setButton('action1',stateList[actionState]['button'],actionButtons)
                        showTiles()
                        if whoWon():
                            gameOver = True
                            return
    if not isEmpty():
        if isTouched('action2',actionButtons):   # on reset Action
            reset()


if __name__ == '__main__':
    try:
        setup()
        while True:
            loop()
    except (Exception, KeyboardInterrupt) as e:
        try:
            from utility import print_error_msg
            print_error_msg(e)
        except ImportError:
            print("please update to latest firmware")


