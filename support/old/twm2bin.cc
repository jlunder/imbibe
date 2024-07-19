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


class layout_element;


typedef vector < layout_element * > layout_element_p_list;
typedef vector < token > token_list;
typedef vector < string > string_list;
typedef vector < bitmap * > bitmap_p_list;
typedef map < string, color, less < string > > string_color_map;
enum justification_style {left, center, right, justify};
enum indent_style {no_indent, normal_indent, hanging_indent};


struct aside
{
  int line;
  int column;
  string author;
  bitmap * picture;
};


typedef vector < aside > aside_list;


#include "twmlayout.cc"
#include "twmutil.cc"


string_color_map color_name_map;

string_list authors;
string title;
aside_list asides;

color footnote_marking_color(color::hi_white);
color aside_footnote_marking_color(color::hi_white);
color aside_author_color(color::white);
color aside_text_color(color::hi_black);
color recipe_title_color(color::hi_green);
color recipe_label_color(color::hi_green);
color recipe_list_item_bullet_color(color::hi_yellow);
color recipe_list_item_color(color::green);
color recipe_text_color(color::red);
color article_text_color(color::hi_blue);
pixel recipe_fill(' ');


string parse_single_string(token_list::const_iterator first, token_list::const_iterator last)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  int tag_depth = 0;
  string s;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        if(!s.empty())
        {
          s.append(' ');
        }
      }
      s.append(j->text);
      break;
    case token::tag_begin:
      ++tag_depth;
      clog << "ignoring illegal tag (single string context): " << j->text << endl;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth < 0)
      {
        clog << "very serious internal parse error #97675431" << endl;
        abort();
      }
      break;
    }
  }
  return s;
}


layout_element * parse_simple_paragraph(token_list::const_iterator first, token_list::const_iterator last, int width, justification_style justification, indent_style indent, color default_color)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  string_list text;
  string_list::iterator k;
  string_color_map::const_iterator l;
  bitmap_p_list colorized_text;
  bitmap * b;
  color c = default_color;
  int tag_depth = 0;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        separate_text(j->text, text);
        for(k = text.begin(); k != text.end(); ++k)
        {
          b = new bitmap(k->length(), 1);
          b->g().draw_text(0, 0, c, *k);
          colorized_text.push_back(b);
        }
      }
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == "color")
        {
          l = color_name_map.find(parse_single_string(i, j));
          if(l == color_name_map.end())
          {
            clog << "ignoring bad color name: " << i->text << endl;
          }
          else
          {
            c = l->ref;
          }
        }
        else
        {
          clog << "ignoring unrecognized tag (p context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #13322895" << endl;
        abort();
      }
      break;
    }
  }
  return format_paragraph(colorized_text, width, justification, indent);
}


void parse_search_for_tag(token_list::const_iterator first, token_list::const_iterator last, string tag_name, token_list::const_iterator & i, token_list::const_iterator & j)
{
  int tag_depth = 0;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == tag_name)
        {
          ++i;
          return;
        }
      }
      break;
    }
  }
  i = last;
  j = last;
}


layout_element * parse_aside(token_list::const_iterator first, token_list::const_iterator last, int width)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  bitmap * b;
  aside a;
  int tag_depth = 0;
  hbox_element * author_hbox;
  vbox_element * author_vbox;
  vbox_element * p_vbox;
  hbox_element * hbox;
  word_element * footnote_marking;

  parse_search_for_tag(first, last, string("author"), i, j);
  if(i != last)
  {
    a.author = parse_single_string(i, j);
  }
  footnote_marking = new word_element("[" + int_to_string(asides.size() + 1) + "]", footnote_marking_color);
  author_hbox = new hbox_element;
  author_hbox->add_element(new word_element("[" + int_to_string(asides.size() + 1) + "]", aside_footnote_marking_color));
  if(!a.author.empty())
  {
    author_hbox->add_element(new glue_element(pixel(' ', aside_author_color), 1, 1, 1));
    author_hbox->add_element(new word_element(a.author + ":", aside_author_color));
  }
  author_hbox->add_element(new glue_element(pixel(' ', aside_author_color), 1, 1, 1));
  author_vbox = new vbox_element;
  author_vbox->add_element(author_hbox);
  author_vbox->add_element(new glue_element(pixel(' ', aside_author_color)));
  hbox = new hbox_element;
  hbox->add_element(author_vbox);
  p_vbox = new vbox_element;
  width -= author_vbox->ideal_width();

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        clog << "ignoring extraneous text (aside context): " << j->text << endl;
      }
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
          //already got
        }
        else if(i->text == "p")
        {
          p_vbox->add_element(parse_simple_paragraph(i + 1, j, width, left, no_indent, aside_text_color));
        }
        else
        {
          clog << "ignoring unrecognized tag (aside context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #8837264" << endl;
        abort();
      }
      break;
    }
  }
  hbox->add_element(p_vbox);
  a.picture = new bitmap(80, hbox->ideal_height());
  hbox->width(80);
  hbox->height(hbox->ideal_height());
  hbox->draw(a.picture->g());
  delete hbox;
  asides.push_back(a);
  return footnote_marking;
}


layout_element * parse_paragraph(token_list::const_iterator first, token_list::const_iterator last, int width, justification_style justification, indent_style indent, color default_color)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  string_list text;
  string_list::iterator k;
  string_color_map::const_iterator l;
  bitmap_p_list colorized_text;
  bitmap * b;
  color c = default_color;
  int tag_depth = 0;
  layout_element * le;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        separate_text(j->text, text);
        for(k = text.begin(); k != text.end(); ++k)
        {
          b = new bitmap(k->length(), 1);
          b->g().draw_text(0, 0, c, *k);
          colorized_text.push_back(b);
        }
      }
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == "color")
        {
          l = color_name_map.find(parse_single_string(i + 1, j));
          if(l == color_name_map.end())
          {
            clog << "ignoring bad color name: " << i->text << endl;
          }
          else
          {
            c = l->ref;
          }
        }
        else if(i->text == "aside")
        {
          le = parse_aside(i + 1, j, width);
          b = new bitmap(le->ideal_width(), le->ideal_height());
          le->width(le->ideal_width());
          le->height(le->ideal_height());
          le->draw(b->g());
          delete le;
          colorized_text.push_back(b);
        }
        else
        {
          clog << "ignoring unrecognized tag (p context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #01238973" << endl;
        abort();
      }
      break;
    }
  }
  return format_paragraph(colorized_text, width, justification, indent);
}


layout_element * parse_recipe_list_item(token_list::const_iterator first, token_list::const_iterator last, int width)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  int tag_depth = 0;
  string label;
  hbox_element * hbox = new hbox_element;
  vbox_element * bullet_vbox = new vbox_element;
  hbox_element * bullet_hbox = new hbox_element;
  vbox_element * p_vbox = new vbox_element;

  bullet_hbox->add_element(new glue_element(recipe_fill, 1, 1, 1));
  bullet_hbox->add_element(new word_element(string("-"), recipe_list_item_bullet_color));
  bullet_hbox->add_element(new glue_element(recipe_fill, 1, 1, 1));
  bullet_vbox->add_element(bullet_hbox);
  bullet_vbox->add_element(new glue_element(recipe_fill));
  hbox->add_element(bullet_vbox);
  width -= bullet_vbox->ideal_width();

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        clog << "ignoring extraneous text (recipe list item context): " << j->text << endl;
      }
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == "p")
        {
          p_vbox->add_element(parse_paragraph(i + 1, j, width, left, no_indent, recipe_list_item_color));
        }
        else
        {
          clog << "ignoring unrecognized tag (recipe list item context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #00285474" << endl;
        abort();
      }
      break;
    }
  }

  hbox->add_element(p_vbox);
  return hbox;
}


layout_element * parse_recipe_list(token_list::const_iterator first, token_list::const_iterator last, int width)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  int tag_depth = 0;
  string label;
  vbox_element * vbox = new vbox_element;
  vbox_element * p_vbox = new vbox_element;
  hbox_element * label_hbox;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        clog << "ignoring extraneous text (recipe list context): " << j->text << endl;
      }
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == "label")
        {
          if(label.empty())
          {
            label = parse_single_string(i + 1, j);
            label_hbox = new hbox_element;
            label_hbox->add_element(new word_element(label, recipe_label_color));
            label_hbox->add_element(new glue_element(recipe_fill));
            vbox->add_element(label_hbox);
          }
          else
          {
            clog << "ignoring redundant label (recipe list context): " << parse_single_string(i + 1, j) << endl;
          }
        }
        else if(i->text == "item")
        {
          p_vbox->add_element(parse_recipe_list_item(i + 1, j, width));
        }
        else
        {
          clog << "ignoring unrecognized tag (recipe list context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #39840039" << endl;
        abort();
      }
      break;
    }
  }

  if((vbox->ideal_height() != 0) && (p_vbox->ideal_height() != 0))
  {
    vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
  }
  if(p_vbox->ideal_height() != 0)
  {
    vbox->add_element(p_vbox);
  }
  return vbox;
}


layout_element * parse_recipe_text(token_list::const_iterator first, token_list::const_iterator last, int width)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  int tag_depth = 0;
  string label;
  vbox_element * vbox = new vbox_element;
  vbox_element * p_vbox = new vbox_element;
  hbox_element * label_hbox;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        clog << "ignoring extraneous text (recipe list context): " << j->text << endl;
      }
      break;
    case token::tag_begin:
      if(tag_depth == 0) i = j;
      ++tag_depth;
      break;
    case token::tag_end:
      --tag_depth;
      if(tag_depth == 0)
      {
        if(i->text == "label")
        {
          if(label.empty())
          {
            label = parse_single_string(i + 1, j);
            label_hbox = new hbox_element;
            label_hbox->add_element(new word_element(label, recipe_label_color));
            label_hbox->add_element(new glue_element(recipe_fill));
            vbox->add_element(label_hbox);
          }
          else
          {
            clog << "ignoring redundant label (recipe text context): " << parse_single_string(i + 1, j) << endl;
          }
        }
        else if(i->text == "p")
        {
          p_vbox->add_element(parse_paragraph(i + 1, j, width, left, normal_indent, recipe_text_color));
        }
        else
        {
          clog << "ignoring unrecognized tag (recipe list context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #95736215" << endl;
        abort();
      }
      break;
    }
  }

  if((vbox->ideal_height() != 0) && (p_vbox->ideal_height() != 0))
  {
    vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
  }
  if(p_vbox->ideal_height() != 0)
  {
    vbox->add_element(p_vbox);
  }
  return vbox;
}


layout_element * parse_recipe(token_list::const_iterator first, token_list::const_iterator last, int width)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  int tag_depth = 0;
  string title;
  vbox_element * vbox = new vbox_element;
  vbox_element * body_vbox = new vbox_element;
  vbox_element * p_vbox = new vbox_element;
  hbox_element * title_hbox;
  vbox_element * title_vbox;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        clog << "ignoring extraneous text (recipe context): " << j->text << endl;
      }
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
          if(title.empty())
          {
            title = parse_single_string(i + 1, j);
            title_vbox = new vbox_element;
            title_vbox->add_element(new word_element(title, recipe_title_color));
            title_vbox->add_element(new word_element(string(title.length(), '-'), recipe_title_color));
            title_hbox = new hbox_element;
            title_hbox->add_element(new glue_element(recipe_fill));
            title_hbox->add_element(title_vbox);
            title_hbox->add_element(new glue_element(recipe_fill));
            vbox->add_element(title_hbox);
          }
          else
          {
            clog << "ignoring redundant title (recipe context): " << parse_single_string(i + 1, j) << endl;
          }
        }
        else if(i->text == "ingredients")
        {
          if(body_vbox->ideal_height() > 0)
          {
            body_vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
          }
          body_vbox->add_element(parse_recipe_list(i + 1, j, width));
        }
        else if(i->text == "equipment")
        {
          if(body_vbox->ideal_height() > 0)
          {
            body_vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
          }
          body_vbox->add_element(parse_recipe_list(i + 1, j, width));
        }
        else if(i->text == "procedure")
        {
          if(body_vbox->ideal_height() > 0)
          {
            body_vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
          }
          body_vbox->add_element(parse_recipe_text(i + 1, j, width));
        }
        else if(i->text == "yield")
        {
          if(body_vbox->ideal_height() > 0)
          {
            body_vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
          }
          body_vbox->add_element(parse_recipe_text(i + 1, j, width));
        }
        else
        {
          clog << "ignoring unrecognized tag (recipe context): " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "very serious internal parse error #28374129" << endl;
        abort();
      }
      break;
    }
  }

  if((vbox->ideal_height() != 0) && (body_vbox->ideal_height() != 0))
  {
    vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
  }
  if(body_vbox->ideal_height() != 0)
  {
    vbox->add_element(body_vbox);
  }
  else
  {
    delete body_vbox;
  }
  return vbox;
}


layout_element * parse_article(token_list::const_iterator first, token_list::const_iterator last, int width)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  int tag_depth = 0;
  vbox_element * vbox = new vbox_element;
  enum {no_addition, recipe_addition, paragraph_addition} last_addition = no_addition;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        clog << "ignoring extraneous text (article context): " << j->text << endl;
      }
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
        }
        else if(i->text == "title")
        {
        }
        else if(i->text == "recipe")
        {
          if(last_addition != no_addition)
          {
            vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
          }
          vbox->add_element(parse_recipe(i + 1, j, width));
          last_addition = recipe_addition;
        }
        else if(i->text == "p")
        {
          if((last_addition != no_addition) && (last_addition != paragraph_addition))
          {
            vbox->add_element(new glue_element(recipe_fill, 0, 0, -1, 1, 1, 1));
          }
          vbox->add_element(parse_paragraph(i + 1, j, width, left, normal_indent, article_text_color));
          last_addition = paragraph_addition;
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

  return vbox;
}


layout_element * parse_main(token_list::const_iterator first, token_list::const_iterator last, int width)
{
  token_list::const_iterator i;
  token_list::const_iterator j;
  vbox_element * vbox = new vbox_element;
  int tag_depth = 0;

  for(i = first, j = first; j != last; ++j)
  {
    switch(j->type)
    {
    case token::tag_text:
      if(tag_depth == 0)
      {
        clog << "ignoring extraneous text: " << j->text << endl;
      }
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
          vbox->add_element(parse_article(i + 1, j, 80));
        }
        else
        {
          clog << "ignoring unrecognized tag: " << i->text << endl;
        }
      }
      else if(tag_depth < 0)
      {
        clog << "ignoring extra tag ends ('}'), bitch" << endl;
      }
      break;
    }
  }
  return vbox;
}


void process(istream & twmf, ostream & binf)
{
  int c;
  char a;
  token_list tokens;
  token_list::iterator ti;
  aside_list::iterator ai;
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
  layout_element * layout;
  bitmap * b;

  color_name_map.insert(string_color_map::value_type(string("black"), color(color::black)));
  color_name_map.insert(string_color_map::value_type(string("blue"), color(color::blue)));
  color_name_map.insert(string_color_map::value_type(string("green"), color(color::green)));
  color_name_map.insert(string_color_map::value_type(string("cyan"), color(color::cyan)));
  color_name_map.insert(string_color_map::value_type(string("red"), color(color::red)));
  color_name_map.insert(string_color_map::value_type(string("magenta"), color(color::magenta)));
  color_name_map.insert(string_color_map::value_type(string("yellow"), color(color::yellow)));
  color_name_map.insert(string_color_map::value_type(string("white"), color(color::white)));
  color_name_map.insert(string_color_map::value_type(string("hi_black"), color(color::hi_black)));
  color_name_map.insert(string_color_map::value_type(string("hi_blue"), color(color::hi_blue)));
  color_name_map.insert(string_color_map::value_type(string("hi_green"), color(color::hi_green)));
  color_name_map.insert(string_color_map::value_type(string("hi_cyan"), color(color::hi_cyan)));
  color_name_map.insert(string_color_map::value_type(string("hi_red"), color(color::hi_red)));
  color_name_map.insert(string_color_map::value_type(string("hi_magenta"), color(color::hi_magenta)));
  color_name_map.insert(string_color_map::value_type(string("hi_yellow"), color(color::hi_yellow)));
  color_name_map.insert(string_color_map::value_type(string("hi_white"), color(color::hi_white)));
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
  layout = parse_main(tokens.begin(), tokens.end(), 80);
  clog << "processing parsed output" << endl;
  layout->width(80);
  layout->height(layout->ideal_height());
  b = new bitmap(80, layout->ideal_height());
  layout->draw(b->g());
  clog << "writing bin file" << endl;
  binf.write((unsigned char *)b->data(), b->width() * b->height() * sizeof(unsigned short));
  for(ai = asides.begin(); ai != asides.end(); ++ai)
  {
    binf.write((unsigned char *)ai->picture->data(), ai->picture->width() * ai->picture->height() * sizeof(unsigned short));
    delete ai->picture;
  }
  delete layout;
  delete b;
}


void print_usage()
{
  clog << "usage:" << endl;
  clog << "  twm2bin <twmfile> <binfile>" << endl;
  clog << endl;
  clog << "twmfile: text file with markup codes to parse" << endl;
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
  cout << int_to_string(50) << endl;
  cout << int_to_string(1) << endl;
  cout << int_to_string(2) << endl;
  cout << int_to_string(-277) << endl;
  return 0;
}

