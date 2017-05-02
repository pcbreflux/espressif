/* Copyright (c) 2016 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */
#ifndef APA102_H_
#define APA102_H_

#define clockPin GPIO_NUM_22     // GPIO-PIN
#define dataPin GPIO_NUM_23     // GPIO-PIN
#define MAX_COLORS 60  // Number of APA102 LEDs in String

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

class apa102 {
private:
	uint32_t ledcount;
	uint32_t looppos;
	void writeRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness);
	void startFrame();
	void endFrame();
	void writeByte(uint8_t b);
	uint8_t random(uint8_t min,uint8_t max);
public:
	apa102(uint32_t ledcount);
	typedef struct colorRGBB  {
	    uint8_t red, green, blue, brightness;
	} colorRGBB;
	uint32_t getLEDCount() const;
	void setLEDCount(uint32_t ledcount);

	void writeColor(colorRGBB color);
	void writeColors(colorRGBB * colors, uint16_t count);
	void setColor(colorRGBB color);
	void fadeInOutColor(colorRGBB color, uint16_t delayms);
	void ramdomBlink(uint32_t loops, uint16_t delayms);
	void ramdomFade(uint32_t loops, uint16_t delayms);
	void ramdomWalk(uint32_t loops, uint16_t delayms);
	void ramdomBackWalk(uint32_t loops, uint16_t delayms);
	void ramdomStep(uint32_t loops, uint16_t delayms);
	void ramdomStepR(uint32_t loops, uint16_t delayms);
	void ramdomBackStep(uint32_t loops, uint16_t delayms);
	void ramdomBackStepR(uint32_t loops, uint16_t delayms);
	void ramdomSingle(uint32_t loops, uint16_t delayms);
	void test();
	void test2();
};

#endif /* APA102_H_ */
