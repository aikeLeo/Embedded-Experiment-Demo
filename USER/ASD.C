#define beep_sound 	beep=1;beep_cnt=0;
int main(void)
{
	while(1)
	{
		delay_us(10);
		LED_LIGHT
		LCD_CNT++;
		beep_cnt++;
		if(beep_cnt>20000)beep=0;
		if(key_cnt<100000)key_cnt++;
		if(LCD_CNT==10000)
		{LCD_DISPLAY();LCD_CNT=0;
		}
		if(key_cnt>5000){
			key=get_key();
			if(key!=0)
			{key_cnt=0;
				beep=1;
				beep_cnt=0;
			}
		}
	}
}