#ifndef HANOI_H
#define HANOI_H

#include "oled.h"
#include "xbmp_describ.h"
#include <cstdio>

extern const uint8_t hanoi_hand_sign[];
extern const uint8_t hanoi_ground[];
extern const uint8_t* hanoi_blocks[];
extern const uint8_t hanoi_title[];
extern const uint8_t hanoi_copyleft[];
extern char buf[];
class game_hanoi {
private:
	U8G2* u8g2;                 // 从外部传入用于驱动 u8g2 的指针
	int used_time = 0;          // 从开始到现在的用时，最小单位为 0.1 秒
	int used_step = 0;          // 移动的总步数
	int8_t selected_block = -1; // 当前选中的盘子（-1 表示没有选中）

	int8_t cursor = 0;          // 当前光标所在的柱子

	 // 三个柱子，每个柱子最多 6 个盘子，第一个元素为当前盘子数量
	uint8_t stack[3][7] {
		{6, 5, 4, 3, 2, 1, 0},
		{0},
		{0}
	};

	/**
	 * @brief 检查移动是否合法
	 * @details 检测当前选中的盘子是否小于 to 柱子上最上层的盘子
	 * @param to 移动的目标柱子
	*/
	bool check_stack(uint8_t to) {
		if (stack[to][0] == 0) return true;
		else return selected_block < stack[to][stack[to][0]];
	}

	/**
	 * @brief 拿起放下盘子
	 * @details 将当前选中的盘子放到 where 柱子上
	 * @param where 放置的目标柱子
	*/
	void getput_block(uint8_t where) {

		if (selected_block == -1) {
			// 如果当前没有选中盘子，且 where 柱子上有盘子，则选中
			if (stack[where][0] != 0) {
				selected_block = stack[where][stack[where][0]--];
			}
		} else if (check_stack(where)) {
			// 柱子 where 的最上放放置当前盘子，并处理数量
			stack[where][++stack[where][0]] = selected_block;
			selected_block = -1; // 取消选中
			used_step++;  // 步数加一
		}
	}

	/**
	 * @brief 绘制图片，左上角为原点，自动读取图片大小
	*/
	void draw_pic(uint8_t x, uint8_t y, const uint8_t* pic) {
		u8g2 -> drawXBMP(x, y,
			get_pic_width(pic),
			get_pic_height(pic),
			pic2xbmp(pic));
	}
	/**
	 * @brief 绘制柱子
	 * @param x 柱子中心的 x 坐标
	*/
	void draw_zhuzi(uint8_t x) {
		u8g2 -> drawRBox(x-2,8,6,75,1);
	}
	/**
	 * @brief 绘制空心柱子
	 * @param x 柱子中心的 x 坐标
	*/
	void draw_kong_zhuzi(uint8_t x) {
		u8g2 -> drawRFrame(x-2,8,6,75,1);
	}
	void draw_block(uint8_t x, uint8_t y, uint8_t id) {
		draw_pic(x-get_pic_width(hanoi_blocks[id])/2, y, hanoi_blocks[id]);
	}
	void draw_blocks(void) {
		for (int i = 1; i <= stack[0][0]; i++) {
			draw_block (20+1, 82-i*11, stack[0][i]);
		}
		for (int i = 1; i <= stack[1][0]; i++) {
			draw_block (63+1, 82-i*11, stack[1][i]);
		}
		for (int i = 1; i <= stack[2][0]; i++) {
			draw_block (106+1, 82-i*11, stack[2][i]);
		}
	}
public:
	game_hanoi(U8G2* u8g2) {
		this->u8g2 = u8g2;
	}
	game_hanoi(){}
	void set_U8G2(U8G2* d) {
		u8g2 = d;
	}

	void cursor_left(void) {
		cursor--;
		if (cursor < 0) cursor = 2;
	}
	void cursor_right(void) {
		cursor++;
		if (cursor > 2) cursor = 0;
	}
	void cursor_select(void) {
		getput_block(cursor);
	}

	void welcom_scene() {
		u8g2 -> clearBuffer();
		draw_pic(17, 23, hanoi_title);
		draw_pic(14, 99, hanoi_copyleft);
	}
	void start_scene() {
		u8g2 -> clearBuffer();
		u8g2 -> setFont(u8g2_font_NokiaLargeBold_tf);
		switch(cursor) {
			case 0: {
				draw_kong_zhuzi(20);
				draw_zhuzi(63);
				draw_zhuzi(106);
				draw_block(20, 0, selected_block);
			} break;
			case 1: {
				draw_zhuzi(20);
				draw_kong_zhuzi(63);
				draw_zhuzi(106);
				draw_block(63, 0, selected_block);
			} break;
			case 2: {
				draw_zhuzi(20);
				draw_zhuzi(63);
				draw_kong_zhuzi(106);
				draw_block(106, 0, selected_block);
			} break;
		}
		draw_blocks();
		for (int i = 0; i < 16; i++) {
			draw_pic(i*8, 82, hanoi_ground);
		}
		draw_pic(0, 89, hanoi_hand_sign);


		// sprintf(buf, "stack[0][0] = %d", stack[0][0]);
		// u8g2 -> drawUTF8(0, 10, buf);
		// sprintf(buf, "stack[1][0] = %d", stack[1][0]);
		// u8g2 -> drawUTF8(0, 20, buf);
		// sprintf(buf, "stack[2][0] = %d", stack[2][0]);
		// u8g2 -> drawUTF8(0, 30, buf);

		sprintf(buf, "%06d", used_time);
		u8g2 -> drawUTF8(70, 110, buf);
		sprintf(buf, "%06d", used_step);
		u8g2 -> drawUTF8(70, 127, buf);
	}

	void game_scene() {

	}

};

#endif