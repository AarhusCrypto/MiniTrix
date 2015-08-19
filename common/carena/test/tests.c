/*!
 * \file Tests.c
 *
 * This test tests that we can accept incoming connections and
 * communicate with the created peer. To run this test successfully
 * start "tests" and "telnet localhost 2020" in two different terminals.
 *
 * In the telnet-terminal observe "Hi to you peer 1". Typing "quit"
 * and pressing enter in this terminal will make "tests"
 * terminate. This behaviour is the expected one.
 */

#include <carena.h>
#include <osal.h>
#include <common.h>
#include <stdio.h>
#include <string.h>


int main(int c, char **a) {
  OE oe = 0;
  CArena arena = 0;
  oe = OperatingEnvironment_New();

  arena = CArena_new(oe);

  arena->listen(2020);

  while(1) {
    uint id = 0;
    usleep(1000000);
    printf("Peers %d\n", arena->get_no_peers());
    for(id = 0;id<arena->get_no_peers();++id) {
      Data d = Data_new(oe,32);
      MpcPeer peer = arena->get_peer(id);
      sprintf((char*)d->data,"Hi to you peer %u\n",id+1);
      peer->send(d);
      if (peer->has_data()) {
	Data d = Data_new(oe,4);
	peer->receive(d);
	oe->p((char*)d->data);
	if (d->ldata == 4 && memcmp("quit",d->data,4) == 0)
	  goto out;
      } else {
	oe->p("no data");
      }
    }
  }
 out:
  oe->p("Gone !");
  CArena_destroy( &arena);

  

  OperatingEnvironment_Destroy(&oe);
  return 0;
}
