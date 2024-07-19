#include "cplusplus.hh"

#include <assert.h>
#include <fstream.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include "string.hh"

#include "string.ii"


string read_word(istream & i)
{
  string s;
  int c;

  switch(i.peek())
  {
  case '\"':
    i.get();
    while(i.good())
    {
      c = i.peek();
      if(c == '\\')
      {
        i.get();
        c = i.peek();
      }
      else if(c == '\"')
      {
        i.get();
        break;
      }
      if(c == EOF) break;
      i.get();
      s.append((char)c);
    }
    return s;
    break;
  case EOF:
    break;
  default:
    while(i.good())
    {
      c = i.peek();
      if(c == '\\')
      {
        i.get();
        c = i.peek();
      }
      else if((c <= ',') || (c <= ' ')) break;
      if(c == EOF) break;
      i.get();
      s.append((char)c);
    }
    return s;
    break;
  }
  return string();
}


void eat_white(istream & i)
{
  while(i.good() && i.peek() <= ' ') i.get();
}


void skip_comma(istream & i)
{
  int c;

  eat_white(i);
  c = i.get();
  assert(c == ',');
  eat_white(i);
}


void process(istream & srcf, ostream & menuf)
{
  ifstream f;
  unsigned char buf[256];
  size_t picture_size = 1024;
  unsigned char * picture_buf = new unsigned char[picture_size];
  streampos p;
  int count = 0;
  int x;
  int y;
  int w;
  int h;
  string s;

  srcf >> w;
  skip_comma(srcf);
  srcf >> h;
  eat_white(srcf);
  buf[0] = (w >> 0) & 0xFF;
  buf[1] = (w >> 8) & 0xFF;
  menuf.write(buf, 2);
  buf[0] = (h >> 0) & 0xFF;
  buf[1] = (h >> 8) & 0xFF;
  menuf.write(buf, 2);

  p = menuf.tellp();
  menuf.write(buf, 2);

  while(srcf.good())
  {
    s = read_word(srcf);
    skip_comma(srcf);
    assert(s.length() < (1 << 8));
    buf[0] = s.length();
    memcpy(buf + 1, s.begin(), s.length());
    menuf.write(buf, s.length() + 1);

    s = read_word(srcf);
    skip_comma(srcf);
    assert(s.length() < (1 << 8));
    buf[0] = s.length();
    memcpy(buf + 1, s.begin(), s.length());
    menuf.write(buf, s.length() + 1);

    srcf >> x;
    skip_comma(srcf);
    assert(x >= -(1 << 16));
    assert(x < (1 << 16));
    if(x < 0)
    {
      buf[1] = 0x80;
      x += (1 << 15);
    }
    else
    {
      buf[1] = 0;
    }
    buf[0] = (x >> 0) & 0xFF;
    buf[1] |= (x >> 8) & 0x7F;
    menuf.write(buf, 2);

    srcf >> y;
    skip_comma(srcf);
    assert(y >= -(1 << 16));
    assert(y < (1 << 16));
    if(y < 0)
    {
      buf[1] = 0x80;
      y += (1 << 15);
    }
    else
    {
      buf[1] = 0;
    }
    buf[0] = (y >> 0) & 0xFF;
    buf[1] |= (y >> 8) & 0x7F;
    menuf.write(buf, 2);

    srcf >> w;
    skip_comma(srcf);
    assert(h > 0);
    assert(w < (1 << 16));
    buf[0] = (w >> 0) & 0xFF;
    buf[1] = (w >> 8) & 0xFF;
    menuf.write(buf, 2);

    srcf >> h;
    skip_comma(srcf);
    assert(h > 0);
    assert(h < (1 << 16));
    buf[0] = (h >> 0) & 0xFF;
    buf[1] = (h >> 8) & 0xFF;
    menuf.write(buf, 2);

    if(w * h * sizeof(unsigned short) > picture_size)
    {
      delete[] picture_buf;
      picture_size = (w * h * sizeof(unsigned short) * 3) / 2 + 1;
      picture_buf = new unsigned char[picture_size];
    }

    s = read_word(srcf);
    skip_comma(srcf);
    f.open(s.c_str());
    if(!f.good())
    {
      clog << "unable to open file: \"" << s << "\"" << endl;
      abort();
    }
    f.read(picture_buf, w * h * sizeof(unsigned short));
    f.close();
    menuf.write(picture_buf, w * h * sizeof(unsigned short));

    s = read_word(srcf);
    eat_white(srcf);
    f.open(s.c_str());
    if(!f.good())
    {
      clog << "unable to open file: \"" << s << "\"" << endl;
      abort();
    }
    f.read(picture_buf, w * h * sizeof(unsigned short));
    f.close();
    menuf.write(picture_buf, w * h * sizeof(unsigned short));

    ++count;
  }
  assert(count < (1 << 16));
  buf[0] = (count >> 0) & 0xFF;
  buf[1] = (count >> 8) & 0xFF;
  menuf.seekp(p);
  menuf.write(buf, 2);

  delete[] picture_buf;
}


void print_usage()
{
  clog << "usage:" << endl;
  clog << "  mkmenu <sourcefile> <menufile>" << endl;
  clog << endl;
  clog << "sourcefile: menu source (.ms) file to parse" << endl;
  clog << "menufile: menu (.m) file to write" << endl;
}


int main(int argc, char * argv[])
{
  ifstream srcf;
  ofstream menuf;

  clog << "  mkmenu, version the 0th" << endl;
  if(argc < 3)
  {
    clog << "not enough arguments" << endl;
    print_usage();
    return 1;
  }
  if(argc > 3)
  {
    clog << "too many arguments" << endl;
    print_usage();
    return 1;
  }

  clog << "opening source file \"" << argv[1] << "\"" << endl;
  srcf.open(argv[1], ios::in | ios::text);
  if(!srcf.good())
  {
    clog << "couldn't open src file" << endl;
    print_usage();
    return 1;
  }

  clog << "opening menu file \"" << argv[2] << "\"" << endl;
  menuf.open(argv[2], ios::out | ios::binary);
  if(!menuf.good())
  {
    clog << "couldn't open menu file" << endl;
    print_usage();
    return 1;
  }

  process(srcf, menuf);

  clog << "closing menu file" << endl;
  menuf.close();
  clog << "closing src file" << endl;
  srcf.close();
  return 0;
}


