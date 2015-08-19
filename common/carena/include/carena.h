#ifndef CARENA_H
#define CARENA_H

#include <osal.h>

#define RETERR(MSG,RC) {\
    CAR c = {{0}};				       \
    c.lmsg=osal_strlen((MSG));			       \
    mcpy(c.msg,(MSG),c.lmsg);			       \
    c.rc = (RC);				       \
    return c;}

typedef enum _car_rc_ {
  OK,
  NO_MEM,
  BAD_ARG,
  BAD_DATA,
  NO_CONN
} CarRc;

typedef struct _carena_result_ {
  char msg[512];
  uint lmsg;
  uint rc;
} CAR;

/*!
 * Representation of a remote peer.
 */
typedef struct _mpc_peer_ {

  int ffd;
  /*!
   * Returns true if {read} can be invoked with out blocking.
   */
  bool (*has_data)(void);

  /*!
   * Send data to this peer.  Data is sent asynchroneously this
   * function returns immediately.
   */
  CAR (*send)(Data data);

  /*!
   * Receive data from peer.
   * this function blocks until data->ldata is read.
   */
  CAR (*receive)(Data data);

  /*!
   * Get the IP address of the remote peer
   */
  char * (*get_ip)(void);

  /*!
   * Get the port of the remote peer.
   */
  uint (*get_port)(void);

  /*!
   * Get the id of this peer.
   */
  uint (*get_id)(void);

  void * impl;

} * MpcPeer;


/*!
 * A peer connected or disconnected, the CArena maintains a list of
 * {ConnectingListeners} which are notified when this happens.
 * 
 */
typedef struct _connection_listener_ {
  OE oe;

  void (*client_connected)(MpcPeer peer);

  void (*client_disconnected)(MpcPeer * peer);

  void * impl;

} * ConnectionListener;


  
/*!
 * During computation notifications about progress might be
 * given. CArena maintains a list of {ComputationListener}'s which are
 * notified if this happens.
 */
typedef struct _computation_listener_ {
  
  void (*begun)(void);
  
  void (*progress)(char * cstr_msg);
  
  void (*end)(void);
  
} * ComputationListener;

/*!
 * The CArena.
 */
typedef struct _carena_ {

  void (*disconnect)(uint peerid);
  /*!
   * Connect to peer at {hostname} on port {port}.
   */
  CAR (*connect)(char * hostname, uint port);
  
  /*!
   * Start listening for incoming peer-connections on {port}.
   */
  CAR (*listen)(uint port);

  /*!
   * Start listening for incoming peers on {port} until {no} peers has
   * connected. Then stop listening and return.
   */
  CAR (*listen_wait)(uint no, uint port);

  /*!
   * Get peer with id {pid}.
   */
  MpcPeer (*get_peer)(uint pid);

  /*!
   * Get the number of peers connected so far.
   */ 
  uint (*get_no_peers)(void);

  /*!
   * Add new connection listener
   */
  void (*add_conn_listener)(ConnectionListener lst);

  /*!
   * Remove the connection lister {lst}
   */
  void (*rem_conn_listener)(ConnectionListener lst);

  void * impl;
} * CArena;

CArena CArena_new(OE oe);
void CArena_destroy( CArena * arena );

#endif
