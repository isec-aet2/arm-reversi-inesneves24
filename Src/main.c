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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_ts.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEMP_REFRESH_PERIOD   1000    /* Internal temperature refresh period */
#define MAX_CONVERTED_VALUE   4095    /* Max converted value */
#define AMBIENT_TEMP            25    /* Ambient Temperature */
#define VSENS_AT_AMBIENT_TEMP  760    /* VSENSE value (mv) at ambient temperature */
#define AVG_SLOPE               25    /* Avg_Solpe multiply by 10 */
#define VREF                  3300
#define lincol0     0 *(BSP_LCD_GetYSize()/8) + 30
#define lincol1		1 *(BSP_LCD_GetYSize()/8) + 30
#define lincol2		2 *(BSP_LCD_GetYSize()/8) + 30
#define lincol3		3 *(BSP_LCD_GetYSize()/8) + 30
#define lincol4		4 *(BSP_LCD_GetYSize()/8) + 30
#define lincol5		5 *(BSP_LCD_GetYSize()/8) + 30
#define lincol6		6 *(BSP_LCD_GetYSize()/8) + 30
#define lincol7		7 *(BSP_LCD_GetYSize()/8) + 30

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
volatile int flag=0;
volatile int contador=0;
volatile int jogada;
volatile int linha;
volatile int coluna;
int getxyBoardPosition=(480/8);
volatile uint8_t f_lcdPressed = 0;
int gameBoard[8][8];
TS_StateTypeDef TS_State;
int linhaverde;
int colunaverde;
int countRed=0;
int countBlack=0;
int playgame=1;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DMA2D_HandleTypeDef hdma2d;

DSI_HandleTypeDef hdsi;

LTDC_HandleTypeDef hltdc;

TIM_HandleTypeDef htim6;

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
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
static void LCD_Config();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void setPosition(uint16_t x, uint16_t y)
{
	if(x<=BSP_LCD_GetYSize()/8)
		coluna=lincol0;
	else if(x<=2*BSP_LCD_GetYSize()/8 && x>BSP_LCD_GetYSize()/8)
		coluna=lincol1;
	else if(x<=3*BSP_LCD_GetYSize()/8 && x>2*BSP_LCD_GetYSize()/8)
		coluna=lincol2;
	else if(x<=4*BSP_LCD_GetYSize()/8 && x>3*BSP_LCD_GetYSize()/8)
		coluna=lincol3;
	else if(x<=5*BSP_LCD_GetYSize()/8 && x>4*BSP_LCD_GetYSize()/8)
		coluna=lincol4;
	else if(x<=6*BSP_LCD_GetYSize()/8 && x>5*BSP_LCD_GetYSize()/8)
		coluna=lincol5;
	else if(x<=7*BSP_LCD_GetYSize()/8 && x>6*BSP_LCD_GetYSize()/8)
		coluna=lincol6;
	else if(x<=8*BSP_LCD_GetYSize()/8 && x>7*BSP_LCD_GetYSize()/8)
		coluna=lincol7;

	if(y<=BSP_LCD_GetYSize()/8)
		linha=lincol0;
	else if(y<=2*BSP_LCD_GetYSize()/8 && y>BSP_LCD_GetYSize()/8)
		linha=lincol1;
	else if(y<=3*BSP_LCD_GetYSize()/8 && y>2*BSP_LCD_GetYSize()/8)
		linha=lincol2;
	else if(y<=4*BSP_LCD_GetYSize()/8 && y>3*BSP_LCD_GetYSize()/8)
		linha=lincol3;
	else if(y<=5*BSP_LCD_GetYSize()/8 && y>4*BSP_LCD_GetYSize()/8)
		linha=lincol4;
	else if(y<=6*BSP_LCD_GetYSize()/8 && y>5*BSP_LCD_GetYSize()/8)
		linha=lincol5;
	else if(y<=7*BSP_LCD_GetYSize()/8 && y>6*BSP_LCD_GetYSize()/8)
		linha=lincol6;
	else if(y<=8*BSP_LCD_GetYSize()/8 && y>7*BSP_LCD_GetYSize()/8)
		linha=lincol7;

}
int checkBoardPlace(int gameBoard[8][8])
{
	int l= (linha-30)/getxyBoardPosition;
	int c= (coluna-30)/getxyBoardPosition;

	int resultado=gameBoard[l][c];

	return resultado;
}
void putInBoard(int gameBoard[8][8], int player)
{
	int l=(linha-30)/getxyBoardPosition;
	int c=(coluna-30)/getxyBoardPosition;
	gameBoard[l][c]=player;
}
void putInBoardFirstPositions(int gameBoard[8][8], int player, int lin, int col)
{

	gameBoard[lin][col]=player;
}
void BoardMatrixInitial() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			gameBoard[i][j] = 0;

		}
	}
}
int checkTrapped(int gameBoard[8][8], int player, int row, int col, int i, int j){ //checks if the enemy pieces are trapped

    int mult;
    for(mult=1; 1 ; mult++) {
        if (mult*i+row<0 || mult*i+row>=8 || //stops from crossing the matrix's borders
            mult*j+col<0 || mult*j+col>=8 )
            return 0;
        if (gameBoard[mult*i+row][mult*j+col] == 0) //returns false if after the enemy's pieces the next space is empty
            return 0;
        if (gameBoard[mult*i+row][mult*j+col] == player) //returns true if a player's piece is found
            return 1;
    }
}
int indexesToMove(int row, int col){ //converts board indexes to moves

    return (row+1)*10+col+1;

}
int checkEnemies(int gameBoard[8][8], int player, int row, int col){ //checks if there are enemies nearby the player's move

   for(int i=-1; i<=1; i++){ //avoids crossing the matrix's borders
       if(row+i>=8 || row+i<0){
           continue;
       }
       for(int j=-1; j<=1; j++){
           if(col+j>=8|| col+j<0){
                continue;
            }
            if(gameBoard[row+i][col+j] !=player && gameBoard[row+i][col+j]!=0){
                if(checkTrapped(gameBoard,player,row,col,i,j)) //checks if in each direction, if the enemy's pieces are trapped
                    return 1;
            }
        }
    }
    return 0;
}
void checkAllMoves(int gameBoard[8][8],int player, int avail[]){ //checks all possible moves

    int n=0;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            if(gameBoard[i][j] == 0){
                if(checkEnemies(gameBoard, player, i, j)){
                    avail[n]= indexesToMove(i,j); //if checkEnemies is true, the move will be stores in avail[]
                    n++;
                }
            }
        }
    }
}

void printAvailOpt(int avail[], int player)
{
	for(int i=0; avail[i]!=0; i++)
	{
		colunaverde = 2+(avail[i] % 10 -1)*(BSP_LCD_GetYSize()/8); //converts moves to board indexes, col
		linhaverde = 2+(avail[i] / 10 -1)*(BSP_LCD_GetYSize()/8);//converts moves to board indexes, line

		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
		BSP_LCD_FillRect(colunaverde, linhaverde, 57, 57);
		if(player==1)
		{
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		}
		if(player==2)
		{
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
		}
	}
}
void unprintAvailOpt(int avail[], int player)
{
	for(int i=0; avail[i]!=0; i++)
		{
			colunaverde = 2+(avail[i] % 10 -1)*(BSP_LCD_GetYSize()/8); //converts moves to board indexes, col
			linhaverde = 2+(avail[i] / 10 -1)*(BSP_LCD_GetYSize()/8);//converts moves to board indexes, line
			avail[i]=0;
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			BSP_LCD_FillRect(colunaverde, linhaverde, 57, 57);
		}

}
void resetAllEnemies(int allEnemies[8][2]){ //cleans the allEnemies[] array

    for(int i=0; i<8; i++){
        for(int j=0; j<2; j++){
            allEnemies[i][j]=-2;
        }
    }
}
void exposeAllEnemies(int gameBoard[8][8], int row, int col, int player, int allEnemies[8][2]){ //fills the allEnemies[] array

    int n = 0;
    for(int i=-1; i<=1; i++){ //checks if we don't cross the matrix's borders
       if(row+i>=8 || row+i<0){
           continue;
       }
       for(int j=-1; j<=1; j++){
           if(col+j>=8|| col+j<0){
                continue;
            }
            if(gameBoard[row+i][col+j] !=player && gameBoard[row+i][col+j] !=0){
                if(checkTrapped(gameBoard,player,row,col,i,j)){ //checks if the enemies are trapped, and, if so, stores the coordinates' differences from the move in the allEnemies array
                    allEnemies[n][0]=i;
                    allEnemies[n][1]=j;
                    n++;
                }
            }
       }
    }
}
void theConverter(int gameBoard[8][8], int dirRow, int dirCol, int row, int col, int player)//converts the trapped enemy's pieces to player's pieces
{
    do{
        gameBoard[row][col] = player;//follows the enemy's direction until it reaches player's piece and convert all the pieces in between
        if(player==1)
        	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        if(player==2)
        	BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_FillCircle(col*(BSP_LCD_GetYSize()/8) + 30, row*(BSP_LCD_GetYSize()/8) + 30, 20);
        row += dirRow;
        col += dirCol;
        if(row<0 || row>=8 || col<0 || col>=8){
            break;
        }
    }while(gameBoard[row][col] != player && gameBoard[row][col] !=0);

}
void countScores()
{

	for(int i=0; i<8; i++)
	{
		for(int j=0; j<8; j++)
		{
			if(gameBoard[i][j]==1)
				countRed++;
			if(gameBoard[i][j]==2)
				countBlack++;
		}
	}
}
void printScores()
{
	countScores();
	char red[15];
	char black[15];

	BSP_LCD_SetFont(&Font12);
	sprintf(red, "Vermelho = %d ", countRed);
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t *) red, RIGHT_MODE);

	sprintf(black, "Preto = %d ", countBlack);
	BSP_LCD_DisplayStringAt(0, 150, (uint8_t *) black, RIGHT_MODE);
	countRed=0;
	countBlack=0;
}

void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim)
{

	if(htim->Instance == TIM6)
		{
			flag++;
		}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(f_lcdPressed == 0)
	{
		//If pressed (first interruption)
		if(GPIO_Pin== GPIO_PIN_13)
		{
			BSP_TS_GetState(&TS_State);
			jogada = 1;
		}
		f_lcdPressed = 1;

	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	char string[10];
	char space[]={"                  "};
	uint32_t ConvertedValue;
	long int JTemp;
	int avail[8*8]={0};//create array of available moves and set it to all zeros
	int play=1;
	int allEnemies[8][2]; //creates array with the directions(differences between enemy coordinates and the move's coordinates) of all the enemies nearby
	char nextPlayer[20];

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
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6);
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
		if(flag)  //quando passar 2 segundos	//Sample and print temperature on lcd
		{
			flag = 0;  //coltar a colocar o contador a 0
			ConvertedValue = HAL_ADC_GetValue(&hadc1); //tirar o valor do adc 1
			JTemp = ((((ConvertedValue * VREF) / MAX_CONVERTED_VALUE)
					- VSENS_AT_AMBIENT_TEMP) * 10 / AVG_SLOPE) + AMBIENT_TEMP; //converter a temperatura

			BSP_LCD_SetFont(&Font24);
			sprintf(string, "Temp = %d ", (int) JTemp);
			BSP_LCD_DisplayStringAt(0, 9 * BSP_LCD_GetYSize() / 10,
					(uint8_t *) space, RIGHT_MODE);
			BSP_LCD_DisplayStringAt(0, 9 * BSP_LCD_GetYSize() / 10,
					(uint8_t *) string, RIGHT_MODE);
		}
		if(playgame==1)
		{
			if(play==1)
			{
				BSP_LCD_SetFont(&Font16);
				sprintf(nextPlayer, "Black it's your time!");
				BSP_LCD_DisplayStringAt(0, 300, (uint8_t *) nextPlayer, RIGHT_MODE);
				checkAllMoves(gameBoard, 1, avail);
				printAvailOpt( avail, 1);
				printScores();
			}
			if(play==2)
			{
				BSP_LCD_SetFont(&Font16);
				sprintf(nextPlayer, "  Red it's your time!");
				BSP_LCD_DisplayStringAt(0, 300, (uint8_t *) nextPlayer, RIGHT_MODE);
				checkAllMoves(gameBoard, 2, avail);
				printAvailOpt( avail, 2);
				printScores();
			}

			if(jogada)
			{
				HAL_Delay(100);
				if(TS_State.touchX[0]<BSP_LCD_GetYSize() )
				{
					setPosition(TS_State.touchX[0], TS_State.touchY[0]);
					if(play==1)
					{
						if(checkBoardPlace(gameBoard)==0)
						{
							//check if avail
							if(BSP_LCD_ReadPixel(coluna, linha)==LCD_COLOR_LIGHTGRAY)
							{
								putInBoard(gameBoard, 1);
								unprintAvailOpt(avail, 1);
								BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
								BSP_LCD_FillCircle(coluna, linha, 20);
								resetAllEnemies(allEnemies);
								exposeAllEnemies(gameBoard,(linha-30)/getxyBoardPosition,(coluna-30)/getxyBoardPosition,1,allEnemies);
								for(int i=0; allEnemies[i][0]!=-2; i++){ //converts all the trapped enemies into own's symbols
									  theConverter(gameBoard, allEnemies[i][0], allEnemies[i][1], (linha-30)/getxyBoardPosition, (coluna-30)/getxyBoardPosition, 1);
								}
								//exposeAllEnemies(int gameBoard[8][8], int row, int col, int player, int allEnemies[8][2])
								play=2;
							}
						}
					}
					else
					{
						if(checkBoardPlace(gameBoard)==0)
						{
							if(BSP_LCD_ReadPixel(coluna, linha)==LCD_COLOR_LIGHTGRAY)
							{
								putInBoard(gameBoard, 2);
								unprintAvailOpt(avail, 1);
								BSP_LCD_SetTextColor(LCD_COLOR_RED);
								BSP_LCD_FillCircle(coluna, linha, 20);
								unprintAvailOpt( avail, 2);
								resetAllEnemies(allEnemies);
								exposeAllEnemies(gameBoard,(linha-30)/getxyBoardPosition,(coluna-30)/getxyBoardPosition,2,allEnemies);
								for(int i=0; allEnemies[i][0]!=-2; i++){ //converts all the trapped enemies into own's symbols
									theConverter(gameBoard, allEnemies[i][0], allEnemies[i][1], (linha-30)/getxyBoardPosition, (coluna-30)/getxyBoardPosition, 2);
								}
								play=1;
							}
						}
					}
					contador++;
				}

				jogada=0;
				f_lcdPressed = 0;

			}
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
  RCC_OscInitStruct.PLL.PLLQ = 2;
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
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
  htim6.Init.Period = 9999;
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
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();

  /*Configure GPIO pin : PI13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
static void LCD_Config(void)
{
  uint32_t  lcd_status = LCD_OK;

  /* Initialize the LCD */
  lcd_status = BSP_LCD_Init();
  while(lcd_status != LCD_OK);

  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);

  /* Clear the LCD */
  BSP_LCD_Clear(LCD_COLOR_WHITE);

  /* Set LCD Example description */

  BSP_LCD_SetTextColor(LCD_COLOR_DARKRED);
  BSP_LCD_FillRect(BSP_LCD_GetYSize(), 0, BSP_LCD_GetXSize()-BSP_LCD_GetYSize(), 60);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_DARKRED);
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/9, 10, (uint8_t *)"REVERSI", RIGHT_MODE);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_DrawRect(0, 0, BSP_LCD_GetYSize(), BSP_LCD_GetYSize());
  BSP_LCD_DrawRect(0, 60, BSP_LCD_GetYSize()/8, BSP_LCD_GetYSize());

  for(int i=1; i<8; i++)//colunas
  {
	  BSP_LCD_DrawRect(i*BSP_LCD_GetYSize()/8, 0, BSP_LCD_GetYSize()/8, BSP_LCD_GetYSize());
  }
  for(int j=0; j<8; j++)//linhas
  {
	  BSP_LCD_DrawRect(1,j*BSP_LCD_GetYSize()/8, BSP_LCD_GetYSize(), BSP_LCD_GetYSize()/8);
  }

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_SetFont(&Font24);


  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillCircle(lincol3, lincol3, 20);
  putInBoardFirstPositions(gameBoard, 1, 3, 3);
  BSP_LCD_FillCircle(lincol4, lincol4, 20);
  putInBoardFirstPositions(gameBoard, 1, 4, 4);

  BSP_LCD_SetTextColor(LCD_COLOR_RED);
  BSP_LCD_FillCircle(lincol3, lincol4, 20);
  putInBoardFirstPositions(gameBoard, 2, 4, 3);
  BSP_LCD_FillCircle(lincol4, lincol3, 20);
  putInBoardFirstPositions(gameBoard, 2, 3, 4);

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
