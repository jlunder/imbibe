typedef unsigned char bool;
bool const true = 1;
bool const false = 0;


#include <iostream.h>
#include "vector.hh"
#include "vector.ii"
#include "functional.hh"
#include "functional.ii"
#include "map.hh"
#include "map.ii"


int main(int argc, char * argv[])
{
  typedef map < int, char const *, less < int > > my_map;
  typedef my_map::iterator my_iterator;
  typedef my_map::value_type my_value_type;
  my_map m;

  m.insert(my_value_type(7, "seven"));
  m.insert(my_value_type(5, "five"));
  m.insert(my_value_type(2, "two"));
  m.insert(my_value_type(9, "nine"));
  m.insert(my_value_type(3, "three"));
  m.insert(my_value_type(8, "eight"));
  m.insert(my_value_type(1, "one"));
  m.insert(my_value_type(6, "six"));
  m.insert(my_value_type(0, "zero"));
  m.insert(my_value_type(4, "four"));

  cout << m[0].ref << endl;
  cout << m[1].ref << endl;
  cout << m[2].ref << endl;
  cout << m[3].ref << endl;
  cout << m[4].ref << endl;
  cout << m[5].ref << endl;
  cout << m[6].ref << endl;
  cout << m[7].ref << endl;
  cout << m[8].ref << endl;
  cout << m[9].ref << endl;

  return 0;
}


