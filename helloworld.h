#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <vector>

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
  std::vector<Gtk::Button*> m_fd_buttons;
};

#endif //GTKMM_EXAMPLE_HELLOWORLD_H
