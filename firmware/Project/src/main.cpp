/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ff.h"
#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"
#include "main.h"
#include "serial_debug.h"
#include "common.h"
#include "systick.h"
#include "filesystem.h"
#include "HTTPServer.h"
#include "HTTPRestHandler.h"
#include "HTTPFileWriting.h"
#include "HTTPFS.h"
#include "serial_redirect.h"
#include "tasks.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define IPv4(a,b,c,d) (((d)<<24)|((c)<<16)|((b)<<8)|(a))
/* Private variables ---------------------------------------------------------*/
static ip_addr IP = {.addr = IPv4(192,168,1,42)};
static ip_addr Mask = {.addr = IPv4(255,255,255,0)};
static ip_addr GW = {.addr = IPv4(192,168,1,1)};
static ip_addr DNS = {.addr = IPv4(8,8,8,8)};
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

HTTPServer httpd (
    "mBed",   // hostname
    IP,    // IP address
    Mask,    // Netmask
    GW,     // Gateway
    DNS,     // DNS
    80       // Port
);

static void CoreInit(void)
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  SysTick_Init();
  DebugComPort_Init();  
}

static void PeriphInit(void)
{
  /* configure ethernet (GPIOs, clocks, MAC, DMA) */
  ETH_BSP_Config();

  FileSystem_Init();
}

static void ServerInit(void)
{
  httpd.addHandler(new HTTPRestHandler("/rest"));
  httpd.addHandler(new HTTPFileWritingHandler("/config/", "/"));
  httpd.addHandler(new HTTPFileSystemHandler("/", "/"));
  httpd.bind();
}

static void ThinpadInit(void)
{
  SerialRedirect_Init(&httpd);
}


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured to 
       168 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */
  CoreInit();

  printf("**     \r\n%s\r\n", "---- System Started ----");

  PeriphInit();

  ThinpadInit();

  ServerInit();

  INFO_MSG("Initialization Done");

  /* Infinite loop */
  while (1)
  {  
    httpd.poll();
    SerialRedirect_Task();
  } 
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  printf("Assertion Failed: file %s on line %d\r\n", file, line);
  /* Infinite loop */
  while (1)
  {}
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
