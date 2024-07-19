#include <assert.h>
#include <fstream.h>
#include <iostream.h>
#include <stddef.h>
#include <string.h>


struct dir_entry
{
  char filename[56];
  unsigned long location;
  unsigned long length;
};


void pack(ifstream & descf, ofstream & packagef)
{
  char buf[1024];
  unsigned char data_buf[1024];
  size_t len;
  ifstream f;
  dir_entry * directory;
  unsigned long dir_len = 0;
  unsigned long data_end = -1;
  unsigned long curr_dir = 0;

  clog << "  beginning pack" << endl;
  while(descf.good() && !descf.eof())
  {
    descf.getline(buf, 1024);
    len = strlen(buf);
    assert(len < 1024);
    if((len > 0) && (buf[0] != '#'))
    {
      if(len >= 56)
      {
        clog << "can't pack file \"" << buf << "\": filename is too long :p" << endl;
      }
      else
      {
        f.open(buf, ios::in | ios::binary);
        if(!f.good())
        {
          clog << "can't pack file \"" << buf << "\": couldn't open for read" << endl;
        }
        else
        {
          ++dir_len;
        }
        f.close();
      }
    }
  }
  clog << "  packing " << dir_len << " files" << endl;
  directory = new dir_entry[dir_len];
  assert(directory != NULL);
  clog << hex;
  clog << "writing directory pointer " << data_end << "..." << endl;
  descf.clear();
  descf.seekg(0);
  packagef.write((unsigned char *)&data_end, sizeof(data_end));
  while(descf.good() && !descf.eof())
  {
    descf.getline(buf, 1024);
    len = strlen(buf);
    assert(len < 1024);
    if((len > 0) && (len < 56) && (buf[0] != '#'))
    {
      assert(curr_dir < dir_len);
      if(buf[len - 1] == '\n')
      {
        buf[len - 1] = '\000';
        --len;
      }
      strncpy(directory[curr_dir].filename, buf, 56);
      clog << "  packing file \"" << buf << "\" as \"" << directory[curr_dir].filename << "\" " << flush;
      directory[curr_dir].location = packagef.tellp();
      clog << "at " << directory[curr_dir].location << endl;

      f.open(buf, ios::in | ios::binary);
      if(f.good())
      {
        clog << "open, reading" << flush;
        while(f.good() && !f.eof() && packagef.good())
        {
          f.read(data_buf, 1024);
          packagef.write(data_buf, f.gcount());
          clog << '.' << flush;
        }
        f.close();
        clog << ", closed, " << flush;
      }
      else
      {
        clog << "input error, " << flush;
      }

      directory[curr_dir].length = packagef.tellp() - directory[curr_dir].location;
      clog << dec << directory[curr_dir].length << hex << " bytes copied" << endl;
      ++curr_dir;
    }
  }
  data_end = packagef.tellp();
  clog << "writing directory" << endl;
  packagef.write((unsigned char *)directory, dir_len * sizeof(dir_entry));
  clog << "  package file is " << dec << packagef.tellp() << hex << " bytes long" << endl;
  clog << "writing directory pointer " << data_end << endl;
  packagef.seekp(0);
  packagef.write((unsigned char *)&data_end, sizeof(data_end));
  clog << dec;
  clog << "  done pack" << endl;
}


void print_usage()
{
  clog << "usage:" << endl;
  clog << "  pack <description file> <package file>" << endl;
  clog << endl;
  clog << "description file: the list of files to include in the package" <<endl;
  clog << "package file: the generated package file" << endl;
}


int main(int argc, char * argv[])
{
  ifstream descf;
  ofstream packagef;

  clog << "  pack, version the 0th" << endl;
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

  clog << "opening description file \"" << argv[1] << "\"" << endl;
  descf.open(argv[1], ios::in | ios::text);
  if(!descf.good())
  {
    clog << "couldn't open description file" << endl;
    print_usage();
    return 1;
  }

  clog << "opening package file \"" << argv[2] << "\"" << endl;
  packagef.open(argv[2], ios::out | ios::binary);
  if(!packagef.good())
  {
    clog << "couldn't open package file" << endl;
    print_usage();
    return 1;
  }

  pack(descf, packagef);

  clog << "closing package file" << endl;
  packagef.close();
  clog << "closing description file" << endl;
  descf.close();
  return 0;
}


