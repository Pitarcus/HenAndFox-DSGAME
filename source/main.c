#include <nds.h>
#include <stdio.h>
#include <time.h>
// Includes librerias propias
#include <nf_lib.h>

#define FRAMES_PER_ANIMATION 2

#define TAMAÑO_SLIME  15

#define TIMER_SPEED (BUS_CLOCK/1024)

void PantallaInicio();
void FueraInicio();
void ConfigurarInterrupciones();
void AnimarSprite();
void MoverZorro();
void RealizarAnimación();
void MenuPrincipal();
void CargarJuego();
u8 Tienda(u8 subMenu);
void MovimientoEnemigos();
void GeneradorEnemigos();
void Colisiones();
void ActualizarStatsInferior();
void FinDePartida();
void InicializarVariablesGlobales();
void Contador();
void Compra(u8 opcion);

// El sprite del zorro
// necesita un puntero a la dirección de memoria del sprite
// y referencia al frame de los gráficos

typedef struct {
	float posX, posY;

	u8 id;
	u8 state;
	u8 anim_frame;

	int HP;
	u8 daño;
	u8 defensa;
} Personaje;
 

// Direcciones
enum SpriteState {DOWN = 0, RIGHT = 1, UP = 2, LEFT = 3};

// Variables Globales

bool menu = true;

bool generar = true;


// Pantalla de inicio
u8 x = 96;

u8 y = 60;

// Menu
int dinero = 0;

u8 dificultad = 0;
u8 nivel = 0;

bool victoria = false;

bool FinDeJuego = false;

bool primeraPartida = true;

int contadorFinal; // delay

u8 contador = 0;

// Enemigos

u8 enemigosEliminados = 0;

u8 enemigosGenerados = 0;

u8 enemigosEnPantalla = 0; // Controlar la densidad de enemigos en pantalla

// Personajes

Personaje zorro = {90, 96, .id = 0, .HP = 100, .daño = 10, .defensa = 0};

Personaje gallina = {128, 96, .id = 1, .HP = 100, .defensa = 5};

Personaje slimeSample =  {.state = 0, .HP = 60, .daño = 6, .defensa = 0};

Personaje slime[TAMAÑO_SLIME];

// Para escribir las variables de los stats de los personajes
char vida[20]; // zorro.HP
char daño[20]; // zorro.daño
char defensa[20]; // zorro.defensa
char pollo[20]; // gallina.HP
char eliminados[20];
char monedero[20]; // dinero

bool realizarZorro = false;

bool zorroCansado = false;

bool pantallaInicio = true;

bool navidad = false;
/***************** Programa principal *******************/

int main( void )
{
	NF_Set2D(0, 0);
	NF_Set2D(1, 0);	
	
	soundEnable();

	// Define el ROOT e inicializa el sistema de archivos
	NF_SetRootFolder("NITROFS");	// Define la carpeta ROOT para usar NITROFS

	// Inicializa el motor 2D
	NF_Set2D(0, 0);				// Modo 2D_0 en ambas pantallas
	NF_Set2D(1, 0);

	// Inicializa los sonidos
	NF_InitRawSoundBuffers();

	// Inicializa los textos
	NF_InitTextSys(1); // texto pantalla inferior
	NF_InitTextSys(0); // texto pantalla superior

	// Inicializa los fondos tileados
	NF_InitTiledBgBuffers();	// Inicializa los buffers para almacenar fondos
	NF_InitTiledBgSys(0);		// Inicializa los fondos Tileados para la pantalla superior
	NF_InitTiledBgSys(1);		// Iniciliaza los fondos Tileados para la pantalla inferior

	// Inicializa los Sprites
	NF_InitSpriteBuffers();		// Inicializa los buffers para almacenar sprites y paletas
	NF_InitSpriteSys(0);		// Inicializa los sprites para la pantalla superior

	// Carga los archivos de sonido desde la FAT / NitroFS a la RAM
				// File, ID, freq, format
	NF_LoadRawSound("sounds/Fox Sound", 0, 22050, 0);
	NF_LoadRawSound("sounds/Chicken Sound", 1, 22050, 0);
	NF_LoadRawSound("sounds/Slime Sound", 2, 22050, 0);
	NF_LoadRawSound("sounds/Hit Sound", 3, 22050, 0);
	NF_LoadRawSound("sounds/Money Sound", 4, 22050, 0);
	NF_LoadRawSound("sounds/Error", 5, 22050, 0);
	NF_LoadRawSound("sounds/Select", 6, 22050, 0);
	//NF_LoadRawSound("sounds/Main", 7, 22050, 0);


	// Carga los archivos de sprites desde la FAT / NitroFS a la RAM
	NF_LoadSpriteGfx("sprite/Spritesheet", zorro.id, 16, 16);	// Zorro
	NF_LoadSpritePal("sprite/Spritesheet", zorro.id);

	NF_LoadSpriteGfx("sprite/Spritesheet-Navidad", 4, 16, 16);	// ZorroNavidad
	NF_LoadSpritePal("sprite/Spritesheet-Navidad", 7);

	NF_LoadSpriteGfx("sprite/Cuco", gallina.id, 16, 16);		// Gallina
	NF_LoadSpritePal("sprite/Cuco", gallina.id);


	NF_LoadSpriteGfx("sprite/Slime", 2, 16, 16);		// Slime
	NF_LoadSpritePal("sprite/Slime", 2);
	// Otros colores de Slime
	NF_LoadSpritePal("sprite/SlimeB", 3); 
	NF_LoadSpritePal("sprite/SlimeP", 4);
	NF_LoadSpritePal("sprite/SlimeM", 5);

	NF_LoadSpriteGfx("sprite/LogoLow", 3, 64, 64); // Logo
	NF_LoadSpritePal("sprite/LogoLow", 6); 
 
   	// Inicia la semilla de aleatoriedad
	srand(time(NULL));

	irqSet (IRQ_KEYS, FueraInicio);
	irqEnable (IRQ_KEYS);
	REG_KEYCNT = 0x4001; // A

    while(1) // BUCLE PRINCIPAL
    {	
    	if (pantallaInicio || REG_KEYINPUT != 0x03FE)
    		PantallaInicio();

    	else if (menu){
    		scanKeys();
    		if (keysDown() & KEY_A)
    			MenuPrincipal();
    	}

    	while (!FinDeJuego && !menu) {
			// Mueve el personaje
			if (!zorroCansado)
				MoverZorro();
		
			Colisiones();	

			// Actualiza el array de OAM
			NF_SpriteOamSet(0);

			swiWaitForVBlank();	// Espera al sincronismo vertical

			// Actualiza el OAM
			oamUpdate(&oamMain);
			//oamUpdate(&oamSub);
		}

		if (menu && FinDeJuego) {
			FinDePartida();
		}

		// Actualiza el array de OAM
		NF_SpriteOamSet(0);

		swiWaitForVBlank();		// Espera al sincronismo vertical

		// Actualiza el OAM
		oamUpdate(&oamMain);
		//oamUpdate(&oamSub);
    }
}

/************************ FUNCIONES **********************/

void PantallaInicio() {
	
	if (pantallaInicio) {
		NF_LoadTextFont("fnt/default", "titulo", 256, 256, 0);
		NF_LoadTiledBg("bg/HenandfoxmainLOW", "Inicio", 256, 256);
		NF_LoadTiledBg("bg/BaseLogoLow", "Logo", 256, 256);
		menu = false;

		//NF_CreateTiledBg(0, 0, "Inicio"); // Imagen en la pantalla inferior

		NF_CreateTiledBg(1, 0, "Inicio"); // Imagen en la pantalla inferior
		NF_CreateTiledBg(0, 1, "Logo"); // Imagen en la pantalla inferior

		NF_CreateTextLayer(0, 0, 0, "titulo");
		//screen, ram, vram, keepframes
		NF_VramSpriteGfx(0, 3, 3, false);	// Logo
						//screen, ram, slot
		NF_VramSpritePal(0, 6, 6);

			//screen, id, gfx, pal, x, y
		NF_CreateSprite(0, 3, 3, 6, x, y);

		pantallaInicio = false;
	}

	NF_WriteText(0, 0, 2, 20, "POR MARC PITARCH DOS SANTOS");
	NF_WriteText(0, 0, 20, 22, "Pulsa A");
	NF_UpdateTextLayers();

	if (x <= 120)
		NF_MoveSprite(0, 3, ++x, ++y);
	else
		NF_MoveSprite(0, 3, x--, y--);
}

void FueraInicio () {

if (!menu){

	NF_ResetTiledBgBuffers(); // Borra todos los tiles cargados de la RAM

	irqDisable(IRQ_KEYS);

	NF_DeleteSprite(0, 3);
	NF_UnloadSpriteGfx(3);
	NF_DeleteTiledBg(1, 0);
	NF_DeleteTiledBg(0, 1);
	NF_DeleteTextLayer(0, 0);

	menu = true;
	
	// Actualiza el array de OAM
	NF_SpriteOamSet(0);

	swiWaitForVBlank();		// Espera al sincronismo vertical

	// Actualiza el OAM
	oamUpdate(&oamMain);
	//oamUpdate(&oamSub);

	
}
}

void MenuPrincipal() {
	
	NF_LoadTextFont("fnt/default", "titulo", 256, 256, 0);
	NF_LoadTextFont("fnt/default", "titulo1", 256, 256, 0);
						// screen, layer, rotation, name
	NF_CreateTextLayer(0, 0, 0, "titulo");
	NF_CreateTextLayer(1, 1, 0, "titulo1");


	u8 opcion = 0;

	u8 subMenu = 0;

	scanKeys();

	while (menu) {
					//screen, layer, x, y, texto
		NF_WriteText(0, 0, 5, 1, "BIENVENIDO A HEN & FOX");
		NF_WriteText(0, 0, 2, 7, "Jugar");
		NF_WriteText(0, 0, 2, 9, "Tienda");
		
		NF_UpdateTextLayers();

		if (opcion == 0) {
			NF_ClearTextLayer(0,0);
			NF_WriteText(0, 0, 8, 7, "~");
			NF_WriteText(1, 1, 0, 1, "Protege al pollo de los slimes");
		}
		else if (opcion == 1){
			NF_ClearTextLayer(0,0);
			NF_WriteText(0, 0, 9, 9, "~");
			NF_WriteText(1, 1, 0, 1, "Gasta las monedas que consigas ");
		}
		
		scanKeys();
		int pressed = keysDown();

		if (pressed & KEY_UP){
			if (opcion == 1){
				NF_PlayRawSound(6, 127, 64, false, 0); // Select
				opcion--;
			}
			else 
				NF_PlayRawSound(5, 127, 64, false, 0); // Error
		}

		else if (pressed & KEY_DOWN) {
			if (opcion == 0){
				NF_PlayRawSound(6, 127, 64, false, 0); // Select
				opcion++;
			}
			else
				NF_PlayRawSound(5, 127, 64, false, 0); // Error
		}

		else if (pressed & KEY_A) { // Submenú TIENDA/JUGAR
			subMenu = 1;
			NF_ClearTextLayer(0,0);
			NF_ClearTextLayer(1, 1);
			NF_UpdateTextLayers();
			while (opcion == 0 && subMenu == 1) {// Jugar
				//NF_WriteText(0, 0, 2, 5, "Escoge dificultad:");
				NF_WriteText(0, 0, 3, 7, "Nivel 1");
				if (nivel > 0)
					NF_WriteText(0, 0, 3, 9, "Nivel 2");

				NF_UpdateTextLayers();

				NF_WriteText(1, 1, 0, 5, "Muevete con la cruceta, y asestacontra los slimes que buscan    comerse al pollo.               Vigila tu energia");

				if (dificultad == 0)
				{
					NF_ClearTextLayer(0,0);
					NF_WriteText(0, 0, 11, 7, "~");
					NF_WriteText(1, 1, 0, 1, "Protege al pollo tranquilo");
				}
				else if (dificultad == 1)
				{
					NF_ClearTextLayer(0,0);
					NF_WriteText(0, 0, 11, 9, "~");
					NF_WriteText(1, 1, 0, 1, "Te vas a c*gar            ");
				}

				scanKeys();
				int pressed = keysDown();

				if (pressed & KEY_UP){
					if (dificultad == 1){
						NF_PlayRawSound(6, 127, 64, false, 0); // Select
						dificultad--;
					}
					else
						NF_PlayRawSound(5, 127, 64, false, 0); // Error
				}

				else if (pressed & KEY_DOWN) {
					if (dificultad == 0 && nivel > 0){
						NF_PlayRawSound(6, 127, 64, false, 0); // Select
						dificultad++;
					}
					else
						NF_PlayRawSound(5, 127, 64, false, 0); // Error
				}
				if (pressed & KEY_A) {
					NF_PlayRawSound(6, 127, 64, false, 0); // Select
					// Borramos la pantalla
					NF_ClearTextLayer(0, 0);
					NF_ClearTextLayer(1, 1);
					menu = false;
					subMenu = 0;
					// Eliminamos todos los pasos que se repetirán al cargar el menú
					NF_DeleteTextLayer(0, 0);
					NF_DeleteTextLayer(1, 1);
					/********** EMPEZAR EL JUEGO **********/
					CargarJuego(); 
				}

				else if (pressed & KEY_B)
				{
					NF_ClearTextLayer(1, 1);
					subMenu = 0;
				}
			}

			while (opcion == 1 && subMenu == 1) { // TIENDA
				subMenu = Tienda(subMenu);
			}
		}
	}
}

u8 Tienda(u8 subMenu) { // MEJORAS
	irqSet (IRQ_TIMER2, GeneradorEnemigos); // Animaciones lentas
	TIMER_DATA (2) = 0; // Cada ~2 sec
	TIMER_CR (2) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ;	

	u8 opcion = 0;

	while(subMenu == 1) {
		NF_WriteText(0, 0, 2, 3, "Bienvenido a la tienda");
		NF_WriteText(0, 0, 1, 7, "100$  Afilar dientes (ATQ+5)"); // Opcion 0
		NF_WriteText(0, 0, 1, 9, "100$  Crecer pelaje (DFS+5)"); // Opcion 1
		NF_WriteText(0, 0, 1, 11, "500$  Disfraz de navidad"); // Opcion 2

		sprintf(monedero, "%i $", dinero);
		sprintf(daño, "ATQ actual: %i ", zorro.daño);
		sprintf(defensa, "DFS actual: %i ", zorro.defensa);

		NF_WriteText(0, 0, 1, 20, monedero);

		NF_UpdateTextLayers();

		if (opcion == 0)
		{
			NF_ClearTextLayer(0,0);
			NF_WriteText(0, 0, 0, 7, "~");
			NF_WriteText(1, 1, 0, 1, daño);
		}
		else if (opcion == 1)
		{
			NF_ClearTextLayer(0,0);
			NF_WriteText(0, 0, 0, 9, "~");
			NF_WriteText(1, 1, 0, 1, defensa);
		}

		else if (opcion == 2)
		{
			NF_ClearTextLayer(0,0);
			NF_WriteText(0, 0, 0, 11, "~");
			if (navidad)
				NF_WriteText(1, 1, 0, 1, "¡HO, HO, HO!");
		}

		scanKeys();
		int pressed = keysDown();

		if (pressed & KEY_UP){
			if (opcion > 0){
				NF_PlayRawSound(6, 127, 64, false, 0); // Select
				opcion--;
			}
			else
				NF_PlayRawSound(5, 127, 64, false, 0); // Error
		}

		else if (pressed & KEY_DOWN) {
			if (opcion < 2){
				NF_PlayRawSound(6, 127, 64, false, 0); // Select
				opcion++;
			}
			else
				NF_PlayRawSound(5, 127, 64, false, 0); // Error
		}

		else if (pressed & KEY_A) { // Escoge un artículo
			irqEnable(IRQ_TIMER2);
			if (opcion == 0) {
				if (dinero >= 100) {
					NF_PlayRawSound(4, 100, 64, false, 0);
					zorro.daño += 5;
					dinero -= 100;
					Compra(opcion);
				}
				else if (dinero < 100)
					Compra(9);
				
			}
			else if (opcion == 1) {
				if (dinero >= 100) {
					NF_PlayRawSound(4, 100, 64, false, 0);
					zorro.defensa += 5;
					dinero -= 100;				
					Compra(opcion);
				}
				else if ( dinero < 100)
					Compra(9);
			}
			else if (opcion == 2) { // TRAJE DE NAVIDAD
				if (dinero >= 500) {
					dinero-=500;
					NF_PlayRawSound(4, 100, 64, false, 0);
					navidad = true;
					Compra(opcion);
				}
				else
					Compra(9);
			}
			irqDisable(IRQ_TIMER2);
			contador = 0;

			NF_UpdateTextLayers();
		}

		else if (pressed & KEY_B)
			subMenu = 0;
	}
	return subMenu;
}

void Compra(u8 opcion) {
	if (opcion == 9)
		NF_PlayRawSound(5, 127, 64, false, 0); // Error
	while (contador < 1) {
		NF_WriteText(0, 0, 2, 3, "Bienvenido a la tienda");
		NF_WriteText(0, 0, 1, 7, "100$  Afilar dientes (ATQ+5)"); // Opcion 0
		NF_WriteText(0, 0, 1, 9, "100$  Crecer pelaje (DFS+5)"); // Opcion 1
		NF_WriteText(0, 0, 1, 11, "500$  Disfraz de navidad"); // Opcion 2

		sprintf(monedero, "%i $", dinero);

		NF_WriteText(0, 0, 1, 20, monedero);


		if (opcion == 1 || opcion == 0 || opcion == 2)
			NF_WriteText(0, 0, 1, 15, "¡Gracias por la compra!");
		else
			NF_WriteText(0, 0, 1, 15, "No tienes dinero -.-");

		NF_UpdateTextLayers();
	}
}

void CargarJuego() {

	// Carga los archivos de fondo desde la FAT / NitroFS a la RAM
	if (dificultad == 0)
		NF_LoadTiledBg("bg/FondoVerde", "FondoVerde", 256, 256);			// Carga el fondo para la pantalla superior
	else if (dificultad == 1)
		NF_LoadTiledBg("bg/FondoVerdeNoche", "FondoVerde", 256, 256);
	NF_LoadTiledBg("bg/FondoPantallaInferior", "MaderaInferior", 256, 256);		// Carga el fondo para la  pantalla inferior
	NF_LoadTiledBg("bg/DatosInferior", "DatosInferior", 256, 256);		// Carga el fondo para los datos de la pantalla inferior

	// Crea los fondos de la pantalla superior
	// screen, layer (0-3), nombre
	
	NF_CreateTiledBg(0, 0, "FondoVerde");
	// Crea los fondos de la pantalla inferior
	NF_CreateTiledBg(1, 2, "MaderaInferior");
	NF_CreateTiledBg(1, 1, "DatosInferior");   
					// screen, layer, rotation, name
	NF_CreateTextLayer(1, 0, 0, "titulo"); // Layer diferente a los tiled BG

	// Ecribe los datos de los personajes		
	ActualizarStatsInferior();

	NF_LoadTextFont("fnt/default", "titulo", 256, 256, 0);		

	if (primeraPartida) { // Solo cargamos los sprites en la VRAM la primera vez
					
		// Transfiere a la VRAM los sprites necesarios

		// Los de zorro los ponemos luego, según el traje que lleve
		/*				//screen, ram, vram, keepframes
		NF_VramSpriteGfx(0, 0, 0, false);	// Zorro, copia todos los frames a la VRAM
		NF_VramSpriteGfx(0, 4, 0, false);
						//screen, ram, slot
		NF_VramSpritePal(0, 0, 0);
		//NF_VramSpritePal(0, 7, 0); // Navidad */

					// screen, ram, vram, keepframes
		NF_VramSpriteGfx(0, 1, 1, false);	// Gallina, copia todos los frames a la VRAM
		NF_VramSpritePal(0, 1, 1);

		NF_VramSpriteGfx(0, 2, 2, false);	// Slime, manten los frames adicionales en RAM
		NF_VramSpritePal(0, 2, 2);
		NF_VramSpritePal(0, 3, 3);
		NF_VramSpritePal(0, 4, 4);
		NF_VramSpritePal(0, 5, 5);

		

	}
	// Cambiar el daño de los slimes según el nivel
	if (dificultad == 0)
		slimeSample.daño = 6;
	else if (dificultad == 1)
		slimeSample.daño = 12;

	// Cargar los gráficos del Zorro según el traje que lleve
	if (!navidad){
		if (!primeraPartida)
			NF_FreeSpriteGfx(0, 0);
		NF_VramSpriteGfx(0, 0, 0, false);	// Zorro, copia todos los frames a la VRAM
		NF_VramSpritePal(0, 0, 0);
		//screen, id, gfx, pal, x, y
		NF_CreateSprite(0, zorro.id, 0, 0, zorro.posX, zorro.posY);
	}
	else{
		NF_FreeSpriteGfx(0, 0);
		NF_VramSpriteGfx(0, 4, 0, false);
		NF_VramSpritePal(0, 7, 0); // Navidad
		NF_CreateSprite(0, 0, 0, 0, zorro.posX, zorro.posY);
	}

	NF_CreateSprite(0, gallina.id, 1, 1, gallina.posX, gallina.posY);

	for (int i = 0; i < TAMAÑO_SLIME; i++)
		slime[i] = slimeSample;

	primeraPartida = false;

	ConfigurarInterrupciones();
}

void ConfigurarInterrupciones() {

	irqEnable(IRQ_TIMER0);
	irqSet (IRQ_TIMER0, RealizarAnimación); // Animaciones mUY RÁPIDAS
	TIMER_DATA (0) = 60000; // Cada ~0.1 sec
	TIMER_CR (0) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ;

	irqEnable(IRQ_TIMER1); // Animaciones rápidas
	irqSet (IRQ_TIMER1, MovimientoEnemigos);
	TIMER_DATA (1) = 50000; // Cada ~0.3 sec
	TIMER_CR (1) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ;	

	irqEnable(IRQ_TIMER2);
	irqSet (IRQ_TIMER2, GeneradorEnemigos); // Animaciones lentas
	TIMER_DATA (2) = 0; // Cada ~2 sec
	TIMER_CR (2) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ;	

	//irqEnable(IRQ_TIMER3);
	irqSet (IRQ_TIMER3, Contador); // Extra rápido
	TIMER_DATA (3) = 64000; // Cada ~2 sec
	TIMER_CR (3) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ;	
}

void MoverZorro() {

 	scanKeys();
	int pressed=keysHeld();
	if (pressed & KEY_UP || pressed & KEY_DOWN || pressed & KEY_RIGHT || pressed & KEY_LEFT) {
			if (pressed & KEY_UP && zorro.posY > 0)
			{
				zorro.posY--;
				zorro.state = UP;
			}
			else if (pressed & KEY_DOWN && zorro.posY + 16 < 192)
			{
				zorro.posY++;
				zorro.state = DOWN;
			}
			if (pressed & KEY_RIGHT && zorro.posX + 16 < 256)
			{	
				zorro.posX++;
				zorro.state = RIGHT;
			}
			else if (pressed & KEY_LEFT && zorro.posX > 0)
			{
				zorro.posX--;
				zorro.state = LEFT;
			}
		if (realizarZorro) { // Si puede animarse, solo cuando se mueve
			// Sonido de caminar
					// ID (Fox sound), volumen, pan, loop, loopstart
			NF_PlayRawSound(0, 22, 64, false, 0);
			// Animacion del personaje
			zorro.anim_frame ++;
			if (zorro.anim_frame >= 2) zorro.anim_frame = 0;
			// Salta al número de animación indicado
			NF_SpriteFrame(0, zorro.id, zorro.anim_frame + 2 * zorro.state); 
			realizarZorro = false;
		}	
		NF_MoveSprite(0, zorro.id, zorro.posX, zorro.posY);
	}	
}

// Animaciones y esperas
void RealizarAnimación () {
	if (!menu) {

		realizarZorro = true;

		gallina.anim_frame ++;
		if (gallina.anim_frame >= 2) gallina.anim_frame = 0;
		NF_SpriteFrame(0, gallina.id, gallina.anim_frame); // Salta al número de animación indicado

		for (int i = 0; i < TAMAÑO_SLIME; i++) {
			if (slime[i].state == 1)
			{
				slime[i].anim_frame ++;
				if (slime[i].anim_frame >= 6) slime[i].anim_frame = 0;
				NF_SpriteFrame(0, i+2, slime[i].anim_frame);
			}	
		}
		ActualizarStatsInferior(); // AQUÍ HAbía UN ERROR GORDO, no sé porque, dentro del if(zorroCansado), que estaba aquí antes, no iba :////
	}   
}

void MovimientoEnemigos() {
	if (!menu){

		//NF_PlayRawSound(2, 20, 64, false, 0);

		//Segun esten más arriba/abajo, derecha/izquierda del pollo
		for (int i = 0; i < TAMAÑO_SLIME; i++) {
			if (slime[i].state == 1){ // Si está activo
				if (slime[i].posX < gallina.posX)
					slime[i].posX += 3;
				else if (slime[i].posX > gallina.posX)
					slime[i].posX -= 3;
				if (slime[i].posY < gallina.posY)
					slime[i].posY += 3;
				else if (slime[i].posY > gallina.posY)
					slime[i].posY -= 3;
				// Movimiento del Slime
				NF_MoveSprite(0, i+2, slime[i].posX, slime[i].posY);
			}
		}
	}
}

void GeneradorEnemigos() {
	if (!menu) {

		if (enemigosGenerados == TAMAÑO_SLIME-1)
			enemigosGenerados = 0;

		if (enemigosEliminados == 0 && generar) {

			slime[0].state = 1;
			slime[0].posX = -15;
			slime[0].posY = rand() % 193;
			if (dificultad == 1)
				slime[0].defensa = 2;
						// screen, id, gfx, pal, x, y
			NF_CreateSprite(0, 2, 2, 2, slime[0].posX, slime[0].posY);
			enemigosEnPantalla++;
			enemigosGenerados ++;
			generar = false;
		}
		else {
			// Asegurarnos que hay una cantidad limitada de Slimes en la pantalla
			if (enemigosEnPantalla > 4 && dificultad == 0)
				generar = false;
			else if (enemigosEnPantalla > 5 && dificultad == 1)
				generar = false;
			if (enemigosGenerados > 11) generar = false;

			if (generar) {

				enemigosGenerados++;
				u8 sel = rand() % 4;

				if (sel == 0) { // Derecha
					slime[enemigosGenerados].posX = -15;
					slime[enemigosGenerados].posY = rand() % 193;
				}
				else if (sel == 1) { // Arriba
					slime[enemigosGenerados].posX = rand() % 257;
					slime[enemigosGenerados].posY = -15;
				}
				else if (sel == 2) { // Izquierda
					slime[enemigosGenerados].posX = 271;
					slime[enemigosGenerados].posY = rand() % 193;
				}

				else { // Abajo
					slime[enemigosGenerados].posX = rand() % 257;
					slime[enemigosGenerados].posY = 207;

				}

				if (dificultad == 1)
					slime[enemigosGenerados].defensa = 1;

				slime[enemigosGenerados].state = 1;

				NF_CreateSprite(0, enemigosGenerados+2, 2, sel+2, slime[enemigosGenerados].posX, slime[enemigosGenerados].posY);

				enemigosEnPantalla++;
			}
		}
	}
	else {
		contador++;
	}
}

// Por cada slime
void Colisiones() {

	for (int i = 0; i < TAMAÑO_SLIME; i++)
		if (slime[i].state == 1){ // Si está activo

			// Colisión con Zorro si no está cansado
			if (zorro.posX + 8 <= slime[i].posX + 16 && zorro.posX + 8 >= slime[i].posX &&
				zorro.posY + 8 <= slime[i].posY + 16 && zorro.posY + 8 >= slime[i].posY && !zorroCansado) { 

							// ID (HIT), volumen, pan, loop, loopstart
				NF_PlayRawSound(3, rand() % 30 + 98, 64, false, 0);

				// Actualizar puntos de vida
				slime[i].HP -= zorro.daño - slime[i].defensa;
				zorro.HP -= (slime[i].daño - zorro.defensa/2);

				// Actualizar posiciones
				u8 empuje = 10;
				if (zorro.state == UP)
					for (u8 j = 0; j< empuje; j++) {
						slime[i].posY -= 1;
						zorro.posY += 1;
					}

				else if (zorro.state == DOWN)
					for (u8 j = 0; j< empuje; j++) {
						slime[i].posY += 1;
						zorro.posY -= 1;
					}

				else if (zorro.state == RIGHT)
					for (u8 j = 0; j< empuje; j++) {
						slime[i].posX += 1;
						zorro.posX -= 1;
					}

				else if (zorro.state == LEFT)
					for (u8 j = 0; j< empuje; j++) {
						slime[i].posX -= 1;
						zorro.posX += 1;
					}
			}

			// Si se muere UN SLIME
			if (slime[i].HP <= 0) {

				slime[i].state = 0;

				NF_PlayRawSound(4, 115, 64, false, 0); // Money Sound
				
				enemigosEliminados++;
				enemigosEnPantalla--;

				if (dificultad == 0) // El dinero que devuelve según la dificultad
					dinero += rand() % 5 + 3;
				else dinero += rand () % 10 + 4;

				NF_DeleteSprite(0, i+2);
				if (enemigosEliminados >= 12 && enemigosEnPantalla == 0){
					FinDeJuego = true;
					menu = true;
					victoria = true;
				}
				else generar = true;
				
				
			}

			if (zorro.HP <= 0)
				zorroCansado = true;

			// Si un slime toca la gallina
			if (slime[i].posX + 8 <= gallina.posX + 16 && slime[i].posX + 8 >= gallina.posX &&
				slime[i].posY + 8 <= gallina.posY + 16 && slime[i].posY + 8 >= gallina.posY) {

				NF_PlayRawSound(1, 127, 64, false, 0); // Gallina grito

				gallina.HP -= slime[i].daño;

				if (gallina.HP <= 0){
					FinDeJuego = true;
					menu = true;
				}

				u8 empuje = 10;

				if (slime[i].posX > gallina.posX)
					for (u8 j = 0; j< empuje; j++)
						slime[i].posX += 1;

				if (slime[i].posX <= gallina.posX)
					for (u8 j = 0; j< empuje; j++)
						slime[i].posX -= 1;
						
					
				if (slime[i].posY > gallina.posY)
					for (u8 j = 0; j< empuje; j++)
						slime[i].posY += 1;
						

				if (slime[i].posY <= gallina.posY)
					for (u8 j = 0; j< empuje; j++)
						slime[i].posY -= 1;
			}

			// Actualizar Sprites
			NF_MoveSprite(0, i+2, slime[i].posX, slime[i].posY);
			NF_MoveSprite(0, zorro.id, zorro.posX, zorro.posY);

		}
}

// Actualizar los datos de la pantalla inferior de juego
void ActualizarStatsInferior() {

	NF_ClearTextLayer(1, 0);
	sprintf(vida, "%i", zorro.HP);
	sprintf(daño, "%i", zorro.daño);
	sprintf(defensa, "%i", zorro.defensa);
	sprintf(pollo, "%i", gallina.HP);
	sprintf(eliminados, "%i", enemigosGenerados);
	sprintf(monedero, "%i $", dinero);

			//screen, layer, x, y, texto
	NF_WriteText(1, 0, 17, 7, vida);

	NF_WriteText(1, 0, 8, 7, daño);

	NF_WriteText(1, 0, 29, 7, defensa);

	NF_WriteText(1, 0, 17, 16, pollo);

	NF_WriteText(1, 0, 1, 1, monedero);

	if (zorroCansado){
		NF_WriteText(1, 0, 10, 1, "¡EXHAUSTO!");
		zorro.HP += 4;
		if (zorro.HP >= 100){
			zorroCansado = false;
		}
		if (zorro.HP > 100){
			zorro.HP = 100;
		}
		
	}

	NF_UpdateTextLayers();
}

void FinDePartida() {

	irqDisable(IRQ_TIMER0);
	irqDisable(IRQ_TIMER1);
	irqDisable(IRQ_TIMER2);
	irqEnable(IRQ_TIMER3);
	// reseteo de variables inmediatas

	NF_DeleteTiledBg(0, 0); // "FondoVerde"
	NF_DeleteTiledBg(1, 2); // "MaderaInferior"
	NF_DeleteTiledBg(1, 1);   // "DatosInferior"
		
	NF_DeleteTextLayer(1, 0); 

	NF_ResetTiledBgBuffers();

	NF_DeleteSprite(0, 0);
	NF_DeleteSprite(0, 1);
	for (u8 i = 0; i < TAMAÑO_SLIME; i++)
		if (slime[i].state == 1){
			slime[i].state = 0;
			NF_DeleteSprite(0, i+2);
		}
	
	// Actualiza el array de OAM
	NF_SpriteOamSet(0);

	swiWaitForVBlank();	// Espera al sincronismo vertical

	// Actualiza el OAM
	oamUpdate(&oamMain);
	//oamUpdate(&oamSub);

	// Introducimos los fondos de texto otra vez
	NF_LoadTextFont("fnt/default", "titulo", 256, 256, 0);
	NF_LoadTextFont("fnt/default", "titulo1", 256, 256, 0);
	NF_CreateTextLayer(0, 0, 0, "titulo");
	NF_CreateTextLayer(1, 1, 0, "titulo1");

	
	if (victoria && dificultad == 0)
		dinero += 20;
	else if (victoria)
		dinero += 50;

	if (gallina.HP == 100)
		dinero += 30;

	nivel++;

	while (FinDeJuego) { // Bucle final
			NF_ClearTextLayer(0, 0);
			if (!victoria) {
				NF_WriteText(0, 0, 7, 3, "Derrota :(");
			}
			else{
				NF_WriteText(0, 0, 7, 3, "VICTORIA");
				if (gallina.HP == 100)
					NF_WriteText(0, 0, 17, 3, "EXCELENTE");

			}

			sprintf(monedero, "Tu dinero : %i $", contadorFinal);
			NF_WriteText(0, 0, 3, 6, monedero);
	
			if (contadorFinal == dinero){

				NF_WriteText(0, 0, 1, 20, "A para volver al menu principal...");

				scanKeys();

				int pressed = keysDown();

				if (pressed & KEY_A) {
					irqDisable(IRQ_TIMER3);
					InicializarVariablesGlobales(); // Reseteo de variables del juego
					MenuPrincipal(); // Vuelta al inicio
				}
			}

			NF_UpdateTextLayers();
	}
}

void InicializarVariablesGlobales() {

	victoria = false;

	FinDeJuego = false;

	dificultad = 0;

	contadorFinal = 0; // delay

	zorroCansado = false;
	// Enemigos

	generar = true;

	enemigosEliminados = 0;

	enemigosGenerados = 0;

	enemigosEnPantalla = 0; // Controlar la densidad de enemigos en pantalla

	// Personajes

	zorro.posX = 90;
	zorro.posY = 96;
	zorro.HP = 100;
	
	gallina.posX = 128;
	gallina.posY = 96;
	gallina.HP = 100; 
}

void Contador () {
	if (contadorFinal < dinero)
	{
		contadorFinal+=4;
	}
	if (contadorFinal > dinero)
		contadorFinal = dinero;
}