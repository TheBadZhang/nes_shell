#include "core.h"
#include "common.h"


#include <span>
#include <functional>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <array>

#include "lua.hpp"

#include "hardware_timer.h"
#include "oled.h"
#include "sys.h"
#include "trick.h"
// #include "usbd_cdc.h"
// #include "usbd_cdc_if.h"
#include "st7735.h"

#include "FreeRTOS.h"
#include "task.h"

#include "libxbmp.hpp"

#include "tinyexpr.h"
#include "hanoi.hpp"
#include "snake.hpp"


extern const unsigned short test_img_128x128[][128];

#include "app.hpp"

#include "alert.hpp"


#include "animation.hpp"
#include "round_watch_face.hpp"
#include "square_watch.hpp"
#include "sound_wave.hpp"

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

#include "list_selector.hpp"
#include "app_selector.hpp"


#include "qrcode.hpp"
#include "modern_art_generator.hpp"

// #include "fmt.h"
// #include "tinyformat.h"
// #include <format>

// char print_buffer[128]{0};
// int uart_print(const char*fmt,...) {
// 	va_list list;
// 	va_start(list, fmt);
// 	int str_len = vsnprintf(print_buffer,sizeof(print_buffer), fmt, list);
// 	va_end(list);
// 	HAL_UART_Transmit(&huart4, (uint8_t*)print_buffer, str_len, 1000);
// }
// fmt/core
// #include "format.h"
// void my_print (const std::string& str) {
// 	HAL_UART_Transmit(&huart4, (uint8_t*)str.c_str(), str.length(), 1000);
// }
#define my_printf(buf_, fmt_, x_...)    \
	int len_ = sprintf(buf_, fmt_, x_); \
	HAL_UART_Transmit(&huart4, (uint8_t*)buf_, len_, 1000);

int fps_count0 = 0;
int fps_count = 30;

uint8_t keyboard[8] {0,0,0,0,0,0,0,0};


bool key[16] {false};               // 这一次扫描被按下的按键
bool key_pressed_flag[16] {false};  // 单次按键

void key_scan(void* argument) {
	while (true) {
		set(R1);
		key[0] = read(C5);
		key[1] = read(C4);
		key[2] = read(C3);
		key[3] = read(C2);
		key[4] = read(C1);
		clr(R1);
		set(R2);
		key[5] = read(C5);
		key[6] = read(C4);
		key[7] = read(C3);
		key[8] = read(C2);
		key[9] = read(C1);
		clr(R2);
		set(R3);
		key[10] = read(C5);
		key[11] = read(C4);
		key[12] = read(C3);
		key[13] = read(C2);
		key[14] = read(C1);
		clr(R3);
		for (int i = 0; i < 15; i++) {
			if (key[i] == false) {
				key_pressed_flag[i] = false;
			}
		}
		vTaskDelay(4);
	}
}

bool key_pressed_func (int id_of_key, bool _pressed_key_ = true) {
	if (key[id_of_key] && !key_pressed_flag[id_of_key]) {
		key_pressed_flag[id_of_key] = _pressed_key_;
		return true;
	} else {
		return false;
	}
}

char buf[128];



extern "C" int lua_flip_LED(lua_State *L) {
	flip(LED);
	return 0;
}

const char lua_test[] {R"(
	print("Hello,I am lua!\n--this is newline printf")
	function foo()
		local i = 0
		local sum = 1
		while i <= 10 do
			sum = sum * 2
			i = i + 1
		end
		return sum
	end
	print("sum =", foo())
	print("and sum = 2^11 =", 2 ^ 11)
	print("exp(200) =", math.exp(200))
)"};


extern "C" int uart_send(lua_State* L) {
	char* str = (char*)luaL_checkstring(L, 1);
	HAL_UART_Transmit(&huart4, (uint8_t*)str, strlen(str), 0xffff);
	return 0;
}

static const struct luaL_Reg mylib[] ={
	{"flip",lua_flip_LED},
	{NULL,NULL}
};
extern "C" void led0_task(void* argument) {
	lua_State *L;
	L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_base(L);

	lua_register(L, "flip", lua_flip_LED);
	lua_register(L, "uart_send", uart_send);

	while (true) {
		fps_count = fps_count0;
		fps_count0 = 0;
		if (luaL_dostring(L, "flip()") != LUA_OK) {
			char* err = (char*)lua_tostring(L, -1);
			my_printf(buf, "lua error:%s\n", err);
		}
		// my_printf(buf, "fps:%d\n", fps_count);
		// flip(LED);
		// https://blog.csdn.net/qq_32216815/article/details/116934350
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
		vTaskDelay(500);
	}
}

osThreadId_t LEDHandle;
const osThreadAttr_t LED_attributes = {
	.name = "LED",
	.stack_size = 1024 * 4,    // 调用 lua 函数需要一个足够的栈空间
	.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t ANOOLEDHandle;
const osThreadAttr_t ANOOLED_attributes = {
	.name = "OLED",
	.stack_size = 1024 * 20,
	.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t keyscanHandle;
const osThreadAttr_t keyscan_attributes = {
	.name = "key_scan",
	.stack_size = 1024,
	.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t ipsHandle;
const osThreadAttr_t ips_attributes = {
	.name = "ips",
	.stack_size = 1024 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};
void oled_function(void* argument);
void ips_func(void* argument);
extern "C" void load(void) {
	uint32_t random_seed = 0;
	HAL_RNG_GenerateRandomNumber(&hrng, &random_seed);
	srand(random_seed);
	ipsHandle = osThreadNew(ips_func, NULL, &ips_attributes);
	ANOOLEDHandle = osThreadNew(oled_function, NULL, &ANOOLED_attributes);
	keyscanHandle = osThreadNew(key_scan, NULL, &keyscan_attributes);
	keyscanHandle = osThreadNew(led0_task, NULL, &LED_attributes);
}

extern "C" time_t time(time_t * time){
	return 0;
}
extern "C" int system(const char * string){
	return 0;
}



void ips_func(void* argument) {
	// tbz::tft::st7735::init();
	// vTaskDelay(100);
	// 屏幕背光 PWM
	// __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, 99);
	// HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);

	tbz::tft::st7735::fillScreen(tbz::tft::st7735::COLOR::BLACK);
	// int i = 0;
	// uint16_t c[] { 0x0000,0x001F,0xF800,0x07E0,0x07FF,0xF81F,0xFFE0,0xFFFF };
	while (true) {
		// 依次使用各种背景颜色
		// tbz::tft::st7735::fillScreen(c[i++]);
		// i %= sizeof(c)/sizeof(c[0]);

		tbz::tft::st7735::writeString(0, 0, "pressed:42", Font_11x18, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::MAGENTA);
		vTaskDelay(50);
	}

}

tbz::LIST_SELECTOR list_selector;


enum class WINDOW {
	unexist_scene,   // 不存在的场景
	fade_animation,
	layer_in_animation,
	layer_out_animation,
	window_start_animation,
	alert,

	normal_status

} now_scene = WINDOW::normal_status, next_scene = WINDOW::unexist_scene;

// u8g2_font_NokiaSmallBold_tf



void fade_to_next_scene(APP_ENUM app) {
	// swap_animation(u8g2);
	next_scene = now_scene;
	now_scene = WINDOW::fade_animation;
	now_app = app;
}
void next_scene_func(WINDOW scene) {
	now_scene = scene;
}

extern const uint8_t psychic_swamp[];
extern const uint8_t tsetBones[];
extern const uint8_t tsetLava[];
extern const uint8_t tsetSand[];
extern const uint8_t tsetTower[];
extern const uint8_t normal_keys[];

// tbz::PIC normal_keys(pic_normal_keys);

extern const uint8_t snow_animation_pic[];
tbz::SPRITE_ANIMATION<10> ani1(snow_animation_pic, 72, 8, 8, 8, 128, 128, 9);
tbz::SPRITE_ANIMATION<10> ani2(snow_animation_pic+72, 54, 8, 8, 8, 128, 128, 6);

tbz::game::hanoi hanoi;
tbz::round_watch_face rwf;
tbz::SquareWatch sw;
tbz::MODERN_ART_GENERATOR mag;
tbz::game::SNAKE snake;
tbz::QRCode<3> qrcode;
tbz::APP_SELECTOR app_selector;
tbz::SOUND_WAVE sound_wave;

#include "base64.hpp"
char base64_in[] {"Hello World!"};
uint8_t base64_out[tbz::base64::encode_out_size(sizeof(base64_in))];

// uint8_t __attribute__((section (".dtcm"))) screen_buffer[128*128/2];
// uint8_t __attribute__((section (".dtcm"))) screen_buffer2[128*128/2+2] = { 0x7f, 0x7f };
uint8_t screen_buffer[128*128/2];
uint8_t screen_buffer2[128*128/2+2] = { 0x7f, 0x7f };
tbz::PIC screen_pic(screen_buffer2, [](tbz::PIC& pic) {
	pic.setMode(tbz::PIC::MODE::BIT4).setColor(0x01);
});

uint8_t scrren_buffer_16bit[240*320*2];
constexpr uint16_t bit16_rgb(uint8_t r, uint8_t g, uint8_t b) {
	uint16_t c = ((r&0b11111000) << 11) | ((g&0b11111100) << 5) | (b&0b11111000);
	return (c&0xff00) >> 8 | (c&0x00ff) << 8;
}
constexpr uint16_t bit16_gray(uint8_t gray) {
	return bit16_rgb(gray, gray, gray);
}

// #define bit16_gray(gray) bit16_rgb(gray, gray, gray)

// 因为大小端的问题，所以这个颜色的高低字节需要对调
// 所以你会看到这个颜色和 colors.h 给出的颜色是不一样的
uint16_t bit4_to_bit16_convert_color_set[16] {
	0x0000,	0x0000,
	0x4D6B,	0x4D6B,
	0x1084,	0x1084,
	0x55AD,	0x55AD,
	0x18C6,	0x18C6,
	0x9AD6,	0x9AD6,
	0xFBDE,	0xFBDE,
	0xFFFF,	0xFFFF
	// bit16_gray(0x00), bit16_gray(0x11), bit16_gray(0x22), bit16_gray(0x33),
	// bit16_gray(0x44), bit16_gray(0x55), bit16_gray(0x66), bit16_gray(0x77),
	// bit16_gray(0x88), bit16_gray(0x99), bit16_gray(0xAA), bit16_gray(0xBB),
	// bit16_gray(0xCC), bit16_gray(0xDD), bit16_gray(0xEE), bit16_gray(0xFF)
};
void bit4_to_bit16(uint8_t* src, uint8_t* dst, int size) {
	for (int i = 0; i < size; i++) {
		dst[i*2] = bit4_to_bit16_convert_color_set[src[i] >> 4];
		dst[i*2+1] = bit4_to_bit16_convert_color_set[src[i] & 0x0f];
	}
}


void bit4_to_bit16(uint8_t* src, uint8_t* dst, int src_w, int src_h, int dst_w, int dst_h) {

	int src_blen = src_w/2;
	int dst_blen = dst_w*2;

	for (int i = 0; i < src_h; i++) {
		for (int j = 0; j < src_blen; j++) {
			// int ptr_offset = i*dst_blen+j*2*2;   // 行、列
			// dst[ptr_offset] = *((uint8_t*)bit4_to_bit16_convert_color_set+(src[i*src_blen+j] >> 4));
			// dst[ptr_offset+1] = *((uint8_t*)bit4_to_bit16_convert_color_set+(src[i*src_blen+j] >> 4)+1);
			// dst[ptr_offset+2] = *((uint8_t*)bit4_to_bit16_convert_color_set+(src[i*src_blen+j] & 0x0f >> 4));
			// dst[ptr_offset+3] = *((uint8_t*)bit4_to_bit16_convert_color_set+(src[i*src_blen+j] & 0x0f >> 4)+1);
			uint16_t* dst_ptr = ((uint16_t*)(dst))+i*dst_w+j*2;
			*dst_ptr = bit4_to_bit16_convert_color_set[src[i*src_blen+j] >> 4];
			*(dst_ptr+1) = bit4_to_bit16_convert_color_set[src[i*src_blen+j] & 0x0f];
		}
	}
}
namespace tbz{
struct POINT {
	float x, y;
	float dir_x, dir_y;
};
class 动态蜘蛛网 {
private:
	PIC* pic;
	std::array<POINT, 10> points;
	// POINT points[10];
	struct {
		int point_size = 2;
		int line_distance = 20;
	} config;
	void update(void) {
		for (int i = 0; i < points.size(); i++) {
			points[i].x += points[i].dir_x;
			points[i].y += points[i].dir_y;
			if (points[i].x < config.point_size || points[i].x > 128-config.point_size) {
				points[i].dir_x = -points[i].dir_x;
			}
			if (points[i].y < config.point_size || points[i].y > 128-config.point_size) {
				points[i].dir_y = -points[i].dir_y;
			}
		}

	}
public:
	动态蜘蛛网(void) {}
	动态蜘蛛网& setup(void) {
		for (int i = 0; i < points.size(); i++) {
			points[i].x = rand()%128;
			points[i].y = rand()%128;
			float speed = 0.5;
			float angle = rand()%36000/100.0;
			points[i].dir_x = speed*cos(radians(angle));
			points[i].dir_y = speed*sin(radians(angle));
		}
		return *this;
	}
	动态蜘蛛网& setPic(PIC* p) {
		pic = p;
		return *this;
	}
	动态蜘蛛网& draw(void) {
		pic -> setColor(0x0a);
		update();
		for (int i = 0; i < points.size(); i++) {
			pic -> fillCircle(points[i].x, points[i].y, config.point_size);
			for (int j = 0; j < points.size(); j++) {
				if (i != j)
				if (std::hypot(points[i].x-points[j].x, points[i].y-points[j].y) < config.line_distance) {
					pic -> drawLine(points[i].x, points[i].y, points[j].x, points[j].y);
				}
			}
		}
		return *this;
	}
};
}
tbz::动态蜘蛛网 spider_web;

#include "spin_dice.hpp"
tbz::SPIN_DICE dice;

#define USE_HORIZONTAL 0

void ssd1327_init(void) {

	clr(M_RST);
	// delay_ms(200);
	HAL_Delay(200);
	set(M_RST);

	screen_writeCommand(0xae);//Set display off
	screen_writeCommand(0xa0);//Set re-map
	if(USE_HORIZONTAL){screen_writeCommand(0x66);}
	else{screen_writeCommand(0x51);}
	// 这一套命令真的好奇怪（
	screen_writeCommand(0xa1);//Set display start line
	screen_writeCommand(0x00);
	screen_writeCommand(0xa2);//Set display offset
	screen_writeCommand(0x00);
	screen_writeCommand(0xa4);//Normal Display
	screen_writeCommand(0xa8);//Set multiplex ratio
	screen_writeCommand(0x7f);
	screen_writeCommand(0xab);//Function Selection A
	screen_writeCommand(0x01);//Enable internal VDD regulator
	screen_writeCommand(0x81); // 设置对比度，值(0~127)
	screen_writeCommand(0x77); // 值不宜过大，否则，看不到亮度较低的颜色
	screen_writeCommand(0xb1);//Set Phase Length
	screen_writeCommand(0x51);
	screen_writeCommand(0xb3);//Set Front Clock Divider /Oscillator Frequency
	screen_writeCommand(0x01);
	// b1 还是 b3 对OLED啸叫产生了很明显的影响
	screen_writeCommand(0xb1);//
	screen_writeCommand(0x03);//0X03 enable
	screen_writeCommand(0xb6);//Set Second pre-charge Period
	screen_writeCommand(0x01);
	screen_writeCommand(0xbc);//Set Pre-charge voltage
	screen_writeCommand(0x07);
	screen_writeCommand(0xbe);//Set VCOMH
	screen_writeCommand(0x07);
	screen_writeCommand(0xd5);//Function Selection B
	screen_writeCommand(0x62);//Enable second pre-charge

	screen_writeCommand(0xaf);//Display on
}

#include "streamer.hpp"

tbz::STREAMER streamer;

extern FATFS SDFatFS;
extern char SDPath[];
extern FIL SDFile;

void sd_card_opration(void) {
	uint32_t byteswritten;                /* File write counts */
	uint32_t bytesread;                   /* File read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
	uint8_t rtext[100];                   /* File read buffers */
	char filename[] = "STM32cube.txt";
	char SensorBuff[100];
	// printf("********* STM32CubeMX FatFs Example *********\r\n\r\n");
	if(f_mount(&SDFatFS,SDPath,1) == FR_OK){
		// printf("f_mount sucess!!! \r\n");
		if(f_open(&SDFile,filename,FA_CREATE_ALWAYS|FA_WRITE) == FR_OK){
			// printf("f_open file sucess!!! \r\n");
			if(f_write(&SDFile,wtext,sizeof(wtext),(UINT*)&byteswritten) == FR_OK){
				// printf("f_write file sucess!!! \r\n");
				// printf("f_write Data : %s\r\n",wtext);
				// if(f_close(&SDFile) == FR_OK)
					// printf("f_close sucess!!! \r\n");
				// else
					// printf("f_close error : %d\r\n",retSD);
			}
			// else
				// printf("f_write file error\r\n");
		}
		// else
			// printf("f_open file error\r\n");
	}
	// else
		// printf("f_mount error : %d \r\n",retSD);

	auto retSD = f_open(&SDFile, filename, FA_READ);
	// if(retSD)
	// 	printf("f_open file error : %d\r\n",retSD);
	// else
	// 	printf("f_open file sucess!!! \r\n");

	retSD = f_read(&SDFile, rtext, sizeof(rtext), (UINT*)&bytesread);
	// if(retSD)
	// 	printf("f_read error!!! %d\r\n",retSD);
	// else{
	// 	printf("f_read sucess!!! \r\n");
	// 	printf("f_read Data : %s\r\n",rtext);
	// }

	retSD = f_close(&SDFile);
	// if(retSD)
	// 	printf("f_close error!!! %d\r\n",retSD);
	// else
	// 	printf("f_close sucess!!! \r\n");

	// if(bytesread == byteswritten)
	// 	printf("FatFs is working well!!!\r\n");

	if(f_open(&SDFile,(const char*)"Sensor.csv",FA_CREATE_ALWAYS|FA_WRITE) == FR_OK){
		// printf("Sensor.csv was opened/created!!!\r\n");
		sprintf(SensorBuff, "Item,Temp,Humi,Light\r\n");
		f_write(&SDFile,SensorBuff,strlen(SensorBuff),(UINT*)&byteswritten);

		for(int i = 0; i < 10; i++){
			sprintf(SensorBuff, "%d,%d,%d,%d\r\n",i + 1, i + 20, i + 30, i + 40);
			f_write(&SDFile,SensorBuff,strlen(SensorBuff),(UINT*)&byteswritten);
			f_sync(&SDFile);
		}
		f_close(&SDFile);
	}
}

int show_keyboard(tbz::PIC& pic) {
	int pressed_key_count = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 5; j++) {
			int index_of_res = 2+(i*5+j)*2*16;
			if (key[i*5+j]) {
				if (pressed_key_count < 6)
					keyboard[2+pressed_key_count] = 4+5*i+j;
				pressed_key_count ++;

				int offset = (16/8)*16*32;
				pic.draw1BitXBMP2x(10+22*j, 30+22*i, 16, 16, normal_keys+index_of_res+offset);
			} else {
				keyboard[2+pressed_key_count] = 0;
				pic.draw1BitXBMP2x(10+22*j, 30+22*i, 16, 16, normal_keys+index_of_res);
			}
		}
	}

	return pressed_key_count;
}



void LCD_WR_DATA8(u8 dat) {
	screen_writeData(dat);
}

void LCD_WR_DATA(u16 dat) {
	screen_writeDatas((uint8_t*)&dat, 2);
}

void LCD_WR_REG(u8 dat) {
	screen_writeCommand(dat);
}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2) {
	x2--; y2--;
	if(USE_HORIZONTAL==0)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
}

void LCD_Init(void) {

	// set(M_RST);
	// vTaskDelay(120);
	// clr(M_RST);



	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); //Sleep out
	vTaskDelay(120);
	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x36);
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA8(0x35);

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x32); //Vcom=1.35V

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x15); //GVDD=4.8V  颜色深度

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20); //VDV, 0x20:0v

	LCD_WR_REG(0xC6);
	LCD_WR_DATA8(0x0F); //0x0F:60Hz

	LCD_WR_REG(0xD0);
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x0E);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x31);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x48);
	LCD_WR_DATA8(0x17);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x31);
	LCD_WR_DATA8(0x34);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x0E);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x31);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x48);
	LCD_WR_DATA8(0x17);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x31);
	LCD_WR_DATA8(0x34);
	LCD_WR_REG(0x21);

	LCD_WR_REG(0x29);
}



void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color) {
	u16 i,j;
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);
	for(i=ysta;i<yend;i++) {
		for(j=xsta;j<xend;j++) {
			LCD_WR_DATA(color);
		}
	}
}

void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[]) {
	u16 i,j;
	u32 k=0;
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++) {
		for(j=0;j<width;j++) {
			LCD_WR_DATA8(pic[k*2]);
			LCD_WR_DATA8(pic[k*2+1]);
			k++;
		}
	}
}


int screen_width = 240;
int screen_height = 128;
int single_buffer_size = screen_width*screen_height;

void oled_function(void* argument) {

	// u8g2 在这里只用于绘制图形，屏幕的初始化与显示由独立的驱动实现
	U8G2_SSD1327_MIDAS_128X128_f_4W_HW_SPI u8g2(U8G2_R0);

	// sd_card_opration();

	// for (int i = 0; i < 16; i++) {
	// 	bit4_to_bit16_convert_color_set[i] = bit16_gray(i*0x11);
	// }

	qrcode.setPIC(&screen_pic).setContent("Hello World!");
	hanoi.set_U8G2(&u8g2);
	snake.set_U8G2(&u8g2);
	ani1.set_U8G2(&u8g2);
	ani2.set_U8G2(&u8g2);
	sw.set_U8G2(&u8g2);
	rwf.set_U8G2(&u8g2);
	mag.set_U8G2(&u8g2);
	mag.random_to_next();
	dice.setPic(&screen_pic).setup();
	sound_wave.setPic(&screen_pic).setup();
	spider_web.setPic(&screen_pic).setup();
	streamer.setPic(&screen_pic).setup();
	list_selector.set_U8G2(&u8g2);
	app_selector.set_U8G2(&u8g2);
	app_selector.setPIC(&screen_pic);
	app_selector.setTime(sTime);
	std::hash<const char*> hash_fn;
	tbz::base64::encode((const unsigned char*)base64_in, sizeof(base64_in), (char*)base64_out);

	LCD_Init();

	// ssd1327_init();
	// vTaskDelay(100);
	// ssd1327SetPosition(0, 0, 128, 128);


	while (true) {
		// 最高层级的弹窗
		if (key_pressed_func(4)) {
			switch(now_scene) {
				default: {
					next_scene = now_scene;
					now_scene = WINDOW::alert;
				} break;
				case WINDOW::alert: {
					now_scene = next_scene;
				}
			}
		}

		// 退出 APP 的动画
		if (key_pressed_func(9)) {
			if (now_app != APP_ENUM::main) {
				next_app = APP_ENUM::main;
				fade_to_next_scene(next_app);
				app_selector.slide_in();
			}
		}

		// screen_pic.clear();
		screen_pic.fade_clear();
		// 当前场景（部分动画或者弹窗依赖此状态）
		switch (now_scene) {
			case WINDOW::fade_animation: {
				tbz::animation::slide(&u8g2, now_scene, next_scene);
			} break;
			case WINDOW::alert: {
				tbz::ui::menu::alert::draw(&u8g2);
			} break;

			case WINDOW::normal_status: {
				switch (now_app) {
					case APP_ENUM::main: {
						if (key_pressed_func(0)) {
							app_selector.select_prev_app();
						}
						if (key_pressed_func(10)) {
							app_selector.select_next_app();
						}
						if (key_pressed_func(5)) {
							next_app = static_cast<APP_ENUM>(tbz::APP::now_select_app_id);
							fade_to_next_scene(next_app);
						}
						app_selector.setFPS(fps_count/0.5);
						app_selector.draw();
					} break;
					case APP_ENUM::SNAKE_GAME: {
						if (key_pressed_func(1)) {
							snake.move_left();
						}
						if (key_pressed_func(11)) {
							snake.move_right();
						}
						if (key_pressed_func(6)) {
							snake.move_down();
						}
						if (key_pressed_func(7)) {
							snake.move_up();
						}
						if (key_pressed_func(2)) {
							snake.speed_down();
						}
						if (key_pressed_func(12)) {
							snake.speed_up();
						}
						snake.game();
					} break;
					case APP_ENUM::ui_test3: {
						u8g2.drawBox(0,0,20,20);
						u8g2.drawBox(20,20,20,20);
						u8g2.drawFrame(10,40,20,20);
						u8g2.setFont(u8g2_font_DigitalDiscoThin_tf);
						sprintf(buf,"%d",114514);
						u8g2.drawStr(0,20,buf);
					} break;
					case APP_ENUM::qrcode_test: {
						qrcode.draw();
					} break;
					case APP_ENUM::animation1: {
						alert_font = u8g2_font_wqy16_t_gb2312;
						sprintf(alert_message, "你好，世界");
						u8g2.setFont(u8g2_font_wqy16_t_gb2312);
						int w = u8g2.getUTF8Width(alert_message);
						int h = u8g2.getMaxCharHeight();
						u8g2.drawUTF8(64-w/2, 64-h/2, "你好，世界");
						ani1.draw();
						ani2.draw();
					} break;
					case APP_ENUM::adc_animation: {
						// 动画2
						rwf.draw(adc_value2[0]*1000/65536);
						sound_wave.draw();
					} break;
					case APP_ENUM::animation3: {
						// 动画3
						sw.draw(sTime.Hours, sTime.Minutes, sTime.Seconds);
					} break;
					case APP_ENUM::HANOI_GAME: {
						if (key_pressed_func(1)) {
							hanoi.cursor_left();
						}
						if (key_pressed_func(11)) {
							hanoi.cursor_right();
						}
						if (key_pressed_func(6)) {
							hanoi.cursor_select();
						}
						alert_font = u8g2_font_wqy16_t_gb2312;
						sprintf(alert_message, "汉诺塔");
						hanoi.start_scene();
					} break;
					case APP_ENUM::WELCOM_SCENE: {
						alert_font = u8g2_font_NokiaSmallBold_tf;
						sprintf(alert_message, "WELCOM TO PLAY!");
						hanoi.welcom_scene();
					} break;
					case APP_ENUM::art_generator: {
						if (key_pressed_func(0)) {
							mag.random_to_next();
						}
						mag.draw();
					} break;
					case APP_ENUM::ui_test: {
						if (key_pressed_func(5, false)) {
							list_selector.select_next_item();
						}
						if (key_pressed_func(6, false)) {
							list_selector.select_prev_item();
						}
						list_selector.draw();
					} break;
					case APP_ENUM::key_test: {
						alert_font = u8g2_font_NokiaSmallBold_tf;
						sprintf(alert_message, "Hello World!");
						u8g2.clearBuffer();
						u8g2.setFont(u8g2_font_6x10_tf);
						// u8g2.setContrast(25);
						u8g2.drawStr(0, 10, "Hello World!");

						auto pressed_key_count = show_keyboard(screen_pic);

						sprintf (buf, "pressed:%d", pressed_key_count);
						u8g2.drawStr(0, 20, buf);
						sprintf (buf, "hash:%x", hash_fn("hello world!"));
						u8g2.drawStr(0, 110, buf);
						u8g2.setFont(u8g2_font_6x10_tf);
						u8g2.drawStr(0, 120, (const char*)base64_out);

					} break;
					case APP_ENUM::ui_test2: {
						u8g2.clearBuffer();
						for (int i = 0; i < 128; i++) {
							for (int j = 0; j < 128; j++) {
								if (i < 64 || i > 128 || j < 20 || j > 44)
								if ((i^j)&1) u8g2.drawPixel(i, j);
							}
						}
						// u8g2.drawLine(64, 20, 128, 20);
						// u8g2.drawLine(64, 20, 64, 44);
						// u8g2.drawLine(128,20, 128,44);
						// u8g2.drawLine(64,44,128,44);
						u8g2.drawStr(33,38, "WARNING");
						// u8g2.drawRBox()
						// u8g2.drawBox(64, 20, 64, 24);
						int error;

						// int a = (int)te_interp("(5+5)", 0); // Returns 10.
						int b = (int)te_interp("(5+5)", &error); // Returns 10, error is set to 0.
						// int c = (int)te_interp("(5+5", &error);
						sprintf(buf, "(5+5)=%d", b);
						u8g2.drawStr(0, 70, buf);
					} break;
					case APP_ENUM::MINESWEEPER_GAME: {
						// draw_pic(&u8g2, 0, 0, psychic_swamp);
						// draw_pic(&u8g2, 0, 0, tsetBones);
						// screen_pic.setMode(tbz::PIC::MODE::BIT4);
						screen_pic.drawXBMP(0, 0, 64, 64, tsetBones+2);
						screen_pic.drawXBMP(64, 0, 64, 64, tsetLava+2);
						screen_pic.drawXBMP(0, 64, 64, 64, tsetSand+2);
						screen_pic.drawXBMP(64, 64, 64, 64, tsetTower+2);
					} break;
					case APP_ENUM::TETRIS_GAME: {
						dice.draw();
					} break;
					case APP_ENUM::streamer: {
						streamer.draw();
					} break;
					default: {
						u8g2.clearBuffer();
						u8g2.setFont(u8g2_font_wqy16_t_gb2312);
						sprintf(buf, "-无信号-");
						int str_w = u8g2.getUTF8Width(buf);
						u8g2.drawUTF8(64-str_w/2, 64+16/2, buf);
					} break;
				}
			} break;


			// case WINDOW::unexist_scene:
			default: {
				now_scene = WINDOW::normal_status;
			} break;
		}

		// u8g2.setFont(u8g2_font_6x10_tf);
		// 	u8g2.drawStr(0, 30, err);
		// sprintf(buf, "address:%X", rrrrr);
		// u8g2.drawStr(0, 10, buf);
		// sprintf(buf, "adc:%dmv", adc_value2[0]);
		// u8g2.drawStr(0, 20, buf);
		// sprintf(buf, "count:%d", adc_count);
		// u8g2.drawStr(0, 30, buf);
		// adc_count = 0;
		// sprintf(buf, "address:%X", rrrrr+13);
		// u8g2.drawStr(0, 40, buf);

		fps_count0 ++;


		// screen_pic.drawBox(30, 30, 100, 100);
		screen_pic.setColor(0x3);
		ani1.draw2(screen_pic);
		ani2.draw2(screen_pic);
		// screen_pic.setColor(0xa);
		spider_web.draw();


		tbz::trans_u8g2buffer_to_4bitxbmp(u8g2.getBufferPtr(), screen_buffer, 128, 128);
		screen_pic.mixBuffer(screen_buffer);
		// screenWrite(screen_buffer, 128*128/2);

		// flip(LED);

		LCD_Address_Set(0, 0, screen_width, screen_height);
		// bit4_to_bit16(screen_buffer, scrren_buffer_16bit, 128*128/2);
		bit4_to_bit16(screen_buffer, scrren_buffer_16bit, 128, 128, 240, 320);
		screen_writeDatas(scrren_buffer_16bit, single_buffer_size);
		screen_writeDatas(scrren_buffer_16bit+single_buffer_size, single_buffer_size);
		// screen_writeDatas((uint8_t*)scrren_buffer_16bit, 57600);
		// screen_writeDatas((uint8_t*)scrren_buffer_16bit+57600, 57600);
		// for (int i = 0; i < 320; i++) {
		// 	for (int j = 0; j < 120; j++) {
		// 		auto pix = screen_buffer[i*64+j];
		// 		uint16_t color1 = (pix>>4) & 0xff ? 0xffff : 0x0000;
		// 		uint16_t color2 = pix & 0xff ? 0xffff : 0x0000;
		// 		LCD_WR_DATA(color1);
		// 		LCD_WR_DATA(color2);
		// 	}
		// }

		// LCD_Fill(0, 0, 240, 240, rand());
		vTaskDelay(10);
	}
}


void core(void) {

	while (true) {
		flip(LED);
		delay(500);
	}

}


// DMA 完成一次设置的完整转换（2048下）进入这个中断
// 这里对波形进行傅里叶变换可以避免数据拷贝造成的频谱混叠
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if(hadc==&hadc2) {

		std::copy(adc_value, adc_value+adc_size, adc_value2);
		// 但是它不应该总是在进行计算，所以应该想办法避免应用未启动时计算频谱
		switch (now_app) {
			case APP_ENUM::adc_animation: {
				sound_wave.fft_calc();
			}
			default: {}
		}
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {

	if (huart -> Instance == UART4) {
		// streamer.switchBuffer();
		HAL_UARTEx_ReceiveToIdle_DMA(&huart4, streamer.getRxBuffer(), 128*128);
		// switch (now_app) {
		// 	case APP_ENUM::streamer: {
		// 		memcpy(screen_pic.getBasePic2()+2, streamer.getPicBuffer(), 128*128/2);
		// 	}
		// }
		// HAL_UART_Receive_DMA(&huart4, streamer.getStreamBuffer(), 128*128/8);
	}
}


// Void HAL_SPI_TxRxCallback ( SPI_HandleTypeDef *hspi) {
// 	if (hspi -> Instance == SPI6) {
// 		HAL_SPI_Transmit_IT()
// 	}
// }