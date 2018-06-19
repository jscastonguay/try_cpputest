/*
 * test_process.cpp
 *
 *  Created on: 18 juin 2018
 *      Author: Proprietaire
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "process.h"
#include "execute.h"
#include "eusart1.h"


bool EUSART1_is_tx_ready() {
	return mock().actualCall("EUSART1_is_tx_ready").returnBoolValue();
}

void EUSART1_Write(uint8_t txData) {
	mock().actualCall("EUSART1_Write").withParameter("txData", txData);
}

int execute_setCmd(ProcessCmd * cmd) {
	return mock().actualCall("execute_setCmd").withParameterOfType("ProcessCmd", "cmd", cmd).returnBoolValue();
}

TEST_GROUP(process_init) {
	void teardown() {
		mock().clear();
	}
};

TEST(process_init, uartTxNotReady) {
	mock().expectOneCall("EUSART1_is_tx_ready").andReturnValue(false);
	mock().expectNoCall("EUSART1_Write");
	process_init();
	mock().checkExpectations();
}

TEST(process_init, uartTxReady) {
	mock().expectOneCall("EUSART1_is_tx_ready").andReturnValue(true);
	mock().expectOneCall("EUSART1_Write").withIntParameter("txData", '@');
	process_init();
	mock().checkExpectations();
	CHECK_EQUAL(state, IDLE);
}

TEST_GROUP(process_send) {
	void teardown() {
		mock().clear();
	}
};

TEST(process_send, sendNormalData) {

	uint8_t * data = (uint8_t *)"test";

	mock().expectOneCall("EUSART1_Write").withIntParameter("txData", 't');
	mock().expectOneCall("EUSART1_Write").withIntParameter("txData", 'e');
	mock().expectOneCall("EUSART1_Write").withIntParameter("txData", 's');
	mock().expectOneCall("EUSART1_Write").withIntParameter("txData", 't');

	send(data, strlen((const char *)data));
	mock().checkExpectations();
}

TEST(process_send, sendEmptyData) {

	uint8_t * data = (uint8_t *)"";

	mock().expectNoCall("EUSART1_Write");

	send(data, strlen((const char *)data));
	mock().checkExpectations();
}

TEST(process_send, passNull) {

	uint8_t * data = NULL;

	mock().expectNoCall("EUSART1_Write");

	send(data, 1);
	mock().checkExpectations();
}

TEST_GROUP(process_setCmd) {
	void teardown() {
		mock().clear();
	}
};

TEST(process_setCmd, passNull) {
	CHECK_FALSE(process_setCmd(NULL));
}

class ProcessCmdComparator : public MockNamedValueComparator
{
public:
    virtual bool isEqual(const void* object1, const void* object2)
    {
    	if (memcmp(object1, object2, sizeof(ProcessCmd)) == 0) {
    		return true;
    	}
        return false;
    }
    virtual SimpleString valueToString(const void* object)
    {
    	char buffer[80];
    	ProcessCmd * obj = (ProcessCmd *)object;
    	snprintf(buffer, 80, "ProcessCmd - cmd: \%s\"  - timeRef: %lu  - option: %d",
    			obj->cmd, obj->timeRef, obj->option);
    	SimpleString s(buffer);
        return s;
    }
};

TEST(process_setCmd, validCmdStart) {

	ProcessCmd cmd = {
			.cmd = "start",
			.timeRef = 1000,
			.option = 0
	};

	ProcessCmdComparator comparator;
	mock().installComparator("ProcessCmd", comparator);
	mock().expectOneCall("execute_setCmd").withParameterOfType("ProcessCmd", "cmd", &cmd).andReturnValue(true);

	POINTERS_EQUAL(currentCmd.cmd, NULL);

	bool result = process_setCmd(&cmd);

	CHECK(result);
	CHECK_EQUAL(currentCmd.timeRef, 1000);
	CHECK_EQUAL(currentCmd.option, 0);
	mock().checkExpectations();
	mock().removeAllComparatorsAndCopiers();
}
