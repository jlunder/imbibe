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
  virtual int min_width() const {return m_min_width;}
  virtual int ideal_width() const {return m_ideal_width;}
  virtual int max_width() const {return m_max_width;}
  virtual int min_height() const {return m_min_height;}
  virtual int ideal_height() const {return m_ideal_height;}
  virtual int max_height() const {return m_max_height;}
  virtual void width(int n_width) {assert((n_width >= m_min_width) && ((m_max_width < 0) || (n_width <= m_max_width))); m_width = n_width;}
  virtual int width() const {return m_width;}
  virtual void height(int n_height) {assert((n_height >= m_min_height) && ((m_max_height < 0) || (n_height <= m_max_height))); m_height = n_height;}
  virtual int height() const {return m_height;}
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
    layout_element(pe), m_picture(new bitmap(*pe.m_picture))
  {
  }
  picture_element(bitmap * n_picture):
    layout_element(n_picture->width(), n_picture->width(), n_picture->width(), n_picture->height(), n_picture->height(), n_picture->height(), n_picture->width(), n_picture->height()), m_picture(n_picture)
  {
  }
  virtual ~picture_element()
  {
    delete m_picture;
  }
  virtual void draw(graphics & g)
  {
    assert(g.bounds_width() == width());
    assert(g.bounds_height() == height());

    g.draw_bitmap(0, 0, *m_picture);
  }

private:
  bitmap * m_picture;
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
    layout_element_p_list::iterator i;

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
    layout_element_p_list::iterator i;
    int num_elem = 0;
    int int_pos;
    double pos = 0.0;
    double new_pos = 0.0;
    double width_per_elem = 0.0;

    assert(g.bounds_width() == width());
    assert(g.bounds_height() == height());

    if(ideal_width() == width())
    {
      int_pos = 0;
      for(i = m_elements.begin(); i != m_elements.end(); ++i)
      {
        (*i)->width((*i)->ideal_width());
        (*i)->height(height());
        g.set_bounds(x1 + int_pos, y1, x1 + int_pos + (*i)->width(), y1 + height());
        (*i)->draw(g);
        int_pos += (*i)->ideal_width();
      }
    }
    else if(ideal_width() > width())
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
  layout_element_p_list m_elements;
};


class vbox_element: public layout_element
{
public:
  vbox_element(): layout_element(0, 0, -1, 0, 0, 0, 0, 0) {}
  virtual ~vbox_element()
  {
    layout_element_p_list::iterator i;

    for(i = m_elements.begin(); i != m_elements.end(); ++i)
    {
      delete *i;
    }
  }
  void add_element(layout_element * l)
  {
    m_elements.push_back(l);

    assert((max_width() < 0) || (l->min_width() <= max_width()));
    assert((l->max_width() < 0) || (l->max_width() >= min_width()));

    if(l->min_width() > min_width()) min_width(l->min_width());
    if(ideal_width() < min_width()) ideal_width(min_width());
    if(max_width() < 0)
    {
      if(!(l->max_width() < 0))
      {
        max_width(l->max_width());
        if(ideal_width() > max_width()) ideal_width(max_width());
      }
    }
    else
    {
      if(!(l->max_width() < 0))
      {
        if(l->max_width() < max_width()) max_width(l->max_width());
        if(ideal_width() > max_width()) ideal_width(max_width());
      }
    }
    if(l->ideal_width() > ideal_width())
    {
      if((max_width() < 0) || (l->ideal_width() < max_width()))
      {
        ideal_width(l->ideal_width());
      }
      else
      {
        ideal_width(max_width());
      }
    }

    min_height(min_height() + l->min_height());
    ideal_height(ideal_height() + l->ideal_height());
    if((max_height() < 0) || (l->max_height() < 0)) max_height(-1);
    else max_height(max_height() + l->max_height());
  }
  virtual void draw(graphics & g)
  {
    int x1 = g.bounds_x1();
    int y1 = g.bounds_y1();
    layout_element_p_list::iterator i;
    int num_elem = 0;
    int int_pos;
    double pos = 0.0;
    double new_pos = 0.0;
    double height_per_elem = 0.0;

    assert(g.bounds_height() == height());
    assert(g.bounds_width() == width());

    if(ideal_height() == height())
    {
      int_pos = 0;
      for(i = m_elements.begin(); i != m_elements.end(); ++i)
      {
        (*i)->width(width());
        (*i)->height((*i)->ideal_height());
        g.set_bounds(x1, y1 + int_pos, x1 + width(), y1 + int_pos + (*i)->height());
        (*i)->draw(g);
        int_pos += (*i)->ideal_height();
      }
    }
    else if(ideal_height() > height())
    { //shrink
      num_elem = 0;
      height_per_elem = (double)(height() - ideal_height()) / (double)(min_height() - ideal_height());
      for(i = m_elements.begin(); i != m_elements.end(); ++i)
      {
        new_pos = pos + (double)((*i)->min_height() - (*i)->ideal_height()) * height_per_elem + (double)(*i)->ideal_height();
        (*i)->width(width());
        (*i)->height(floor(new_pos + 0.5) - floor(pos + 0.5));
        g.set_bounds(x1, y1 + floor(pos + 0.5), x1 + width(), y1 + floor(pos + 0.5) + (*i)->height());
        (*i)->draw(g);
        pos = new_pos;
      }
    }
    else
    { //expand
      if(max_height() < 0)
      {
        num_elem = 0;
        for(i = m_elements.begin(); i != m_elements.end(); ++i)
        {
          if((*i)->max_height() < 0) ++num_elem;
        }
        height_per_elem = ((double)height() - ideal_height()) / (double)num_elem;
        for(i = m_elements.begin(); i != m_elements.end(); ++i)
        {
          new_pos = (double)(*i)->ideal_height() + pos;
          if((*i)->max_height() < 0) new_pos += height_per_elem;
          (*i)->width(width());
          (*i)->height(floor(new_pos + 0.5) - floor(pos + 0.5));
          g.set_bounds(x1, y1 + floor(pos + 0.5), x1 + width(), y1 + floor(pos + 0.5) + (*i)->height());
          (*i)->draw(g);
          pos = new_pos;
        }
      }
      else
      {
        num_elem = 0;
        height_per_elem = (double)(height() - ideal_height()) / (double)(max_height() - ideal_height());
        for(i = m_elements.begin(); i != m_elements.end(); ++i)
        {
          new_pos = pos + (double)((*i)->max_height() - (*i)->ideal_height()) * height_per_elem + (double)(*i)->ideal_height();
          (*i)->height(floor(new_pos + 0.5) - floor(pos + 0.5));
          (*i)->width(width());
          g.set_bounds(x1, y1 + floor(pos + 0.5), x1 + width(), y1 + floor(pos + 0.5) + (*i)->height());
          (*i)->draw(g);
          pos = new_pos;
        }
      }
    }
    g.set_bounds(x1, y1, x1 + height(), y1 + height());
  }

private:
  layout_element_p_list m_elements;
};


