#include "helloworld.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <gtkmm/application.h> 
#include <gtkmm/cssprovider.h> 
#include <gtkmm/stylecontext.h> 
#include <gdkmm/screen.h> // <-- NEW: Include for Gdk::Screen

HelloWorld::HelloWorld()
// Initialize the box as vertical
: m_button(), m_box(Gtk::Orientation::ORIENTATION_VERTICAL) 
{
  // Sets the border width of the window.
  set_border_width(10);
  
  // *** STEP 1: MAXIMIZE THE WINDOW ***
  maximize(); 

  // *** STEP 2: CONFIGURE TRANSPARENCY AND VISIBILITY (FIXED) ***
  
  // 1. Set the window's type to allow transparency
  set_app_paintable(true);
  
  // 2. Load the CSS file
  auto css_provider = Gtk::CssProvider::create();
  
  // Load the style.css file created above. Handle errors if the file is missing.
  try {
      css_provider->load_from_path("style.css");
  } catch (const Glib::Error& ex) {
      // Print error to standard error stream
      std::cerr << "Error loading CSS file: " << ex.what() << std::endl;
  }

  // 3. Apply the CSS to the screen (FIXED LINE)
  // Get the screen the widget is currently on
  Glib::RefPtr<Gdk::Screen> screen = get_screen(); 
  
  // Use the correct gtkmm method for applying CSS to the screen
  Gtk::StyleContext::add_provider_for_screen(
      screen, 
      css_provider, 
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
  );

  // OPTIONAL: Remove window decorations (title bar/border)
  set_decorated(false); 
  // ******************************************************

  // set button labels to current date and next 4 days
  std::time_t t = std::time(nullptr);
  for (int i = 0; i < 5; ++i) {
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    switch (i) {
      case 0: m_button.set_label(oss.str()); break;
      case 1: m_button1.set_label(oss.str()); break;
      case 2: m_button2.set_label(oss.str()); break;
      case 3: m_button3.set_label(oss.str()); break;
      case 4: m_button4.set_label(oss.str()); break;
    }
    t += 24 * 60 * 60; // next day
  }

  // Optionally connect signals if needed (currently only m_button has a handler)
  m_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_button_clicked));

  // Add all buttons to the box and center them
  m_box.pack_start(m_button, Gtk::PackOptions::PACK_SHRINK);
  m_box.pack_start(m_button1, Gtk::PackOptions::PACK_SHRINK);
  m_box.pack_start(m_button2, Gtk::PackOptions::PACK_SHRINK);
  m_box.pack_start(m_button3, Gtk::PackOptions::PACK_SHRINK);
  m_box.pack_start(m_button4, Gtk::PackOptions::PACK_SHRINK);
  m_button.set_halign(Gtk::Align::ALIGN_CENTER);
  m_button1.set_halign(Gtk::Align::ALIGN_CENTER);
  m_button2.set_halign(Gtk::Align::ALIGN_CENTER);
  m_button3.set_halign(Gtk::Align::ALIGN_CENTER);
  m_button4.set_halign(Gtk::Align::ALIGN_CENTER);
  m_box.set_valign(Gtk::Align::ALIGN_CENTER);
  add(m_box);

  show_all();
}

HelloWorld::~HelloWorld()
{
}

void HelloWorld::on_button_clicked()
{
  std::cout << "Hello World" << std::endl;
}
