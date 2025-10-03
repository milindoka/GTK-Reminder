#include "helloworld.h"
#include <gtkmm/application.h>

#include <glibmm/main.h>

int main(int argc, char* argv[])
{
  // create app with argc/argv (gtkmm3)
  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

  // instantiate your window and run the application
  HelloWorld window;

  // Auto-exit the application after 4 seconds (4000 ms)
  Glib::signal_timeout().connect([app]() {
    app->quit();
    return false; // don't repeat
  }, 4000);
  return app->run(window);
}

