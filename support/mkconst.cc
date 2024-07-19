#include <iomanip.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>


void print_usage()
{
  clog << "usage:" << endl;
  clog << "  mkconst <binary file> <header file> <c file>" << endl;
  clog << endl;
  clog << "binary file: the file you want to convert into a c constant" << endl;
  clog << "header file: the file to put the constant declaration in" << endl;
  clog << "c file: the file to put the constant definition in" << endl;
}


int main(int argc, char * argv[])
{
  ifstream binaryf;
  ofstream f;
  char ident[64];
  char fname_upper[64];
  char * s;
  int c;
  int col;
  unsigned long count;
  unsigned long len;

  if(argc < 4)
  {
    clog << "too few arguments" << endl;
    print_usage();
    return 1;
  }
  if(argc > 4)
  {
    clog << "too many arguments" << endl;
    print_usage();
    return 1;
  }

  binaryf.open(argv[1], ios::in | ios::binary);
  strncpy(ident, argv[1], 64);
  ident[63] = '\000';
  for(s = ident; *s; ++s)
  {
    if(!(((*s >= 'a') && (*s <= 'z')) || ((*s >= 'A') && (*s <= 'Z')) || (*s == '_')))
    {
      *s = '_';
    }
  }
  strncpy(fname_upper, argv[2], 64);
  fname_upper[63] = '\000';
  for(s = fname_upper; *s; ++s)
  {
    if(!(((*s >= 'a') && (*s <= 'z')) || ((*s >= 'A') && (*s <= 'Z')) || (*s == '_')))
    {
      *s = '_';
    }
    if((*s >= 'a') && (*s <= 'z'))
    {
      *s += 'A' - 'a';
    }
  }

  binaryf.seekg(0, ios::end);
  len = binaryf.tellg();
  binaryf.seekg(0);

  f.open(argv[2]);
  f << "#ifndef __" << fname_upper << "_INCLUDED" << endl;
  f << "#define __" << fname_upper << "_INCLUDED" << endl;
  f << endl;
  f << endl;
  f << "unsigned long const " << ident << "_length = " << len << ";" << endl;
  f << "extern unsigned char " << ident << "_data[" << ident << "_length];" << endl;
  f << endl;
  f << endl;
  f << "#endif //__" << fname_upper << "_INCLUDED" << endl;
  f << endl;
  f << endl;
  f.close();

  f.open(argv[3]);
  f << "#include \"" << argv[2] << "\"" << endl;
  f << endl;
  f << endl;
  f << "unsigned char " << ident << "_data[" << ident << "_length] =" << endl;
  f << "{" << endl << "  ";
  count = 0;
  col = 0;
  f.unsetf(ios::basefield);
  f.setf(ios::hex | ios::showbase);
  while(1)
  {
    ++count;
    ++col;
    c = binaryf.get();
    if(c == EOF)
    {
      break;
    }
    if(count == len)
    {
      f << setw(4) << c << endl;
    }
    else
    {
      if(col == 12)
      {
        f << setw(4) << c << ',' << endl << "  ";
        col = 0;
      }
      else
      {
        f << setw(4) << c << ", ";
      }
    }
  }
  f << "};" << endl;
  f << endl;
  f << endl;
  f.close();

  return 0;
}


