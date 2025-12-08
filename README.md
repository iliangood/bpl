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
- После сборки в папке build появится исполняемый файл bpl

- Есть примеры программ в examples в корне репозитория

## Синтаксис
```bpl
[Название команды]:[модификатор1]:[модификатор2]:...
[тип аргумента1]:[подтип аргумента1]:[значение1]
[тип аргумента2]:[подтип аргумента2]:[значение2]
...
```
> [!NOTE]
> - Есть команды без аргументов, в этом случае аргументы не указываются
> - Так же есть аргументы без подтипов, в этом случае подтип не указывается
> - Еще есть аргумент instructions, который содержит в себе набор других команд. В этом случае после строки с названием команды идет набор строк с командами, и заканчивается он строкой "endInstructions"


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
