#include <stdio.h>
#include <string.h>
#include <datetime.h>
#include <testcase.h>
#include <errno.h>
#include <osal.h>

#define ITER 256

static void * client(void * a) {
	RC rc = RC_OK;
	OE oe = (OE)a;
	uint socket_fd = 0;
	uint i = 0;
	uint ret = 0;

	rc = oe->open("ip 127.0.0.1 2020", &socket_fd);
	if (rc != RC_OK) return 0;

	byte * data = oe->getmem(1024 );

  //oe->print("Sending\n");
  for(i = 0; i < ITER;++i) {
	  uint ldata = 1024 ;
	  uint written = ldata;
	  
	  rc = oe->write(socket_fd, data, &written);
	  if (rc != RC_OK) goto fail;

	  rc = oe->read(socket_fd, data, &ldata); written = ldata;
	  if (rc != RC_OK) goto fail;
	  
      rc = oe->write(socket_fd, data, &written);
	  if (rc != RC_OK) goto fail;
  }
  //oe->print("Data sent\n");

  ret = 1;
fail:
  oe->close(socket_fd);
  oe->putmem(data);
  return (void*)(ull)ret;
}


static int test_listen_and_send(OE oe) {

	ThreadID tid = 0;
	uint socket_fd = 0;
	DateTime dt = DateTime_New(oe);
	RC rc = RC_OK;
	int success = 1;
	int port = 2020;
	int reuse_addr_option = 1;

	oe->open("listen 2020", &socket_fd);
	oe->newthread(&tid, client, oe);
	{
		int i = 0;
		FD client_fd = 0;
		char * buf = oe->getmem(1024);
		while ((oe->accept(socket_fd, &client_fd)) == RC_OK) {
			unsigned long long start = dt->getNanoTime();

			for (i = 0; i < ITER; ++i) {
				uint lbuf = 1024, written = lbuf;

				rc = oe->read(client_fd, buf, &lbuf);
				if (rc != RC_OK) success = 0;

				rc = oe->write(client_fd, buf, &written);
				if (rc != RC_OK) success = 0;

				rc = oe->read(client_fd, buf, &lbuf);
				if (rc != RC_OK) success = 0;
			}
			oe->jointhread(tid);
			oe->close(client_fd);
			oe->print("Time %llu\n", dt->getNanoTime() - start);
			oe->putmem(buf);
			break;
		}

		return 1;
	}
}

static int test_dummy(OE oe) {
	oe->print("Oh I love %d\n", 6);
	return True;
}

static Test tests[] = { 
	{ "Dummy", test_dummy },
	{ "Test listen and send", test_listen_and_send }
};

TestSuit testsendraw_suit = {
	"CArena - Testing we can send raw data.",
	0,0,
	tests, sizeof(tests) / sizeof(Test)
};

TestSuit * get_testsendraw_testsuit() {
	return &testsendraw_suit;
}

