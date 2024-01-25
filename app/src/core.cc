#include "core.h"
#include "common.h"

// https://github.com/Robert-van-Engelen/tinylisp
// 尝试添加一个 lisp repl 到项目当中

// https://www.taterli.com/3199/
// https://twitter.com/shuichi_takano/status/1477702448907419649
// https://github.com/pondahai
// nes 模拟器

// 迷宫

// 烟花动画

// 扫雷

// 2048

// 五子棋（？

// https://www.bilibili.com/video/BV1DP4y1S7XM
// 俄罗斯方块

// 生命游戏

// 摄像头

// base64 编解码库

// 滚动数字时钟

//TODO: PIC.drawXBMP 右边界绘制，还有左边界绘制
// APP 进入退出动画可以考虑重新设计一个更富有动画效果的
// 文字超过范围滚动显示
// 列表动画、滑动条、复选框（开关）（方形或者圆形样式）、滑动条、进度条、按钮
// 多级菜单，列表

// malloc 的问题

// libjpeg dma2d

// lua 和 ADC 输入两个问题没解决
// APP 调试的问题

#include <span>
#include <functional>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
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

#include "xbmp_describ.hpp"

// #include "tinyexpr.h"
#include "hanoi.hpp"
#include "snake.hpp"
#include "sound_wave.hpp"


extern const unsigned short test_img_128x128[][128];

#include "app.hpp"

#include "alert.hpp"


#include "animation.hpp"
#include "round_watch_face.hpp"
#include "square_watch.hpp"

#include "list_selector.hpp"



RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

#include "qrcode.hpp"
#include "modern_art_generator.hpp"

int count = 0;

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




int lua_flip_LED(lua_State *L) {
	flip(LED);
	return 0;
}


extern "C" void led0_task(void* argument) {


	// auto L = luaL_newstate();
	// // bLua::reg_global_func(L, "flip_LED1", lua_flip_LED1);
	// // bLua::reg_global_func(L, "delay", HAL_Delay);
	// luaL_openlibs(L);

	// lua_register(L, "flip_LED", lua_flip_LED);

	// const char* test = R"(
	// 	function test()
	// 		flip_LED()
	// 	end
	// )";

	while (true) {
		flip(LED);
		// https://blog.csdn.net/qq_32216815/article/details/116934350
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
		// luaL_dostring(L, test);
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


// 这两的顺序必须保持一致才可以
enum class APP_ENUM {
	WELCOM_SCENE = 0,
	key_test,
	art_generator,
	ui_test,
	ui_test2,
	ui_test3,
	animation1,
	animation2,
	animation3,
	HANOI_GAME,
	SNAKE_GAME,

	main
} now_app = APP_ENUM::main, next_app = APP_ENUM::main;

void fade_to_next_scene(APP_ENUM app) {
	// swap_animation(u8g2);
	next_scene = now_scene;
	now_scene = WINDOW::fade_animation;
	now_app = app;
}
void next_scene_func(WINDOW scene) {
	now_scene = scene;
}

uint8_t clock_app_icon[202] { 0x28, 0x28 };
uint8_t animation_app_icon[202] { 0x28, 0x28 };
uint8_t animation2_app_icon[202] { 0x28, 0x28 };
extern const uint8_t bottom_arc[];
int tbz::APP::now_select_app_id = 0;
tbz::APP apps[] {
	tbz::APP(unknow_app_icon, "WELCOM_SCENE"),
	tbz::APP(unknow_app_icon, "按键测试"),
	tbz::APP(unknow_app_icon, "艺术发生器"),
	tbz::APP(unknow_app_icon, "ui_test"),
	tbz::APP(unknow_app_icon, "ui_test2"),
	tbz::APP(unknow_app_icon, "ui_test3"),
	tbz::APP(animation_app_icon, "animation1"),
	tbz::APP(animation2_app_icon, "animation2"),
	tbz::APP(clock_app_icon,  "时钟"),
	tbz::APP(hanoi_app_icon,  "汉诺塔"),
	tbz::APP(unknow_app_icon, "贪吃蛇"),
};
tbz::game::hanoi hanoi;

extern const uint8_t snow_animation_pic[];
tbz::SPRITE_ANIMATION<10> ani1(snow_animation_pic, 72, 8, 8, 8, 128, 128, 9);
tbz::SPRITE_ANIMATION<10> ani2(snow_animation_pic+72, 54, 8, 8, 8, 128, 128, 6);
tbz::SPRITE_ANIMATION<4> ani3(snow_animation_pic, 72, 8, 8, 8, 40, 40, 9);
tbz::SPRITE_ANIMATION<4> ani4(snow_animation_pic+72, 54, 8, 8, 8, 40, 40, 6);

tbz::round_watch_face rwf;

tbz::SquareWatch sw;


tbz::game::SNAKE snake;
// const char str122[] {"你好，世界"};
// uint8_t str122_x, str122_y;
// Create the QR code
QRCode qrcode;
const int qrcode_version = 3;
uint8_t qrcodeData[tbz::qrcode::get_BufferSize(qrcode_version)];
// uint8_t qrcodeData[512];
uint32_t g_address;


double radians(double degrees) {
	return degrees * 3.14159265357 / 180;
}

void oled_func(void* argument) {

	// __HAL_DMA_DISABLE_IT(&hdma_spi6_tx, DMA_IT_HT);  // 关闭DMA hite

	U8G2_SSD1327_MIDAS_128X128_f_4W_HW_SPI u8g2(U8G2_R0);
	// U8G2_SSD1607_200x200_F_4W_HW_SPI u8g2(U8G2_R0);
	u8g2.begin();
	// game_hanoi hanoi(&u8g2);
	hanoi.set_U8G2(&u8g2);
	snake.set_U8G2(&u8g2);
	ani1.set_U8G2(&u8g2);
	ani2.set_U8G2(&u8g2);
	sw.set_U8G2(&u8g2);
	rwf.set_U8G2(&u8g2);
	list_selector.set_U8G2(&u8g2);
	get_art_index_random();
	std::hash<const char*> hash_fn;
	apps[6].setIconUpdateFunc([&ani3, &ani4](tbz::APP& app) {
		app.getPic().clear();
		ani3.draw2(app.getPic());
		ani4.draw2(app.getPic());
		app.getPic().drawFrame(0, 0, 40, 40);

	});

	apps[7].setIconUpdateFunc([](tbz::APP& app) {
		for (int i = 0; i < 40; i++) {
			for (int j = 0; j < 40; j++) {
				if ((i^j) & 1)
				app.getPic().drawPixel(i, j);
			}
		}
	});

	apps[8].setIconUpdateFunc([&sTime](tbz::APP& app) {
		auto& app_icon = app.getPic();

		app_icon.clear();
		app_icon.drawFilledCircle(20, 20, 4);

		int hand_angle = sTime.Seconds*6;
		int hand_lenght_long = 16;
		int hand_legth_short = 10;
		int center_x = 20;
		int center_y = 20;
		float xpos;
		float ypos;
		float xpos2;
		float ypos2;


		// draw 60 dots (pixels) around the circle, one for every minute/second
		for (int i=0; i<12; i++) { // draw 60 pixels around the circle
			xpos = round(center_x + sin(radians(i * 30)) * 17); // calculate x pos based on angle and radius
			ypos = round(center_y - cos(radians(i * 30)) * 17); // calculate y pos based on angle and radius

			app_icon.drawPixel(xpos,ypos); // draw white pixel on position xpos and ypos
		}

		// calculate starting and ending position of the second hand
		xpos = round(center_x + sin(radians(hand_angle)) * hand_lenght_long); // calculate x pos based on angle and radius
		ypos = round(center_y - cos(radians(hand_angle)) * hand_lenght_long); // calculate y pos based on angle and radius
		xpos2 = round(center_x + sin(radians(hand_angle + 180)) * hand_legth_short); // calculate x pos based on angle and radius
		ypos2 = round(center_y - cos(radians(hand_angle + 180)) * hand_legth_short); // calculate y pos based on angle and radius

		app_icon.drawLine(xpos, ypos, xpos2, ypos2); // draw the main line
		// u8g2 -> drawLine(xpos, ypos, xpos2, ypos2); // draw the main line
		// u8g2 -> setDrawColor(0); // black color
		// u8g2 -> drawDisc(xpos2, ypos2, 3); // draw small filled black circle
		// u8g2 -> setDrawColor(1); // white color
		app_icon.drawCircle(xpos2, ypos2, 3);
		app_icon.drawCircle(center_x, center_y, 19);

	});

	// now_scene = static_cast<WINDOW>(0);

	qrcode_initText(&qrcode, qrcodeData, qrcode_version, 0, "HELLO WORLD");
	// char* sss = (char*)malloc(128);

	double ani_n = 0;
	double ani_n_2 = 0;
	double ani_n_3 = 1;
	// ADC 定时采集
	// HAL_TIM_Base_Start_IT(&htim16);
	// str122_x = (128-u8g2.getUTF8Width(str122))/2;
	// str122_y = (64+16)/2;
	while (true) {

		// u8g2.clearBuffer();


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

		if (key_pressed_func(9)) {
			if (now_app != APP_ENUM::main) {
				next_app = APP_ENUM::main;
				fade_to_next_scene(next_app);
				ani_n_2 = 0;
			}
		}

		switch (now_scene) {
			case WINDOW::fade_animation: {
				tbz::animation::slide(&u8g2, now_scene, next_scene);
			} break;
			// case WINDOW::window_start_animation: {
			// 	tbz::animation::window_start(&u8g2, now_scene, next_scene);
			// } break;
			// case WINDOW::layer_in_animation: {
			// 	tbz::animation::layer_in(&u8g2, now_scene, next_scene);
			// } break;
			// case WINDOW::layer_out_animation: {
			// 	tbz::animation::layer_out(&u8g2, now_scene, next_scene);
			// } break;
			// case WINDOW::layer_in_animation: {
			// 	ani_n_2 = 0;
			// }
			case WINDOW::alert: {
				tbz::ui::menu::alert::draw(&u8g2);
			} break;

			case WINDOW::normal_status: {
				switch (now_app) {
					case APP_ENUM::main: {
						if (key_pressed_func(0)) {
							tbz::APP::now_select_app_id--;
							if (tbz::APP::now_select_app_id < 0) {
								tbz::APP::now_select_app_id = sizeof(apps)/sizeof(tbz::APP)-1;
								ani_n -= sizeof(apps)/sizeof(tbz::APP)-1;
							} else {
								ani_n += 1;
							}
							ani_n_3 = 1;
						}
						if (key_pressed_func(10)) {
							tbz::APP::now_select_app_id++;
							if (tbz::APP::now_select_app_id >= sizeof(apps)/sizeof(tbz::APP)) {
								tbz::APP::now_select_app_id = 0;
								ani_n += sizeof(apps)/sizeof(tbz::APP)-1;
							} else {
								ani_n -= 1;
							}
							ani_n_3 = 1;
						}
						if (key_pressed_func(5)) {
							next_app = static_cast<APP_ENUM>(tbz::APP::now_select_app_id);
							fade_to_next_scene(next_app);
						}
						if (ani_n>0.01) ani_n -= 0.1*(ani_n+0.1);
						else if(ani_n < -0.01) ani_n += 0.1*(-ani_n+0.1);

						if (ani_n_2 < 1.0) ani_n_2 += 0.1;
						else ani_n_2 = 1.0;

						if (ani_n_3 > 0) ani_n_3 -= 0.1;
						else ani_n_3 = 0;
						u8g2.clearBuffer();
						u8g2.setBitmapMode(1);


						// APP 图标显示
						for (int i = 0; i < sizeof(apps)/sizeof(tbz::APP); i++) {
							// 使不同APP产生一定的弧度
							// 使当前选中app显示在画面中间，且有切换动画
							int x = 44+44*(i-tbz::APP::now_select_app_id-ani_n)*ani_n_2;
							int y = 30+11*abs(i-tbz::APP::now_select_app_id-ani_n)*ani_n_2;
							// 设置字体计算宽度使得文字居中
							u8g2.setFont(u8g2_font_micro_mr);
							int wstr = u8g2.getUTF8Width(apps[i].getName());
							u8g2.drawStr(x+22-wstr/2, y-2, apps[i].getName());
							// 跳过屏幕之外的元素
							if (x < -40 || x > 127) continue;
							// 更新APP图标，然后，绘制APP图标
							apps[i].updateIcon();
							draw_picture(&u8g2, x, y, apps[i].getPic().getBasePic());
						}


						// 底栏和顶栏显示
						draw_pic(&u8g2, 0, 90, bottom_arc);
						const int bubble_y = 109;
						for (int i = 0; i < sizeof(apps)/sizeof(tbz::APP); i++) {
							int x = 64+6*(i-tbz::APP::now_select_app_id-ani_n);
							if (x < 0 || x > 127) continue;
							u8g2.setDrawColor(0);
							u8g2.drawFilledEllipse(x, bubble_y, 4, 4, U8G2_DRAW_ALL);
							u8g2.setDrawColor(1);
							if (i == tbz::APP::now_select_app_id) {
								u8g2.drawFilledEllipse(x, bubble_y, 3, 3, U8G2_DRAW_ALL);
							} else {
								u8g2.drawCircle(x, bubble_y, 3, U8G2_DRAW_ALL);
							}
						}

						u8g2.drawRBox(0, 0, 128, 15, 3);
						u8g2.setFont(u8g2_font_wqy12_t_gb2312);
						u8g2.setFontMode(1);
						u8g2.setDrawColor(2);
						u8g2.drawUTF8(
							64-u8g2.getUTF8Width(apps[tbz::APP::now_select_app_id].getName())/2,
							126+ani_n_3*14,
							apps[tbz::APP::now_select_app_id].getName()
						);
						sprintf(buf, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
						u8g2.drawUTF8(64-u8g2.getUTF8Width(buf)/2, 12, buf);
						u8g2.setDrawColor(1);
						u8g2.setDrawColor(1);
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
						u8g2.sendBuffer();
						u8g2.drawFrame(10,40,20,20);
						u8g2.sendBuffer();
						u8g2.setFont(u8g2_font_DigitalDiscoThin_tf);
						sprintf(buf,"%d",114514);
						u8g2.drawStr(0,20,buf);
					} break;
					case APP_ENUM::ui_test2: {
						const int screen_width = 128;
						const int screen_height = 128;
						const int screen_size = std::min(screen_width, screen_height);

						auto size = 17+4*qrcode_version;
						auto scale = screen_size/size;

						auto offset_x = (screen_width-size*scale)/2;
						auto offset_y = (screen_height-size*scale)/2;

						for (uint8_t y = 0; y < qrcode.size; y++) {
							for (uint8_t x = 0; x < qrcode.size; x++) {
								u8g2.setDrawColor(qrcode_getModule(&qrcode, x, y) ? 1 : 0);
								u8g2.drawBox(offset_x+scale*x, offset_y+scale*y, scale, scale);
							}
						}
						u8g2.setDrawColor(1);
					} break;
					case APP_ENUM::animation1: {
						alert_font = u8g2_font_wqy16_t_gb2312;
						sprintf(alert_message, "你好，世界");
						// 动画1
						// u8g2.drawUTF8(str122_x, str122_y, str122);
						ani1.draw();
						ani2.draw();
					} break;
					case APP_ENUM::animation2: {
						// 动画2
						HAL_ADC_Start(&hadc1);
						HAL_ADC_PollForConversion(&hadc1, 50);

						rwf.draw(HAL_ADC_GetValue(&hadc1)/33.0);
						// sound_wave.draw_sound_wave();
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
				}
			} break;


			// case WINDOW::unexist_scene:
			// default: {
			// 	now_scene = WINDOW::normal_status;
			// } break;
		}


		// u8g2.setFont(u8g2_font_6x10_tf);
		// sprintf(buf, "address:%X", g_address);
		// u8g2.drawStr(0, 10, buf);

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