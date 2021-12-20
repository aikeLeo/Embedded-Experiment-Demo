#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "usmart.h"
#include "rtc.h"
#include "beep.h"
#include "key.h"
#define BEEP_ON GPIO_SetBits(GPIOF,GPIO_Pin_8); beep_cnt = 10000;
#define BEEP_OFF GPIO_ResetBits(GPIOF,GPIO_Pin_8);
u32 beep_cnt = 0;
u8 flag = 0, type = 0;
u8 h = 0, m = 0, s = 0, year = 0, month = 0, date = 0, week = 0;
u32 a = 0;
u32 Step = 0;
void KeyOperation(void) {
	static unsigned int KeyCnt = 0;
	u8 days[15] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	if (0 != KeyCnt) {
		KeyCnt--;
		return;
	}
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	switch (KEY_Scan(0)) {
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
			h = RTC_TimeStruct.RTC_Hours;
			m = RTC_TimeStruct.RTC_Minutes;
			s = RTC_TimeStruct.RTC_Seconds;

			RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
			year = RTC_DateStruct.RTC_Year;
			month = RTC_DateStruct.RTC_Month;
			date = RTC_DateStruct.RTC_Date;
			week = RTC_DateStruct.RTC_WeekDay;
		case 0:
			return;
		case 1:               // right - 切换
			if (flag) {
				if (type < 7) type ++;
				else type = 0;
				BEEP_ON;
			}
			break;
		case 2:
			if (flag) {
				BEEP_ON;
				switch (type) {
					case 0:
						if (h > 0) h --;
						else h = 23;
						break;
					case 1:
						if (m > 0) m --;
						else m = 59;
						break;
					case 2:
						s = 0;
						break;
					case 3:
						if (year > 10) year --;
						break;
					case 4:
						if (month > 1) month --;
						else month = 12;
						break;
					case 5:
						if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) days[2] = 29;
						else days[2] = 28;
						if (date > days[month]) date = days[month];
						else if (date > 1) date --;
						else date = days[month];
						break;
					case 6:
						if (week > 1) week --;
						else week = 7;
						break;
					case 7:
						if (a > 0) a --;
						break;
				}

			}
			// down -
			break;
		case 3:
			// left - 开关
			if (flag) flag = 0;
			else flag = 1;
			BEEP_ON;
			break;
		case 4:
			if (flag) {
				BEEP_ON;
				switch (type) {
					case 0:
						if (h < 23) h ++;
						else h = 0;
						break;
					case 1:
						if (m < 59) m ++;
						else m = 0;
						break;
					case 2:
						s = 0;
						break;
					case 3:
						if (year < 50) year ++;
						break;
					case 4:
						if (month < 12) month ++;
						else month = 1;
						break;
					case 5:
						if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) days[2] = 29;
						else days[2] = 28;
						if (date > days[month]) date = days[month];
						else if (date < days[month]) date ++;
						else date = 1;
						break;
					case 6:
						if (week < 7) week ++;
						else week = 1;
						break;
					case 7:
						if (a < 100) a ++;
						break;
				}
			}
			// up - 修改时间
			break;
		default:
			break;
	}
	KeyCnt = 10000;
//			if(key && flag) {
	RTC_Set_Time(h, m, s, RTC_H12_AM);
	RTC_Set_Date(year, month, date, week);
//			}
}
u8 CompareDate(RTC_DateTypeDef RTC_DateStruct, RTC_DateTypeDef Old_Date) {
	if (RTC_DateStruct.RTC_Year != Old_Date.RTC_Year) return 1;
	if (RTC_DateStruct.RTC_Month != Old_Date.RTC_Month) return 2;
	if (RTC_DateStruct.RTC_Date != Old_Date.RTC_Date) return 3;
	if (RTC_DateStruct.RTC_WeekDay != Old_Date.RTC_WeekDay) return 4;
}
u8 CompareTime(RTC_TimeTypeDef RTC_TimeStruct, RTC_TimeTypeDef Old_Time) {
	if (RTC_TimeStruct.RTC_Hours != Old_Time.RTC_Hours) return 1;
	if (RTC_TimeStruct.RTC_Minutes != Old_Time.RTC_Minutes) return 2;
	if (RTC_TimeStruct.RTC_Seconds != Old_Time.RTC_Seconds) return 3;
}
void Display(void) {
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	static RTC_TimeTypeDef Old_Time;
	static RTC_DateTypeDef Old_Date;
	static u32 old_a = 0, old_flag = 0, old_type = 0;
	u8 tbuf[40];
	static u32 t = 0;

	if (t == 0) {	//每100ms更新一次显示数据
		if (Step < a) return;
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
		if (CompareDate(RTC_DateStruct, Old_Date)) {
	
			sprintf((char*)tbuf, "Time:%02d:%02d:%02d", RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes, RTC_TimeStruct.RTC_Seconds);
			LCD_ShowString(30, 140, 210, 16, 16, tbuf);
			RTC_GetTime(RTC_Format_BIN, &Old_Time);
		}
		RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
		if (CompareDate(RTC_DateStruct, Old_Date)) {
			RTC_GetDate(RTC_Format_BIN, &Old_Date);
			sprintf((char*)tbuf, "Date:20%02d-%02d-%02d", RTC_DateStruct.RTC_Year, RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Date);
			LCD_ShowString(30, 160, 210, 16, 16, tbuf);
			sprintf((char*)tbuf, "Week:%d", RTC_DateStruct.RTC_WeekDay);
			LCD_ShowString(30, 180, 210, 16, 16, tbuf);
		}
		if (old_a != a) {
			sprintf((char*)tbuf, "Brightness: %d", a);
			LCD_ShowString(30, 210, 210, 16, 16, tbuf);
			old_a = a;
		}
		if ((old_flag != flag) || (old_type != type)) {
			old_flag = flag;
			old_type = type;
			sprintf((char*)tbuf, "SET: %s      ", flag ? "OPEN " : "CLOSE");
			LCD_ShowString(30, 250, 210, 16, 16, tbuf);
			if (flag) sprintf((char*)tbuf, "Type: %d ", type);
			else sprintf((char*)tbuf, "                             ");
			LCD_ShowString(30, 270, 210, 16, 16, tbuf);
		}
		t = 10000;
	}
	t--;
}
void Sound(void) {
	if (beep_cnt) {
		beep_cnt--;
		return;
	}
	BEEP_OFF;

}
void Light(u32 a) {
//static u32 Step=0;
	if (Step < 100)Step++;
	else Step = 0;
	if (Step < a) {
		GPIO_ResetBits(GPIOF, GPIO_Pin_9);  //LED0对应引脚GPIOF.9拉低，亮  等同LED0=0;
	} else {
		GPIO_SetBits(GPIOF, GPIO_Pin_9);
	}
}
int main(void) {
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;


	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);      //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200

	usmart_dev.init(84); 	//初始化USMART
	LED_Init();					  //初始化LED
	LCD_Init();					  //初始化LCD
	My_RTC_Init();		 		//初始化RTC
	BEEP_Init();
	KEY_Init();
	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits, 0);		//配置WAKE UP中断,1秒钟中断一次

	POINT_COLOR = RED;
	LCD_ShowString(30, 50, 200, 16, 16, "Explorer STM32F4");
	LCD_ShowString(30, 70, 200, 16, 16, "RTC TEST");
	LCD_ShowString(30, 90, 200, 16, 16, "ATOM@ALIENTEK");
	LCD_ShowString(30, 110, 200, 16, 16, "2014/5/5");

	while (1) {
	
		t++;
		if ((t % 10) == 0) {	//每100ms更新一次显示数据
			;
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);

			sprintf((char*)tbuf, "Time:%02d:%02d:%02d", RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes, RTC_TimeStruct.RTC_Seconds);
			LCD_ShowString(30, 140, 210, 16, 16, tbuf);

			RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

			sprintf((char*)tbuf, "Date:20%02d-%02d-%02d", RTC_DateStruct.RTC_Year, RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Date);
			LCD_ShowString(30, 160, 210, 16, 16, tbuf);
			sprintf((char*)tbuf, "Week:%d", RTC_DateStruct.RTC_WeekDay);
			LCD_ShowString(30, 180, 210, 16, 16, tbuf);
		}
		if ((t % 20) == 0)LED0 = !LED0;	//每200ms,翻转一次LED0
		KeyOperation();
		Display();
		Sound();
		Light(a);
		delay_us(10);
	}
}
