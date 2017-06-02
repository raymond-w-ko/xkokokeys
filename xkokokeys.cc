#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <pthread.h>
#include <unistd.h>

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/record.h>
#include <X11/keysym.h>

#include <string>
#include <unordered_map>

#include "xkokokeys.h"

int main() {
  xkokokeys keys;
  keys.wait();
  return EXIT_SUCCESS;
}

xkokokeys::xkokokeys()
    : data_conn_(NULL),
      ctrl_conn_(NULL),
      record_context_(0),
      record_range_(NULL) {
  init_x();
  init_signals();
  init_record_context();
}

xkokokeys::~xkokokeys() {
  if (!XRecordFreeContext(ctrl_conn_, record_context_)) {
    fprintf(stderr, "Failed to free xrecord context\n");
  }
  XFree(record_range_);
  XCloseDisplay(ctrl_conn_);
  XCloseDisplay(data_conn_);
}

void xkokokeys::init_x() {
  if (!XInitThreads()) {
    fprintf(stderr, "Failed to initialize threads.\n");
    exit(EXIT_FAILURE);
  }

  data_conn_ = XOpenDisplay(NULL);
  ctrl_conn_ = XOpenDisplay(NULL);
  if (!data_conn_ || !ctrl_conn_) {
    fprintf(stderr, "Unable to connect to X11 display. Is $DISPLAY set?\n");
    exit(EXIT_FAILURE);
  }

  int dummy;

  if (!XQueryExtension(ctrl_conn_, "XTEST", &dummy, &dummy, &dummy)) {
    fprintf(stderr, "Xtst extension missing\n");
    exit(EXIT_FAILURE);
  }
  if (!XRecordQueryVersion(ctrl_conn_, &dummy, &dummy)) {
    fprintf(stderr, "Failed to obtain xrecord version\n");
    exit(EXIT_FAILURE);
  }
  if (!XkbQueryExtension(ctrl_conn_, &dummy, &dummy, &dummy, &dummy, &dummy)) {
    fprintf(stderr, "Failed to obtain xkb version\n");
    exit(EXIT_FAILURE);
  }
}

void xkokokeys::init_signals() {
  sigemptyset(&sigset_);
  sigaddset(&sigset_, SIGINT);
  sigaddset(&sigset_, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &sigset_, NULL);

  pthread_create(&thread_, NULL, &xkokokeys::sig_handler, this);
}

void* xkokokeys::sig_handler(void* user_data) {
  return reinterpret_cast<xkokokeys*>(user_data)->sig_handler();
}
void* xkokokeys::sig_handler() {
  int sig;
  sigwait(&sigset_, &sig);

  XLockDisplay(ctrl_conn_);

  if (!XRecordDisableContext(ctrl_conn_, record_context_)) {
    fprintf(stderr, "Failed to disable xrecord context\n");
    exit(EXIT_FAILURE);
  }

  XSync(ctrl_conn_, False);
  XUnlockDisplay(ctrl_conn_);

  return NULL;
}

void xkokokeys::init_record_context() {
  XRecordClientSpec client_spec = XRecordAllClients;

  XRecordRange* record_range_ = XRecordAllocRange();
  record_range_->device_events.first = KeyPress;
  record_range_->device_events.last = ButtonRelease;

  record_context_ =
      XRecordCreateContext(ctrl_conn_, 0, &client_spec, 1, &record_range_, 1);
  if (record_context_ == 0) {
    fprintf(stderr, "Failed to create xrecord context\n");
    exit(EXIT_FAILURE);
  }
  XSync(ctrl_conn_, False);
  if (!XRecordEnableContext(data_conn_, record_context_, &xkokokeys::intercept,
                            (XPointer)this)) {
    fprintf(stderr, "Failed to enable xrecord context\n");
    exit(EXIT_FAILURE);
  }
}

void xkokokeys::wait() { pthread_join(thread_, NULL); }

void xkokokeys::intercept(XPointer user_data, XRecordInterceptData* data) {
  return reinterpret_cast<xkokokeys*>(user_data)->intercept(data);
}

void xkokokeys::intercept(XRecordInterceptData* data) {
  XLockDisplay(ctrl_conn_);
  if (data->category == XRecordFromServer) {
    int key_event = data->data[0];
    KeyCode key_code = data->data[1];

    fprintf(stdout, "Intercepted key event %d, key code %d\n", key_event,
            key_code);
    goto exit;
  }
exit:
  XUnlockDisplay(ctrl_conn_);
  XRecordFreeData(data);
}
