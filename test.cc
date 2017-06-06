#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

class VirtualKeyboard {
 public:
  VirtualKeyboard() {
    ssize_t ret;

    fd_ = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd_ < 0) {
      exit(EXIT_FAILURE);
    }

    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Virtual uinput Keyboard");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x01;
    uidev.id.product = 0x01;
    uidev.id.version = 1;

    ret = ioctl(fd_, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < 255; ++i) {
      ret = ioctl(fd_, UI_SET_KEYBIT, i);
    }
    ret = ioctl(fd_, UI_SET_EVBIT, EV_SYN);
    ret = write(fd_, &uidev, sizeof(uidev));
    ret = ioctl(fd_, UI_DEV_CREATE);

    sleep(1);
    if (ret) {
      ret = 0;
    }
  }
  ~VirtualKeyboard() {
    ssize_t ret;
    ret = ioctl(fd_, UI_DEV_DESTROY);
    sleep(1);
    if (ret) {
      ret = 0;
    }
  }

  void TypeKey() {
    ssize_t ret;

    struct input_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.type = EV_KEY;
    ev.code = KEY_Q;
    ev.value = 1;
    ret = write(fd_, &ev, sizeof(ev));

    ev.value = 0;
    ret = write(fd_, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    ret = write(fd_, &ev, sizeof(ev));

    if (ret) {
      ret = 0;
    }
  }

 private:
  int fd_;
};

int main() {
  {
    VirtualKeyboard keyboard;
    keyboard.TypeKey();
  }
  return 0;
}
