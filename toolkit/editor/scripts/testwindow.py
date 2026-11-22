import editor
import deargui as dg
category ="Скриптовые тесты"
check =False
def draw ():
    global check
    check =dg .checkbox ("проверь меня!",check )[1 ]
    if check :
        dg .text ("ты меня проверил!")