#include "helloworld.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

HelloWorld::HelloWorld()
: m_button() // default-construct; label set to current date/time below
{
  // Sets the border width of the window.
  set_border_width(10);

  // set button label to current date and time
  {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    m_button.set_label(oss.str());
  }

  // When the button receives the "clicked" signal, it will call the
  // on_button_clicked() method defined below.
  m_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_button_clicked));

  // This adds the button to the window.
  add(m_button);

  // The final step is to display this newly created widget.
  m_button.show();
}

HelloWorld::~HelloWorld()
{
}

void HelloWorld::on_button_clicked()
{
  std::cout << "Hello World" << std::endl;
}

