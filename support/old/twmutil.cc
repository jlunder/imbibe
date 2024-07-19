layout_element * format_paragraph(bitmap_p_list const & words, int width, justification_style justification, indent_style indent)
{
  bitmap_p_list::const_iterator i;
  vbox_element * vbox = new vbox_element;
  hbox_element * hbox = new hbox_element;
  bool first_word_in_line = true;
  bool after_punctuation = false;

  if(indent == normal_indent)
  {
    hbox->add_element(new glue_element(pixel(' '), 4, 4, 4));
  }
  if((justification == right) || (justification == center))
  {
    hbox->add_element(new glue_element(pixel(' ')));
  }
  for(i = words.begin(); i != words.end(); ++i)
  {
    if(hbox->ideal_width() + (*i)->width() + 1 > width)
    {
      if((justification == left) || (justification == center))
      {
        hbox->add_element(new glue_element(pixel(' ')));
      }
      vbox->add_element(hbox);
      hbox = new hbox_element;
      if((justification == right) || (justification == center))
      {
        hbox->add_element(new glue_element(pixel(' ')));
      }
      if(indent == hanging_indent)
      {
        hbox->add_element(new glue_element(pixel(' '), 4, 4, 4));
      }
      first_word_in_line = true;
    }
    if(!first_word_in_line)
    {
      if(justification == justify)
      {
        if(after_punctuation)
        {
          hbox->add_element(new glue_element(pixel(' '), 1, 1, width * 4 + 1));
        }
        else
        {
          hbox->add_element(new glue_element(pixel(' '), 1, 1, width + 1));
        }
        switch(pixel((*i)->at((*i)->width() - 1, 0)).character())
        {
        case '.':
        case '?':
        case ':':
          after_punctuation = true;
          break;
        default:
          after_punctuation = false;
          break;
        }
      }
      else
      {
        hbox->add_element(new glue_element(pixel(' '), 1, 1, 1));
      }
    }
    hbox->add_element(new picture_element(*i));
    first_word_in_line = false;
  }
  if((justification == left) || (justification == center) || (justification == justify))
  {
    hbox->add_element(new glue_element(pixel(' ')));
  }
  vbox->add_element(hbox);
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


string int_to_string(int i)
{
  string s;
  bool neg;

  neg = i < 0;
  if(neg)
  {
    i = -i;
  }
  do
  {
    s.insert(s.begin(), (char)('0' + i % 10));
    i /= 10;
  } while(i);
  if(neg)
  {
    s.insert(s.begin(), '-');
  }

  return s;
}


