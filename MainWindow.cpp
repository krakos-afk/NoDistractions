#include "MainWindow.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <glibmm/miscutils.h>
#include <cstdint>

#ifndef NO_DISTRACTIONS_DATADIR
#define NO_DISTRACTIONS_DATADIR .
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

namespace {
const int kCharacterPanelWidth = 240;
const char* kInstallDataDir = STR(NO_DISTRACTIONS_DATADIR);
}

MainWindow::MainWindow()
    : m_isRunning(false),
    m_accumulatedTime(std::chrono::seconds{0}) {
    set_title("nodistactions");
    set_default_size(760, 420);

    setup_css();
    setup_ui();
    load_characters();
    load_sessions_from_file();
    refresh_sessions_list();
    update_running_state(false);

    show_all_children();
}

MainWindow::~MainWindow() {}

void MainWindow::setup_css() {
    auto cssProvider = Gtk::CssProvider::create();
    auto cssPath = find_asset_path("style.css");
    if (cssPath.empty()) {
        std::cerr << "Could not find style.css; continuing with default theme." << std::endl;
        return;
    }
    try {
        cssProvider->load_from_path(cssPath);
        Gtk::StyleContext::add_provider_for_screen(
            Gdk::Screen::get_default(),
            cssProvider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Gtk::CssProviderError& ex) {
        std::cerr << "CssProviderError: " << ex.what() << std::endl;
    } catch (const Glib::Error& ex) {
        std::cerr << "Css load error: " << ex.what() << std::endl;
    }
}

void MainWindow::setup_ui() {
    // Main container (Horizontal)
    m_mainHBox.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    m_mainHBox.set_spacing(0);
    m_mainHBox.set_valign(Gtk::ALIGN_FILL);
    add(m_mainHBox);

    // Character panel
    m_imageFrame.set_shadow_type(Gtk::SHADOW_NONE);
    m_imageFrame.get_style_context()->add_class("character-frame");
    m_imageFrame.set_size_request(kCharacterPanelWidth, -1);
    m_imageFrame.set_hexpand(false);
    m_imageFrame.set_vexpand(true);

    m_characterColumn.set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_characterColumn.set_spacing(12);
    m_characterColumn.set_homogeneous(true); // equal spacing vertically
    m_characterColumn.set_hexpand(false);
    m_characterColumn.set_vexpand(true);
    m_characterColumn.set_halign(Gtk::ALIGN_CENTER);
    m_characterColumn.set_valign(Gtk::ALIGN_FILL);
    m_imageFrame.add(m_characterColumn);
    m_mainHBox.pack_start(m_imageFrame, Gtk::PACK_SHRINK);

    // Controls panel
    m_controlsVBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_controlsVBox.set_spacing(18);
    m_controlsVBox.set_margin_top(10);
    m_controlsVBox.set_margin_bottom(10);
    m_controlsVBox.set_margin_start(10);
    m_controlsVBox.set_margin_end(10);
    m_controlsVBox.set_hexpand(true);
    m_controlsVBox.set_vexpand(true);
    m_controlsVBox.set_halign(Gtk::ALIGN_CENTER);
    m_controlsVBox.set_valign(Gtk::ALIGN_CENTER);
    m_mainHBox.pack_start(m_controlsVBox, Gtk::PACK_EXPAND_WIDGET);

    // Header: Timer row + status
    m_headerBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_headerBox.set_spacing(6);
    m_headerBox.set_halign(Gtk::ALIGN_CENTER);
    m_headerBox.set_valign(Gtk::ALIGN_CENTER);

    m_timerRow.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    m_timerRow.set_spacing(8);
    m_timerRow.set_halign(Gtk::ALIGN_CENTER);
    m_timerRow.set_valign(Gtk::ALIGN_CENTER);
    m_timerLabel.set_text("00:00:00");
    m_timerLabel.set_name("timer-label");
    m_timerLabel.set_halign(Gtk::ALIGN_CENTER);
    m_timerLabel.set_valign(Gtk::ALIGN_CENTER);
    m_timerRow.pack_start(m_timerLabel, Gtk::PACK_EXPAND_WIDGET);

    m_spinner.set_size_request(24, 24);
    m_spinner.set_halign(Gtk::ALIGN_CENTER);
    m_spinner.set_valign(Gtk::ALIGN_CENTER);
    m_timerRow.pack_start(m_spinner, Gtk::PACK_SHRINK);

    m_headerBox.pack_start(m_timerRow, Gtk::PACK_SHRINK);

    m_statusLabel.set_text("Ready");
    m_statusLabel.set_name("status-label");
    m_statusLabel.set_halign(Gtk::ALIGN_CENTER);
    m_headerBox.pack_start(m_statusLabel, Gtk::PACK_SHRINK);

    m_controlsVBox.pack_start(m_headerBox, Gtk::PACK_SHRINK);

    // Inputs
    m_nameEntry.set_placeholder_text("Session Name");
    m_controlsVBox.pack_start(m_nameEntry, Gtk::PACK_SHRINK);

    m_descEntry.set_placeholder_text("Description");
    m_controlsVBox.pack_start(m_descEntry, Gtk::PACK_SHRINK);

    // Buttons
    m_buttonBox.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    m_buttonBox.set_spacing(10);
    m_buttonBox.set_halign(Gtk::ALIGN_CENTER);
    m_controlsVBox.pack_start(m_buttonBox, Gtk::PACK_SHRINK);

    m_startButton.set_label("Start");
    m_stopButton.set_label("Pause");
    m_saveButton.set_label("Save");
    m_resetButton.set_label("Reset");

    m_buttonBox.pack_start(m_startButton, Gtk::PACK_SHRINK);
    m_buttonBox.pack_start(m_stopButton, Gtk::PACK_SHRINK);
    m_buttonBox.pack_start(m_saveButton, Gtk::PACK_SHRINK);
    m_buttonBox.pack_start(m_resetButton, Gtk::PACK_SHRINK);

    // Connect Signals
    m_startButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_start_clicked));
    m_stopButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_stop_clicked));
    m_saveButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_save_clicked));
    m_resetButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_reset_clicked));

    // Sessions panel
    setup_sessions_panel();
    // Place sessions panel on the right, similar styling/width to character panel
    m_sessionsFrame.set_size_request(kCharacterPanelWidth, -1);
    m_sessionsFrame.set_margin_top(8);
    m_sessionsFrame.set_margin_bottom(8);
    m_sessionsFrame.set_margin_start(0);
    m_sessionsFrame.set_margin_end(0);
    m_sessionsFrame.set_halign(Gtk::ALIGN_END);
    m_sessionsFrame.set_hexpand(false);
    m_sessionsFrame.set_vexpand(true);
    m_mainHBox.pack_end(m_sessionsFrame, Gtk::PACK_SHRINK);
}

Glib::RefPtr<Gdk::Pixbuf> MainWindow::load_scaled_pixbuf(const std::string& path, int target_width) {
    try {
        auto pixbuf = Gdk::Pixbuf::create_from_file(path);
        int width = pixbuf->get_width();
        int height = pixbuf->get_height();

        if (width > target_width) {
            float ratio = static_cast<float>(target_width) / static_cast<float>(width);
            width = target_width;
            height = static_cast<int>(height * ratio);
        }

        // cap height to keep panel neat
        const int maxHeight = 380;
        if (height > maxHeight) {
            float ratio = static_cast<float>(maxHeight) / static_cast<float>(height);
            height = maxHeight;
            width = static_cast<int>(width * ratio);
        }

        if (width != pixbuf->get_width() || height != pixbuf->get_height()) {
            pixbuf = pixbuf->scale_simple(width, height, Gdk::INTERP_BILINEAR);
        }
        return pixbuf;
    } catch (...) {
        return Glib::RefPtr<Gdk::Pixbuf>();
    }
}

void MainWindow::load_characters() {
    m_characterImages.clear();

    auto children = m_characterColumn.get_children();
    for (auto c : children) m_characterColumn.remove(*c);

    std::vector<std::string> candidates = {
        "character0.jpg", "character1.jpg", "character2.jpg",
        "character0.png", "character1.png", "character2.png",
        "character.jpg",  "character.png"
    };

    for (const auto& file : candidates) {
        auto path = find_asset_path(file);
        if (path.empty()) continue;
        auto pix = load_scaled_pixbuf(path, kCharacterPanelWidth);
        if (pix) {
            auto img = Gtk::manage(new Gtk::Image(pix));
            img->set_halign(Gtk::ALIGN_CENTER);
            img->set_valign(Gtk::ALIGN_CENTER);
            img->set_hexpand(false);
            img->set_vexpand(true);
            m_characterColumn.pack_start(*img, Gtk::PACK_EXPAND_WIDGET);
            m_characterImages.push_back(img);
        }
    }

    if (m_characterImages.empty()) {
        auto fallback = Gtk::manage(new Gtk::Image());
        fallback->set_from_icon_name("face-smile", Gtk::ICON_SIZE_DIALOG);
        fallback->set_halign(Gtk::ALIGN_CENTER);
        fallback->set_valign(Gtk::ALIGN_CENTER);
        fallback->set_hexpand(false);
        fallback->set_vexpand(true);
        m_characterColumn.pack_start(*fallback, Gtk::PACK_EXPAND_WIDGET);
        m_characterImages.push_back(fallback);
    }

    m_characterColumn.show_all_children();
}

void MainWindow::setup_sessions_panel() {
    m_sessionsFrame.set_label("");
    m_sessionsFrame.set_shadow_type(Gtk::SHADOW_NONE);
    m_sessionsFrame.get_style_context()->add_class("sessions-frame");

    m_sessionsBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_sessionsBox.set_spacing(8);
    m_sessionsBox.set_margin_top(8);
    m_sessionsBox.set_margin_bottom(8);
    m_sessionsBox.set_margin_start(8);
    m_sessionsBox.set_margin_end(8);
    m_sessionsFrame.add(m_sessionsBox);

    m_sessionsScroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_sessionsScroll.set_hexpand(true);
    m_sessionsScroll.set_vexpand(true);
    m_sessionsScroll.add(m_sessionsList);
    m_sessionsList.set_name("sessions-list");
    m_sessionsList.set_selection_mode(Gtk::SELECTION_SINGLE);
    m_sessionsList.signal_row_selected().connect(sigc::mem_fun(*this, &MainWindow::on_session_row_selected));
    m_sessionsBox.pack_start(m_sessionsScroll, Gtk::PACK_EXPAND_WIDGET);

    // Edit panel
    m_editBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_editBox.set_spacing(6);

    m_editName.set_placeholder_text("Edit name");
    m_editDesc.set_placeholder_text("Edit description");
    m_editDuration.set_text("Duration: -");

    m_editBox.pack_start(m_editName, Gtk::PACK_SHRINK);
    m_editBox.pack_start(m_editDesc, Gtk::PACK_SHRINK);
    m_editBox.pack_start(m_editDuration, Gtk::PACK_SHRINK);

    m_updateButton.set_label("Update Session");
    m_updateButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_update_session_clicked));
    m_editBox.pack_start(m_updateButton, Gtk::PACK_SHRINK);

    m_deleteButton.set_label("Delete Session");
    m_editBox.pack_start(m_deleteButton, Gtk::PACK_SHRINK);
    m_deleteButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_delete_session_clicked));

    m_sessionsBox.pack_start(m_editBox, Gtk::PACK_SHRINK);

    // Hide edit controls until a session is selected
    m_editBox.hide();
    m_updateButton.set_sensitive(false);
    m_deleteButton.set_sensitive(false);

    // Removed packing into controls VBox; now placed in mainHBox (right side)
}

void MainWindow::refresh_sessions_list() {
    // Clear existing rows
    auto children = m_sessionsList.get_children();
    for (auto child : children) {
        m_sessionsList.remove(*child);
    }

    for (size_t i = 0; i < m_sessions.size(); ++i) {
        const auto& s = m_sessions[i];
        auto row = Gtk::manage(new Gtk::ListBoxRow());
           auto box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
           box->set_spacing(2);

           auto title = Gtk::manage(new Gtk::Label());
           title->set_markup("<b>" + Glib::Markup::escape_text(s.name) + "</b>");
           title->set_xalign(0.0);
           box->pack_start(*title, Gtk::PACK_SHRINK);

           std::ostringstream meta;
           meta << (s.dateString.empty() ? "" : s.dateString + " • ")
               << std::fixed << std::setprecision(1) << s.getDurationInMinutes() << " min";
           auto subtitle = Gtk::manage(new Gtk::Label(meta.str()));
           subtitle->set_xalign(0.0);
           subtitle->get_style_context()->add_class("subtitle");
           box->pack_start(*subtitle, Gtk::PACK_SHRINK);

           auto desc = Gtk::manage(new Gtk::Label());
           desc->set_text(s.description);
           desc->set_xalign(0.0);
           desc->set_line_wrap(true);
           desc->set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
           desc->set_max_width_chars(40);
           box->pack_start(*desc, Gtk::PACK_SHRINK);

        row->add(*box);
        row->set_data("session-index", reinterpret_cast<void*>(static_cast<intptr_t>(i)));
        m_sessionsList.add(*row);
    }

    m_sessionsList.show_all_children();
}

void MainWindow::load_sessions_from_file() {
    m_sessions.clear();
    std::ifstream infile("work_log.txt");
    if (!infile.is_open()) return;

    WorkSession ws;
    std::string line;
    auto flush_session = [&]() {
        if (!ws.name.empty()) {
            m_sessions.push_back(ws);
        }
        ws = WorkSession{};
    };

    while (std::getline(infile, line)) {
        if (line.rfind("Date:", 0) == 0) {
            ws.dateString = line.substr(5);
            if (!ws.dateString.empty() && ws.dateString[0] == ' ') ws.dateString.erase(0,1);
        } else if (line.rfind("Session:", 0) == 0) {
            ws.name = line.substr(8);
            if (!ws.name.empty() && ws.name[0] == ' ') ws.name.erase(0,1);
        } else if (line.rfind("Description:", 0) == 0) {
            ws.description = line.substr(12);
            if (!ws.description.empty() && ws.description[0] == ' ') ws.description.erase(0,1);
        } else if (line.rfind("Duration:", 0) == 0) {
            std::string rest = line.substr(9);
            if (!rest.empty() && rest[0] == ' ') rest.erase(0,1);
            auto pos = rest.find(" ");
            if (pos != std::string::npos) rest = rest.substr(0, pos);
            try {
                ws.durationMinutes = std::stod(rest);
            } catch (...) {
                ws.durationMinutes = 0.0;
            }
        } else if (line.find("---") != std::string::npos) {
            flush_session();
        }
    }
    flush_session();
}

void MainWindow::write_sessions_to_file() {
    std::ofstream outfile("work_log.txt", std::ios_base::trunc);
    if (!outfile.is_open()) return;

    for (const auto& s : m_sessions) {
        std::string dateStr = s.dateString;
        if (dateStr.empty()) {
            auto now = std::chrono::system_clock::now();
            auto t = std::chrono::system_clock::to_time_t(now);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
            dateStr = oss.str();
        }
        outfile << "Date: " << dateStr << "\n";
        outfile << "Session: " << s.name << "\n";
        outfile << "Description: " << s.description << "\n";
        outfile << "Duration: " << s.getDurationInMinutes() << " minutes\n";
        outfile << "----------------------------------------\n";
    }
}

std::string MainWindow::find_asset_path(const std::string& filename) const {
    std::vector<std::string> search_dirs;
    search_dirs.push_back(Glib::get_current_dir());
    if (kInstallDataDir && std::string(kInstallDataDir).size()) {
        search_dirs.push_back(kInstallDataDir);
    }

    for (const auto& dir : search_dirs) {
        auto candidate = Glib::build_filename(dir, filename);
        if (Glib::file_test(candidate, Glib::FILE_TEST_EXISTS)) {
            return candidate;
        }
    }
    return std::string();
}

void MainWindow::on_session_row_selected(Gtk::ListBoxRow* row) {
    if (!row) {
        m_editBox.hide();
        m_updateButton.set_sensitive(false);
        m_deleteButton.set_sensitive(false);
        return;
    }
    auto idx = reinterpret_cast<intptr_t>(row->get_data("session-index"));
    if (idx < 0 || static_cast<size_t>(idx) >= m_sessions.size()) {
        m_editBox.hide();
        m_updateButton.set_sensitive(false);
        m_deleteButton.set_sensitive(false);
        return;
    }
    const auto& s = m_sessions[static_cast<size_t>(idx)];
    m_editName.set_text(s.name);
    m_editDesc.set_text(s.description);
    std::ostringstream oss;
    oss << "Duration: " << std::fixed << std::setprecision(1) << s.getDurationInMinutes() << " minutes";
    m_editDuration.set_text(oss.str());
    m_editBox.show_all();
    m_updateButton.set_sensitive(true);
    m_deleteButton.set_sensitive(true);
}

void MainWindow::on_update_session_clicked() {
    auto row = m_sessionsList.get_selected_row();
    if (!row) return;
    auto idx = reinterpret_cast<intptr_t>(row->get_data("session-index"));
    if (idx < 0 || static_cast<size_t>(idx) >= m_sessions.size()) return;

    auto& s = m_sessions[static_cast<size_t>(idx)];
    s.name = m_editName.get_text();
    s.description = m_editDesc.get_text();

    write_sessions_to_file();
    refresh_sessions_list();
    // Reselect updated row
    auto newRow = m_sessionsList.get_row_at_index(static_cast<int>(idx));
    if (newRow) m_sessionsList.select_row(*newRow);
}

void MainWindow::on_delete_session_clicked() {
    auto row = m_sessionsList.get_selected_row();
    if (!row) return;
    auto idx = reinterpret_cast<intptr_t>(row->get_data("session-index"));
    if (idx < 0 || static_cast<size_t>(idx) >= m_sessions.size()) return;

    m_sessions.erase(m_sessions.begin() + static_cast<size_t>(idx));
    write_sessions_to_file();
    refresh_sessions_list();
    m_editName.set_text("");
    m_editDesc.set_text("");
    m_editDuration.set_text("Duration: -");
    m_editBox.hide();
    m_updateButton.set_sensitive(false);
    m_deleteButton.set_sensitive(false);
}

void MainWindow::on_reset_clicked() {
    if (m_isRunning) {
        on_stop_clicked();
    }
    m_accumulatedTime = std::chrono::seconds{0};
    m_timerLabel.set_text("00:00:00");
    update_running_state(false);
    m_statusLabel.set_text("Ready");
    m_spinner.stop();
}

void MainWindow::update_running_state(bool running) {
    auto timerCtx = m_timerLabel.get_style_context();
    auto statusCtx = m_statusLabel.get_style_context();

    if (running) {
        timerCtx->add_class("running");
        statusCtx->remove_class("paused");
        statusCtx->remove_class("saved");
        statusCtx->add_class("running");
        m_statusLabel.set_text("Focus mode engaged!");
        m_spinner.start();
    } else {
        timerCtx->remove_class("running");
        statusCtx->remove_class("running");
        m_spinner.stop();

        if (m_accumulatedTime.count() > 0) {
            statusCtx->add_class("paused");
            statusCtx->remove_class("saved");
            m_statusLabel.set_text("Paused");
        } else {
            statusCtx->remove_class("paused");
            statusCtx->remove_class("saved");
            m_statusLabel.set_text("Ready");
        }
    }

    bool hasTime = m_accumulatedTime.count() > 0;
    m_startButton.set_sensitive(!running);
    m_stopButton.set_sensitive(running);
    m_saveButton.set_sensitive(running || hasTime);
    m_resetButton.set_sensitive(running || hasTime);
}

void MainWindow::on_start_clicked() {
    if (!m_isRunning) {
        m_startTime = std::chrono::steady_clock::now();
        m_timeoutConnection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_timeout), 1000);
        m_isRunning = true;

        update_running_state(true);
    }
}

void MainWindow::on_stop_clicked() {
    if (m_isRunning) {
        m_timeoutConnection.disconnect();
        auto now = std::chrono::steady_clock::now();
        m_accumulatedTime += std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime);
        m_isRunning = false;

        update_running_state(false);
    }
}

void MainWindow::on_save_clicked() {
    // Stop if running
    if (m_isRunning) {
        on_stop_clicked();
    }

    // Get data
    std::string name = m_nameEntry.get_text();
    std::string desc = m_descEntry.get_text();
    
    // Calculate total minutes
    double minutes = static_cast<double>(m_accumulatedTime.count()) / 60.0;

    WorkSession ws;
    ws.name = name;
    ws.description = desc;
    ws.durationMinutes = minutes;
    auto now = std::chrono::system_clock::now();
    ws.startTime = now;
    ws.endTime = now + std::chrono::duration_cast<std::chrono::milliseconds>(m_accumulatedTime);
    auto t = std::chrono::system_clock::to_time_t(now);
    {
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
        ws.dateString = oss.str();
    }

    m_sessions.push_back(ws);
    write_sessions_to_file();
    refresh_sessions_list();

    // Reset
    m_accumulatedTime = std::chrono::seconds{0};
    m_timerLabel.set_text("00:00:00");
    m_nameEntry.set_text("");
    m_descEntry.set_text("");
    update_running_state(false);

    // Show saved state
    auto statusCtx = m_statusLabel.get_style_context();
    statusCtx->remove_class("paused");
    statusCtx->remove_class("running");
    statusCtx->add_class("saved");
    m_statusLabel.set_text("Session saved!");
    m_spinner.stop();
}

bool MainWindow::on_timeout() {
    auto now = std::chrono::steady_clock::now();
    auto currentDuration = m_accumulatedTime + std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime);
    
    int totalSeconds = static_cast<int>(currentDuration.count());
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
    m_timerLabel.set_text(buffer);

    return true; // Keep calling
}
