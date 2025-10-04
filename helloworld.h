#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h> // <-- NEW: Include Gtk::Box for centering

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
  Gtk::Button m_button;
  Gtk::Button m_button1;
  Gtk::Button m_button2;
  Gtk::Button m_button3;
  Gtk::Button m_button4;
  // NEW: A Gtk::Box to hold and center the buttons
  Gtk::Box m_box; 
};

#endif //GTKMM_EXAMPLE_HELLOWORLD_H
