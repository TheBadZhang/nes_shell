#include "core.h"
#include "common.h"

// https://github.com/Robert-van-Engelen/tinylisp
// 尝试添加一个 lisp repl 到项目当中

// nes 模拟器

// 俄罗斯方块

// 摄像头

#include <span>
#include <functional>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
// #include <string>

#include "hardware_timer.h"
#include "oled.h"
#include "sys.h"
#include "trick.h"
// #include "usbd_cdc.h"
// #include "usbd_cdc_if.h"
#include "st7735.h"

#include "FreeRTOS.h"
#include "task.h"

#include "xbmp_describ.h"

// #include "tinyexpr.h"
#include "hanoi.hpp"
#include "sound_wave.hpp"


extern const unsigned short test_img_128x128[][128];




const uint8_t pic[] = {
	0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,
	0x00,0x00,0x28,0x10,0x28,0x00,0x00,0x00,
	0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00,
	0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,
	0x82,0x44,0x00,0x10,0x00,0x44,0x82,0x00,
	0x82,0x00,0x10,0x38,0x10,0x00,0x82,0x00,
	0x00,0x10,0x10,0x6c,0x10,0x10,0x00,0x00,
	0x10,0x10,0x00,0xc6,0x00,0x10,0x10,0x00,
	0x10,0x00,0x00,0x82,0x00,0x00,0x10,0x00,
	0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,
	0x00,0x00,0x10,0x28,0x10,0x00,0x00,0x00,
	0x00,0x54,0x10,0x7c,0x10,0x54,0x00,0x00,
	0x92,0x54,0x10,0xee,0x10,0x54,0x92,0x00,
	0x92,0x10,0x00,0xc6,0x00,0x10,0x92,0x00,
	0x10,0x00,0x00,0x82,0x00,0x00,0x10,0x00
};


#include "animation.hpp"
#include "round_watch_face.hpp"
#include "square_watch.hpp"


// hardcoded time values
int time_minutes = 10;
int time_hours = 10;
int time_seconds = 45;

#include "qrcode.h"

typedef const char* strss;
extern strss verb[];
extern strss adj[];
extern strss noun[];

// void dataUpdate(void* argument) {

// 	while (true) {
// 		aaa = rand () % 112;
// 		bbb = rand () % 115;
// 		ccc = rand () % 180;
// 		vTaskDelay(2000);
// 	}
// }

int count = 0;

uint8_t keyboard[8] {0,0,0,0,0,0,0,0};


bool key[16] {false};
bool key_pressed_flag[16] {false};

int current_time = 0;

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

bool key_pressed_func (int id_of_key) {
	if (key[id_of_key] && !key_pressed_flag[id_of_key]) {
		key_pressed_flag[id_of_key] = true;
		return true;
	} else {
		return false;
	}
}

char buf[128];





extern "C" void led0_task(void* argument) {
	while (true) {
		flip(LED);
		time_seconds++;
		vTaskDelay(500);
	}
}

osThreadId_t ANOOLEDHandle;
const osThreadAttr_t ANOOLED_attributes = {
	.name = "OLED",
	.stack_size = 1024 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t keyscanHandle;
const osThreadAttr_t keyscan_attributes = {
	.name = "key_scan",
	.stack_size = 1024 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t ipsHandle;
const osThreadAttr_t ips_attributes = {
	.name = "ips",
	.stack_size = 1024 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};
void oled_func(void* argument);
void ips_func(void* argument);
extern "C" void load(void) {
	uint32_t random_seed = 0;
	HAL_RNG_GenerateRandomNumber(&hrng, &random_seed);
	srand(random_seed);
	ANOOLEDHandle = osThreadNew(oled_func, NULL, &ANOOLED_attributes);
	keyscanHandle = osThreadNew(key_scan, NULL, &keyscan_attributes);
	ipsHandle = osThreadNew(ips_func, NULL, &ips_attributes);
}

void ips_func(void* argument) {
	// tbz::tft::st7735::init();
	// vTaskDelay(100);
	tbz::tft::st7735::fillScreen(tbz::tft::st7735::COLOR::BLACK);
	while (true) {
		tbz::tft::st7735::writeString(0, 0, "pressed:42", Font_11x18, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::MAGENTA);
		vTaskDelay(50);
	}

}

extern int aaa, bbb, ccc;
extern "C" void get_art_index_random(void);

using str = const char*;
str ui_list[] = {
	"[ Setting ]",
	"~ Disp Brightness",
	"~ List Cur",
	"~ List Ani",
	"~ Min Ani",
	"~ Fade Ani",
	"~ Btn SPT",
	"~ BtN LPT",
	"+ L ufd Fm Scr"
};


enum class NOW_SCENE {
	unexist_scene,   // 不存在的场景
	WELCOM_SCENE, START_SCENE,
	normal_scene,
	art_generator,
	ui_test,
	ui_test2,
	fade_animation,
	alert,
	animation1,
	animation2,
	animation3,
} now_scene = NOW_SCENE::normal_scene, next_scene = NOW_SCENE::unexist_scene;


char alert_message[128] {"Hello World!"};
auto alert_font = u8g2_font_wqy16_t_gb2312;
// u8g2_font_NokiaSmallBold_tf
/**
 * @brief 绘制图片，左上角为原点，自动读取图片大小
*/
void draw_pic(U8G2* u8g2, uint8_t x, uint8_t y, const uint8_t* pic) {
	u8g2 -> drawXBMP(x, y,
		get_pic_width(pic),
		get_pic_height(pic),
		pic2xbmp(pic));
}

void erase1(U8G2* u8g2) {
	// 保存绘制状态
	auto color = u8g2 -> getDrawColor();
	u8g2 -> setDrawColor(0);
	for(int i = 0; i < 128; i++)
	for(int j = 0; j < 128; j++)
	if (i&1 && j&1) u8g2 -> drawPixel(i, j);
	// 恢复绘制状态
	u8g2 -> setDrawColor(color);
}
void erase2(U8G2* u8g2) {
	// 保存绘制状态
	auto color = u8g2 -> getDrawColor();
	u8g2 -> setDrawColor(0);
	for (int i = 0; i < 128; i++)
	for (int j = 0; j < 128; j++)
	if ((i^j)%2) u8g2 -> drawPixel(i, j);
	// 恢复绘制状态
	u8g2 -> setDrawColor(color);
}
void erase3(U8G2* u8g2) {
	// 保存绘制状态
	auto color = u8g2 -> getDrawColor();
	u8g2 -> setDrawColor(0);
	for(int i = 0; i < 128; i++)
	for(int j = 0; j < 128; j++)
	if (i&1 || j&1) u8g2 -> drawPixel(i, j);
	// 恢复绘制状态
	u8g2 -> setDrawColor(color);
}
void erase4(U8G2* u8g2) {
	// 保存绘制状态
	auto color = u8g2 -> getDrawColor();
	u8g2 -> setDrawColor(0);
	u8g2 -> drawBox(0, 0, 128, 128);
	// 恢复绘制状态
	u8g2 -> setDrawColor(color);
}

void swap_animation(U8G2* u8g2) {
	static enum class ANI {
		FADE_1, FADE_2, FADE_3, FADE_4
	} ani = ANI::FADE_1;
	const int fade_frames = 2;
	static int count = 0;
	switch (ani) {
		case ANI::FADE_1: {
			erase1(u8g2);
		} break;
		case ANI::FADE_2: {
			erase2(u8g2);
		} break;
		case ANI::FADE_3: {
			erase3(u8g2);
		} break;
		case ANI::FADE_4: {
			erase4(u8g2);
		} break;
	}

	count ++;
	if (count == fade_frames) {
		count = 0;
		switch (ani) {
			case ANI::FADE_1: {
				ani = ANI::FADE_2;
			} break;
			case ANI::FADE_2: {
				ani = ANI::FADE_3;
			} break;
			case ANI::FADE_3: {
				ani = ANI::FADE_4;
			} break;
			case ANI::FADE_4: {
				ani = ANI::FADE_1;
				now_scene = next_scene;
			} break;
		}
	}
}

void fade_to_next_scene(U8G2* u8g2, NOW_SCENE scene) {
	// swap_animation(u8g2);
	now_scene = NOW_SCENE::fade_animation;
	next_scene = scene;
}
void next_scene_func(NOW_SCENE scene) {
	now_scene = scene;
}



game_hanoi hanoi;


tbz::animation<10> ani1(pic, 72, 8, 8, 8, 9);
tbz::animation<10> ani2(pic+72, 54, 8, 8, 8, 6);

tbz::round_watch_face rwf;

tbz::SquareWatch sw;



// const char str122[] {"你好，世界"};
// uint8_t str122_x, str122_y;
// Create the QR code
QRCode qrcode;
const int qrcode_version = 3;
namespace tbz {
	constexpr int get_size_from_version_size(int size) {
		return (((size * size) + 7) / 8);
	}
	constexpr int get_qrcode_BufferSize(int version) {
		return get_size_from_version_size(4 * version + 17);
	}
}
uint8_t qrcodeData[tbz::get_qrcode_BufferSize(qrcode_version)];
// uint8_t qrcodeData[512];

void oled_func(void* argument) {

	U8G2_SSD1327_MIDAS_128X128_f_4W_HW_SPI u8g2(U8G2_R0);
	// U8G2_SSD1607_200x200_F_4W_HW_SPI u8g2(U8G2_R0);
	u8g2.begin();
	// game_hanoi hanoi(&u8g2);
	hanoi.set_U8G2(&u8g2);
	ani1.set_U8G2(&u8g2);
	ani2.set_U8G2(&u8g2);
	sw.set_U8G2(&u8g2);
	rwf.set_U8G2(&u8g2);
	get_art_index_random();
	std::hash<const char*> hash_fn;

	qrcode_initText(&qrcode, qrcodeData, qrcode_version, 0, "HELLO WORLD");
	// char* sss = (char*)malloc(128);

	// ADC 定时采集
	// HAL_TIM_Base_Start_IT(&htim16);
	// str122_x = (128-u8g2.getUTF8Width(str122))/2;
	// str122_y = (64+16)/2;
	while (true) {

		// u8g2.clearBuffer();
		// sprintf(buf, "address:%X", sss);
		// u8g2.drawStr(0, 10, buf);
		if (key_pressed_func(4)) {
			switch(now_scene) {
				default: {
					next_scene = now_scene;
					now_scene = NOW_SCENE::alert;
				} break;
				case NOW_SCENE::alert: {
					now_scene = next_scene;
				}
			}
		}
		if (key_pressed_func(10)) {
			switch (now_scene) {
				case NOW_SCENE::normal_scene: {
					fade_to_next_scene(&u8g2, NOW_SCENE::WELCOM_SCENE);
				} break;
				case NOW_SCENE::WELCOM_SCENE: {
					fade_to_next_scene(&u8g2, NOW_SCENE::START_SCENE);
				} break;
				case NOW_SCENE::START_SCENE: {
					fade_to_next_scene(&u8g2, NOW_SCENE::art_generator);
				} break;
				case NOW_SCENE::art_generator: {
					fade_to_next_scene(&u8g2, NOW_SCENE::ui_test);
				} break;
				case NOW_SCENE::ui_test: {
					fade_to_next_scene(&u8g2, NOW_SCENE::ui_test2);
				} break;
				case NOW_SCENE::ui_test2: {
					fade_to_next_scene(&u8g2, NOW_SCENE::animation1);
				} break;
				case NOW_SCENE::animation1: {
					fade_to_next_scene(&u8g2, NOW_SCENE::animation2);
				} break;
				case NOW_SCENE::animation2: {
					fade_to_next_scene(&u8g2, NOW_SCENE::animation3);
				} break;
				case NOW_SCENE::animation3: {
					fade_to_next_scene(&u8g2, NOW_SCENE::normal_scene);
				} break;
			}
		}
		switch (now_scene) {
			case NOW_SCENE::ui_test2: {
				const int screen_size = 128;
				auto size = 17+4*qrcode_version;
				auto scale = screen_size/size;
				auto offset_x = (screen_size-size*scale)/2;
				auto offset_y = (screen_size-size*scale)/2;
				for (uint8_t y = 0; y < qrcode.size; y++) {
					// Each horizontal module
					for (uint8_t x = 0; x < qrcode.size; x++) {
						// Print each module (UTF-8 \u2588 is a solid block)
						// Serial.print(qrcode_getModule(&qrcode, x, y) ? "\u2588\u2588": "  ");
						u8g2.setDrawColor(qrcode_getModule(&qrcode, x, y) ? 1 : 0);
						for (int i = 0; i < scale; i++) {
							for (int j = 0; j < scale; j++) {
								u8g2.drawPixel(offset_x+scale*x+i, offset_y+scale*y+j);
							}
						}
						// u8g2.drawPixel(offset_x+2*x, 2*y+offset_y);
						// u8g2.drawPixel(offset_x+2*x+1, 2*y+offset_y);
						// u8g2.drawPixel(offset_x+2*x, 2*y+1+offset_y);
						// u8g2.drawPixel(offset_x+2*x+1, 2*y+1+offset_y);
					}
				}
				u8g2.setDrawColor(1);
			} break;
			case NOW_SCENE::animation1: {
				alert_font = u8g2_font_wqy16_t_gb2312;
				sprintf(alert_message, "你好，世界");
				// 动画1
				// u8g2.drawUTF8(str122_x, str122_y, str122);
				ani1.draw();
				ani2.draw();
			} break;
			case NOW_SCENE::animation2: {
				// 动画2
				HAL_ADC_Start(&hadc1);
				HAL_ADC_PollForConversion(&hadc1, 50);

				rwf.draw(HAL_ADC_GetValue(&hadc1)/33.0);
				// sound_wave.draw_sound_wave();
			} break;
			case NOW_SCENE::animation3: {
				// 动画3
				if (time_seconds >= 60) {
					time_seconds = 0;
					time_minutes++;
					if (time_minutes >= 60) {
						time_minutes = 0;
						time_hours++;
						if (time_hours >= 12) {
							time_hours = 0;
						}
					}
				}
				sw.draw(time_hours, time_minutes, time_seconds);
			} break;
			case NOW_SCENE::alert: {
				erase2(&u8g2);
				const int8_t offset_x = 4;
				const int8_t offset_y = -4;
				const int8_t margin_x = 6;
				const int8_t margin_y = 6;
				const uint8_t radius = 2;
				// u8g2.setFont(u8g2_font_micro_mr);
				u8g2.setFont(alert_font);
				auto h = u8g2.getMaxCharHeight();
				auto w = u8g2.getUTF8Width(alert_message);
				int x = 64-w/2-3;
				int y = 64-h/2-3;
				u8g2.setDrawColor(0);
				u8g2.drawRBox(x+offset_x, y+offset_y, w+margin_x, h+margin_y, radius);
				u8g2.setDrawColor(1);
				u8g2.drawRFrame(x+offset_x, y+offset_y, w+margin_x, h+margin_y, radius);
				u8g2.drawRBox(x, y, w+margin_x, h+margin_y, radius);
				u8g2.setDrawColor(2);
				u8g2.setFontMode(1);
				u8g2.drawUTF8(x+margin_x/2, y+h-1+margin_y/2, alert_message);
				u8g2.setDrawColor(1);
				u8g2.setFontMode(0);
			} break;
			case NOW_SCENE::fade_animation: {
				swap_animation(&u8g2);
			} break;
			case NOW_SCENE::START_SCENE: {
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
			case NOW_SCENE::WELCOM_SCENE: {
				alert_font = u8g2_font_NokiaSmallBold_tf;
				sprintf(alert_message, "WELCOM TO PLAY!");
				hanoi.welcom_scene();
			} break;
			case NOW_SCENE::art_generator: {
				alert_font = u8g2_font_wqy16_t_gb2312;
				sprintf(alert_message, "艺术发生器");
				u8g2.clearBuffer();
				if (key_pressed_func(0)) {
					get_art_index_random();
				}
				u8g2.setFont(u8g2_font_wqy16_t_gb2312);
				sprintf(buf, "%s%s的", verb[aaa], adj[bbb]);
				u8g2.drawUTF8(0, 16, buf);
				sprintf(buf, "%s", noun[ccc]);
				u8g2.drawUTF8(0, 32, buf);

			} break;
			case NOW_SCENE::ui_test: {
				u8g2.clearBuffer();
				u8g2.setFont(u8g2_font_micro_mr);
				for (int i = 0; i < sizeof(ui_list)/sizeof(str); i++) {
					u8g2.drawUTF8(0, 6+6*i, ui_list[i]);
				}

			} break;
			case NOW_SCENE::normal_scene: {
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
		}
		u8g2.sendBuffer();
		vTaskDelay(16);
	}
}


// void test_span_print(U8G2* u8g2, std::span<char> span) {
// 	u8g2->drawStr(0, 10, span.data());
// }

void core(void) {


	// char test_span[] {"Hello World!"};
	// std::span<char> test_span2(test_span, 6);


	// extern void cdc_acm_init(void);
	// cdc_acm_init();
	// extern void cdc_acm_data_send_with_dtr_test();




	// xTaskCreate((TaskFunction_t)led0_task,
	// 			(const char*)"led_task",
	// 			(uint16_t)128,
	// 			(void*)NULL,
	// 			(UBaseType_t)1,
	// 			(TaskHandle_t*)&StartTask_Handler);
	// vTaskStartScheduler();



	U8G2_SSD1327_MIDAS_128X128_f_4W_HW_SPI u8g2(U8G2_R0);

	u8g2.setFont(u8g2_font_6x10_tf);
	u8g2.begin();

	while (true) {
		// key_scan();
		u8g2.clearBuffer();
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

				// if (i==0) {
				// 	if (key[i*5+j])
				// 		keyboard[2+j] = 4+j;
				// 	else
				// 		keyboard[2+j] = 0;
				// }
				// sprintf(buf, "key:%d", key[i]);
			}
		}

		// HAL_SPI_Transmit()

		sprintf (buf, "pressed:%d", pressed_key_count);
		u8g2.drawStr(0, 20, buf);
		u8g2.sendBuffer();
		// USBD_HID_SendReport(&hUsbDeviceHS,(uint8_t*)&keyboard,sizeof(keyboard));
		flip(LED);
		// delay(500);
	}

}

// void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin) {
// 	if (GPIO_Pin == KEY_Pin) {
// 		USBVcom_printf("Key pressed:%d\r\nresult:%lf\r\n", count++, te_interp("1+2+3+4+5+6+7+8+9+10", nullptr));
// 		// __HAL_GPIO_EXTI_CLEAR_IT(KEY_Pin);
// 	}
// }