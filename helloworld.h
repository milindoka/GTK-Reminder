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

  //Member widgets:
  Gtk::Button m_button;
  // NEW: A Gtk::Box to hold and center the button
  Gtk::Box m_box; 
};

#endif //GTKMM_EXAMPLE_HELLOWORLD_H
