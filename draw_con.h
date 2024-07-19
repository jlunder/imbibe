#include "bitmap.h"
#include "pixel.h"
#include "rc.h"

class draw_context {
private:
  rc_ptr<bitmap> _b;
  int clip_x;
  int clip_y;
  int clip_w;
  int clip_h;
  int bounds_x;
  int bounds_y;
  int bounds_w;
  int bounds_h;
public:
  rc_ptr<bitmap> b() {return _b;}
  void b(rc_ptr<bitmap> const & n_b) {_b = n_b;}
  int clip_x() {return clip_x;}
  int clip_y() {return clip_y;}
  int clip_w() {return clip_w;}
  int clip_h() {return clip_h;}
  void clip_x(int n_clip_x) {clip_x = n_clip_x;}
  void clip_y(int n_clip_y) {clip_y = n_clip_y;}
  void clip_w(int n_clip_w) {clip_w = n_clip_w;}
  void clip_h(int n_clip_h) {clip_h = n_clip_h;}
  int bounds_x() {return bounds_x;}
  int bounds_y() {return bounds_y;}
  int bounds_w() {return bounds_w;}
  int bounds_h() {return bounds_h;}
  void bounds_x(int n_bounds_x) {bounds_x = n_bounds_x;}
  void bounds_y(int n_bounds_y) {bounds_y = n_bounds_y;}
  void bounds_w(int n_bounds_w) {bounds_w = n_bounds_w;}
  void bounds_h(int n_bounds_h) {bounds_h = n_bounds_h;}
  void draw_string(int x, int y, string const & s, attribute a);
  void draw_bitmap(int x, int y, bitmap & b);
  void draw_transparent_bitmap(int x, int y, bitmap & b);
  void draw_rect(int x, int y, int w, int h, pixel p);
  pixel at(int x, int y);
  pixel & at(int x, int y);
};

