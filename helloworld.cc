#include "helloworld.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <gtkmm/application.h> 
#include <gtkmm/cssprovider.h> 
#include <gtkmm/stylecontext.h> 
#include <gdkmm/screen.h> // <-- NEW: Include for Gdk::Screen
#include <fstream>
#include <regex>
#include <optional>
#include <unistd.h>
#include <cctype>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <gtkmm/label.h>

HelloWorld::HelloWorld()
// Initialize the scrolled window and box
: m_scrolled_window(), m_box(Gtk::Orientation::ORIENTATION_VERTICAL)
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
  // Listen for key presses and pointer motion so we can exit on any user action
  add_events(Gdk::KEY_PRESS_MASK | Gdk::POINTER_MOTION_MASK);
  // ******************************************************

  // Read FD-List.html from executable directory
  auto get_exec_dir = []() -> std::string {
    // On Linux, read /proc/self/exe
    char pathbuf[4096];
    ssize_t len = ::readlink("/proc/self/exe", pathbuf, sizeof(pathbuf)-1);
    if (len == -1) return std::string();
    pathbuf[len] = '\0';
    std::string full(pathbuf);
    auto pos = full.rfind('/');
    if (pos == std::string::npos) return std::string();
    return full.substr(0, pos+1);
  };

  std::string exec_dir = get_exec_dir();
  std::string fdpath = exec_dir + "FD-List.html";

  auto trim = [](std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    return s;
  };

  auto strip_tags = [](const std::string &in){
    std::string out = std::regex_replace(in, std::regex("<[^>]*>"), "");
    // collapse whitespace
    out = std::regex_replace(out, std::regex("[\t\n\r]+"), " ");
    return out;
  };

  std::ifstream ifs(fdpath);
  if (!ifs.is_open()) {
    // Show an error label inside the grid
  auto *lbl = Gtk::manage(new Gtk::Label("Error reading FD file"));
  lbl->set_halign(Gtk::Align::ALIGN_CENTER);
  lbl->set_margin_start(4);
  lbl->set_margin_end(4);
  lbl->set_margin_top(4);
  lbl->set_margin_bottom(4);
    m_grid.attach(*lbl, 0, 0, 1, 1);

    m_scrolled_window.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_AUTOMATIC);
    m_scrolled_window.set_min_content_height(100);
    m_scrolled_window.add(m_grid);

    add(m_scrolled_window);

    show_all();
    return;
  }

  std::string html((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

  // Find table rows
  // Use std::regex::icase for case-insensitive matching; avoid unsupported inline flags like (?i)
  std::regex tr_re("<tr[^>]*>([\\s\\S]*?)</tr>", std::regex::icase);
  std::regex cell_re("<t[dh][^>]*>([\\s\\S]*?)</t[dh]>", std::regex::icase);
  std::sregex_iterator tr_it(html.begin(), html.end(), tr_re);
  std::sregex_iterator tr_end;

  int maturity_col = -1;
  std::vector<std::vector<std::string>> rows;
  std::unordered_map<std::string,int> header_index;

  bool header_found = false;
  for (; tr_it != tr_end; ++tr_it) {
    std::string tr = (*tr_it)[1].str();
    std::sregex_iterator cell_it(tr.begin(), tr.end(), cell_re);
    std::vector<std::string> cells;
    for (; cell_it != std::sregex_iterator(); ++cell_it) {
      std::string c = (*cell_it)[1].str();
      c = strip_tags(c);
      c = trim(c);
      cells.push_back(c);
    }
    if (!header_found) {
      // Treat first non-empty row as header; build header_index map
      for (size_t i = 0; i < cells.size(); ++i) {
        std::string low = cells[i];
        std::transform(low.begin(), low.end(), low.begin(), [](unsigned char ch){ return std::tolower(ch); });
        low = trim(low);
        header_index[low] = (int)i;
        if (low == "maturity date") maturity_col = (int)i;
      }
      header_found = true;
      continue;
    }
    // Only consider rows with td cells (non-empty)
    if (!cells.empty()) rows.push_back(cells);
  }

  auto parse_ddmmyy = [](const std::string &s) -> std::optional<std::time_t> {
    std::smatch m;
    // support DD/MM/YY or DD/MM/YYYY
    std::regex d_re("(\\d{1,2})/(\\d{1,2})/(\\d{2,4})");
    if (std::regex_search(s, m, d_re)) {
      int d = std::stoi(m[1].str());
      int mo = std::stoi(m[2].str());
      int yy = std::stoi(m[3].str());
      int year = (yy < 100) ? (2000 + yy) : yy;
      std::tm tm{};
      tm.tm_mday = d;
      tm.tm_mon = mo - 1;
      tm.tm_year = year - 1900;
      tm.tm_hour = 0; tm.tm_min = 0; tm.tm_sec = 0;
      std::time_t tt = std::mktime(&tm);
      if (tt == -1) return std::nullopt;
      return tt;
    }
    return std::nullopt;
  };

  auto add_months = [](std::time_t t, int months) -> std::optional<std::time_t> {
    std::tm tm = *std::localtime(&t);
    int total = tm.tm_mon + months;
    tm.tm_year += total / 12;
    tm.tm_mon = total % 12;
    return std::mktime(&tm);
  };

  auto parse_double = [](const std::string &s) -> std::optional<double> {
    std::string tmp;
    for (char c : s) if ((c >= '0' && c <= '9') || c == '.' || c == '-') tmp.push_back(c);
    if (tmp.empty()) return std::nullopt;
    try { return std::stod(tmp); } catch (...) { return std::nullopt; }
  };

  std::time_t now_t = std::time(nullptr);
  int found = 0;
  std::vector<std::vector<std::string>> reminders; // each reminder is a vector of columns

  // iterate rows and find maturity dates - COLLECT ALL FDs (no limit)
  for (const auto &r : rows) {
    if (maturity_col < 0 || maturity_col >= (int)r.size()) continue;
    auto opt = parse_ddmmyy(r[maturity_col]);
    if (!opt) continue;
    std::time_t mt = *opt;
    double diff_days = std::difftime(mt, now_t) / (60*60*24);

    // Normal maturity-based reminder (if within next 30 days)
    if (diff_days >= 0.0 && diff_days <= 30.0) {
      // concatenate all fields with '-' but skip 'Sr' column; format maturity date as DD-MM-YY
      std::string concat;
      for (size_t i = 0; i < r.size(); ++i) {
        // detect if this column is a 'sr' column by checking header keys that contain 'sr'
        bool is_sr = false;
        for (auto &kv : header_index) {
          if (kv.second == (int)i) {
            std::string hk = kv.first;
            if (hk.find("sr") != std::string::npos || hk == "s.no" || hk == "sno" || hk == "srno") {
              is_sr = true;
            }
            break;
          }
        }
        if (is_sr) continue; // skip Sr column

        if (!concat.empty()) concat += " - ";
        if ((int)i == maturity_col) {
          // parse and reformat maturity date as DD-MM-YY
          auto mt_opt = parse_ddmmyy(r[i]);
          if (mt_opt) {
            std::tm mtm = *std::localtime(&(*mt_opt));
            char dbuf[16];
            std::snprintf(dbuf, sizeof(dbuf), "%02d/%02d/%02d", mtm.tm_mday, mtm.tm_mon+1, (mtm.tm_year+1900) % 100);
            concat += dbuf;
          } else {
            concat += r[i];
          }
        } else {
          concat += r[i];
        }
      }
      // build reminder fields by splitting concat on ' - ' (we built it earlier excluding Sr)
      std::vector<std::string> fields;
      std::istringstream iss(concat);
      std::string token;
      while (std::getline(iss, token, '-')) {
        // trim token
        auto t = trim(token);
        if (!t.empty()) fields.push_back(t);
      }
      // Determine bank name for this row to check GSEC; if not GSEC, insert third column 'FD-Mat'
      std::string bank_name_for_row;
      for (auto &kv : header_index) {
        if (kv.first.find("bank") != std::string::npos) {
          if (kv.second < (int)r.size()) bank_name_for_row = r[kv.second];
          break;
        }
      }
      std::string bank_low_for_row = bank_name_for_row;
      std::transform(bank_low_for_row.begin(), bank_low_for_row.end(), bank_low_for_row.begin(), [](unsigned char c){ return std::tolower(c); });
      if (bank_low_for_row.find("gsec") == std::string::npos) {
        // ensure at least two columns exist, then insert 'FD-Mat' at index 2 (third column)
        size_t insert_pos = 2;
        if (fields.size() < insert_pos) fields.resize(insert_pos);
        fields.insert(fields.begin() + insert_pos, std::string("FD-Mat"));
      }
      reminders.push_back(fields);
      ++found;
    }

    // GSEC-specific coupon payments: if bank name contains 'gsec'
    std::string bank_name;
    for (auto &kv : header_index) {
      if (kv.first.find("bank") != std::string::npos) {
        if (kv.second < (int)r.size()) bank_name = r[kv.second];
        break;
      }
    }
    std::string bank_low = bank_name;
    std::transform(bank_low.begin(), bank_low.end(), bank_low.begin(), [](unsigned char c){ return std::tolower(c); });
    if (bank_low.find("gsec") != std::string::npos) {
      // find indices for opening date, principal, rate, account
      int opening_idx = -1, principal_idx = -1, rate_idx = -1, acc_idx = -1;
      auto find_header = [&](const std::vector<std::string> &cands)->int{
        for (auto &p : cands) {
          auto it = header_index.find(p);
          if (it != header_index.end()) return it->second;
        }
        return -1;
      };
      opening_idx = find_header({"opening date","open date","opening","openingdate","open_date","opening_date"});
      principal_idx = find_header({"principal","principal amount","amount","amt","principal amt","deposit amount"});
      rate_idx = find_header({"interest","interest rate","rate"});
      acc_idx = find_header({"fd-account no","fd account no","account no","account","fd acc","account number"});

      if (opening_idx >= 0 && principal_idx >= 0 && rate_idx >= 0) {
        std::string opening_str = (opening_idx < (int)r.size()) ? r[opening_idx] : std::string();
        std::string principal_str = (principal_idx < (int)r.size()) ? r[principal_idx] : std::string();
        std::string rate_str = (rate_idx < (int)r.size()) ? r[rate_idx] : std::string();
        std::string acc_str = (acc_idx >=0 && acc_idx < (int)r.size()) ? r[acc_idx] : std::string();
        auto open_t_opt = parse_ddmmyy(opening_str);
        if (open_t_opt) {
          std::time_t open_t = *open_t_opt;
          for (int months = 6;; months += 6) {
            auto pay_opt = add_months(open_t, months);
            if (!pay_opt) break;
            std::time_t pay_t = *pay_opt;
            if (pay_t > mt) break;
            double diff_pay_days = std::difftime(pay_t, now_t) / (60*60*24);
            if (diff_pay_days >= 0.0 && diff_pay_days <= 30.0) {
              auto prin_opt = parse_double(principal_str);
              auto rate_opt = parse_double(rate_str);
              double interest_amt = 0.0;
              if (prin_opt && rate_opt) interest_amt = (*prin_opt) * ((*rate_opt)/100.0) * 0.5;
              std::tm ptm = *std::localtime(&pay_t);
              char buf[64];
              // format payment date as DD/MM/YY
              std::snprintf(buf, sizeof(buf), "%02d/%02d/%02d", ptm.tm_mday, ptm.tm_mon+1, (ptm.tm_year+1900) % 100);
              std::ostringstream entry;
              entry << buf << " - ";
              entry << std::fixed << std::setprecision(2) << interest_amt << " - ";
              entry << "coupan - ";
              entry << principal_str << " - ";
              entry << opening_str << " - ";
              entry << acc_str << " - ";
              entry << rate_str << " - ";
              entry << bank_name;
              // append Name column if present (use header_index to find it)
              int name_idx = -1;
              for (auto &kv2 : header_index) {
                if (kv2.first.find("name") != std::string::npos && kv2.first.find("bank") == std::string::npos) {
                  name_idx = kv2.second; break;
                }
              }
              if (name_idx >= 0 && name_idx < (int)r.size()) {
                entry << " - " << r[name_idx];
              }
              // remove any Sr fields - we don't include them here
              // coupon reminder fields: date, interest amount, 'coupan', principal, opening, acc no, rate, bank, optional name
              std::vector<std::string> fields;
              fields.push_back(std::string(buf));
              std::ostringstream amt; amt << std::fixed << std::setprecision(2) << interest_amt;
              fields.push_back(amt.str());
              fields.push_back(std::string("coupan"));
              fields.push_back(principal_str);
              fields.push_back(opening_str);
              fields.push_back(acc_str);
              fields.push_back(rate_str);
              fields.push_back(bank_name);
              if (name_idx >= 0 && name_idx < (int)r.size()) fields.push_back(r[name_idx]);
              reminders.push_back(fields);
              ++found;
            }
          }
        }
      }
    }
  }

  // If no matching records found at all, show a message
  if (found == 0 && reminders.empty()) {
    reminders.push_back(std::vector<std::string>{"No upcoming FDs in next 30 days"});
  }

  // Sort reminders by the first column interpreted as a date (DD/MM/YY) when possible
  std::sort(reminders.begin(), reminders.end(), [&](const std::vector<std::string>& a, const std::vector<std::string>& b){
    if (a.empty() || b.empty()) return a.size() < b.size();
    // try to parse first field as date
    auto da = parse_ddmmyy(a[0]);
    auto db = parse_ddmmyy(b[0]);
    if (da && db) return *da < *db;
    if (da) return true; // date sorts before non-date
    if (db) return false;
    return a[0] < b[0];
  });

  // Populate grid with reminders: each reminder row -> grid row, columns as cells
  int row = 0;
  for (const auto &rfields : reminders) {
    int col = 0;
    for (const auto &cell : rfields) {
  auto *lbl = Gtk::manage(new Gtk::Label(cell));
  lbl->set_halign(Gtk::Align::ALIGN_START);
  lbl->set_margin_start(4);
  lbl->set_margin_end(4);
  lbl->set_margin_top(4);
  lbl->set_margin_bottom(4);
      m_grid.attach(*lbl, col, row, 1, 1);
      ++col;
    }
    ++row;
  }

  // Configure the scrolled window with fixed height
  // Set policy to show vertical scrollbar only when needed
  m_scrolled_window.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_AUTOMATIC);

  // Fixed height - shows ~5 buttons but scrolls for more
  m_scrolled_window.set_min_content_height(250);
  m_scrolled_window.set_max_content_height(250);

  // Center the scrolled window vertically and horizontally
  m_scrolled_window.set_valign(Gtk::Align::ALIGN_CENTER);
  m_scrolled_window.set_halign(Gtk::Align::ALIGN_CENTER);

  // Add grid to scrolled window and main window
  m_scrolled_window.add(m_grid);
  add(m_scrolled_window);

  // Show window and children
  show_all();
}

HelloWorld::~HelloWorld()
{
  // No dynamic button cleanup required; grid/labels are managed by GTKmm
}

void HelloWorld::on_button_clicked()
{
  std::cout << "Hello World" << std::endl;
}

bool HelloWorld::on_key_press_event(GdkEventKey* key_event)
{
  // Close the window on any key press
  hide();
  return true; // event handled
}

bool HelloWorld::on_motion_notify_event(GdkEventMotion* motion_event)
{
  // Do NOT close the window on mouse movement - only exit on key press
  return false; // event not handled - allow normal mouse interaction
}
