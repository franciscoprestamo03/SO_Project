# SO_Project

compilar usando:

```bash
gcc main.c -lncurses
```

## Documentación

La razón de su existencia de este proyecto es demostrar conocimientos sobre algoritmos análogos a los usados por los sistemas operativos, sigue una explicación del flujo lógico del proyecto para facilitar su entendimiento

### API de Malloc

En la base de este proyecto está el manejo de la memoria. En esencia, en el código se define un arreglo de enteros estático, de un tamaño considerable, que será la única fuente de memoria de todo el proyecto.

Sobre este arreglo, se crea una API similar a malloc: Se puede reservar y liberar memoria usando esta API, con la garantía de que la memoria reservada solo será usada por los apuntadores correspondientes. Por ejemplo, se puede pedir memoria de la siguiente forma:

```C
int a = allocateMemory(n);
```

De esta forma, se reserva el espacio de `n` enteros en el arreglo de la memoria, a la que se puede acceder usando `a`. El programa divide la memoria en bloques, cada bloque tiene un prefijo con su información, como su tamaño, si está libre o no, y un apuntador al inicio del siguiente bloque. Con esta información, un apuntador al primer bloque y otro al bloque actual, se implementa el algoritmo de first-fit para alocar memoria. Cuando se libera memoria, lo cual se hace de la siguiente forma:

```C
deleteMem(a);
```

Lo que se hace es liberar el bloque que representa `a` y se fusiona con bloques libres adyacentes si es posible.

### Tipos

Ahora, esto claramente es de muy bajo nivel, vimos necesario crear más niveles de abstracción para facilitar la creación de un juego sobre esta API. Para eso, creamos varias funciones más, por ejemplo:

```C
int createInteger();
int createIntegerInit(int init);
void writeInteger(int blockPtr, int value);
int getInteger(int blockPtr);


int createChar();
int createCharInit(char init);
void writeChar(int blockPtr, char value);
char getChar(int blockPtr);
```

Estas son las declaraciones de las funciones para tipos básicos. También definimos funciones para manejar arreglos:

```C
int createIntegerArray(int size);
void writeIntegerInArray(int blockPtr, int index, int value);
int getIntegerInArray(int blockPtr, int index);

int createCharArray(int size);
int createCharArrayInit(int size, char string[]);
void writeCharInArray(int blockPtr, int index, char value);
char getCharInArray(int blockPtr, int index);
```

Usando estas abstracciones, creamos niveles más altos de abstracción, añadiendo tipos `alien` o `bullet`, con toda la información necesaria, como posición, dirección o puntos de golpe.

### Flujo del juego, hilos

Usando lo expuesto para manejar la memoria, lo próximo fue maximizar la concurrencia:

Hicimos lo absolutamente necesario para empezar: separamos un hilo para capturar input del usuario.

Además de esto, añadimos hilos que calculan las nuevas posiciones de aliens y proyectiles simultáneamente. En cada frame, se crean ambos y se espera a que termine su ejecución, luego se chequea si hubo colisiones (si el jugador eliminó un alien), en cuyo caso se disminuyen los puntos de golpe del alien o se elimina del juego y se aumenta la puntuación del jugador. Claramente es necesario esperar a que los dos hilos mencionados terminen su ejecución antes de chequear colisiones.

También se tiene otro hilo aparte que genera los aliens, basado en una serie de temporizadores de generación.

### Guardado y Cargado

También se implementaron dos funcionalidades para la persistencia de datos:

1. Salvar y Cargar partida: Durante el juego, si se pulsa la tecla `G`, se guarda el estado del juego en un archivo, después, desde el menú, se puede cargar este estado.

2. Puntajes: Al salir del juego o perder, se le pide al usuario un nombre, bajo el cual se guardará su puntaje, los 10 puntajes más altos son mostrados en la pantalla de juego
