#ifndef __TBZ_APP_H_
#define __TBZ_APP_H_

#include "xbmp_describ.h"

class APP {
private:
	PIC* app_icon;
public:
	APP() {}
	APP(PIC* icon) : app_icon(icon) {}
	// APP(const uint8_t* icon, uint8_t w, uint8_t h) {
	// 	app_icon = new PIC(icon, w, h);
	// }
};

#endif // __TBZ_APP_H_