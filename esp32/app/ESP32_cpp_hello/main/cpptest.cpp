/*
 * cpptest.cpp
 *
 *  Created on: 12.10.2016
 *      Author: pcbreflux
 */

#include "cpptest.h"

uint8_t cpptest::getTx() const {
	return tx;
}

void cpptest::setTx(uint8_t tx) {
	this->tx = tx;
}

uint8_t cpptest::getTy() const {
	return ty;
}

void cpptest::setTy(uint8_t ty) {
	this->ty = ty;
}

void cpptest::next() {
	tx++;
	ty++;
}
