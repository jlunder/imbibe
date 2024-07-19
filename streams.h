class istream {
  virtual bool skip(streamsize n) throw() = 0;
  virtual bool read(void * buf, size_t n) = 0;
  virtual char read_char() = 0;
  virtual string read_line() = 0;
  virtual bool read_line(char * buf, size_t n) = 0;
  virtual bool at_end() = 0;
  virtual bool empty() = 0;
  virtual stream_size avail_read() = 0;
  virtual stream_size read_count() = 0;
};

class ostream {
  virtual bool write(void * buf, size_t n) = 0;
  virtual bool write_line(char * buf) = 0;
  virtual bool write_char(char * c) = 0;
  virtual bool full() = 0;
  virtual stream_size avail_write() = 0;
  virtual stream_size write_count() = 0;
};

class sstream {
  virtual streamsize seek_to(streamsize pos) = 0;
  virtual streamsize seek_by(streamofs ofs) = 0;
  virtual streamsize position() = 0;
  virtual streamsize size() = 0;
};

class iostream: virtual public istream, virtual public ostream {};
class isstream: virtual public istream, virtual public sstream {};
class osstream: virtual public ostream, virtual public sstream {};
class iosstream: virtual public isstream, virtual public osstream,
  virtual public iostream {};

