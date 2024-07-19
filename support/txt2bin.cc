#include "cplusplus.hh"

#include <iostream.h>
#include <fstream.h>

#include "vector.hh"

#include "vector.ii"


typedef vector < unsigned short > line;
typedef vector < line > line_list;


void process(istream & textf, ostream & binf)
{
  int c;
  line_list l;
  line_list::iterator i;
  unsigned long count = 0;
  unsigned short attr = 0x0700;
  unsigned short const head = 0x0B00;
  unsigned short const body = 0x0700;
  unsigned short const bold = 0x0F00;
  unsigned short const comment = 0x0800;
  unsigned short * temp;

  clog << "parsing text file" << endl;
  l.push_back(line());
  while(textf.good() && !textf.eof())
  {
    if((c = textf.get()) == EOF) break;

    switch(c)
    {
    case '\n':
      l.push_back(line());
      break;
    case '\t':
      l.back().insert(l.back().end(), 8 - l.back().size() % 8, attr | ' ');
      break;
    case '{':
      attr = comment;
      l.back().push_back(attr | c);
      break;
    case '}':
      l.back().push_back(attr | c);
      attr = body;
      break;
    default:
      l.back().push_back(attr | c);
      break;
    }
  }
  clog << "processing" << endl;
  temp = new unsigned short[l.size() * 80];
  for(count = 0; count < l.size() * 80; ++count)
  {
    temp[count] = 0x0700 | ' ';
  }
  count = 0;
  for(i = l.begin(); i != l.end(); ++i)
  {
    memcpy(temp + count, i->begin(), (i->size() > 80 ? 80 : i->size()) * sizeof(unsigned short));
    count += 80;
  }
  clog << "writing bin file" << endl;
  binf.write((unsigned char *)temp, l.size() * 80 * sizeof(unsigned short));
  delete[] temp;
}


void print_usage()
{
  clog << "usage:" << endl;
  clog << "  txt2bin <textfile> <binfile>" << endl;
  clog << endl;
  clog << "textfile: ordinary plaintext file to parse" << endl;
  clog << "binfile: bin-format formatted coloured file to write" << endl;
}


int main(int argc, char * argv[])
{
  ifstream textf;
  ofstream binf;

  clog << "  txt2bin, version the 0th" << endl;
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

  clog << "opening bin file \"" << argv[2] << "\"" << endl;
  binf.open(argv[2], ios::out | ios::binary);
  if(!binf.good())
  {
    clog << "couldn't open bin file" << endl;
    print_usage();
    return 1;
  }

  process(textf, binf);

  clog << "closing bin file" << endl;
  binf.close();
  clog << "closing text file" << endl;
  textf.close();
  return 0;
}


