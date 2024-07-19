#include "cplusplus.hh"

#include <assert.h>
#include <fstream.h>
#include <iostream.h>
#include <math.h>
#include <stdlib.h>

#include "bitmap.hh"
#include "functional.hh"
#include "graphics.hh"
#include "map.hh"
#include "string.hh"
#include "vector.hh"

#include "bitmap.ii"
#include "functional.ii"
#include "graphics.ii"
#include "map.ii"
#include "string.ii"
#include "vector.ii"


struct token
{
  enum
  {
    tag_text,
    tag_begin,
    tag_end
  } type;
  string text;
};


class layout_element
{
public:
  layout_element(layout_element const & le):
    m_min_width(le.m_min_width), m_ideal_width(le.m_ideal_width), m_max_width(le.m_max_width), m_min_height(le.m_min_height), m_ideal_height(le.m_ideal_height), m_max_height(le.m_max_height), m_width(le.m_width), m_height(le.m_height)
  {
    assert((m_width >= m_min_width) && (m_width <= m_max_width));
    assert((m_height >= m_min_height) && (m_height <= m_max_height));
  }
  layout_element(int n_min_width, int n_ideal_width, int n_max_width, int n_min_height, int n_ideal_height, int n_max_height, int n_width, int n_height):
    m_min_width(n_min_width), m_ideal_width(n_ideal_width), m_max_width(n_max_width), m_min_height(n_min_height), m_ideal_height(n_ideal_height), m_max_height(n_max_height), m_width(n_width), m_height(n_height)
  {
    assert((m_width >= m_min_width) && ((m_max_width < 0) || (m_width <= m_max_width)));
    assert((m_height >= m_min_height) && ((m_max_height < 0) || (m_height <= m_max_height)));
  }
  int min_width() const {return m_min_width;}
  int ideal_width() const {return m_ideal_width;}
  int max_width() const {return m_max_width;}
  int min_height() const {return m_min_height;}
  int ideal_height() const {return m_ideal_height;}
  int max_height() const {return m_max_height;}
  void width(int n_width) {assert((n_width >= m_min_width) && ((m_max_width < 0) || (n_width <= m_max_width))); m_width = n_width;}
  int width() const {return m_width;}
  void height(int n_height) {assert((n_height >= m_min_height) && ((m_max_height < 0) || (n_height <= m_max_height))); m_height = n_height;}
  int height() const {return m_height;}
  virtual void draw(graphics & g) = 0;

protected:
  void min_width(int n_min_width) {assert(n_min_width >= 0); m_min_width = n_min_width;}
  void ideal_width(int n_ideal_width) {m_ideal_width = n_ideal_width;}
  void max_width(int n_max_width) {m_max_width = n_max_width;}
  void min_height(int n_min_height) {assert(n_min_height >= 0); m_min_height = n_min_height;}
  void ideal_height(int n_ideal_height) {m_ideal_height = n_ideal_height;}
  void max_height(int n_max_height) {m_max_height = n_max_height;}

private:
  int m_min_width;
  int m_ideal_width;
  int m_max_width;
  int m_min_height;
  int m_ideal_height;
  int m_max_height;
  int m_width;
  int m_height;
};


typedef vector < layout_element * > element_list;
typedef vector < unsigned short > line;
typedef vector < line > line_list;
typedef vector < token > token_list;
typedef vector < string > string_list;
typedef void (* parser)(token_list::iterator & i);
typedef map < string, parser, less < string > > parser_list;


class word_element: public layout_element
{
public:
  word_element(word_element const & we):
    layout_element(we), m_word(we.m_word), m_c(we.m_c)
  {
  }
  word_element(string const & n_word, color n_c):
    layout_element(n_word.length(), n_word.length(), n_word.length(), 1, 1, 1, n_word.length(), 1), m_word(n_word), m_c(n_c)
  {
  }
  virtual void draw(graphics & g)
  {
    assert(g.bounds_width() == width());
    assert(g.bounds_height() == height());

    g.draw_text(0, 0, m_c, m_word.c_str());
  }

private:
  string m_word;
  color m_c;
};


class picture_element: public layout_element
{
public:
  picture_element(picture_element const & pe):
    layout_element(pe), m_picture(pe.m_picture)
  {
  }
  picture_element(bitmap const & n_picture):
    layout_element(n_picture.width(), n_picture.width(), n_picture.width(), n_picture.height(), n_picture.height(), n_picture.height(), n_picture.width(), n_picture.height()), m_picture(n_picture)
  {
  }
  virtual void draw(graphics & g)
  {
    assert(g.bounds_width() == width());
    assert(g.bounds_height() == height());

    g.draw_bitmap(0, 0, m_picture);
  }

private:
  bitmap m_picture;
};


class glue_element: public layout_element
{
public:
  glue_element(glue_element const & ge):
    layout_element(ge), m_fill(ge.m_fill)
  {
  }
  glue_element(pixel n_fill, int n_min_width = 0, int n_ideal_width = 0, int n_max_width = -1, int n_min_height = 0, int n_ideal_height = 0, int n_max_height = -1):
    layout_element(n_min_width, n_ideal_width, n_max_width, n_min_height, n_ideal_height, n_max_height, n_ideal_width, n_ideal_height), m_fill(n_fill)
  {
  }
  glue_element(pixel n_fill, int n_min_width, int n_ideal_width, int n_max_width, int n_min_height, int n_ideal_height, int n_max_height, int n_width, int n_height):
    layout_element(n_min_width, n_ideal_width, n_max_width, n_min_height, n_ideal_height, n_max_height, n_width, n_height), m_fill(n_fill)
  {
  }
  virtual void draw(graphics & g)
  {
    assert(g.bounds_width() == width());
    assert(g.bounds_height() == height());

    g.draw_rectangle(0, 0, width(), height(), m_fill);
  }

private:
  pixel m_fill;
};


class hbox_element: public layout_element
{
public:
  hbox_element(): layout_element(0, 0, 0, 0, 0, -1, 0, 0) {}
  virtual ~hbox_element()
  {
    element_list::iterator i;

    for(i = m_elements.begin(); i != m_elements.end(); ++i)
    {
      delete *i;
    }
  }
  void add_element(layout_element * l)
  {
    m_elements.push_back(l);
    min_width(min_width() + l->min_width());
    ideal_width(ideal_width() + l->ideal_width());
    if((max_width() < 0) || (l->max_width() < 0)) max_width(-1);
    else max_width(max_width() + l->max_width());

    assert((max_height() < 0) || (l->min_height() <= max_height()));
    assert((l->max_height() < 0) || (l->max_height() >= min_height()));

    if(l->min_height() > min_height()) min_height(l->min_height());
    if(ideal_height() < min_height()) ideal_height(min_height());
    if(max_height() < 0)
    {
      if(!(l->max_height() < 0))
      {
        max_height(l->max_height());
        if(ideal_height() > max_height()) ideal_height(max_height());
      }
    }
    else
    {
      if(!(l->max_height() < 0))
      {
        if(l->max_height() < max_height()) max_height(l->max_height());
        if(ideal_height() > max_height()) ideal_height(max_height());
      }
    }
    if(l->ideal_height() > ideal_height())
    {
      if((max_height() < 0) || (l->ideal_height() < max_height()))
      {
        ideal_height(l->ideal_height());
      }
      else
      {
        ideal_height(max_height());
      }
    }
  }
  virtual void draw(graphics & g)
  {
    int x1 = g.bounds_x1();
    int y1 = g.bounds_y1();
    element_list::iterator i;
    int num_elem = 0;
    double pos = 0.0;
    double new_pos = 0.0;
    double width_per_elem = 0.0;

    assert(g.bounds_width() == width());
    assert(g.bounds_height() == height());

    if(ideal_width() > width())
    { //shrink
      num_elem = 0;
      width_per_elem = (double)(width() - ideal_width()) / (double)(min_width() - ideal_width());
      for(i = m_elements.begin(); i != m_elements.end(); ++i)
      {
        new_pos = pos + (double)((*i)->min_width() - (*i)->ideal_width()) * width_per_elem + (double)(*i)->ideal_width();
        (*i)->width(floor(new_pos + 0.5) - floor(pos + 0.5));
        (*i)->height(height());
        g.set_bounds(x1 + floor(pos + 0.5), y1, x1 + floor(pos + 0.5) + (*i)->width(), y1 + height());
        (*i)->draw(g);
        pos = new_pos;
      }
    }
    else
    { //expand
      if(max_width() < 0)
      {
        num_elem = 0;
        for(i = m_elements.begin(); i != m_elements.end(); ++i)
        {
          if((*i)->max_width() < 0) ++num_elem;
        }
        width_per_elem = ((double)width() - ideal_width()) / (double)num_elem;
        for(i = m_elements.begin(); i != m_elements.end(); ++i)
        {
          new_pos = (double)(*i)->ideal_width() + pos;
          if((*i)->max_width() < 0) new_pos += width_per_elem;
          (*i)->width(floor(new_pos + 0.5) - floor(pos + 0.5));
          (*i)->height(height());
          g.set_bounds(x1 + floor(pos + 0.5), y1, x1 + floor(pos + 0.5) + (*i)->width(), y1 + height());
          (*i)->draw(g);
          pos = new_pos;
        }
      }
      else
      {
        num_elem = 0;
        width_per_elem = (double)(width() - ideal_width()) / (double)(max_width() - ideal_width());
        for(i = m_elements.begin(); i != m_elements.end(); ++i)
        {
          new_pos = pos + (double)((*i)->max_width() - (*i)->ideal_width()) * width_per_elem + (double)(*i)->ideal_width();
          (*i)->width(floor(new_pos + 0.5) - floor(pos + 0.5));
          (*i)->height(height());
          g.set_bounds(x1 + floor(pos + 0.5), y1, x1 + floor(pos + 0.5) + (*i)->width(), y1 + height());
          (*i)->draw(g);
          pos = new_pos;
        }
      }
    }
    g.set_bounds(x1, y1, x1 + width(), y1 + width());
  }

private:
  element_list m_elements;
};


//class vbox_element: public layout_element
//{
//};
typedef hbox_element vbox_element;


layout_element * format_paragraph(string_list const & words)
{
  vbox_element * vbox = new vbox_element;
  return vbox;
}


layout_element * format_page()
{
  vbox_element * vbox = new vbox_element;
  return vbox;
}


int hex_digit(char c)
{
  switch(c)
  {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'a': return 10;
  case 'b': return 11;
  case 'c': return 12;
  case 'd': return 13;
  case 'e': return 14;
  case 'f': return 15;
  case 'A': return 10;
  case 'B': return 11;
  case 'C': return 12;
  case 'D': return 13;
  case 'E': return 14;
  case 'F': return 15;
  default: cerr << "illegal character in hex escape code -- did you forget to escape a \\ ?" << endl; abort();
  }
  return -1;
}


bool is_whitespace(char c)
{
  return c <= ' ';
}


void chop(string & s)
{
  string::iterator i;

  for(i = s.end(); (i != s.begin()) && is_whitespace(*(i - 1)); --i);
  s.erase(i, s.end());
  for(i = s.begin(); (i != s.end()) && is_whitespace(*i); ++i);
  s.erase(s.begin(), i);
}


void separate_text(string const & s, string_list & t)
{
  string::const_iterator i;
  string::const_iterator j;

  t.clear();
  for(i = s.begin(), j = s.begin(); j != s.end(); ++j)
  {
    if(is_whitespace(*j))
    {
      if(i != j)
      {
        t.push_back(string(i, j));
      }
      i = j + 1;
    }
  }
  if(i != j)
  {
    t.push_back(string(i, j));
  }
}


/*
void write_words(string_list::iterator first, string_list::iterator last, format_style format, int width, line_list & l)
{
  string_list::iterator i;
  string s;

  for(i = first; i != last; ++i)
  {
    if(i->length() > width())
    {
      cerr << "word is longer than the format width: " << *i << endl;
    }
    if(s.length() + i->length() + 1 > width)
    {
      switch(format)
      {
      case left:
        break;
      case right:
        s.insert(s.begin(), width - s.length(), ' ');
        break;
      case centered:
        s.insert(s.begin(), (width - s.length()) / 2, ' ');
        break;
      case justified:
        //todo
        cerr << "justified text is not supported" << endl;
        break;
      default:
        cerr << "very serious internal error. poo!" << endl;
        abort();
        break;
      }
      s.clear();
    }
    s.append(*i);
  }
}
*/


line_list lines;
string_list authors;
string title;

/*
void parse_recipe(token_list::iterator first, token_list::iterator last)
{
  token_list::iterator i;
  token_list::iterator j;
  string_list text;
  bool buffered = true; //is there whitespace between here and the text above?
  int tag_depth = 0;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      clog << "ignoring extraneous text: " << j->text << endl;
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == "title")
        {
          parse_recipe_title(i + 1, j);
        }
        if(i->text == "ingredients")
        {
        }
        else if(i->text == "procedure")
        {
        }
        else if(i->text == "yield")
        {
          lines->push_back(line());
          lines->push_back(line());
          parse_recipe(i + 1, j);
          lines->push_back(line());
        }
        else
        {
          clog << "ignoring unrecognized tag (recipe context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #2837419" << endl;
        abort();
      }
      break;
    }
  }
}


void parse_article(token_list::iterator first, token_list::iterator last)
{
  token_list::iterator i;
  token_list::iterator j;
  string_list text;
  int tag_depth = 0;

  for(i = first, j = first; j != last; ++j)
  {
    switch(i->type)
    {
    case token::tag_text:
      clog << "ignoring extraneous text: " << j->text << endl;
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == "author")
        {
          parse_author(i + 1, j);
        }
        else if(i->text == "title")
        {
          parse_title(i + 1, j);
        }
        else if(i->text == "recipe")
        {
          lines->push_back(line());
          lines->push_back(line());
          parse_recipe(i + 1, j);
          lines->push_back(line());
        }
        else
        {
          clog << "ignoring unrecognized tag (article context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #09235847" << endl;
        abort();
      }
      break;
    }
  }
}


void parse_main(token_list::iterator first, token_list::iterator last)
{
  token_list::iterator i;
  token_list::iterator j;
  int tag_depth = 0;

  for(i = first, j = first; j != last; ++j)
  {
    switch(i->type)
    {
    case token::tag_text:
      clog << "ignoring extraneous text: " << j->text << endl;
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == "article")
        {
          parse_article(i + 1, j);
        }
        else
        {
          clog << "ignoring unrecognized tag: " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "ignoring out-of-place tag end" << endl;
        tag_depth = 0;
      }
      break;
    }
  }
}


void process(istream & twmf, ostream & binf)
{
  int c;
  char a;
  line_list::iterator li;
  token_list tokens;
  token_list::iterator ti;
  token t;
  unsigned long count;
  unsigned short * temp;
  bool escape = false;
  enum
  {
    initial,
    hex_digit_1,
    hex_digit_2
  } escape_state = initial;

  t.type = token::tag_text;
  clog << "tokenizing text file" << endl;
  while(twmf.good() && !twmf.eof())
  {
    if((c = twmf.get()) == EOF) break;

    if(escape)
    {
      switch(escape_state)
      {
      case initial:
        switch(c)
        {
        case '\\':
          c = '\\';
          escape = false;
          break;
        case 'x':
          escape_state = hex_digit_1;
          continue;
        default:
          cerr << "illegal escape character -- did you forget to escape a \\ ?" << endl;
          abort();
          break;
        }
        break;
      case hex_digit_1:
        a = hex_digit(c) * 16;
        escape_state = hex_digit_2;
        continue;
      case hex_digit_2:
        c = hex_digit(c) + a;
        escape_state = initial;
        escape = false;
        break;
      }
    }
    else
    {
      if(c == '\\')
      {
        escape_state = initial;
        escape = true;
        continue;
      }
    }
    switch(c)
    {
    case '{':
      tokens.push_back(t);
      t.type = token::tag_begin;
      t.text.clear();
      break;
    case '}':
      tokens.push_back(t);
      t.type = token::tag_end;
      t.text.clear();
      break;
    default:
      switch(t.type)
      {
      case token::tag_text:
        t.text.append((char)c);
        break;
      case token::tag_begin:
        if(is_whitespace(c))
        {
          tokens.push_back(t);
          t.type = token::tag_text;
          t.text.assign((char)c);
        }
        else
        {
          t.text.append((char)c);
        }
        break;
      case token::tag_end:
        tokens.push_back(t);
        t.type = token::tag_text;
        t.text.assign((char)c);
        break;
      }
      break;
    }
  }
  for(ti = tokens.begin(); ti != tokens.end(); )
  {
    chop(ti->text);
    if((ti->type == token::tag_text) && ti->text.empty())
    {
      count = ti - tokens.begin();
      tokens.erase(ti);
      ti = tokens.begin() + count;
    }
    else
    {
      ++ti;
    }
  }
  clog << "parsing tokenlist" << endl;
  parse_main(tokens.begin(), tokens.end());
  clog << "processing parsed output" << endl;
  temp = new unsigned short[lines.size() * 80];
  for(count = 0; count < lines.size() * 80; ++count)
  {
    temp[count] = 0x0700 | ' ';
  }
  count = 0;
  for(li = lines.begin(); li != lines.end(); ++li)
  {
    memcpy(temp + count, li->begin(), (li->size() > 80 ? 80 : li->size()) * sizeof(unsigned short));
    count += 80;
  }
  clog << "writing bin file" << endl;
  binf.write((unsigned char *)temp, lines.size() * 80 * sizeof(unsigned short));
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
  ifstream twmf;
  ofstream binf;

  clog << "  twm2bin, version the 0th" << endl;
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
  twmf.open(argv[1], ios::in | ios::text);
  if(!twmf.good())
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

  process(twmf, binf);

  clog << "closing bin file" << endl;
  binf.close();
  clog << "closing text file" << endl;
  twmf.close();
  return 0;
}


*/



int main(int argc, char * argv[])
{
  bitmap b(80, 25);
  ofstream f("temp.bin", iostream::binary);
  hbox_element * h;

  b.g().set_bounds(0, 0, 80, 25);
  b.g().draw_rectangle(0, 0, 80, 25, pixel('+'));

  h = new hbox_element;
  h->add_element(new word_element(string("hi"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("there."), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("this"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("text"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("is"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("supposed"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("to"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("be"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("left-justified."), color()));
  h->add_element(new glue_element(pixel(' ')));
  b.g().set_bounds(0, 0, 80, 1);
  h->width(80);
  h->height(1);
  h->draw(b.g());
  delete h;

  h = new hbox_element;
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("hi"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("there."), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("this"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("text"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("is"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("supposed"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("to"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("be"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("right-justified."), color()));
  b.g().set_bounds(0, 1, 80, 2);
  h->width(80);
  h->height(1);
  h->draw(b.g());
  delete h;

  h = new hbox_element;
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("hi"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("there."), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("this"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("text"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("is"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("supposed"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("to"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("be"), color()));
  h->add_element(new word_element(string(" "), color()));
  h->add_element(new word_element(string("centered."), color()));
  h->add_element(new glue_element(pixel(' ')));
  b.g().set_bounds(0, 2, 80, 3);
  h->width(80);
  h->height(1);
  h->draw(b.g());
  delete h;

  h = new hbox_element;
  h->add_element(new word_element(string("hi"), color()));
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("there."), color()));
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("this"), color()));
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("text"), color()));
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("is"), color()));
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("supposed"), color()));
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("to"), color()));
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("be"), color()));
  h->add_element(new glue_element(pixel(' ')));
  h->add_element(new word_element(string("justified."), color()));
  b.g().set_bounds(0, 3, 80, 4);
  h->width(80);
  h->height(1);
  h->draw(b.g());
  delete h;

  f.write((unsigned char *)b.data(), 80 * 25 * sizeof(unsigned short));

  return 0;
}

