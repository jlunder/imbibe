#include "cplusplus.h"

#include <assert.h>
#include <iostream.h>
#include <fstream.h>

#include "string.h"
#include "vector.h"


int hex_value(char c)
{
  if((c >= '0') && (c <= '9')) return c - '0';
  else if((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
  else if((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
  else
  {
    clog << "expected hex digit, found \'" << c << "\'" << endl;
  }
  return 0;
}


int read_hex_pair(istream & textf)
{
  int result;

  if(textf.peek() == EOF) return 0;
  result = hex_value(textf.get()) * 16;
  if(textf.peek() == EOF) return 0;
  result += hex_value(textf.get());
  return result;
}


int read_hex_single(istream & textf)
{
  if(textf.peek() == EOF) return 0;
  return hex_value(textf.get());
}


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


typedef vector<uint16_t> line;
typedef vector<line> line_list;


struct link
{
  int l, c;
  string action;
  string target;
  line data;
};


typedef vector<link> link_list;


uint16_t const head = 0x0B00;
uint16_t const body = 0x0900;
uint16_t const bold = 0x0F00;
uint16_t const parens = 0x0500;
uint16_t const brackets = 0x0600;
uint16_t const braces = 0x0800;
uint16_t const link_selected = 0x0B00;
uint16_t const link_normal = 0x0300;


void read_link_data(istream & textf, line & l, uint16_t const link_color)
{
  int c;
  vector<uint16_t> attr;
  uint16_t cur_attr;

  attr.push_back(link_color);
  cur_attr = attr.back();

  while(textf.good())
  {
    c = textf.peek();
    if(c == EOF) break;

    switch(c)
    {
    case '\n':
      clog << "newline in the middle of link text!";
      return;
      break;
    case '\t':
      l.insert(l.end(), 8 - l.size() % 8, cur_attr | ' ');
      break;
//    case '(':
      attr.push_back(parens);
      cur_attr = attr.back();
      l.push_back(cur_attr | c);
      break;
//    case '[':
      attr.push_back(brackets);
      cur_attr = attr.back();
      l.push_back(cur_attr | c);
      break;
    case '{':
      attr.push_back(braces);
      cur_attr = attr.back();
      l.push_back(cur_attr | c);
      break;
//    case ')':
    case '}':
      l.push_back(attr.back() | c);
      attr.pop_back();
      if(attr.empty())
      {
        clog << "popped too many colors off the color stack!" << endl;
        attr.push_back(link_color);
      }
      cur_attr = attr.back();
      break;
    case ']':
//      if(attr.size() <= 1) return;
//      l.push_back(attr.back() | c);
//      attr.pop_back();
//      cur_attr = attr.back();
//      break;
      return;
    case '\\':
      if((c = textf.get()) == EOF) break;

      switch(c)
      {
      case 'a':
        cur_attr = read_hex_pair(textf);
        break;
      case 'b':
        cur_attr = ((read_hex_single(textf) << 4) | (cur_attr & 0x0F)) << 8;
        break;
      case 'f':
        cur_attr = ((read_hex_single(textf) << 0) | (cur_attr & 0xF0)) << 8;
        break;
      case 'l':
        cur_attr = attr.back();
        break;
      case 'r':
        attr.pop_back();
        if(attr.empty())
        {
          clog << "popped too many colors off the color stack!" << endl;
          attr.push_back(link_color);
        }
        cur_attr = attr.back();
        break;
      case 's':
        attr.push_back(cur_attr);
        break;
      case 'x':
        l.push_back(cur_attr | read_hex_pair(textf));
        break;
      case 'B':
        attr.push_back(bold);
        cur_attr = attr.back();
        break;
      case 'L':
        clog << "you can't have a link within a link, buster!" << endl;
        break;
      default:
        if(c == '\n')
        {
          textf.get();
        }
        else
        {
          if(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')))
          {
            clog << "illegal escape code: \\" << (char)c << endl;
          }
          else
          {
            l.push_back(cur_attr | c);
          }
        }
        break;
      }
      break;
    default:
      l.push_back(cur_attr | c);
      break;
    }
    textf.get();
  }
}


void process(istream & textf, ostream & hbinf)
{
  int c;
  line_list l;
  line_list::iterator i;
  link_list links;
  link_list::iterator link_i;
  uint32_t count;
  vector<uint16_t> attr;
  uint16_t cur_attr;
  uint16_t * temp;
  uint8_t buf[256];

  attr.push_back(body);
  cur_attr = attr.back();

  clog << "parsing text file" << endl;
  l.push_back(line());
  while(textf.good())
  {
    if((c = textf.get()) == EOF) break;

    switch(c)
    {
    case '\n':
      l.push_back(line());
      break;
    case '\t':
      l.back().insert(l.back().end(), 8 - l.back().size() % 8, cur_attr | ' ');
      break;
//    case '(':
      attr.push_back(parens);
      cur_attr = attr.back();
      l.back().push_back(cur_attr | c);
      break;
//    case '[':
      attr.push_back(brackets);
      cur_attr = attr.back();
      l.back().push_back(cur_attr | c);
      break;
    case '{':
      attr.push_back(braces);
      cur_attr = attr.back();
      l.back().push_back(cur_attr | c);
      break;
//    case ')':
//    case ']':
    case '}':
      l.back().push_back(attr.back() | c);
      attr.pop_back();
      if(attr.empty())
      {
        clog << "popped too many colors off the color stack!" << endl;
        attr.push_back(body);
      }
      cur_attr = attr.back();
      break;
    case '\\':
      if((c = textf.get()) == EOF) break;

      switch(c)
      {
      case 'a':
        cur_attr = read_hex_pair(textf);
        break;
      case 'b':
        cur_attr = ((read_hex_single(textf) << 4) | (cur_attr & 0x0F)) << 8;
        break;
      case 'f':
        cur_attr = ((read_hex_single(textf) << 0) | (cur_attr & 0xF0)) << 8;
        break;
      case 'l':
        cur_attr = attr.back();
        break;
      case 'r':
        attr.pop_back();
        if(attr.empty())
        {
          clog << "popped too many colors off the color stack!" << endl;
          attr.push_back(body);
        }
        cur_attr = attr.back();
        break;
      case 's':
        attr.push_back(cur_attr);
        break;
      case 'x':
        l.push_back(cur_attr | read_hex_pair(textf));
        break;
      case 'B':
        attr.push_back(bold);
        cur_attr = attr.back();
        break;
      case 'L':
        assert(textf.get() == '[');
        links.push_back(link());
        links.back().l = l.size() - 1;
        links.back().c = l.back().size();
        eat_white(textf);
        links.back().action = read_word(textf);
        skip_comma(textf);
        links.back().target = read_word(textf);
        eat_white(textf);
        assert(textf.get() == ']');
        assert(textf.get() == '[');
        read_link_data(textf, l.back(), link_normal);
        assert(textf.get() == ']');
        assert(textf.get() == '[');
        read_link_data(textf, links.back().data, link_selected);
        assert(textf.get() == ']');
        break;
      default:
        if(c == '\n')
        {
          textf.get();
        }
        else
        {
          if(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')))
          {
            clog << "illegal escape code: \\" << (char)c << endl;
          }
          else
          {
            l.back().push_back(cur_attr | c);
          }
        }
        break;
      }
      break;
    default:
      l.back().push_back(cur_attr | c);
      break;
    }
  }

  clog << "writing hbin file" << endl;

  buf[0] = (80 >> 0) & 0xFF;
  buf[1] = (80 >> 8) & 0xFF;
  hbinf.write(buf, 2);

  buf[0] = (l.size() >> 0) & 0xFF;
  buf[1] = (l.size() >> 8) & 0xFF;
  hbinf.write(buf, 2);

  temp = new uint16_t[l.size() * 80];
  for(count = 0; count < l.size() * 80; ++count)
  {
    temp[count] = 0x0700 | ' ';
  }
  count = 0;
  for(i = l.begin(); i != l.end(); ++i)
  {
    memcpy(temp + count, i->begin(), (i->size() > 80 ? 80 : i->size()) * sizeof(uint16_t));
    count += 80;
  }
  hbinf.write((uint8_t *)temp, l.size() * 80 * sizeof(uint16_t));
  delete[] temp;

  assert(links.size() < (1 << 16));
  buf[0] = (links.size() >> 0) & 0xFF;
  buf[1] = (links.size() >> 8) & 0xFF;
  hbinf.write(buf, 2);
  for(link_i = links.begin(); link_i != links.end(); ++link_i)
  {
    assert(link_i->c < (1 << 16));
    buf[0] = (link_i->c >> 0) & 0xFF;
    buf[1] = (link_i->c >> 8) & 0xFF;
    hbinf.write(buf, 2);

    assert(link_i->l < (1 << 16));
    buf[0] = (link_i->l >> 0) & 0xFF;
    buf[1] = (link_i->l >> 8) & 0xFF;
    hbinf.write(buf, 2);

    assert(link_i->action.length() < 256);
    buf[0] = link_i->action.length();
    memcpy(buf + 1, link_i->action.begin(), link_i->action.length());
    hbinf.write(buf, link_i->action.length() + 1);

    assert(link_i->target.length() < 256);
    buf[0] = link_i->target.length();
    memcpy(buf + 1, link_i->target.begin(), link_i->target.length());
    hbinf.write(buf, link_i->target.length() + 1);

    assert(link_i->data.size() < (1 << 16));
    buf[0] = (link_i->data.size() >> 0) & 0xFF;
    buf[1] = (link_i->data.size() >> 8) & 0xFF;
    hbinf.write(buf, 2);
    buf[0] = (1 >> 0) & 0xFF;
    buf[1] = (1 >> 8) & 0xFF;
    hbinf.write(buf, 2);

    hbinf.write((uint8_t *)link_i->data.begin(), link_i->data.size() * sizeof(uint16_t));
  }
}


void print_usage()
{
  clog << "usage:" << endl;
  clog << "  mkhbin <sourcefile> <hbinfile>" << endl;
  clog << endl;
  clog << "sourcefile: name of source file with to parse" << endl;
  clog << "hbinfile: name of hyperlinked binary file to write" << endl;
}


int main(int argc, char * argv[])
{
  ifstream textf;
  ofstream hbinf;

  clog << "  mkhbin, version the 1st" << endl;
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

  clog << "opening text file \"" << argv[1] << "\"" << endl;
  textf.open(argv[1], ios::in | ios::text);
  if(!textf.good())
  {
    clog << "couldn't open text file" << endl;
    print_usage();
    return 1;
  }

  clog << "opening hbin file \"" << argv[2] << "\"" << endl;
  hbinf.open(argv[2], ios::out | ios::binary);
  if(!hbinf.good())
  {
    clog << "couldn't open hbin file" << endl;
    print_usage();
    return 1;
  }

  process(textf, hbinf);

  clog << "closing hbin file" << endl;
  hbinf.close();
  clog << "closing text file" << endl;
  textf.close();
  return 0;
}


