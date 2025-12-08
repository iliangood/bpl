# BPL - bad programming language
- интерпретируемый язык программирования

Все еще находится в разработке

## Сборка
```bash
git clone https://github.com/iliangood/bpl.git
cd bpl
mkdir build
cd build
cmake ..
make
```

## Синтаксис
```bpl
[Название команды]:[модификатор1]:[модификатор2]:...
[тип аргумента1]:[подтип аргумента1]:[значение1]
[тип аргумента2]:[подтип аргумента2]:[значение2]
...
```

## Команды:
- end

- call
- ret
- inFunc

- init
- get
- set
- valfromstlink
- valfromarg
- getSublink


- if
- while

- runInstsVec

- add
- sub
- mul
- div
- mod
- and
- or
- not
- shl
- shr

- stackRealloc

- setNoBlockingInput
- checkBuf
- printCh
- printNum
- readCh
- readNum
- peekCh

- ls
- leq
- bg
- beq
- equ
- neq
