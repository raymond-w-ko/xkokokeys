class xkokokeys {
 public:
  xkokokeys();
  ~xkokokeys();

  void wait();

 private:
  void init_x();
  void init_signals();
  void init_record_context();

  static void* sig_handler(void* user_data);
  void* sig_handler();

  static void intercept(XPointer user_data, XRecordInterceptData* data);
  void intercept(XRecordInterceptData* data);
  void _intercept(XRecordInterceptData* data);

  Display* data_conn_;
  Display* ctrl_conn_;
  XRecordContext record_context_;
  XRecordRange* record_range_;

  sigset_t sigset_;
  pthread_t thread_;
};
