/*
 * coov3test.c
 *
 *  Created on: Dec 8, 2014
 *      Author: rwl
 */

#include <coov3.h>
#include <stdio.h>
#include <osal.h>

typedef struct _my_car_ {
	uint fuel;
	uint km_per_fuel;
	void (*drive)(uint km);
	void (*addfuel)(uint liters);
	ull (*getfuel)();
} * MyCar;

COO_DCL(MyCar,void,drive,uint km);
COO_DEF_NORET_ARGS(MyCar,drive,uint km;,km) {
	uint required_listers = km/(this->km_per_fuel);
	if (required_listers > this->fuel) {
		this->fuel = 0;
	} else {
		this->fuel -= required_listers;
	}
}


COO_DCL(MyCar,void,addfuel, uint liters);
COO_DEF_NORET_ARGS(MyCar,addfuel, uint liters;, liters) {
	this->fuel += liters;
}

COO_DCL(MyCar, ull, getfuel);
COO_DEF_RET_NOARGS(MyCar,ull,getfuel) {
	return this->fuel;
}

MyCar MyCarNew(Memory mem, uint km_per_fuel) {
	MyCar result = mem->alloc(sizeof(*result));
	if (!result) return 0;

	result->km_per_fuel = km_per_fuel;

	COO_ATTACH(MyCar,result,drive);
	COO_ATTACH(MyCar,result,addfuel);
	COO_ATTACH(MyCar,result,getfuel);

	return result;
}

static void test1(Memory linuxMem) {
	MyCar car = 0;
	printf("Coo initialized\n");
	car = MyCarNew(linuxMem,12);
	printf("igetfuel: %p\n",car->getfuel);
	printf("rec     : %p\n",(ull*)car->getfuel());
	printf("car     : %p\n",car);
	printf("getfuel : %p\n",MyCar_getfuel);
	printf("MyCar instance created\n");

}

int main(int c, char ** a) {
	Memory linuxMem = LinuxMemoryNew();
	printf("Got linux mem\n");
	if (InitializeCOO(1024,linuxMem) != True) {
		printf("Initializing COO failed.");
		return -1;
	}
	test1(linuxMem);

	{
		MyCar car = MyCarNew(linuxMem, 13);
		car->addfuel(10);
		car->drive(120);
	    printf("car has %llu liters of fuel after driving 120 km\n",car->getfuel());
	}

	{
	}
	return 0;
}
