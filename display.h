#include "rc_ptr.h"
#include "draw_context.h"

class display {
private:
  rc_ptr<draw_context> _dc;
public:
  display(rc_ptr<draw_context> const & n_dc): _dc(n_dc) {}
  virtual void paint() = 0;
  void dc(rc_ptr<draw_context> const & n_dc) {_dc = n_dc;}
  rc_ptr<draw_context> dc() {return _dc;}
protected:
  void update_rect(rect const & n_update_rect) {update_rect = n_update_rect;}
public:
  rect update_rect() const {return _update_rect;}
};
