/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_ts.h"
#include "stdio.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MAX_CONVERTED_VALUE   4095    // Valor convertido maximo
#define AMBIENT_TEMP            25    // Temperatura ambiente
#define VSENS_AT_AMBIENT_TEMP  760    // valor VSENSE (mv) à temperatura ambiente
#define AVG_SLOPE               25    // Avg_Solpe mmultiplicado por 10
#define VREF                  3300
#define ncol        8 //número de linhas/colunas
#define lincol0     0 *(BSP_LCD_GetYSize()/ncol) + BSP_LCD_GetYSize()/(2*ncol) //linha média entre o ponto inicial e a linha/coluna 1(para encontrar o centro do quadrado)
#define lincol1		1 *(BSP_LCD_GetYSize()/ncol) + BSP_LCD_GetYSize()/(2*ncol)//linha média entre a linnha/coluna 1 e a linha/coluna 2(para encontrar o centro do quadrado)
#define lincol2		2 *(BSP_LCD_GetYSize()/ncol) + BSP_LCD_GetYSize()/(2*ncol)//linha média entre a linnha/coluna 2 e a linha/coluna 3(para encontrar o centro do quadrado)
#define lincol3		3 *(BSP_LCD_GetYSize()/ncol) + BSP_LCD_GetYSize()/(2*ncol)//linha média entre a linnha/coluna 3 e a linha/coluna 4(para encontrar o centro do quadrado)
#define lincol4		4 *(BSP_LCD_GetYSize()/ncol) + BSP_LCD_GetYSize()/(2*ncol)//linha média entre a linnha/coluna 4 e a linha/coluna 5(para encontrar o centro do quadrado)
#define lincol5		5 *(BSP_LCD_GetYSize()/ncol) + BSP_LCD_GetYSize()/(2*ncol)//linha média entre a linnha/coluna 5 e a linha/coluna 6(para encontrar o centro do quadrado)
#define lincol6		6 *(BSP_LCD_GetYSize()/ncol) + BSP_LCD_GetYSize()/(2*ncol)//linha média entre a linnha/coluna 6 e a linha/coluna 7(para encontrar o centro do quadrado)
#define lincol7		7 *(BSP_LCD_GetYSize()/ncol) + BSP_LCD_GetYSize()/(2*ncol)//linha média entre a linnha/coluna 7 e a linha/coluna 8(para encontrar o centro do quadrado)
#define fimdojogo   0 //indica display de fim do jogo
#define emjogo      1 //indica display de jogo
#define menu        2 //indica display de menu
#define jpreto      1 //jogador de bola preta
#define jvermelho   2 //jogador de bola vermelha
#define semjogador  0 //local na matriz sem jogadores
#define ARMplayer   3 //ARM a jogar (bola vermelha)



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
volatile int flag=0;//assinala a passagem dos 2 segundos para ler temperatura
volatile int jogada;//assinala que houve um toque no LCD durante o jogo
volatile int linha;//coordenada y do ponto central do quadrado selecionado pelo jogador
volatile int coluna; //coordenada x do ponto central do quadrado selecionado pelo jogador
int getxyBoardPosition=(480/ncol); // largura de cada quadrado do tabuleiro do jogo
volatile uint8_t f_lcdPressed = 0;//flag que permite detetar apenas um toque no LCD enquanto estiver ativada
int gameBoard[ncol][ncol];//matriz representativa do tabuleiro do jogo, possui indicação da posição de todas as peças
TS_StateTypeDef TS_State;
int linhaverde; //representa uma determinada linha no LCD
int colunaverde; //representa uma determinada coluna no LCD
int countRed=0; //conta o número de peças pretas
int countBlack=0; //conta o número de peças vermelhas
int playgame=menu;//assinala se estamos a ver o menu, a jogar o jogo ou a mostrar a mensagem de vitória
int escolha=0;//flag que assinala que houve um toque no LCD no menu
int avail[ncol*ncol]={0};//cria um array de movimentos possíveis e coloca tudo a 0
int play=1;
int allEnemies[ncol][2]; //cria um array com a direção (diferença entre as coordenadas do inimigo e s coordenadas do movimento) de todos os inimigos próximos
char nextPlayer[25];
char winmessage=0;//flag para assinalar a altura de mostrar a mensagem de vitória
volatile int segundos=0; //segundos que passaram desde o início do jogo
int minutos=0; //minutos que passaram desde o início do jogo
volatile int timeflag=0; //flag do timer 7, timer da duração do jogo
unsigned int nBytes;
volatile int countdown = 20;//20 segundos da jogada
volatile int timeout=0; //flag que assinala fim de 20 segundos da jogada
volatile int start=0;//flag do timer 13, timer da duração da jogada
volatile int ARMplayerflag=0; //se a 0 indica jogo entre 2 jogadores, se a 1 indica jogo entre 1 jogador e o ARM
int jpretotimeout=0; //número de vezes que o jogador preto usou o timeout (demorou + de 20 segundos a jogar)
int jvermelhotimeout=0;//número de vezes que o jogador vermelho usou o timeout (demorou + de 20 segundos a jogar)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DMA2D_HandleTypeDef hdma2d;

DSI_HandleTypeDef hdsi;

LTDC_HandleTypeDef hltdc;

SD_HandleTypeDef hsd2;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim13;

SDRAM_HandleTypeDef hsdram1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_DMA2D_Init(void);
static void MX_DSIHOST_DSI_Init(void);
static void MX_FMC_Init(void);
static void MX_LTDC_Init(void);
static void MX_SDMMC2_SD_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM13_Init(void);
/* USER CODE BEGIN PFP */
static void LCD_Config();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*-----------------------------------------------------------------------------------------------
  Recebe a posição do LCD, x e y em que o jogador carregou
  Verifica entre que linhas e colunas se encontra (para verificar que "quadrado" foi selecionado)
  Coloca nas variáveis coluna e linha o valor de x e y , respetivamente,
  do ponto central do quadrado selecionado.
-------------------------------------------------------------------------------------------------*/
void setPosition(uint16_t x, uint16_t y)
{
	if(x<=BSP_LCD_GetYSize()/ncol)
		coluna=lincol0;
	else if(x<=2*BSP_LCD_GetYSize()/ncol && x>BSP_LCD_GetYSize()/ncol)
		coluna=lincol1;
	else if(x<=3*BSP_LCD_GetYSize()/ncol && x>2*BSP_LCD_GetYSize()/ncol)
		coluna=lincol2;
	else if(x<=4*BSP_LCD_GetYSize()/ncol && x>3*BSP_LCD_GetYSize()/ncol)
		coluna=lincol3;
	else if(x<=5*BSP_LCD_GetYSize()/ncol && x>4*BSP_LCD_GetYSize()/ncol)
		coluna=lincol4;
	else if(x<=6*BSP_LCD_GetYSize()/ncol && x>5*BSP_LCD_GetYSize()/ncol)
		coluna=lincol5;
	else if(x<=7*BSP_LCD_GetYSize()/ncol && x>6*BSP_LCD_GetYSize()/ncol)
		coluna=lincol6;
	else if(x<=8*BSP_LCD_GetYSize()/ncol && x>7*BSP_LCD_GetYSize()/ncol)
		coluna=lincol7;

	if(y<=BSP_LCD_GetYSize()/ncol)
		linha=lincol0;
	else if(y<=2*BSP_LCD_GetYSize()/ncol && y>BSP_LCD_GetYSize()/ncol)
		linha=lincol1;
	else if(y<=3*BSP_LCD_GetYSize()/ncol && y>2*BSP_LCD_GetYSize()/ncol)
		linha=lincol2;
	else if(y<=4*BSP_LCD_GetYSize()/ncol && y>3*BSP_LCD_GetYSize()/ncol)
		linha=lincol3;
	else if(y<=5*BSP_LCD_GetYSize()/ncol && y>4*BSP_LCD_GetYSize()/ncol)
		linha=lincol4;
	else if(y<=6*BSP_LCD_GetYSize()/ncol && y>5*BSP_LCD_GetYSize()/ncol)
		linha=lincol5;
	else if(y<=7*BSP_LCD_GetYSize()/ncol && y>6*BSP_LCD_GetYSize()/ncol)
		linha=lincol6;
	else if(y<=8*BSP_LCD_GetYSize()/ncol && y>7*BSP_LCD_GetYSize()/ncol)
		linha=lincol7;

}
/*-----------------------------------------------------------------------------------------------
 Verifica o que se encontra na posição do array atualmente selecionada
 Com as coordenadas do  ponto central do quadrado selecionado pelo jogador (linha e coluna)
 obtém o valor l(linha) e c(coluna) para colocar no array de posições do jogo.
 Retorna o valor que se encontra nessa posição do array
-------------------------------------------------------------------------------------------------*/
static void LCD_GameBoard(void);
int checkBoardPlace()
{
	//posição no LCD do x e do y a dividir pela largura de cada quadrado no LCD
	int l= (linha-(BSP_LCD_GetYSize()/(2*ncol)))/getxyBoardPosition;//número da linha da matriz
	int c= (coluna-(BSP_LCD_GetYSize()/(2*ncol)))/getxyBoardPosition;//número da coluna na matriz

	int resultado=gameBoard[l][c];

	return resultado;
}
/*-----------------------------------------------------------------------------------------------
Recebe o número do jogador que fez a jogada e colaca-o na posição do array selecionada
-------------------------------------------------------------------------------------------------*/
void putInBoard(int player)
{
	//posição no LCD do x e do y a dividir pela largura de cada quadrado no LCD
	int l=(linha-(BSP_LCD_GetYSize()/(2*ncol)))/getxyBoardPosition;//número da linha da matriz
	int c=(coluna-(BSP_LCD_GetYSize()/(2*ncol)))/getxyBoardPosition;//número da coluna na matriz
	gameBoard[l][c]=player;
}
/*-----------------------------------------------------------------------------------------------
Recebe o numero do jogador e a linha e coluna onde pretende colocar a sua peça
Coloca o símbolo no local pretendido

Usada para colocar as primeiras 4 peças no tabuleiro (visto nesta altura o tabuleiro se encontrar
vazio estas não precisam de verificação antes de serem colocadas)
-------------------------------------------------------------------------------------------------*/
void putInBoardFirstPositions( int player, int lin, int col)
{

	gameBoard[lin][col]=player;
}
/*-----------------------------------------------------------------------------------------------
Limpa a matriz.
Percorre a matriz e coloca-a a 0 ou seja, sem jogadores.
-------------------------------------------------------------------------------------------------*/
void BoardMatrixInitial() {
	for (int i = 0; i < ncol; i++) {
		for (int j = 0; j < ncol; j++) {
			gameBoard[i][j] = semjogador;

		}
	}
}
/*-----------------------------------------------------------------------------------------------
Verifica se a peça inimiga se encontra presa.
Recebe o número do jogador, as coordenadas onde o jogador pretende jogar (row e col)
e a diferença de coordenadas dessa posição e da posição da peça inimiga em estudo.
Segue a direção da peça inimiga e verifica se esta se encontra presa por uma peça do jogador
Retorna 0 se a peça não estiver presa e 1 se estiver presa por uma peça do jogador
-------------------------------------------------------------------------------------------------*/
int checkTrapped( int player, int row, int col, int i, int j){

    int mult;
    for(mult=1; 1 ; mult++) {
        if (mult*i+row<0 || mult*i+row>=ncol || //impede de passar os limites da matriz
            mult*j+col<0 || mult*j+col>=ncol )
            return 0;
        if (gameBoard[mult*i+row][mult*j+col] == 0) //retorna falso se não houver peça a prender a peça inimiga
            return 0;
        if (gameBoard[mult*i+row][mult*j+col] == player) //returna 1 se a peça(ou grupo de peças)estiver presa
            return 1;
    }
}
/*-----------------------------------------------------------------------------------------------
Converte posições da matriz num número onde o valor das dezenas é a linha+1 e o das unidades a coluna+1
Recebe a linha e a coluna da posição que queremos guardar
Retorna o número obtido
-------------------------------------------------------------------------------------------------*/
int indexesToMove(int row, int col){

    return (row+1)*10+col+1;

}
/*-----------------------------------------------------------------------------------------------
Recebe o número do jogador, e a coluna e linha onde quer colocar peça
Verifica se há pessas do inimigo junto a essa posição
Retorna 1 se houver peça inimiga presa e 0 se não houver
-------------------------------------------------------------------------------------------------*/
int checkEnemies( int player, int row, int col){

   for(int i=-1; i<=1; i++){ //não permite passar os limites da matriz
       if(row+i>=ncol || row+i<0){
           continue;
       }
       for(int j=-1; j<=1; j++){ //não permite passar os limites da matriz
           if(col+j>=ncol|| col+j<0){
                continue;
            }
            if(gameBoard[row+i][col+j] !=player && gameBoard[row+i][col+j]!=semjogador){
                if(checkTrapped(player,row,col,i,j)) //verifica em cada direção se a peça do  inimigo está presa
                    return 1;
            }
        }
    }
    return 0;
}
/*-----------------------------------------------------------------------------------------------
Recebe o número do jogador e o array avail
Verifica todos os movimentos possíveis e coloca-os no array avail
-------------------------------------------------------------------------------------------------*/
void checkAllMoves(int player, int avail[]){

    int n=0;
    for(int i=0; i<ncol; i++){
        for(int j=0; j<ncol; j++){
            if(gameBoard[i][j] == semjogador){
                if(checkEnemies( player, i, j)){
                    avail[n]= indexesToMove(i,j); //se checkEnemies for verdade guarda o número correspondente a essa posição no array avail
                    n++;
                }
            }
        }
    }
}
/*-----------------------------------------------------------------------------------------------
Imprime as opções de jogada possíveis
Recebe o array avail onde estão guardados todos os movimentos possíveis e o número do jogador
Assinala os movimentos possíveis com um quadrado cinzento
-------------------------------------------------------------------------------------------------*/
void printAvailOpt(int avail[], int player)
{
	for(int i=0; avail[i]!=0; i++)//percorre array de movimentos possíveis
	{
		colunaverde = 2+(avail[i] % 10 -1)*(BSP_LCD_GetYSize()/ncol); //converte os números dos movimentos para posição do LCD
		linhaverde = 2+(avail[i] / 10 -1)*(BSP_LCD_GetYSize()/ncol); //+ 2 para não tapar as linhas da matriz desenhadas no LCD

		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
		BSP_LCD_FillRect(colunaverde, linhaverde, 57, 57); //desenha quadrado cinzento nas coodenadas guardadas no avail


		if(player==jpreto) //se for o jogador com peças pretas volta a colocar a cor preta
		{
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		}
		if(player==jvermelho || player==ARMplayer)//se for vermelho coloca a cor vermelha
		{
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
		}
	}
}
/*-----------------------------------------------------------------------------------------------
 Recebe array avail e apaga os seus conteúdos (coloca a 0)
 Apaga as opções a cinzento no LCD
-------------------------------------------------------------------------------------------------*/
void unprintAvailOpt(int avail[])
{
	for(int i=0; avail[i]!=0; i++)
		{
			colunaverde = 2+(avail[i] % 10 -1)*(BSP_LCD_GetYSize()/ncol);
			linhaverde = 2+(avail[i] / 10 -1)*(BSP_LCD_GetYSize()/ncol);
			avail[i]=0; //apaga elemento do array avail
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE); //desenha quadrado branco sobre o cinzento
			BSP_LCD_FillRect(colunaverde, linhaverde, 57, 57);
		}

}
/*-----------------------------------------------------------------------------------------------
Limpa o array allEnemies, colocando-o a -2
-------------------------------------------------------------------------------------------------*/
void resetAllEnemies(){

    for(int i=0; i<ncol; i++){
        for(int j=0; j<2; j++){
            allEnemies[i][j]=-2;
        }
    }
}
/*-----------------------------------------------------------------------------------------------
Recebe a linha e coluna da posição a jogar, o número do jogador e o array onde guarda a posição
dos inimigos próximos
Preenche o array allEnemies
-------------------------------------------------------------------------------------------------*/
void exposeAllEnemies( int row, int col, int player, int allEnemies[ncol][2]){

    int n = 0;
    for(int i=-1; i<=1; i++){ //verifica que não passamos o limite da matriz
       if(row+i>=ncol || row+i<0){
           continue;
       }
       for(int j=-1; j<=1; j++){
           if(col+j>=ncol|| col+j<0){
                continue;
            }
            if(gameBoard[row+i][col+j] !=player && gameBoard[row+i][col+j] !=semjogador){
                if(checkTrapped(player,row,col,i,j)){ //verifica se os inimigos estão presos, se sim armazena a diferença de coordenadas no array allEnemies
                    allEnemies[n][0]=i;
                    allEnemies[n][1]=j;
                    n++;
                }
            }
       }
    }
}
/*-----------------------------------------------------------------------------------------------
Recebe a direção da linha e da coluna, a linha e a coluna da posição a jogar e o número do jogador
Converte as peças do inimigo que estão presas em peças do jogador
-------------------------------------------------------------------------------------------------*/
void theConverter(int dirRow, int dirCol, int row, int col, int player)
{
    do{
        gameBoard[row][col] = player;//recebe a direção do inimigo até chegar à peça do jogador e converte todas as peças pelo meio em peças do jogador
        if(player==jpreto)//se for o jogador preto substitui por bolas pretas
        	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        if(player==jvermelho || player==ARMplayer)//se for o jogador vermelho ou o ARM substitui por bolas vermelhas
        	BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_FillCircle(col*(BSP_LCD_GetYSize()/ncol) + 30, row*(BSP_LCD_GetYSize()/ncol) + 30, 20);
        row += dirRow;
        col += dirCol;
        if(row<0 || row>=ncol || col<0 || col>=ncol){
            break;
        }
    }while(gameBoard[row][col] != player && gameBoard[row][col] !=semjogador);

}
/*-----------------------------------------------------------------------------------------------
Percorre matriz e conta quantas peças há de cada jogador
Armazena valores no countBlack e no countRed
-------------------------------------------------------------------------------------------------*/
void countScores()
{

	for(int i=0; i<ncol; i++)
	{
		for(int j=0; j<ncol; j++)
		{
			if(gameBoard[i][j]==jpreto)
				countBlack++;
			if(gameBoard[i][j]==jvermelho || gameBoard[i][j]==ARMplayer)
				countRed++;
		}
	}
}
/*-----------------------------------------------------------------------------------------------
Imprime o número de peças vermelhas e o número de peças pretas  no LCD
-------------------------------------------------------------------------------------------------*/
void printScores()
{
	countScores();
	char red[25];
	char black[25];


	if(ARMplayerflag==0){
		BSP_LCD_SetFont(&Font16);
		sprintf(red, " Red = %d ", countRed);
		BSP_LCD_DisplayStringAt(0, 90, (uint8_t *) red, RIGHT_MODE);
	}
	if(ARMplayerflag==1){
		BSP_LCD_SetFont(&Font16);
		sprintf(red, "  ARM  = %d ", countRed);
		BSP_LCD_DisplayStringAt(0, 90, (uint8_t *) red, RIGHT_MODE);
	}

	sprintf(black, " Black = %d ", countBlack);
	BSP_LCD_DisplayStringAt(0, 110, (uint8_t *) black, RIGHT_MODE);
	countRed=0;
	countBlack=0;

}
/*-----------------------------------------------------------------------------------------------
Converte o valor recebido pelo ADC para temperatura
Imprime temperatura no LCD
-------------------------------------------------------------------------------------------------*/
void setTemp()
{
	char string[10];
	char space[]={"                  "};
	uint32_t ConvertedValue;
	long int JTemp;

	flag = 0;  //coltar a colocar o contador a 0
	ConvertedValue = HAL_ADC_GetValue(&hadc1); //tirar o valor do adc 1
	JTemp = ((((ConvertedValue * VREF) / MAX_CONVERTED_VALUE)
			- VSENS_AT_AMBIENT_TEMP) * 10 / AVG_SLOPE) + AMBIENT_TEMP; //converter a temperatura

	BSP_LCD_SetFont(&Font24);
	sprintf(string, "Temp = %d ", (int) JTemp); //imprimir temperatura no LDC
	BSP_LCD_DisplayStringAt(0, 9 * BSP_LCD_GetYSize() / 10, (uint8_t *) space,
			RIGHT_MODE);
	BSP_LCD_DisplayStringAt(0, 9 * BSP_LCD_GetYSize() / 10, (uint8_t *) string,
			RIGHT_MODE);
}
/*-----------------------------------------------------------------------------------------------
Interrupção dos timers 6, 7 e 13
-------------------------------------------------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim)
{

	if(htim->Instance == TIM6) //de 2 em 2 segundos há um interrupção
		{
			flag++;
		}
	if(htim->Instance == TIM7) //de segundo em segundo há interrupção
	{
		if(playgame==emjogo) //se estiver um jogo a decorrer o timer 7 dá-nos a duração do jogo
		{
			timeflag=1;
			segundos++; //os segundos incrementam até chegar ao 60 (1 minuto)
			if(segundos==60)
			{
				segundos=0; //os segundos voltam a 0
				minutos++; //incrementa os minutos
			}
		}
	}
	if(htim->Instance == TIM13) //de segundo em segundo há interrupção
	{
		if(start==1) //se estiver um jogador a jogar
		{
			countdown--; //tem um contador a contar de forma decrescente desde 20 segundos
			if(countdown==0) //se chagar a 0
			{
				countdown=20; //volta a recarregar com 20 segundos
				timeout=1; //assinala fim do tempo com timeout
			}
		}
	}
}
/*-----------------------------------------------------------------------------------------------
Mostra o tempo que falta para o jogador fazer a sua jogada
-------------------------------------------------------------------------------------------------*/
void projectTimeLeft()
{
	char timeleft[10];


	BSP_LCD_SetFont(&Font24);
	sprintf(timeleft, "%.2d", countdown);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2-50, (uint8_t *) timeleft, RIGHT_MODE);
}
/*-----------------------------------------------------------------------------------------------
Interrupção do GPIO
-------------------------------------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(f_lcdPressed == 0) //se a flag lcdPressed estiver a 0
	{
		if(GPIO_Pin== GPIO_PIN_13)//se for pressionado o LCD
		{
			if(playgame==emjogo) //se estiver a jogar o jogo
			{
				BSP_TS_GetState(&TS_State);

				jogada = 1; //flag que assinala que houve uma interrupção do LCD

			}
			if(playgame==menu)//se estiver no menu
			{
				BSP_TS_GetState(&TS_State);
				escolha = 1; //flag que assinala que houve uma interrupção do LCD
			}
		}
		f_lcdPressed = 1;//assinalar que já houve um toque no LCD

	}
	if(GPIO_Pin==GPIO_PIN_0)//se foi pressionado o pino push button
	{
		if(playgame==emjogo)//se foi durante um jogo
		{

			if (play == jpreto) {
				unprintAvailOpt(avail);
				if (ARMplayerflag == 0)
					play = jvermelho;
				if (ARMplayerflag == 1)
					play = ARMplayer;

			} else if (play == jvermelho || play == ARMplayer) {
				play = jpreto;
				unprintAvailOpt(avail);
			}
			winmessage++; //flag para mostrar a mensagem de vitória
			if(winmessage==2)
			{
				playgame=fimdojogo; //assialar o fim do jogo
				BSP_LCD_Clear(LCD_COLOR_WHITE); //limpa LCD
				unprintAvailOpt(avail);//remove quadrados cinzentos
			}
		}

		else if(playgame==fimdojogo)//se estiver a mostrar mensagem de vitória
		{
			winmessage=0;//a flag para mostrar mensagem de vitória volta a 0
			playgame=menu;//assinalar a passagem para o menu
			BSP_LCD_Clear(LCD_COLOR_WHITE);//limpar LCD
		}
	}

}
/*------------------------------------------------------------------------------------------------
 Função para o ARM jogar para uma posição aleatória
-------------------------------------------------------------------------------------------------*/
void playRandom()
 {
	int i = 0, r = 0;

	while (avail[i] != 0) {//ver quantos movimentos possíveis há
		i++;
	}
	r = rand() % i;//gerar um valor random dentro desse número de movimentos

	colunaverde =  (avail[r] % 10 - 1) * (BSP_LCD_GetYSize() / ncol); //obter posições para esse valor aleatório
	linhaverde =  (avail[r] / 10 - 1) * (BSP_LCD_GetYSize() / ncol);

	coluna = colunaverde + BSP_LCD_GetYSize()/(2*ncol);//tranformar posições para posição no LCD para desenhar a peça
	linha = linhaverde + BSP_LCD_GetYSize()/(2*ncol);

	for(int j=0; j<i; j++)
	{
		avail[j]=0; //limpar o array avail
	}
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_FillCircle(coluna, linha, 20);//desenhar a peça na posição selecionada
	resetAllEnemies(allEnemies); //apagar o array allEnemies
	exposeAllEnemies((linha - 30) / getxyBoardPosition,
			(coluna - 30) / getxyBoardPosition, ARMplayer, allEnemies);//preencher o array allEnemies
	for (int i = 0; allEnemies[i][0] != -2; i++) { //converter os inimigos presos em peças do ARM
		theConverter(allEnemies[i][0], allEnemies[i][1],
				(linha - 30) / getxyBoardPosition,
				(coluna - 30) / getxyBoardPosition, ARMplayer);

	}
}
/*------------------------------------------------------------------------------------------------
Função que implementa as regras do reversi
-------------------------------------------------------------------------------------------------*/
void reversiGame() {


	if (play == jpreto) { //verificar qual o jogador que está a jogar
		BSP_LCD_SetFont(&Font16);
		sprintf(nextPlayer, "Black it's your time!");//dizer qual o jogador que está a jogar
		BSP_LCD_DisplayStringAt(0, 300, (uint8_t *) nextPlayer, RIGHT_MODE);
		checkAllMoves( jpreto, avail);//colocar os movimentos possíveis no array avail

		if (avail[0] == 0) {//se não houver mais movimentos possíveis o jogo acaba
			playgame = fimdojogo;
			unprintAvailOpt(avail);//apagamos o array avail e os quadrados cinzentos
			BSP_LCD_Clear(LCD_COLOR_WHITE);//limpar LCD
			return;//sai da função

		}
		printAvailOpt(avail, jpreto);//imprimir jogadas possíveis
		printScores();//imprimir pontuações

		start=1;//colocar tempo de jogada a contar
		projectTimeLeft();//mostrar quanto tempo falta para acabar a jogada
		if(timeout==1)//se não fizer a jogada em 20 segundos
		{
			timeout=0; //limpar flag timeout
			jpretotimeout++;//aumentar contador de vezes que não jogou a tempo
			unprintAvailOpt(avail);//apagar avail
			if(ARMplayerflag==0)//passar para o outro jogador
				play=jvermelho;
			if(ARMplayerflag==1)
				play=ARMplayer;
			if(jpretotimeout==3)//se já passou 3 vezes sem jogar
			{
				playgame=fimdojogo;//acaba o jogo
				countRed=64;//adversário ganha
				return;//sai da função
			}
		}

	}
	if (play == jvermelho) {//verificar qual o jogador que está a jogar
		BSP_LCD_SetFont(&Font16);
		sprintf(nextPlayer, "  Red it's your time!");//dizer qual o jogador que está a jogar
		BSP_LCD_DisplayStringAt(0, 300, (uint8_t *) nextPlayer, RIGHT_MODE);
		checkAllMoves(jvermelho, avail);//colocar os movimentos possíveis no array avail
		if (avail[0] == 0) {//se não houver mais movimentos possíveis o jogo acaba
			unprintAvailOpt(avail);//apagamos o array avail e os quadrados cinzentos
			playgame = fimdojogo;
			BSP_LCD_Clear(LCD_COLOR_WHITE);//limpar LCD
			return;//sai da função
		}
		printAvailOpt(avail, jvermelho);//imprimir jogadas possíveis
		printScores();//imprimir pontuações

		start=1;//colocar tempo de jogada a contar
		projectTimeLeft();//mostrar quanto tempo falta para acabar a jogada
		if(timeout==1)//se não fizer a jogada em 20 segundos
		{
			timeout=0;//limpar flag timeout
			jvermelhotimeout++;//aumentar contador de vezes que não jogou a tempo
			unprintAvailOpt(avail);//apagar avail
			play=jpreto;
			if(jvermelhotimeout==3)//passar para o outro jogador
			{
				playgame=fimdojogo;//acaba o jogo
				countBlack=64;//adversário ganha
				return;//sai da função
			}
		}
	}
	if (play==ARMplayer)//verificar qual o jogador que está a jogar
	{
		BSP_LCD_SetFont(&Font16);
		sprintf(nextPlayer, "  ARM it's your time!");//dizer qual o jogador que está a jogar
		BSP_LCD_DisplayStringAt(0, 300, (uint8_t *) nextPlayer, RIGHT_MODE);
		checkAllMoves(ARMplayer, avail);//colocar os movimentos possíveis no array avail
		if (avail[0] == 0) {//se não houver mais movimentos possíveis o jogo acaba
			playgame = fimdojogo;
			BSP_LCD_Clear(LCD_COLOR_WHITE);//limpar LCD
			return;
		}
		printScores();//imprimir pontuações
		playRandom();//faz uma jogada aleatória
		play = jpreto;//passa para o próximo jogador
		countdown=20;
	}

	if (jogada) {//se houve um toque no LCD
		HAL_Delay(100);
		if(TS_State.touchX[0]<BSP_LCD_GetXSize && TS_State.touchX[0]>450)
		{
			if(TS_State.touchY[0]>235 && TS_State.touchY[0]<260)
			{
					minutos=0;
					segundos=0;
			}
		}
		if (TS_State.touchX[0] < BSP_LCD_GetYSize()) {
			setPosition(TS_State.touchX[0], TS_State.touchY[0]);//verificar que esse toque foi no tabuleiro
			if (play == jpreto) {//verificar qual o jogador
				if (checkBoardPlace() == 0) {//se o local selecionado estiver disponível
					if (BSP_LCD_ReadPixel(coluna, linha) == LCD_COLOR_LIGHTGRAY) {//se for uma das opções possíveis (cinzento)
						putInBoard(jpreto);//coloca peça na posição selecionada na matriz
						unprintAvailOpt(avail);//apaga as opções de jogo
						BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
						BSP_LCD_FillCircle(coluna, linha, 20);//desenha bola na posição seleciona
						resetAllEnemies(allEnemies);
						exposeAllEnemies((linha - 30) / getxyBoardPosition,(coluna - 30) / getxyBoardPosition, jpreto,allEnemies);//coloca todos os inimigos que está a prender num array
						for (int i = 0; allEnemies[i][0] != -2; i++) { //converte os inimigos presos para peças amigas
							theConverter( allEnemies[i][0],allEnemies[i][1],(linha - 30) / getxyBoardPosition,(coluna - 30) / getxyBoardPosition, jpreto);
						}
						if(ARMplayerflag==0)//muda para o próximo jogador
							play=jvermelho;
						if(ARMplayerflag==1)
							play=ARMplayer;
						countdown=20;//recoloca a contagem decrescente no início
					}
				}
			} else if (play == jvermelho) {//verificar qual o jogador
				if (checkBoardPlace() == 0) {//se o local selecionado estiver disponível
					if (BSP_LCD_ReadPixel(coluna, linha) == LCD_COLOR_LIGHTGRAY) {//se for uma das opções possíveis (cinzento)
						putInBoard(jvermelho);//coloca peça na posição selecionada na matriz
						unprintAvailOpt(avail);//apaga as opções de jogo
						BSP_LCD_SetTextColor(LCD_COLOR_RED);
						BSP_LCD_FillCircle(coluna, linha, 20);//desenha bola na posição seleciona
						resetAllEnemies(allEnemies);
						exposeAllEnemies((linha - 30) / getxyBoardPosition,(coluna - 30) / getxyBoardPosition, jvermelho,allEnemies);//coloca todos os inimigos que está a prender num array
						for (int i = 0; allEnemies[i][0] != -2; i++) { //converte os inimigos presos para peças amigas
							theConverter( allEnemies[i][0],allEnemies[i][1],(linha - 30) / getxyBoardPosition,(coluna - 30) / getxyBoardPosition, jvermelho);
						}
						play = jpreto;//muda para o próximo jogador
						countdown=20;//recoloca a contagem decrescente no início
					}
				}
			}
		}

		jogada = 0;//fim de jogada
		f_lcdPressed = 0;//é possível agora detetar um novo toque no LCD

	}
}
/*------------------------------------------------------------------------------------------------
Escreve resultados do jogo no cartão SD
-------------------------------------------------------------------------------------------------*/
void writeInCard(int player, int pontos)
{
	FRESULT res = FR_OK;
	char string[50];

	res = f_mount (&SDFatFS, SDPath, 0);
	if(res != FR_OK)	//activa o sistema
	{
		Error_Handler();
	}
	HAL_Delay(200);

	res = f_open (&SDFile, "res.txt", FA_OPEN_APPEND | FA_WRITE);
	if(res != FR_OK)	//abre ficheiro res no modo append
	{
		Error_Handler();
	}

	if(player!=0)//se o jogador for o 1(preto), 2(vermelho) ou 3(ARM)
	{
		sprintf(string, "\nGanhou o jogador %d, com %d pontos em %d:%d minutos. ", player, pontos, minutos, segundos);//escrever quem ganhou, com quantos pontos e em quanto tempo
	}
	else if(player==0)//se houve um empate
	{
		sprintf(string, "\nEmpate em %d:%d minutos. ", minutos, segundos);//escreve quanto tempo durou o jogo
	}
	res = f_write(&SDFile, string, strlen(string), &nBytes);//escreve resultados no ficheiro
	if(res != FR_OK)
	{
		Error_Handler();
	}


	res = f_close (&SDFile);//fecha ficheiro
	if(res != FR_OK)
	{
		Error_Handler();
	}

}
/*------------------------------------------------------------------------------------------------
Imprime vencedor do jogo
-------------------------------------------------------------------------------------------------*/
void endOfGame() {

	countScores();
	char winner[20]={0};
	char push[26]={0};

	unprintAvailOpt(avail);
	if (countRed > countBlack) {//se o jogador vermelho tem mais pontos que o preto
		BSP_LCD_Clear(LCD_COLOR_LIGHTRED);
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_SetBackColor(LCD_COLOR_LIGHTRED);
		BSP_LCD_SetFont(&Font24);
		if(ARMplayerflag==0)
		{
			sprintf(winner, "  RED IS THE WINNER");//o vermelho é o vencedor
			writeInCard(jvermelho, countRed);//escreve resultados do jogo no cartão
		}
		if(ARMplayerflag==1)
		{
			sprintf(winner, "  ARM IS THE WINNER");//o ARM é o vencedor
			writeInCard(ARMplayer, countRed);//escreve resultados do jogo no cartão
		}
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2, (uint8_t *) winner, CENTER_MODE);
		setTemp();


	}
	else if (countRed < countBlack) {//se preto tem mais pontos que o vermelho
		BSP_LCD_Clear(LCD_COLOR_LIGHTGRAY);
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
		BSP_LCD_SetFont(&Font24);
		sprintf(winner, "BLACK IS THE WINNER");//preto é o vencedor
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2, (uint8_t *) winner, CENTER_MODE);
		setTemp();
		writeInCard(jpreto, countBlack);//escreve resultados no cartão
	}
	else if(countRed==countBlack){//se houve empate
		BSP_LCD_Clear(LCD_COLOR_WHITE);
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
		BSP_LCD_SetFont(&Font24);
		sprintf(winner, "IT'S A TIE");
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2, (uint8_t *) winner, CENTER_MODE);
		setTemp();
		writeInCard(0, 0);//escreve resultados no cartão
	}


	BSP_LCD_SetFont(&Font16);
	sprintf(push, "Press push button to menu");
	BSP_LCD_DisplayStringAt(0, 3*BSP_LCD_GetYSize() / 4, (uint8_t *) push, CENTER_MODE);

	countBlack=0;
	countRed=0;
	start=0;
	timeout=0;
	countdown=20;
	while(winmessage)//enquanto não for pressionado o push button
	{
		if(flag)
			setTemp();
	}
}

/*------------------------------------------------------------------------------------------------
Imprime menu do jogo
-------------------------------------------------------------------------------------------------*/
void GameMenu()
{

	char reversi[25];

	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetFont(&Font24);

	BSP_LCD_DrawRect(BSP_LCD_GetXSize()/4,BSP_LCD_GetYSize()/4, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/4);
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_SetBackColor(LCD_COLOR_RED);
	BSP_LCD_FillRect(BSP_LCD_GetXSize()/4, BSP_LCD_GetYSize()/4, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/4);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

	sprintf(reversi, "REVERSI - two players");
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/3, (uint8_t *)reversi, CENTER_MODE);

	BSP_LCD_DrawRect(BSP_LCD_GetXSize()/4,BSP_LCD_GetYSize()/2 +10, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/4);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
	BSP_LCD_FillRect(BSP_LCD_GetXSize()/4, BSP_LCD_GetYSize()/2 +10, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/4);
	BSP_LCD_SetTextColor(LCD_COLOR_RED);

	sprintf(reversi, "REVERSI - single player");
	BSP_LCD_DisplayStringAt(0, 2* BSP_LCD_GetYSize()/3 -20, (uint8_t *)reversi, CENTER_MODE);

	BSP_LCD_SetFont(&Font16);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);


	while(escolha!=1)//enquanto não houver um toque no LCD
	{
		f_lcdPressed = 0;
		if(flag)
			setTemp();
	}

	if(escolha==1)
	{
		escolha=0;
		HAL_Delay(100);
		if(TS_State.touchX[0]<3*BSP_LCD_GetXSize()/4 && TS_State.touchX[0]>BSP_LCD_GetXSize()/4)
		{
			if(TS_State.touchY[0]<BSP_LCD_GetYSize()/2 && TS_State.touchY[0]>BSP_LCD_GetYSize()/4 )
			{
				playgame=emjogo;
				ARMplayerflag=0;//se forem 2 jogadore a jogar

			}
			if(TS_State.touchY[0]>BSP_LCD_GetYSize()/2 +10 && TS_State.touchY[0]<3*BSP_LCD_GetYSize()/4 -10)
			{
				playgame=emjogo;
				ARMplayerflag=1;//se for um jogador contra o ARM


			}
			unprintAvailOpt(avail);//apagar avail
			LCD_GameBoard();//limpar boar
			f_lcdPressed = 0;//podemos detetar mais toques no LCD
		}
	}
}
/*------------------------------------------------------------------------------------------------
Apaga gameBoard
-------------------------------------------------------------------------------------------------*/
void resetGame()
{
	for(int i=0; i<ncol; i++)
	{
		for(int j=0; j<ncol; j++)
		{
			gameBoard[i][j]=0;
		}
	}
	minutos=0;
	segundos=0;
	play=jpreto;
	jpretotimeout=0;
	jvermelhotimeout=0;
}
/*------------------------------------------------------------------------------------------------
Imprime no LCD a duração do jogo
-------------------------------------------------------------------------------------------------*/
void printTime()
{
	 char tempo[20];
	 timeflag=0;

	 BSP_LCD_SetFont(&Font24);
	  sprintf(tempo, "Time:%.2d:%.2d",minutos, segundos);
	  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2, (uint8_t *) tempo, RIGHT_MODE);

}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */



  /* USER CODE END 1 */
  

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_DMA2D_Init();
  MX_DSIHOST_DSI_Init();
  MX_FMC_Init();
  MX_LTDC_Init();
  MX_SDMMC2_SD_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_TIM13_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim7);
  HAL_TIM_Base_Start_IT(&htim13);
  BSP_LED_Init(LED_RED);
  BSP_LED_Init(LED_GREEN);
  BoardMatrixInitial();
  BSP_LCD_Init();
  LCD_Config();
  HAL_ADC_Start(&hadc1);
  BSP_TS_Init(BSP_LCD_GetXSize(),BSP_LCD_GetYSize());
  BSP_TS_ITConfig();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
		if(flag)  //quando passar 2 segundos
		{
			setTemp();//Imprime temperatura no LCCD

		}
		if(timeflag)//de segundo a segundo durante o jogo imprime duração do jogo
		{
			printTime();
		}

		if(playgame==emjogo)
		{
			reversiGame();
		}
		else if(playgame==fimdojogo)
		{
			endOfGame();
		}
		else if(playgame==menu)
		{
			GameMenu();
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SDMMC2
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief DSIHOST Initialization Function
  * @param None
  * @retval None
  */
static void MX_DSIHOST_DSI_Init(void)
{

  /* USER CODE BEGIN DSIHOST_Init 0 */

  /* USER CODE END DSIHOST_Init 0 */

  DSI_PLLInitTypeDef PLLInit = {0};
  DSI_HOST_TimeoutTypeDef HostTimeouts = {0};
  DSI_PHY_TimerTypeDef PhyTimings = {0};
  DSI_LPCmdTypeDef LPCmd = {0};
  DSI_CmdCfgTypeDef CmdCfg = {0};

  /* USER CODE BEGIN DSIHOST_Init 1 */

  /* USER CODE END DSIHOST_Init 1 */
  hdsi.Instance = DSI;
  hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  hdsi.Init.TXEscapeCkdiv = 4;
  hdsi.Init.NumberOfLanes = DSI_ONE_DATA_LANE;
  PLLInit.PLLNDIV = 20;
  PLLInit.PLLIDF = DSI_PLL_IN_DIV1;
  PLLInit.PLLODF = DSI_PLL_OUT_DIV1;
  if (HAL_DSI_Init(&hdsi, &PLLInit) != HAL_OK)
  {
    Error_Handler();
  }
  HostTimeouts.TimeoutCkdiv = 1;
  HostTimeouts.HighSpeedTransmissionTimeout = 0;
  HostTimeouts.LowPowerReceptionTimeout = 0;
  HostTimeouts.HighSpeedReadTimeout = 0;
  HostTimeouts.LowPowerReadTimeout = 0;
  HostTimeouts.HighSpeedWriteTimeout = 0;
  HostTimeouts.HighSpeedWritePrespMode = DSI_HS_PM_DISABLE;
  HostTimeouts.LowPowerWriteTimeout = 0;
  HostTimeouts.BTATimeout = 0;
  if (HAL_DSI_ConfigHostTimeouts(&hdsi, &HostTimeouts) != HAL_OK)
  {
    Error_Handler();
  }
  PhyTimings.ClockLaneHS2LPTime = 28;
  PhyTimings.ClockLaneLP2HSTime = 33;
  PhyTimings.DataLaneHS2LPTime = 15;
  PhyTimings.DataLaneLP2HSTime = 25;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 0;
  if (HAL_DSI_ConfigPhyTimer(&hdsi, &PhyTimings) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigFlowControl(&hdsi, DSI_FLOW_CONTROL_BTA) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetLowPowerRXFilter(&hdsi, 10000) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigErrorMonitor(&hdsi, HAL_DSI_ERROR_NONE) != HAL_OK)
  {
    Error_Handler();
  }
  LPCmd.LPGenShortWriteNoP = DSI_LP_GSW0P_DISABLE;
  LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_DISABLE;
  LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_DISABLE;
  LPCmd.LPGenShortReadNoP = DSI_LP_GSR0P_DISABLE;
  LPCmd.LPGenShortReadOneP = DSI_LP_GSR1P_DISABLE;
  LPCmd.LPGenShortReadTwoP = DSI_LP_GSR2P_DISABLE;
  LPCmd.LPGenLongWrite = DSI_LP_GLW_DISABLE;
  LPCmd.LPDcsShortWriteNoP = DSI_LP_DSW0P_DISABLE;
  LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_DISABLE;
  LPCmd.LPDcsShortReadNoP = DSI_LP_DSR0P_DISABLE;
  LPCmd.LPDcsLongWrite = DSI_LP_DLW_DISABLE;
  LPCmd.LPMaxReadPacket = DSI_LP_MRDP_DISABLE;
  LPCmd.AcknowledgeRequest = DSI_ACKNOWLEDGE_DISABLE;
  if (HAL_DSI_ConfigCommand(&hdsi, &LPCmd) != HAL_OK)
  {
    Error_Handler();
  }
  CmdCfg.VirtualChannelID = 0;
  CmdCfg.ColorCoding = DSI_RGB888;
  CmdCfg.CommandSize = 640;
  CmdCfg.TearingEffectSource = DSI_TE_EXTERNAL;
  CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
  CmdCfg.HSPolarity = DSI_HSYNC_ACTIVE_LOW;
  CmdCfg.VSPolarity = DSI_VSYNC_ACTIVE_LOW;
  CmdCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
  CmdCfg.VSyncPol = DSI_VSYNC_FALLING;
  CmdCfg.AutomaticRefresh = DSI_AR_ENABLE;
  CmdCfg.TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_DISABLE;
  if (HAL_DSI_ConfigAdaptedCommandMode(&hdsi, &CmdCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetGenericVCID(&hdsi, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DSIHOST_Init 2 */

  /* USER CODE END DSIHOST_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};
  LTDC_LayerCfgTypeDef pLayerCfg1 = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 7;
  hltdc.Init.VerticalSync = 3;
  hltdc.Init.AccumulatedHBP = 14;
  hltdc.Init.AccumulatedVBP = 5;
  hltdc.Init.AccumulatedActiveW = 654;
  hltdc.Init.AccumulatedActiveH = 485;
  hltdc.Init.TotalWidth = 660;
  hltdc.Init.TotalHeigh = 487;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 0;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 0;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  pLayerCfg.Alpha = 0;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0;
  pLayerCfg.ImageWidth = 0;
  pLayerCfg.ImageHeight = 0;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg1.WindowX0 = 0;
  pLayerCfg1.WindowX1 = 0;
  pLayerCfg1.WindowY0 = 0;
  pLayerCfg1.WindowY1 = 0;
  pLayerCfg1.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  pLayerCfg1.Alpha = 0;
  pLayerCfg1.Alpha0 = 0;
  pLayerCfg1.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg1.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg1.FBStartAdress = 0;
  pLayerCfg1.ImageWidth = 0;
  pLayerCfg1.ImageHeight = 0;
  pLayerCfg1.Backcolor.Blue = 0;
  pLayerCfg1.Backcolor.Green = 0;
  pLayerCfg1.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief SDMMC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC2_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC2_Init 0 */

  /* USER CODE END SDMMC2_Init 0 */

  /* USER CODE BEGIN SDMMC2_Init 1 */

  /* USER CODE END SDMMC2_Init 1 */
  hsd2.Instance = SDMMC2;
  hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd2.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd2.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd2.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC2_Init 2 */

  /* USER CODE END SDMMC2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 19999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 19999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 9999;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 9999;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM13 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM13_Init(void)
{

  /* USER CODE BEGIN TIM13_Init 0 */

  /* USER CODE END TIM13_Init 0 */

  /* USER CODE BEGIN TIM13_Init 1 */

  /* USER CODE END TIM13_Init 1 */
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = 9999;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = 9999;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM13_Init 2 */

  /* USER CODE END TIM13_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK2;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_1;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_DISABLE;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_DISABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 16;
  SdramTiming.ExitSelfRefreshDelay = 16;
  SdramTiming.SelfRefreshTime = 16;
  SdramTiming.RowCycleDelay = 16;
  SdramTiming.WriteRecoveryTime = 16;
  SdramTiming.RPDelay = 16;
  SdramTiming.RCDDelay = 16;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();

  /*Configure GPIO pin : PI13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PI15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
static void LCD_Config(void)//configurações do LCD
{
  uint32_t  lcd_status = LCD_OK;

  /* Initialize the LCD */
  lcd_status = BSP_LCD_Init();
  while(lcd_status != LCD_OK);

  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);

  /* Clear the LCD */
  BSP_LCD_Clear(LCD_COLOR_WHITE);
}

/*-------------------------------------------------------------
 Imprime matriz no LCD e coloca peças iniciais
-------------------------------------------------------------*/
static void LCD_GameBoard(void)//mostra tabuleiro no LCD
{

	BSP_LCD_Clear(LCD_COLOR_WHITE);//limpa LCD
	resetGame();//apaga a matriz do jogo


  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(BSP_LCD_GetYSize(), 0, BSP_LCD_GetXSize()-BSP_LCD_GetYSize(), 60);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/9, 10, (uint8_t *)"REVERSI", RIGHT_MODE);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_DrawRect(0, 0, BSP_LCD_GetYSize(), BSP_LCD_GetYSize());
  BSP_LCD_DrawRect(0, 60, BSP_LCD_GetYSize()/ncol, BSP_LCD_GetYSize());

  for(int i=1; i<ncol; i++)//imprime colunas
  {
	  BSP_LCD_DrawRect(i*BSP_LCD_GetYSize()/ncol, 0, BSP_LCD_GetYSize()/ncol, BSP_LCD_GetYSize());
  }
  for(int j=0; j<ncol; j++)//imprime linhas
  {
	  BSP_LCD_DrawRect(1,j*BSP_LCD_GetYSize()/ncol, BSP_LCD_GetYSize(), BSP_LCD_GetYSize()/ncol);
  }

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_SetFont(&Font24);


  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);//imprime bolas pretas iniciais
  BSP_LCD_FillCircle(lincol3, lincol3, 20);
  putInBoardFirstPositions( jpreto, 3, 3);//coloca peças na matriz
  BSP_LCD_FillCircle(lincol4, lincol4, 20);
  putInBoardFirstPositions( jpreto, 4, 4);

  if(ARMplayerflag==0)
  {
	  BSP_LCD_SetTextColor(LCD_COLOR_RED);//imprime bolas vermelhas iniciais
	  BSP_LCD_FillCircle(lincol3, lincol4, 20);
	  putInBoardFirstPositions(jvermelho, 4, 3);//coloca peças na matriz
	  BSP_LCD_FillCircle(lincol4, lincol3, 20);
	  putInBoardFirstPositions(jvermelho, 3, 4);
  }
  if(ARMplayerflag==1)
  {
	  BSP_LCD_SetTextColor(LCD_COLOR_RED);//imprime bolas vermelhas iniciais
	  BSP_LCD_FillCircle(lincol3, lincol4, 20);
	  putInBoardFirstPositions(ARMplayer, 4, 3);//coloca peças na matriz
	  BSP_LCD_FillCircle(lincol4, lincol3, 20);
	  putInBoardFirstPositions(ARMplayer, 3, 4);
  }
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);


}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
