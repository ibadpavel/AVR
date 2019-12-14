/*
 * fixPointNum.h
 *
 * Created: 23.06.2019 13:48:10
 *  Author: Pasik
 */ 


#ifndef FIXPOINTNUM_H_
#define FIXPOINTNUM_H_

typedef uint32_t fixPointReal;
	
uint32_t fprDivide(uint32_t dividend, uint32_t divider) {
	uint32_t total = 0;
	uint32_t pow = 1;

	//uint32_t tmp = dividend;
	for (uint8_t i = 0; i < 5;) {
		if (dividend < divider) {
			dividend *= 10;
			pow *= 10;
			total *= 10;
			i++;
			continue;
		}

		dividend -= divider;
		total++;
	}

	return (total << 12) / pow;
}

uint32_t fprMultiple(uint32_t m1, uint32_t m2) {
	return (m1 * m2) >> 12;
}

	
uint32_t fprGetTotal(uint32_t value) {
	return (value & 0xFFFFF000) >> 12;
}
	
uint32_t fprGetFract(uint32_t value) {
	return ((value & 0x00000FFF) * 100000) >> 12;
}

#endif /* FIXPOINTNUM_H_ */