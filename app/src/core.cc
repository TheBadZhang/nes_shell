#include "core.h"
#include "common.h"

// https://github.com/Robert-van-Engelen/tinylisp
// 尝试添加一个 lisp repl 到项目当中

// https://www.taterli.com/3199/
// https://twitter.com/shuichi_takano/status/1477702448907419649
// https://github.com/pondahai
// nes 模拟器

// https://wokwi.com/projects/376479299848582145 （电池充电）
// https://wokwi.com/projects/328271658006610514 （旋转立方体）
// https://wokwi.com/projects/376931330900285441 （指南针）
// https://wokwi.com/projects/385987811439126529 （水平指南针）

// 手写数字识别

// 摄像头

// 滚动数字时钟

//TODO: PIC.drawXBMP 右边界绘制，还有左边界绘制
// APP 进入退出动画可以考虑重新设计一个更富有动画效果的
// 文字超过范围滚动显示
// 列表动画、滑动条、复选框（开关）（方形或者圆形样式）、滑动条、进度条、按钮
// 多级菜单，列表

// libjpeg dma2d

// APP 调试的问题

#include <span>
#include <functional>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
// #include <string>

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
#define my_printf(buf_, fmt_, x_...) \
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
		}
		// int retlen = sprintf(buf, "HELLO WORLD");
		// HAL_UART_Transmit(&huart4, (uint8_t*)buf, retlen, 1000);
		// std::string str_r = tfm::format("%s", "Hello World!");
		// std::string buf2;
		// sprintf(buf, "HELLO WORLD");
		// HAL_UART_Transmit(&huart4, (uint8_t*)buf2.c_str(), buf2.size(), 1000);
		// HAL_UART_Transmit(&huart4, (uint8_t*)"HELLO WORLD", 12, 1000);
		// my_print(fmt::format("The answer is {}.", 42));
		// uart_print("HELLO WORLD");
		my_printf(buf, "fps:%d\n", fps_count);
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

extern const uint8_t snow_animation_pic[];
tbz::SPRITE_ANIMATION<10> ani1(snow_animation_pic, 72, 8, 8, 8, 128, 128, 9);
tbz::SPRITE_ANIMATION<10> ani2(snow_animation_pic+72, 54, 8, 8, 8, 128, 128, 6);
// tbz::SPRITE_ANIMATION<1> fireworks(fireworks_pic+2, fireworks_pic[0], fireworks_pic[1], 40, 40, 40, 40, 8);

tbz::game::hanoi hanoi;
tbz::round_watch_face rwf;
tbz::SquareWatch sw;
tbz::MODERN_ART_GENERATOR mag;
tbz::game::SNAKE snake;
tbz::QRCode qrcode;
tbz::APP_SELECTOR app_selector;
tbz::SOUND_WAVE sound_wave;


#include "base64.h"
char base64_in[] {"Hello World!"};
uint8_t base64_out[BASE64_ENCODE_OUT_SIZE(sizeof(base64_in))+4];
int base64_out_len;


uint8_t screen_buffer[128*128/2];
uint8_t screen_buffer2[128*128/2+2] = { 0x7f, 0x7f };
tbz::PIC screen_pic(screen_buffer2, [](tbz::PIC& pic) {
	pic.mode = tbz::PIC::MODE::BIT4;
	pic.setColor(0x01);
});



void oled_function(void* argument) {

	// __HAL_DMA_DISABLE_IT(&hdma_spi6_tx, DMA_IT_HT);  // 关闭DMA hite

	U8G2_SSD1327_MIDAS_128X128_f_4W_HW_SPI u8g2(U8G2_R0);
	// U8G2_SSD1607_200x200_F_4W_HW_SPI u8g2(U8G2_R0);
	u8g2.begin();
	ssd1327SetPosition(0, 0, 128, 128);

	// game_hanoi hanoi(&u8g2);
	qrcode.set_U8G2(&u8g2);
	qrcode.setContent("Hello World!");
	hanoi.set_U8G2(&u8g2);
	snake.set_U8G2(&u8g2);
	ani1.set_U8G2(&u8g2);
	ani2.set_U8G2(&u8g2);
	sw.set_U8G2(&u8g2);
	rwf.set_U8G2(&u8g2);
	mag.set_U8G2(&u8g2);
	mag.random_to_next();
	sound_wave.setPic(&screen_pic);
	sound_wave.setup();
	list_selector.set_U8G2(&u8g2);
	app_selector.set_U8G2(&u8g2);
	app_selector.setTime(sTime);
	std::hash<const char*> hash_fn;

	base64_out_len = base64_encode((const unsigned char*)base64_in, sizeof(base64_in), (char*)base64_out);
	base64_out[base64_out_len] = '\0';


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

		screen_pic.clear();
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
						u8g2.setFont(u8g2_font_6x10_tf);
						u8g2.drawStr(0, 10, (const char*)base64_out);
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
						int pressed_key_count = 0;
						for (int i = 0; i < 3; i++) {
							for (int j = 0; j < 5; j++) {
								sprintf(buf, "%c", 'A'+i*5+j);
								u8g2.drawStr(17+22*j, 43+22*i, buf);
								if (key[i*5+j]) {
									if (pressed_key_count < 6)
										keyboard[2+pressed_key_count] = 4+5*i+j;
									pressed_key_count ++;
									u8g2.drawBox(11+22*j, 31+22*i, 19, 19);
								} else {
									keyboard[2+pressed_key_count] = 0;
									u8g2.drawFrame(10+22*j, 30+22*i, 20, 20);
								}
							}
						}

						sprintf (buf, "pressed:%d", pressed_key_count);
						u8g2.drawStr(0, 20, buf);
						sprintf (buf, "hash:%x", hash_fn("hello world!"));
						u8g2.drawStr(0, 110, buf);

					} break;
					case APP_ENUM::ui_test2: {
						u8g2.clearBuffer();
						for (int i = 0; i < 128; i++) {
							for (int j = 0; j < 128; j++) {
								if (i < 64 || i > 128 || j < 20 || j > 44)
								if ((i^j)&1) u8g2.drawPixel(i, j);
							}
							// u8g2.drawLine(64, 20, 128, 20);
							// u8g2.drawLine(64, 20, 64, 44);
							// u8g2.drawLine(128,20, 128,44);
							// u8g2.drawLine(64,44,128,44);
							u8g2.drawStr(33,38, "WARNING");
							// u8g2.drawRBox()
							// u8g2.drawBox(64, 20, 64, 24);
						}
						int error;

						int a = (int)te_interp("(5+5)", 0); // Returns 10.
						int b = (int)te_interp("(5+5)", &error); // Returns 10, error is set to 0.
						int c = (int)te_interp("(5+5", &error);
						sprintf(buf, "(5+5)=%d", a);
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


		//! 没办法处理BIT4的PIC绘制BIT1的素材
		//! 这个问题可能之后会尝试解决一下吧
		// screen_pic.drawBox(30, 30, 100, 100);
		// screen_pic.setMode(tbz::PIC::MODE::BIT1).setColor(0x1);
		// scrren_pic.setColor(0x1);
		// ani1.draw2(screen_pic);
		// ani2.draw2(screen_pic);


		constexpr uint8_t trans_white = 0xf;
		constexpr uint8_t trans_black = 0x0;
		auto ptr = u8g2.getBufferPtr();
		for (int y = 0; y < 128/8; y++) {     // 8行一组
			for (int x = 0; x < 128/2; x++) { // 2个像素一个字节
				// u8g2 像素先垂直后水平扫描
				auto pixe1 = *ptr++;
				auto pixe2 = *ptr++;
				for (int k = 0; k < 8; k++) {
					auto ppixe1 = (pixe1 >> k) & 0x01 ? trans_white : trans_black;
					auto ppixe2 = (pixe2 >> k) & 0x01 ? trans_white : trans_black;
					screen_buffer[(y*8+k)*64+x] = (ppixe1 << 4) | ppixe2;
				}
			}
		}
		for (int i = 0; i < 128*128/2; i++) {
			screen_buffer[i] |= screen_buffer2[i+2];
		}
		screenWrite(screen_buffer, 128*64);
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