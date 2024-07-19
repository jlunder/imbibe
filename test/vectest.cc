#include <assert.h>
#include <iostream.h>
#include <stddef.h>
#include <stdlib.h>


typedef unsigned char bool;
bool const true = 1;
bool const false = 0;


#include "vector.hh"
#include "vector.ii"


template < class T >
ostream & operator <<(ostream & o, vector < T > const & v)
{
  vector < T > ::size_type i;

  o << '[';
  for(i = 0; i + 1 < v.size(); ++i)
  {
    o << v[i] << ", ";
  }
  if(!v.empty())
  {
    o << v.back();
  }
  o << ']';
  return o;
}


#define ref_size 10000
int ref_data[ref_size];
bool ref_flags[ref_size];
size_t last_capacity = 0;
vector < int > v;
unsigned long single_insert_count = 0;
unsigned long single_erase_count = 0;
unsigned long multi_insert_count = 0;
unsigned long multi_erase_count = 0;
unsigned long multi_insert_total = 0;
unsigned long multi_erase_total = 0;
unsigned long multi_multi_insert_count = 0;
unsigned long multi_multi_erase_count = 0;
unsigned long multi_multi_insert_total = 0;
unsigned long multi_multi_erase_total = 0;


int rand_index()
{
  return rand() % ref_size;
}


size_t vector_index(size_t i)
{
  size_t n;
  size_t v_n;

  assert(i < ref_size);
  for(n = 0, v_n = 0; n < i; ++n)
  {
    if(ref_flags[n]) ++v_n;
    assert(v_n <= v.size());
  }
  return v_n;
}


size_t ref_index(size_t v_i)
{
  size_t n;
  size_t v_n;

  assert(v_i < v.size());
  for(n = 0, v_n = 0; v_n < v_i; ++n)
  {
    assert(n < ref_size);
    if(ref_flags[n]) ++v_n;
  }
  while(!ref_flags[n]) ++n;
  return n;
}


void validate_vector()
{
  size_t n;
  size_t v_n;

  for(n = 0, v_n = 0; n < ref_size; ++n)
  {
    if(ref_flags[n])
    {
      assert(v[v_n] == ref_data[n]);
      ++v_n;
    }
  }
  assert(v.size() == v_n);
  assert(v.capacity() >= v.size());
}


single_insert(size_t i)
{
  assert(!ref_flags[i]);
  v.insert(v.begin() + vector_index(i), ref_data[i]);
  ref_flags[i] = true;
  ++single_insert_count;
}


single_erase(size_t i)
{
  assert(ref_flags[i]);
  v.erase(v.begin() + vector_index(i));
  ref_flags[i] = false;
  ++single_erase_count;
}


multi_insert(size_t i)
{
  size_t v_i = vector_index(i);
  size_t n;
  size_t n_n;

  for(n = i; (n < ref_size) && (!ref_flags[n]); ++n);
  assert(n > i);
  n_n = rand_index() % (n - i);
  n = i + n_n;
  v.insert(v.begin() + v_i, ref_data + i, ref_data + n);
  for(; i != n; ++i) ref_flags[i] = true;
  multi_insert_total += n_n;
  ++multi_insert_count;
  if(n_n > 1)
  {
    multi_multi_insert_total += n_n;
    ++multi_multi_insert_count;
  }
}


multi_erase(size_t i)
{
  size_t v_i = vector_index(i);
  size_t n;
  size_t n_n;

  for(n = i; (n < ref_size) && (ref_flags[n]); ++n);
  assert(n > i);
  n_n = rand_index() % (n - i);
  n = i + n_n;
  v.erase(v.begin() + v_i, v.begin() + v_i + n_n);
  for(; i != n; ++i) ref_flags[i] = false;
  multi_erase_total += n_n;
  ++multi_erase_count;
  if(n_n > 1)
  {
    multi_multi_erase_total += n_n;
    ++multi_multi_erase_count;
  }
}


int main(int argc, char * argv[])
{
  unsigned long i;
  size_t n;

  for(n = 0; n < ref_size; ++n)
  {
    ref_data[n] = rand_index();
  }
  for(i = 0; i < 100000; ++i)
  {
    n = rand_index();
    if(!ref_flags[n])
    {
      if(rand_index() % 2)
      {
        single_insert(n);
      }
      else
      {
        multi_insert(n);
      }
    }
    else
    {
      if(rand_index() % 2)
      {
        single_erase(n);
      }
      else
      {
        multi_erase(n);
      }
    }
    if(((i + 1) % 100) == 0)
    {
      validate_vector();
      cout << '.';
    }
    if(v.capacity() != last_capacity)
    {
      cout << endl;
      cout << v.capacity();
      last_capacity = v.capacity();
    }
  }
  cout << endl;
  cout << "single inserts: " << single_insert_count << endl;
  cout << "single erases: " << single_erase_count << endl;
  cout << "multi inserts: " << multi_insert_count << " @ " << (float)multi_insert_total / (float)multi_insert_count << endl;
  cout << "multi erases: " << multi_erase_count << " @ " << (float)multi_erase_total / (float)multi_erase_count << endl;
  cout << "multi multi inserts: " << multi_multi_insert_count << " @ " << (float)multi_multi_insert_total / (float)multi_multi_insert_count << endl;
  cout << "multi multi erases: " << multi_multi_erase_count << " @ " << (float)multi_multi_erase_total / (float)multi_multi_erase_count << endl;
  return 0;
}


