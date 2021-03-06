// #include <netdb.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
#include <rdma/rdma_cma.h>

#define TEST_NZ(x) do { if ( (x)) die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) die("error: " #x " failed (returned zero/null)."); } while (0)

 struct context {
  struct ibv_context *ctx;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_comp_channel *comp_channel;
  struct connection *conn;

  pthread_t cq_poller_thread;
};

struct connection {
  struct rdma_cm_id   *id;
  struct ibv_qp       *qp;
  struct ibv_mr       *send_mr;
  char                *send_region;
};

void die(const char *reason)
{
  dprintf("%s\n", reason);
  exit(EXIT_FAILURE);
}

static struct context *s_ctx = NULL;

void rdma_init(const char* ip, const char *port) {

  struct addrinfo *addr;
  struct rdma_cm_event event,*_event = NULL;
  struct rdma_cm_id *rdma_cm_conn= NULL;
  struct rdma_event_channel *ec = NULL;

  TEST_NZ(getaddrinfo(ip, port, NULL, &addr));

  TEST_Z(ec = rdma_create_event_channel());
  TEST_NZ(rdma_create_id(ec, &rdma_cm_conn, NULL, RDMA_PS_TCP));
  TEST_NZ(rdma_resolve_addr(rdma_cm_conn, NULL, addr->ai_addr, TIMEOUT_IN_MS));

  freeaddrinfo(addr);

//-------------------------------------------------------------

// TODO: remove cycle!
  while (rdma_get_cm_event(ec, &_event) == 0) {

    memcpy(&event, _event, sizeof(*_event));
    rdma_ack_cm_event(_event);

    if (event.event == RDMA_CM_EVENT_ADDR_RESOLVED) {
      printf("RDMA_CM_EVENT_ADDR_RESOLVED\n");
      break;
    }
  }


//  on_addr_resolved(event.id);
// int on_addr_resolved(struct rdma_cm_id *id)
// {

// build_context(id->verbs);
// void build_context(struct ibv_context *verbs)
// {
  struct ibv_context *verbs = event.id->verbs;
  if (s_ctx) {
    if (s_ctx->ctx != verbs)
      die("cannot handle events in more than one context.");
  }
  else {
    s_ctx = (struct context *)kzalloc(sizeof(struct context));
    s_ctx->ctx = verbs;
    TEST_Z(s_ctx->pd = ibv_alloc_pd(s_ctx->ctx));
    TEST_Z(s_ctx->comp_channel = ibv_create_comp_channel(s_ctx->ctx));
    TEST_Z(s_ctx->cq = ibv_create_cq(s_ctx->ctx, 10, NULL, s_ctx->comp_channel, 0)); /* cqe=10 is arbitrary */
    TEST_NZ(ibv_req_notify_cq(s_ctx->cq, 0));
//    TEST_NZ(pthread_create(&s_ctx->cq_poller_thread, NULL, poll_cq, NULL));
  }
// }


  struct ibv_qp_init_attr qp_attr;
// build_qp_attr(&qp_attr);
// void build_qp_attr(struct ibv_qp_init_attr *qp_attr)
// {
  memset(qp_attr, 0, sizeof(*qp_attr));
  qp_attr.send_cq = s_ctx->cq;
  qp_attr.recv_cq = s_ctx->cq;
  qp_attr.qp_type = IBV_QPT_RC;
  qp_attr.cap.max_send_wr = 10;
  qp_attr.cap.max_recv_wr = 10;
  qp_attr.cap.max_send_sge = 1;
  qp_attr.cap.max_recv_sge = 1;
// }

  TEST_NZ(rdma_create_qp(event.id, s_ctx->pd, &qp_attr));

  struct connection *conn;
  s_ctx->conn =
    event.id->context =
    conn = (struct connection *)kzalloc(sizeof(struct connection));
  conn->id = event.id;
  conn->qp = event.id->qp;
  conn->num_completions = 0;

// register_memory(conn);
// void register_memory(struct connection *conn)
// {
  conn->send_region = nvdimm_alloc(BUFFER_SIZE, GFP_KERNEL);
  /* conn->recv_region = nvdimm_alloc(BUFFER_SIZE, GFP_KERNEL); */

  TEST_Z(conn->send_mr = ibv_reg_mr(
    s_ctx->pd, 
    conn->send_region, 
    BUFFER_SIZE, 
    IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));

  /* TEST_Z(conn->recv_mr = ibv_reg_mr(
    s_ctx->pd, 
    conn->recv_region, 
    BUFFER_SIZE, 
    IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE)); */
// }


/*
// post_receives(conn);
// void post_receives(struct connection *conn)
// {
  struct ibv_recv_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  wr.wr_id = (uintptr_t)&conn;
  wr.next = NULL;
  wr.sg_list = &sge;
  wr.num_sge = 1;

  sge.addr = (uintptr_t)conn.recv_region;
  sge.length = BUFFER_SIZE;
  sge.lkey = conn.recv_mr->lkey;

  TEST_NZ(ibv_post_recv(conn.qp, &wr, &bad_wr));
// }
*/


  TEST_NZ(rdma_resolve_route(event.id, TIMEOUT_IN_MS));


// return 0;
// }

//-------------------------------------------------------------

// TODO: remove cycle!
  while (rdma_get_cm_event(ec, &_event) == 0) {

    memcpy(&event, _event, sizeof(*_event));
    rdma_ack_cm_event(_event);

    if (event.event == RDMA_CM_EVENT_ROUTE_RESOLVED) {
      printf("RDMA_CM_EVENT_ROUTE_RESOLVED\n");
      break;
    }
  }

// on_route_resolved(event->id);
// int on_route_resolved(struct rdma_cm_id *id)
// {
  struct rdma_conn_param cm_params;
  memset(&cm_params, 0, sizeof(cm_params));
  TEST_NZ(rdma_connect(event.id, &cm_params));

// return 0;
// }

//-------------------------------------------------------------

// TODO: remove cycle!
  while (rdma_get_cm_event(ec, &_event) == 0) {

    memcpy(&event, _event, sizeof(*_event));
    rdma_ack_cm_event(_event);

    if (event.event == RDMA_CM_EVENT_ESTABLISHED) {
      printf("RDMA_CM_EVENT_ESTABLISHED\n");
      break;
    }
  }

// on_connection(event->id->context);
// int on_connection(void *context)
// {
  /* struct connection * */conn = (struct connection *)event.id->context;
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  snprintf(conn->send_region, BUFFER_SIZE, "message from active/client side with pid %d", getpid());

  printf("connected. posting send...\n");

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)conn;
  wr.opcode = IBV_WR_SEND;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;

  sge.addr = (uintptr_t)conn->send_region;
  sge.length = BUFFER_SIZE;
  sge.lkey = conn->send_mr->lkey;

  TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));

// return 0;
// }


















}
