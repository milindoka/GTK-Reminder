#include "helloworld.h"
#include <gtkmm/application.h>

int main(int argc, char* argv[])
{
  // create app with argc/argv (gtkmm3)
  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

  // instantiate your window and run the application
  HelloWorld window;

  return app->run(window);
}

