class stream
{
public:
  virtual close() = 0;
  virtual flush() = 0;
};


class istream: virtual public stream
{
public:
  virtual void read(void * p, unsigned n) = 0;
};


class ostream: public stream
{
public:
  virtual void write(void * p, unsigned n) = 0;
};


class iostream: public istream, public ostream


class iformat
{
public:
  virtual void read(signed char & o) = 0;
  virtual void read(unsigned char & o) = 0;
  virtual void read(char & o) = 0;
  virtual void read(signed short & o) = 0;
  virtual void read(unsigned short & o) = 0;
  virtual void read(signed & o) = 0;
  virtual void read(unsigned & o) = 0;
  virtual void read(signed long & o) = 0;
  virtual void read(unsigned long & o) = 0;
  virtual void read(object & o) = 0;
};


class oformat
{
public:
  virtual void write(signed char o) = 0;
  virtual void write(unsigned char o) = 0;
  virtual void write(char o) = 0;
  virtual void write(signed short o) = 0;
  virtual void write(unsigned short o) = 0;
  virtual void write(signed o) = 0;
  virtual void write(unsigned o) = 0;
  virtual void write(signed long o) = 0;
  virtual void write(unsigned long o) = 0;
  virtual void write(object const & o) = 0;
};
