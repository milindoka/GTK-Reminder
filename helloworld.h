#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/grid.h>
#include <vector>
#include <sigc++/sigc++.h>
class HelloWorld : public Gtk::Window
{
public:
  HelloWorld();
  ~HelloWorld() override;

protected:
  //Signal handlers:
  void on_button_clicked();
  // Exit on any key press or mouse movement
  bool on_key_press_event(GdkEventKey* key_event) override;
  bool on_motion_notify_event(GdkEventMotion* motion_event) override;

  //Member widgets:
  Gtk::ScrolledWindow m_scrolled_window;
  Gtk::Box m_box;
  Gtk::Grid m_grid;
  // auto-exit control: timeout connection and user-activity flag
  sigc::connection m_autoexit_conn;
  bool m_user_active{false};
};

#endif //GTKMM_EXAMPLE_HELLOWORLD_H
