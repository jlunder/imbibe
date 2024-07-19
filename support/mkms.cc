#include "cplusplus.hh"

#include <assert.h>
#include <fstream.h>
#include <iostream.h>
#include <stdio.h>

#include "bitmap.hh"
#include "bitmap_graphics.hh"
#include "color.hh"
#include "pixel.hh"
#include "string.hh"

#include "bitmap.ii"
#include "bitmap_graphics.ii"
#include "color.ii"
#include "pixel.ii"
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
  ofstream f;
  int count = 0;
  int t;
  int w;
  int h;
  string s;
  color cn, cs;
  bitmap * b;
  char fn[16];

  eat_white(srcf);

  srcf >> w;
  skip_comma(srcf);
  srcf >> h;
  eat_white(srcf);
  assert(srcf.good());

  menuf << w << ", " << h << endl;

  srcf >> t;
  skip_comma(srcf);
  cn.foreground(t);
  srcf >> t;
  skip_comma(srcf);
  cn.background(t);
  srcf >> t;
  skip_comma(srcf);
  cs.foreground(t);
  srcf >> t;
  eat_white(srcf);
  cs.background(t);

  b = new bitmap(w, 1);

  while(srcf.good())
  {
    s = read_word(srcf);
    skip_comma(srcf);
    menuf << "\"" << s << "\", ";
    s = read_word(srcf);
    skip_comma(srcf);
    menuf << "\"" << s << "\", ";
    menuf << 0 << ", " << count << ", " << w << ", " << 1 << ", ";
    s = read_word(srcf);
    eat_white(srcf);

    sprintf(fn, "%04xn.bin", count);
    menuf << "\"" << fn << "\", ";
    f.open(fn, ios::out | ios::binary);
    b->g().draw_rectangle(0, 0, w, 1, pixel(' ', cn));
    b->g().draw_text(0, 0, cn, s);
    f.write((unsigned char *)b->data(), w * sizeof(unsigned short));
    f.close();

    sprintf(fn, "%04xs.bin", count);
    menuf << "\"" << fn << "\"" << endl;
    f.open(fn, ios::out | ios::binary);
    b->g().draw_rectangle(0, 0, w, 1, pixel(' ', cs));
    b->g().draw_text(0, 0, cs, s);
    f.write((unsigned char *)b->data(), w * sizeof(unsigned short));
    f.close();

    ++count;
  }

  delete b;
}


void print_usage()
{
  clog << "usage:" << endl;
  clog << "  mkms <sourcefile> <menufile>" << endl;
  clog << endl;
  clog << "sourcefile: menu source source (.mss) file to parse" << endl;
  clog << "menufile: menu source (.ms) file to write" << endl;
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
  menuf.open(argv[2], ios::out | ios::text);
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


