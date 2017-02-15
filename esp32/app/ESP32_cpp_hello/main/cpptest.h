/*
 * cpptest.h
 *
 *  Created on: 12.10.2016
 *      Author: pcbreflux
 */

#ifndef CPPTEST_H_
#define CPPTEST_H_
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

class cpptest {
private:
	uint8_t tx,ty;
public:
	cpptest(uint8_t tx,uint8_t ty)
	 { setTx(tx); setTy(ty); }
	uint8_t getTx() const;
	void setTx(uint8_t tx);
	uint8_t getTy() const;
	void setTy(uint8_t ty);
	void next();
};

#endif /* CPPTEST_H_ */
