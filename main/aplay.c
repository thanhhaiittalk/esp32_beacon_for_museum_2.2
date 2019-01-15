
#include "aplay.h"

extern xQueueHandle Audio_Queue_Handle;

#define TAG "aplay"

struct file_bufer {
	unsigned char *buf;
	unsigned long length;
	uint32_t flen;
	uint32_t fpos;
	FILE* f;
};


void aplay_wav(char* filename){
	printf("aplay_wav %s\r\n",filename);
	WAV_HEADER wav_head;
	FILE *f= fopen(filename, "r");
	if (f == NULL) {
			ESP_LOGE(TAG,"Failed to open file:%s",filename);
			return;
	}
	int rlen=fread(&wav_head,1,sizeof(wav_head),f);
	if(rlen!=sizeof(wav_head)){
			ESP_LOGE(TAG,"read faliled");
			return;
	}
	int channels = wav_head.wChannels;
	int frequency = wav_head.nSamplesPersec;
	int bit = wav_head.wBitsPerSample;
	int datalen= wav_head.wSampleLength;
	(void)datalen;
	ESP_LOGI(TAG,"channels:%d,frequency:%d,bit:%d\n",channels,frequency,bit);
	char* samples_data = malloc(1024);
	do{
		rlen=fread(samples_data,1,1024,f);
		//datalen-=rlen;
		hal_i2s_write(0,samples_data,rlen,5000);
	}while(rlen>0);
	fclose(f);
	free(samples_data);
	f=NULL;
}

void WM8978_config()
{
    /*init gpio*/
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_OUTPUT_IO_0, 1);
    /*init codec */
    hal_i2c_init(0,19,18);
    hal_i2s_init(0,22050,16,2);
    WM8978_Init();
    WM8978_ADDA_Cfg(1,1);
    WM8978_Input_Cfg(1,0,0);
    WM8978_Output_Cfg(1,0);
    WM8978_MIC_Gain(25);
    WM8978_AUX_Gain(0);
    WM8978_LINEIN_Gain(0);
    WM8978_SPKvol_Set(63);
    WM8978_HPvol_Set(63,63);
    WM8978_EQ_3D_Dir(0);
    WM8978_EQ1_Set(0,24);
    WM8978_EQ2_Set(0,24);
    WM8978_EQ3_Set(0,24);
    WM8978_EQ4_Set(0,24);
    WM8978_EQ5_Set(0,24);
}


